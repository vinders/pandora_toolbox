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

// -- RasterizerParams - rasterizer state params -- ----------------------------

  RasterizerParams::RasterizerParams() noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _params.cullMode = VK_CULL_MODE_BACK_BIT;
    _params.polygonMode = VK_POLYGON_MODE_FILL;
    _params.frontFace = VK_FRONT_FACE_CLOCKWISE;
    _params.depthClampEnable = VK_FALSE;
    _params.lineWidth = 1.f;
    _depthClipping.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;
    _lineRasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
  }
  
  RasterizerParams::RasterizerParams(CullMode cull, FillMode fill, bool isFrontClockwise,
                                     bool depthClipping, bool scissorClipping) noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _params.cullMode = (VkCullModeFlags)cull;
    _params.frontFace = isFrontClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

    _params.depthClampEnable = VK_FALSE;
    _depthClipping.depthClipEnable = depthClipping ? VK_TRUE : VK_FALSE;
    fillMode(fill);
    _params.lineWidth = 1.f;
    _useScissorClipping = scissorClipping;
  }

  // ---

  // Set filled/wireframe polygon rendering
  RasterizerParams& RasterizerParams::fillMode(FillMode fill) noexcept {
    if (fill == FillMode::linesAA) {
      _params.polygonMode = VK_POLYGON_MODE_LINE;
      _lineRasterization.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
    }
    else {
      _params.polygonMode = (VkPolygonMode)fill;
      _lineRasterization.lineRasterizationMode = (VkLineRasterizationModeEXT)0;
    }
    return *this;
  }

// -- DepthStencilParams - depth/stencil testing state params -- ---------------

  void DepthStencilParams::_init(VkBool32 enableDepth, VkBool32 enableStencil, VkCompareOp depthComp, 
                                 VkCompareOp frontFaceComp, VkCompareOp backFaceComp) noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _params.depthTestEnable = enableDepth;
    _params.depthCompareOp = depthComp;
    _params.depthWriteEnable = VK_TRUE;

    _params.stencilTestEnable = enableStencil;
    _params.front.compareOp = frontFaceComp;
    _params.back.compareOp = backFaceComp;
    _params.front.compareMask = _params.back.compareMask = _params.front.writeMask = _params.back.writeMask = 0xFF;
  }

