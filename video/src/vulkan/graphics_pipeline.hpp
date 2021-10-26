/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_VIDEO_VULKAN_SUPPORT)
// includes + namespaces: in renderer.cpp

  // Create usable shader stage object -- reserved for internal use or advanced usage
  GraphicsPipeline::GraphicsPipeline(std::shared_ptr<Renderer> renderer, const VkGraphicsPipelineCreateInfo& pipelineInfo,
                                     VkPipelineCache parentCache)
    : _renderer(std::move(renderer)),
      _renderPass(pipelineInfo.renderPass),
      _pipelineLayout(pipelineInfo.layout) {
    auto result = vkCreateGraphicsPipelines(this->_renderer->context(), parentCache, 1, &pipelineInfo, nullptr, &(this->_pipeline));
    if (result != VK_SUCCESS || this->_pipeline == VK_NULL_HANDLE)
      throwError(result, "GraphicsPipeline: creation error");
  }

  void GraphicsPipeline::release() noexcept { ///< Destroy pipeline object
    if (this->_pipeline != VK_NULL_HANDLE) {
      try {
        if (this->_renderer->_boundPipeline == this->_pipeline)
          this->_renderer->bindGraphicsPipeline(VK_NULL_HANDLE);
        vkDestroyPipeline(this->_renderer->context(), (VkPipeline)this->_pipeline, nullptr);
      }
      catch (...) {}
      this->_pipeline = VK_NULL_HANDLE;
    }
    if (this->_pipelineLayout != VK_NULL_HANDLE) {
      try {
        vkDestroyPipelineLayout(this->_renderer->context(), this->_pipelineLayout, nullptr);
        vkDestroyRenderPass(this->_renderer->context(), this->_renderPass, nullptr);
      }
      catch (...) {}
      this->_pipelineLayout = VK_NULL_HANDLE;
      this->_renderPass = VK_NULL_HANDLE;
    }
  }


  // -- GraphicsPipeline::Builder - setters -- ---------------------------------

  GraphicsPipeline::Builder::Builder() noexcept {
    this-> _inputLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    this->_inputTopology.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    this->_inputTopology.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    this->_inputTopology.primitiveRestartEnable = VK_FALSE;
    this->_viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    this->_viewport.viewportCount = this->_viewport.scissorCount = 1u;

    this->_graphicsPipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    this->_graphicsPipeline.pVertexInputState = &(this->_inputLayout);
    this->_graphicsPipeline.pInputAssemblyState = &(this->_inputTopology);
    this->_graphicsPipeline.pViewportState = &(this->_viewport);
  }

  // -- pipeline stages --

  // Bind vertex shader input layout + set vertex topology
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputLayout(InputLayoutHandle inputLayout,
                                                                       VertexTopology topology) noexcept {
    this->_inputLayout.vertexBindingDescriptionCount = static_cast<uint32_t>(inputLayout->bindings.length());
    this->_inputLayout.pVertexBindingDescriptions = inputLayout->bindings.value;
    this->_inputLayout.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputLayout->attributes.length());
    this->_inputLayout.pVertexAttributeDescriptions = inputLayout->attributes.value;
    this->_inputTopology.topology = (VkPrimitiveTopology)topology;
    return *this;
  }

  // Bind shader stages to pipeline
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setShaderStages(const Shader shaders[], size_t shaderCount) {
    this->_graphicsPipeline.stageCount = static_cast<uint32_t>(shaderCount);
    this->_shaderStages = DynamicArray<VkPipelineShaderStageCreateInfo>(shaderCount);
    for (auto* it = this->_shaderStages.value; shaderCount; ++it, ++shaders, --shaderCount)
      memcpy(it, &(shaders->descriptor()), sizeof(VkPipelineShaderStageCreateInfo));
    this->_graphicsPipeline.pStages = this->_shaderStages.value;
    return *this;
  }

  // Set viewports and scissor-test rectangles
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setViewports(const Viewport::Descriptor viewports[], const ScissorRectangle::Descriptor scissorTests[],
    size_t viewportCount, bool isDynamicCount) noexcept {
    this->_viewport.viewportCount = this->_viewport.scissorCount = static_cast<uint32_t>(viewportCount);
    this->_viewport.pViewports = viewports;
    this->_viewport.pScissors = scissorTests;
    this->_useDynamicViewportCount = isDynamicCount;
    return *this;
  }

  // -- rendering states --

# define __P_MAX_DYNAMIC_STATE_COUNT 18

  // Set Vulkan-specific dynamic state flags
  uint32_t GraphicsPipeline::Builder::_setDynamicState(VkInstance instance, VkDynamicState* dynamicStates) noexcept {
    VulkanLoader& loader = VulkanLoader::instance();

    uint32_t dynamicStateCount = 0;
    if (this->_useDynamicCulling && loader.isDynamicCullingSupported(instance)) { // +3 -> 3
      dynamicStates[dynamicStateCount]   = VK_DYNAMIC_STATE_CULL_MODE_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_FRONT_FACE_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_LINE_WIDTH;
      ++dynamicStateCount;
    }
    if (this->_useBlendConstants) {
      dynamicStates[dynamicStateCount] = VK_DYNAMIC_STATE_BLEND_CONSTANTS; // +1 -> 4
      ++dynamicStateCount;
    }
    if (this->_useDynamicDepthTest && loader.isDynamicDepthTestSupported(instance)) { // +5 -> 9
      dynamicStates[dynamicStateCount]   = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT;
      ++dynamicStateCount;
    }
    if (this->_useDynamicStencilTest && loader.isDynamicStencilTestSupported(instance)) { // +5 -> 14
      dynamicStates[dynamicStateCount]   = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_OP_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT;
      ++dynamicStateCount;
    }
    if (this->_viewport.pViewports == nullptr) { // +1 -> 15
      dynamicStates[dynamicStateCount] = VK_DYNAMIC_STATE_VIEWPORT;
      ++dynamicStateCount;
    }
    if (this->_viewport.pScissors == nullptr) { // +1 -> 16
      dynamicStates[dynamicStateCount] = VK_DYNAMIC_STATE_SCISSOR;
      ++dynamicStateCount;
    }
    if (this->_useDynamicViewportCount && loader.isDynamicViewportCountSupported(instance)) { // +2 -> 18
      dynamicStates[dynamicStateCount]   = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT;
      dynamicStates[++dynamicStateCount] = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT;
      ++dynamicStateCount;
    }
    static_assert(__P_MAX_DYNAMIC_STATE_COUNT >= 18, "GraphicsPipeline.Builder: dynamicStates array too small");
    return dynamicStateCount;
  }

  // Set Vulkan-specific pipeline flags
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setFlags(VkPipelineCreateFlagBits flags, GraphicsPipeline::Handle basePipeline, 
                                                                 int32_t basePipelineIndex, VkPipelineCache parentCache) noexcept {
    this->_graphicsPipeline.flags = flags;
    if (basePipeline != VK_NULL_HANDLE)
      this->_graphicsPipeline.flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    this->_graphicsPipeline.basePipelineHandle = (VkPipeline)basePipeline;
    this->_graphicsPipeline.basePipelineIndex = basePipelineIndex;
    this->_parentCache = parentCache;
    return *this;
  }


  // -- GraphicsPipeline::Builder - creation -- --------------------------------

  struct __TemporaryVkStates final { // Temporary vulkan state holder (unbinding on destruction)
    inline __TemporaryVkStates(VkGraphicsPipelineCreateInfo& pipeline) noexcept : graphicsPipeline(pipeline) {}
    inline ~__TemporaryVkStates() noexcept {
      graphicsPipeline.pDynamicState = nullptr;
      if ((int)multisampleState.sType != 0)
        graphicsPipeline.pMultisampleState = nullptr;
    }

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleState{};
    VkGraphicsPipelineCreateInfo& graphicsPipeline;
  };

  // Build a graphics pipeline (based on current params)
  GraphicsPipeline GraphicsPipeline::Builder::create(std::shared_ptr<Renderer> renderer,
                                                     GraphicsPipeline::RenderTargetDescription renderTargets[], size_t renderTargetCount) {
    if (renderer == nullptr || renderTargetCount == 0 || renderTargets == nullptr)
      throw std::invalid_argument("GraphicsPipeline: renderer or renderTargets is NULL");
    if (this->_shaderStages.length() == 0)
      throw std::logic_error("GraphicsPipeline: missing shader stage(s)");
    if (this->_graphicsPipeline.pRasterizationState == nullptr || this->_graphicsPipeline.pColorBlendState == nullptr)
      throw std::logic_error("GraphicsPipeline: missing rasterizer or blend state");

    // set default multisampling + dynamic states
    __TemporaryVkStates tmpState(this->_graphicsPipeline);
    if (this->_graphicsPipeline.pMultisampleState == nullptr) {
      tmpState.multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      tmpState.multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      this->_graphicsPipeline.pMultisampleState = &(tmpState.multisampleState);
    }

    VkDynamicState dynamicStates[__P_MAX_DYNAMIC_STATE_COUNT];
    uint32_t dynamicStateCount = _setDynamicState(renderer->vkInstance(), dynamicStates);
    if (dynamicStateCount) {
      tmpState.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      tmpState.dynamicStateInfo.dynamicStateCount = dynamicStateCount;
      tmpState.dynamicStateInfo.pDynamicStates = dynamicStates;
      this->_graphicsPipeline.pDynamicState = &(tmpState.dynamicStateInfo);
    }
    else
      this->_graphicsPipeline.pDynamicState = nullptr;

    // create render-pass + pipeline layout
    /*DynamicArray<VkAttachmentDescription> colorAttachments(renderTargetCount);
    memset(colorAttachments.value, 0, renderTargetCount*sizeof(VkAttachmentDescription));
    for (size_t i = 0; i < renderTargetCount; ++i) {
      auto& colorAttachment = colorAttachments.value[i];
      colorAttachment.format = _getDataFormatComponents(renderTargets[i].backBufferFormat);
      colorAttachment.samples = this->_graphicsPipeline.pMultisampleState->rasterizationSamples;
      colorAttachment.loadOp = ((renderTargets[i].outputFlags & pandora::video::TargetOutputFlag::swapNoDiscard) == true)
                             ? VK_ATTACHMENT_LOAD_OP_LOAD // requires VK_ACCESS_COLOR_ATTACHMENT_READ_BIT read access
                             : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                             //TODO: gérer discard/clear/keep -> VK_ATTACHMENT_LOAD_OP_CLEAR si clear
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment.stencilLoadOp = (this->_graphicsPipeline.pDepthStencilState != nullptr)
                                    ? colorAttachment.loadOp // requires VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT if type is "LOAD"
                                    : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //TODO: optimiser uniquement si mode "LOAD" (keep)
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //TODO: gérer swap-chain / swap-chain shared / texture-target for texture / staging texture-target
    }*/

    //...
    //this->_graphicsPipeline.layout = pipelineLayout;
    //this->_graphicsPipeline.renderPass = renderPass;
    this->_graphicsPipeline.subpass = 0;

    //TODO: dynamic bools dans states liés
    //TODO: garantir lifetime shaders tant que pipeline existe ?

    // create graphics pipeline
    return GraphicsPipeline(renderer, this->_graphicsPipeline, this->_parentCache);
  }

#endif