// -- BlendParams - blend state params -- --------------------------------------

  BlendParams::BlendParams(BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                           BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                           ColorComponentFlag mask) noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _params.logicOp = VK_LOGIC_OP_COPY;
    _attachementState.blendEnable = VK_TRUE;
    _attachementState.colorWriteMask = (VkColorComponentFlags)mask;
    
    _attachementState.srcColorBlendFactor = (VkBlendFactor)srcColorFactor;
    _attachementState.dstColorBlendFactor = (VkBlendFactor)destColorFactor;
    _attachementState.colorBlendOp = (VkBlendOp)colorBlendOp;
    _attachementState.srcAlphaBlendFactor = (VkBlendFactor)srcAlphaFactor;
    _attachementState.dstAlphaBlendFactor = (VkBlendFactor)destAlphaFactor;
    _attachementState.alphaBlendOp = (VkBlendOp)alphaBlendOp;
    // no attachment here -> bound at pipeline creation (+ duplicated for each existing target)
  }

  BlendParams& BlendParams::blendConstant(const ColorChannel constantColorRgba[4]) noexcept {
    memcpy(_params.blendConstants, constantColorRgba, sizeof(ColorChannel)*4u);
    return *this;
  }

  // ---

  BlendPerTargetParams::BlendPerTargetParams() noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _params.logicOp = VK_LOGIC_OP_COPY;
  }

  BlendPerTargetParams& BlendPerTargetParams::setTargetBlend(uint32_t targetIndex,
                                              BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                                              BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                                              ColorComponentFlag mask) noexcept {
    VkPipelineColorBlendAttachmentState target{};
    target.blendEnable = VK_TRUE;
    target.colorWriteMask = (VkColorComponentFlags)mask;
    target.srcColorBlendFactor = (VkBlendFactor)srcColorFactor;
    target.dstColorBlendFactor = (VkBlendFactor)destColorFactor;
    target.colorBlendOp = (VkBlendOp)colorBlendOp;
    target.srcAlphaBlendFactor = (VkBlendFactor)srcAlphaFactor;
    target.dstAlphaBlendFactor = (VkBlendFactor)destAlphaFactor;
    target.alphaBlendOp = (VkBlendOp)alphaBlendOp;

    if (_attachementsPerTarget.size() <= targetIndex) {
      if (_attachementsPerTarget.size() < targetIndex) {
        VkPipelineColorBlendAttachmentState disabled{};
        disabled.blendEnable = VK_FALSE;
        while (_attachementsPerTarget.size() < targetIndex)
          _attachementsPerTarget.push_back(disabled);
      }
      _attachementsPerTarget.push_back(target);
    }
    else
      _attachementsPerTarget[targetIndex] = target;

    return *this; // no attachment here -> bound at pipeline creation (+ add missing targets)
  }

  BlendPerTargetParams& BlendPerTargetParams::disableTargetBlend(uint32_t targetIndex) noexcept {
    VkPipelineColorBlendAttachmentState disabled{};
    disabled.blendEnable = VK_FALSE;

    if (_attachementsPerTarget.size() <= targetIndex) {
      while (_attachementsPerTarget.size() <= targetIndex)
        _attachementsPerTarget.push_back(disabled);
    }
    else
      _attachementsPerTarget[targetIndex] = disabled;

    return *this;
  }

  BlendPerTargetParams& BlendPerTargetParams::blendConstant(const ColorChannel constantColorRgba[4]) noexcept {
    memcpy(_params.blendConstants, constantColorRgba, sizeof(ColorChannel)*4u);
    return *this;
  }


// -----------------------------------------------------------------------------
// graphics pipeline builder
// -----------------------------------------------------------------------------

// -- GraphicsPipeline -- ------------------------------------------------------

  // Create pipeline object -- reserved for internal use or advanced usage
  GraphicsPipeline::GraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo, std::shared_ptr<Renderer> renderer, VkPipelineCache cache)
    : _renderer(std::move(renderer)),
      _renderPass(createInfo.renderPass),
      _pipelineLayout(createInfo.layout) {
    assert(this->_renderer != nullptr && createInfo.renderPass != VK_NULL_HANDLE && createInfo.layout != VK_NULL_HANDLE);
    if (createInfo.stageCount == 0)
      throw std::logic_error("GraphicsPipeline: vertex shader required");
    if (createInfo.pRasterizationState == nullptr || createInfo.pColorBlendState == nullptr)
      throw std::logic_error("GraphicsPipeline: missing required pipeline state");

    auto result = vkCreateGraphicsPipelines(this->_renderer->context(), cache, 1, &createInfo, nullptr, &(this->_pipelineHandle));
    if (result != VK_SUCCESS || this->_pipelineHandle == VK_NULL_HANDLE)
      throwError(result, "GraphicsPipeline: creation error");
  }

  // Destroy pipeline object
  void GraphicsPipeline::release() noexcept {
    // destroy pipeline
    if (this->_pipelineHandle != VK_NULL_HANDLE) {
      if (this->_renderer->_boundPipeline == this->_pipelineHandle)
        this->_renderer->bindGraphicsPipeline(VK_NULL_HANDLE);

      try { vkDestroyPipeline(this->_renderer->context(), this->_pipelineHandle, nullptr); } catch (...) {}
      this->_pipelineHandle = VK_NULL_HANDLE;
    }
    // destroy pipeline components
    if (this->_renderPass != VK_NULL_HANDLE) {
      try { vkDestroyRenderPass(this->_renderer->context(), this->_renderPass, nullptr); } catch (...) {}
      this->_renderPass = VK_NULL_HANDLE;
    }
    if (this->_pipelineLayout != VK_NULL_HANDLE) {
      try { vkDestroyPipelineLayout(this->_renderer->context(), this->_pipelineLayout, nullptr); } catch (...) {}
      this->_pipelineLayout = VK_NULL_HANDLE;
    }
  }


// -- GraphicsPipeline.Builder -- ----------------------------------------------

  // Create pipeline builder
  GraphicsPipeline::Builder::Builder(std::shared_ptr<Renderer> renderer) : _renderer(renderer) { // throws
    if (this->_renderer == nullptr)
      throw std::logic_error("GraphicsPipeline.Builder: renderer is NULL");

    this->_inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    this->_inputTopology.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    this->_inputTopology.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    this->_inputTopology.primitiveRestartEnable = VK_FALSE;
    this->_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    this->_viewportState.viewportCount = this->_viewportState.scissorCount = 1u;
    this->_multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    this->_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    this->_multisampleState.minSampleShading = 1.0f;
#   ifndef __P_DISABLE_TESSELLATION_STAGE
      this->_tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
#   endif

    this->_globalPipelineParams.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    this->_globalPipelineParams.pVertexInputState = &(this->_inputState);
    this->_globalPipelineParams.pInputAssemblyState = &(this->_inputTopology);
    this->_globalPipelineParams.pViewportState = &(this->_viewportState);
    this->_globalPipelineParams.pStages = this->_shaderStages;
  }

  // ---

  // Bind vertex shader input layout description (optional)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputLayout(InputLayout inputLayout) noexcept {
    this->_inputState.pVertexBindingDescriptions = inputLayout->bindings.value;
    this->_inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(inputLayout->bindings.length());
    this->_inputState.pVertexAttributeDescriptions = inputLayout->attributes.value;
    this->_inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputLayout->attributes.length());
    this->_inputLayout = std::move(inputLayout);
    return *this;
  }
  // Set custom vertex shader input layout description (optional) -- only for Vulkan
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputLayout(VkVertexInputBindingDescription* inputBindings, size_t bindingsLength,
                                                          VkVertexInputAttributeDescription* layoutAttributes, size_t attributesLength) noexcept {
    this->_inputState.pVertexBindingDescriptions = inputBindings;
    this->_inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingsLength);
    this->_inputState.pVertexAttributeDescriptions = layoutAttributes;
    this->_inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributesLength);
    return *this;
  }

  // Bind shader stages to pipeline (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setShaderStages(const Shader shaders[], size_t shaderCount) { // throws
    if (shaderCount > __P_MAX_DISPLAY_SHADER_NUMBER)
      throw std::invalid_argument("GraphicsPipeline::Builder: invalid shader types (check project options)");
    this->_globalPipelineParams.stageCount = static_cast<uint32_t>(shaderCount);

    VkPipelineShaderStageCreateInfo* current = this->_shaderStages;
    const Shader* itEnd = shaders + (intptr_t)shaderCount;
    for (const Shader* it = shaders; it < itEnd; ++it, ++current) {
      if (it->type() == ShaderType::compute)
        throw std::invalid_argument("GraphicsPipeline::Builder: invalid shader type (compute)");
      current->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      current->stage = (VkShaderStageFlagBits)it->type();
      current->module = it->handle()->value();
      current->pName = it->entryPoint();
    }
    return *this;
  }
  // Remove all shader stages
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::clearShaderStages() noexcept {
    this->_globalPipelineParams.stageCount = 0;
    return *this;
  }

  // Bind (or replace) shader module for a specific stage (at least vertex+fragment or compute required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::attachShaderStage(const Shader& shaderModule) { // throws
    if (shaderModule.type() == ShaderType::compute)
      throw std::invalid_argument("GraphicsPipeline::Builder: invalid shader type (compute)");

    VkPipelineShaderStageCreateInfo* target;
    const VkPipelineShaderStageCreateInfo* itEnd = this->_shaderStages + (intptr_t)this->_globalPipelineParams.stageCount;
    for (target = this->_shaderStages; target < itEnd; ++target) {
      if (target->stage == (VkShaderStageFlagBits)shaderModule.type())
        break;
    }
    if (target >= itEnd) {
      if (this->_globalPipelineParams.stageCount >= __P_MAX_DISPLAY_SHADER_NUMBER)
        throw std::invalid_argument("GraphicsPipeline::Builder: too many shader types (check project options)");

      target = &_shaderStages[this->_globalPipelineParams.stageCount];
      target->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      target->stage = (VkShaderStageFlagBits)shaderModule.type();
      this->_globalPipelineParams.stageCount += 1u;
    }
    target->module = shaderModule.handle()->value();
    target->pName = shaderModule.entryPoint();
    return *this;
  }
  // Remove a shader stage
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::detachShaderStage(ShaderType stage) noexcept {
    const VkPipelineShaderStageCreateInfo* itLast = this->_shaderStages + ((intptr_t)this->_globalPipelineParams.stageCount - 1);
    for (VkPipelineShaderStageCreateInfo* it = this->_shaderStages; it <= itLast; ++it) {
      if (it->stage == (VkShaderStageFlagBits)stage) {
        if (it < itLast)
          memmove(it, it + (intptr_t)1, (itLast - it)*sizeof(VkPipelineShaderStageCreateInfo));
        this->_globalPipelineParams.stageCount -= 1u;
        break;
      }
    }
    return *this;
  }

  // ---

  // Bind rasterization state (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerState(const RasterizerParams& state) noexcept {
    this->_globalPipelineParams.pRasterizationState = &(state.descriptor());

    // update chained list - depth clipping
    if (state._depthClippingDesc().depthClipEnable && this->_renderer->isExtensionEnabled("VK_EXT_depth_clip_enable")) {
      if (state.descriptor().pNext != &(state._depthClippingDesc())) { // enabled && available -> insert in chained list
        state._depthClippingDesc().pNext = state.descriptor().pNext;
        state.descriptor().pNext = &(state._depthClippingDesc());
      }
    }
    else if (state.descriptor().pNext == &(state._depthClippingDesc())) { // disabled || unavailable -> remove from chained list
      state.descriptor().pNext = nullptr;
      if (state._depthClippingDesc().pNext) {
        state.descriptor().pNext = state._depthClippingDesc().pNext;
        state._depthClippingDesc().pNext = nullptr;
      }
    }
    // update chained list - line rasterization state (for AA)
    if (state._lineStateDesc().lineRasterizationMode && state.descriptor().polygonMode == VK_POLYGON_MODE_LINE
    && this->_renderer->isExtensionEnabled("VK_EXT_line_rasterization")) { // enabled && available -> insert in chained list
      if (state.descriptor().pNext == &(state._depthClippingDesc())) {
        state._lineStateDesc().pNext = state._depthClippingDesc().pNext;
        state._depthClippingDesc().pNext = &(state._lineStateDesc());
      }
      else {
        state._lineStateDesc().pNext = state.descriptor().pNext;
        state.descriptor().pNext = &(state._lineStateDesc());
      }
    }
    else { // disabled || unavailable -> remove from chained list
      if (state._depthClippingDesc().pNext == &(state._lineStateDesc())) {
        state._depthClippingDesc().pNext = state._lineStateDesc().pNext;
        state._lineStateDesc().pNext = nullptr;
      }
      else if (state.descriptor().pNext == &(state._lineStateDesc())) {
        state.descriptor().pNext = state._lineStateDesc().pNext;
        state._lineStateDesc().pNext = nullptr;
      }
    }
    return *this;
  }

  // Bind color/alpha blending state -- common to all render-targets (one of the 2 methods required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setBlendState(const BlendParams& state) noexcept {
    this->_globalPipelineParams.pColorBlendState = &state.descriptor();
    state.descriptor().pAttachments = &(state._attachDesc());
    state.descriptor().attachmentCount = 1u;
    return *this;
  }
  // Bind color/alpha blending state -- customized per render-target (one of the 2 methods required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setBlendState(const BlendPerTargetParams& state) noexcept {
    this->_globalPipelineParams.pColorBlendState = &state.descriptor();
    state.descriptor().pAttachments = state._attachDesc().data();
    state.descriptor().attachmentCount = static_cast<uint32_t>(state._attachDesc().size());
    return *this;
  }

  // ---

  // Set viewports and scissor-test rectangles (optional: dynamic viewports if ignored)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setViewports(const Viewport viewports[], size_t viewportCount,
                                                        const ScissorRectangle scissorTests[], size_t scissorCount,
                                                        bool useDynamicCount) noexcept {
    if (viewports != nullptr) {
      this->_viewports = DynamicArray<VkViewport>(viewportCount);
      for (size_t i = 0; i < viewportCount; ++i)
        memcpy(&(this->_viewports.value[i]), viewports[i].descriptor(), sizeof(VkViewport));
      this->_viewportState.pViewports = this->_viewports.value;
    }
    else {
      this->_viewports.clear();
      this->_viewportState.pViewports = nullptr;
    }
    this->_viewportState.viewportCount = static_cast<uint32_t>(viewportCount);

    if (scissorTests != nullptr) {
      this->_scissors = DynamicArray<VkRect2D>(scissorCount);
      for (size_t i = 0; i < scissorCount; ++i)
        memcpy(&(this->_scissors.value[i]), scissorTests[i].descriptor(), sizeof(VkViewport));
      this->_viewportState.pScissors = this->_scissors.value;
    }
    else {
      this->_scissors.clear();
      this->_viewportState.pScissors = nullptr;
    }
    this->_viewportState.scissorCount = static_cast<uint32_t>(scissorCount);

    this->_useDynamicViewportCount = useDynamicCount;
    return *this;
  }

  // ---

  // Build a graphics pipeline (based on current params)
  GraphicsPipeline GraphicsPipeline::Builder::build(VkPipelineCache parentCache) { // throws
    //set dynamic states
    //...
    VulkanLoader& loader = VulkanLoader::instance();
    if (this->_useDynamicViewportCount) {
      if (!loader.isDynamicViewportCountSupported(this->_renderer->vkInstance()))
        throw std::runtime_error("GraphicsPipeline.Builder: dynamic viewport count not supported by driver");
      //...
    }

    //create RenderPass + PipelineLayout
    //...

    //if (nb_render_targets > this->_globalPipelineParams.pColorBlendState.attachmentCount) {
      // allouer plus grand + copier
      // _params.pAttachments = &attachementState; _params.attachmentCount = nb_render_targets;
      //...
    //}
    // else if nb_render_targets < this->_globalPipelineParams.pColorBlendState.attachmentCount)
      // this->_globalPipelineParams.pColorBlendState.attachmentCount = nb_render_targets;
    return GraphicsPipeline(this->_globalPipelineParams, this->_renderer, parentCache);
  }





  // -- GraphicsPipeline::Builder - setters -- ---------------------------------

# define __P_MAX_DYNAMIC_STATE_COUNT 18

  /*// Set Vulkan-specific dynamic state flags
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
    /-*DynamicArray<VkAttachmentDescription> colorAttachments(renderTargetCount);
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
    }*-/

    //...
    //this->_graphicsPipeline.layout = pipelineLayout;
    //this->_graphicsPipeline.renderPass = renderPass;
    this->_graphicsPipeline.subpass = 0;

    //TODO: dynamic bools dans states liés
    //TODO: garantir lifetime shaders tant que pipeline existe ?

    // create graphics pipeline
    return GraphicsPipeline(renderer, this->_graphicsPipeline, this->_parentCache);
  }*/

#endif
