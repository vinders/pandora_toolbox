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
  }
  
  RasterizerParams::RasterizerParams(CullMode cull, FillMode fill, bool isFrontClockwise,
                                     bool depthClipping, bool scissorClipping) noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _params.cullMode = (VkCullModeFlags)cull;
    _params.frontFace = isFrontClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

    _params.depthClampEnable = VK_FALSE;
    _params.polygonMode = (VkPolygonMode)fill;
    _params.lineWidth = 1.f;
    _useDepthClipping = depthClipping;
    _useScissorClipping = scissorClipping;
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
    _params.blendConstants[0] = _params.blendConstants[1] = _params.blendConstants[2] = _params.blendConstants[3] = 1.f;
    
    _attachementState.srcColorBlendFactor = (VkBlendFactor)srcColorFactor;
    _attachementState.dstColorBlendFactor = (VkBlendFactor)destColorFactor;
    _attachementState.colorBlendOp = (VkBlendOp)colorBlendOp;
    _attachementState.srcAlphaBlendFactor = (VkBlendFactor)srcAlphaFactor;
    _attachementState.dstAlphaBlendFactor = (VkBlendFactor)destAlphaFactor;
    _attachementState.alphaBlendOp = (VkBlendOp)alphaBlendOp;
    // no attachment here -> bound at pipeline creation (+ duplicated for each existing target)
  }

  BlendParams& BlendParams::blendConstant(const ColorChannel constantColorRgba[4], bool isDynamic) noexcept {
    memcpy(_params.blendConstants, constantColorRgba, sizeof(ColorChannel)*4u);
    _useDynamicBlendConstants = isDynamic;
    return *this;
  }

  // ---

  BlendPerTargetParams::BlendPerTargetParams() noexcept {
    _params.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _params.logicOp = VK_LOGIC_OP_COPY;
    _params.blendConstants[0] = _params.blendConstants[1] = _params.blendConstants[2] = _params.blendConstants[3] = 1.f;
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

  BlendPerTargetParams& BlendPerTargetParams::blendConstant(const ColorChannel constantColorRgba[4], bool isDynamic) noexcept {
    memcpy(_params.blendConstants, constantColorRgba, sizeof(ColorChannel)*4u);
    _useDynamicBlendConstants = isDynamic;
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
    assert(this->_renderer != nullptr);
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
      if (this->_renderer->_attachedPipeline == this->_pipelineHandle)
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

    _multisampleDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _multisampleDesc.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampleDesc.minSampleShading = 1.0f;
    _lineRasterDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;

    _viewportsDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    _viewportsDesc.viewportCount = _viewportsDesc.scissorCount = 1u;
    _inputLayoutDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    _inputTopology.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    _inputTopology.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    _inputTopology.primitiveRestartEnable = VK_FALSE;
#   ifndef __P_DISABLE_TESSELLATION_STAGE
      _tessellationTopology.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
#   endif
    _depthClippingDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;

    _descriptor.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    _descriptor.pVertexInputState = &_inputLayoutDesc;
    _descriptor.pInputAssemblyState = &_inputTopology;
    _descriptor.pViewportState = &_viewportsDesc;
    _descriptor.pMultisampleState = &_multisampleDesc;
    _descriptor.pStages = _shaderStagesDesc;
  }

  GraphicsPipeline::Builder::~Builder() noexcept {
    for (int i = 0; i < __P_MAX_DISPLAY_SHADER_NUMBER; ++i)
      _shaderStagesObj[i].release();
    _inputLayoutObj.reset();

    _attachementsPerTarget.clear();
    _viewports.clear();
    _scissors.clear();

    _renderer.reset();
  }


// -- GraphicsPipeline.Builder - input & shaders -- ----------------------------

  // Bind vertex shader input layout description (optional)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputLayout(InputLayout inputLayout) noexcept {
    _inputLayoutDesc.pVertexBindingDescriptions = inputLayout->bindings.value;
    _inputLayoutDesc.vertexBindingDescriptionCount = static_cast<uint32_t>(inputLayout->bindings.length());
    _inputLayoutDesc.pVertexAttributeDescriptions = inputLayout->attributes.value;
    _inputLayoutDesc.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputLayout->attributes.length());
    _inputLayoutObj = std::move(inputLayout);
    return *this;
  }
  // Set custom vertex shader input layout description (optional) -- only for Vulkan
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputLayout(VkVertexInputBindingDescription* inputBindings, size_t bindingsLength,
                                                          VkVertexInputAttributeDescription* layoutAttributes, size_t attributesLength) noexcept {
    _inputLayoutDesc.pVertexBindingDescriptions = inputBindings;
    _inputLayoutDesc.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingsLength);
    _inputLayoutDesc.pVertexAttributeDescriptions = layoutAttributes;
    _inputLayoutDesc.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributesLength);
    _inputLayoutObj = nullptr;
    return *this;
  }

  // ---

  // Bind shader stages to pipeline (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setShaderStages(const Shader shaders[], size_t shaderCount) { // throws
    assert(shaderCount <= __P_MAX_DISPLAY_SHADER_NUMBER); // fails if too many shader types (check cmake options for tessel/geometry)
    VkPipelineShaderStageCreateInfo* curDesc = this->_shaderStagesDesc;
    Shader* curObj = this->_shaderStagesObj;

    const Shader* itEnd = shaders + (intptr_t)shaderCount;
    for (const Shader* it = shaders; it < itEnd; ++it, ++curDesc, ++curObj) {
      if (it->type() == ShaderType::compute)
        throw std::invalid_argument("GraphicsPipeline::Builder: compute shader not allowed");
      *curObj = *it;
      curDesc->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      curDesc->stage = (VkShaderStageFlagBits)curObj->type();
      curDesc->module = curObj->handle()->value();
      curDesc->pName = curObj->entryPoint();
    }

    if (_descriptor.stageCount > shaderCount) { // release previous shader objects that haven't been replaced
      for (Shader* endObj = this->_shaderStagesObj + (intptr_t)_descriptor.stageCount; curObj < endObj; ++curObj)
        curObj->release();
    }
    _descriptor.stageCount = static_cast<uint32_t>(shaderCount);
    return *this;
  }
  // Remove all shader stages
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::clearShaderStages() noexcept {
    const Shader* endObj = _shaderStagesObj + (intptr_t)_descriptor.stageCount;
    for (Shader* curObj = _shaderStagesObj; curObj < endObj; ++curObj)
      curObj->release();

    _descriptor.stageCount = 0;
    return *this;
  }

  // Bind (or replace) shader module for a specific stage (at least vertex+fragment or compute required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::attachShaderStage(const Shader& shaderStage) { // throws
    if (shaderStage.type() == ShaderType::compute)
      throw std::invalid_argument("GraphicsPipeline::Builder: compute shader not allowed");

    VkPipelineShaderStageCreateInfo* target;
    const auto* itEnd = this->_shaderStagesDesc + (intptr_t)_descriptor.stageCount;
    for (target = this->_shaderStagesDesc; target < itEnd && target->stage != (VkShaderStageFlagBits)shaderStage.type(); ++target);

    if (target >= itEnd) { // shader type not yet in array -> append
      assert(_descriptor.stageCount < __P_MAX_DISPLAY_SHADER_NUMBER); // fails if too many types (cmake options for tessel/geometry)
      target = &(this->_shaderStagesDesc[_descriptor.stageCount]);
      ++_descriptor.stageCount;

      target->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      target->stage = (VkShaderStageFlagBits)shaderStage.type();
    }

    Shader* targetObj = this->_shaderStagesObj + (target - this->_shaderStagesDesc);
    *targetObj = shaderStage;
    target->module = targetObj->handle()->value();
    target->pName = targetObj->entryPoint();
    return *this;
  }
  // Remove a shader stage
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::detachShaderStage(ShaderType stage) noexcept {
    const auto* descLast = _shaderStagesDesc + (intptr_t)_descriptor.stageCount - 1;
    for (auto* curDesc = _shaderStagesDesc; curDesc <= descLast; ++curDesc) {
      if (curDesc->stage == (VkShaderStageFlagBits)stage) {
        Shader* curObj = _shaderStagesObj + (curDesc - _shaderStagesDesc);

        // erase + shift next items
        if (curDesc < descLast) {
          memmove(curDesc, curDesc + (intptr_t)1, (descLast - curDesc)*sizeof(VkPipelineShaderStageCreateInfo));
          for (const Shader* lastObj = _shaderStagesObj + (intptr_t)_descriptor.stageCount - 1; curObj < lastObj; ++curObj)
            *curObj = std::move(*(curObj + 1));
        }
        else
          curObj->release();

        --_descriptor.stageCount;
        break;
      }
    }
    return *this;
  }


// -- GraphicsPipeline.Builder - pipeline states & viewports -- ----------------

  // Bind rasterization state (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerState(const RasterizerParams& state) { // throws
    if (state._isDynamicCullingEnabled()) {
      if (!VulkanLoader::instance().isDynamicCullingSupported(this->_renderer->vkInstance()))
        throw std::runtime_error("Vulkan: dynamic culling/order not supported by driver");
      if (!_isExtendedDynamicStateSupported())
        throw std::runtime_error("Vulkan: extension for dynamic states (culling/order) not enabled");
    }

    memcpy(&_rasterizationDesc, &state.descriptor(), sizeof(VkPipelineRasterizationStateCreateInfo));
    _descriptor.pRasterizationState = &_rasterizationDesc;

    if ((FillMode)_rasterizationDesc.polygonMode == FillMode::linesAA) {
      _rasterizationDesc.polygonMode = VK_POLYGON_MODE_LINE;
      if (this->_renderer->isExtensionEnabled("VK_EXT_line_rasterization")) {
        _lineRasterDesc.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
        _lineRasterDesc.pNext = _rasterizationDesc.pNext;
        _rasterizationDesc.pNext = &_lineRasterDesc;
      }
    }
    if (state._isDepthClippingEnabled() && this->_renderer->isExtensionEnabled("VK_EXT_depth_clip_enable")) {
      _depthClippingDesc.depthClipEnable = VK_TRUE;
      _depthClippingDesc.pNext = _rasterizationDesc.pNext;
      _rasterizationDesc.pNext = &_depthClippingDesc;
    }
    _useDynamicCulling = state._isDynamicCullingEnabled();
    return *this;
  }

  // Bind depth/stencil test state (required if depth buffer used)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setDepthStencilState(const DepthStencilParams& state) { // throws
    if (state._isDynamicDepthTestEnabled() || state._isDynamicStencilTestEnabled()) {
      VulkanLoader& loader = VulkanLoader::instance();
      if (state._isDynamicDepthTestEnabled() && !loader.isDynamicDepthTestSupported(this->_renderer->vkInstance()))
        throw std::runtime_error("Vulkan: dynamic depth-test not supported by driver");
      if (state._isDynamicStencilTestEnabled() && !loader.isDynamicStencilTestSupported(this->_renderer->vkInstance()))
        throw std::runtime_error("Vulkan: dynamic stencil-test not supported by driver");
      if (!_isExtendedDynamicStateSupported())
        throw std::runtime_error("Vulkan: extension for dynamic states (depth/stencil-tests) not enabled");
    }

    memcpy(&_depthStencilDesc, &state.descriptor(), sizeof(VkPipelineDepthStencilStateCreateInfo));
    _descriptor.pDepthStencilState = &_depthStencilDesc;

    _useDynamicDepthTest = state._isDynamicDepthTestEnabled();
    _useDynamicStencilTest = state._isDynamicStencilTestEnabled();
    return *this;
  }

  // Bind color/alpha blending state -- common to all render-targets (one of the 2 methods required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setBlendState(const BlendParams& state) noexcept {
    memcpy(&_blendDesc, &state.descriptor(), sizeof(VkPipelineColorBlendStateCreateInfo));
    _descriptor.pColorBlendState = &_blendDesc;

    if (_attachementsPerTarget.length() < size_t{ 1u })
      _attachementsPerTarget = DynamicArray<VkPipelineColorBlendAttachmentState>(size_t{ 1u });
    memcpy(_attachementsPerTarget.value, &state._attachDesc(), sizeof(VkPipelineColorBlendAttachmentState));
    _useBlendPerTarget = false;

    _blendDesc.pAttachments = _attachementsPerTarget.value;
    _blendDesc.attachmentCount = 1u;
    _useDynamicBlendConstants = state._isDynamicConstantEnabled(); // no need for extensions
    return *this;
  }
  // Bind color/alpha blending state -- customized per render-target (one of the 2 methods required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setBlendState(const BlendPerTargetParams& state) noexcept {
    memcpy(&_blendDesc, &state.descriptor(), sizeof(VkPipelineColorBlendStateCreateInfo));
    _descriptor.pColorBlendState = &_blendDesc;

    if (_attachementsPerTarget.length() < state._attachDesc().size())
      _attachementsPerTarget = DynamicArray<VkPipelineColorBlendAttachmentState>(state._attachDesc().size());
    memcpy(_attachementsPerTarget.value, state._attachDesc().data(), sizeof(VkPipelineColorBlendAttachmentState)*state._attachDesc().size());
    _useBlendPerTarget = true;

    _blendDesc.pAttachments = _attachementsPerTarget.value;
    _blendDesc.attachmentCount = static_cast<uint32_t>(state._attachDesc().size());
    _useDynamicBlendConstants = state._isDynamicConstantEnabled(); // no need for extensions
    return *this;
  }

  // ---

  // Set viewports and scissor-test rectangles (optional: dynamic viewports if ignored)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setViewports(const Viewport viewports[], size_t viewportCount,
                                                        const ScissorRectangle scissorTests[], size_t scissorCount,
                                                        bool useDynamicCount) {
    if (useDynamicCount) {
      if (!VulkanLoader::instance().isDynamicViewportCountSupported(this->_renderer->vkInstance()))
        throw std::runtime_error("Vulkan: dynamic viewport count not supported by driver");
      if (!_isExtendedDynamicStateSupported())
        throw std::runtime_error("Vulkan: extension for dynamic states (viewport count) not enabled");
    }
    
    if (viewports != nullptr && viewportCount > 0) {
      this->_viewports = DynamicArray<VkViewport>(viewportCount);
      for (size_t i = 0; i < viewportCount; ++i)
        memcpy(&(this->_viewports.value[i]), viewports[i].descriptor(), sizeof(VkViewport));
      this->_viewportsDesc.pViewports = this->_viewports.value;
    }
    else {
      this->_viewports.clear();
      this->_viewportsDesc.pViewports = nullptr;
    }
    this->_viewportsDesc.viewportCount = static_cast<uint32_t>(viewportCount);

    if (scissorTests != nullptr && scissorCount > 0) {
      this->_scissors = DynamicArray<VkRect2D>(scissorCount);
      for (size_t i = 0; i < scissorCount; ++i)
        memcpy(&(this->_scissors.value[i]), scissorTests[i].descriptor(), sizeof(VkRect2D));
      this->_viewportsDesc.pScissors = this->_scissors.value;
    }
    else {
      this->_scissors.clear();
      this->_viewportsDesc.pScissors = nullptr;
    }
    this->_viewportsDesc.scissorCount = static_cast<uint32_t>(scissorCount);

    this->_useDynamicViewportCount = useDynamicCount;
    return *this;
  }


// -- GraphicsPipeline.Builder - build -- --------------------------------------

  // Provide render-targets to determine the number of targets, their respective format and multisampling (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRenderTargetFormat(void* renderTargets, size_t targetCount,
    uint32_t sampleCount) {
    //if (renderTargets == nullptr || renderTargetCount == 0)
    //  throw std::invalid_argument("GraphicsPipeline: no render target provided");
    //...

    _multisampleDesc.rasterizationSamples = (VkSampleCountFlagBits)sampleCount;
    if (sampleCount > 1 && _renderer->enabledFeatures().sampleRateShading) {
      _multisampleDesc.sampleShadingEnable = VK_TRUE;
      _multisampleDesc.minSampleShading = 0.2f; // note: closer to 1 = smoother
    }
    else {
      _multisampleDesc.sampleShadingEnable = VK_FALSE;
      _multisampleDesc.minSampleShading = 1.0f;
    }
    return *this;
  }

  // ---

# define __P_MAX_DYNAMIC_STATE_COUNT 17

  // Set Vulkan-specific dynamic state flags
  uint32_t GraphicsPipeline::Builder::_setDynamicState(VkDynamicState* dynamicState) noexcept {
    uint32_t dynamicStateCount = 0;
    if (_useDynamicCulling) { // +2 -> 2
      dynamicState[dynamicStateCount]   = VK_DYNAMIC_STATE_CULL_MODE_EXT;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_FRONT_FACE_EXT;
      ++dynamicStateCount;
    }
    if (_useDynamicBlendConstants) {
      dynamicState[dynamicStateCount] = VK_DYNAMIC_STATE_BLEND_CONSTANTS; // +1 -> 3
      ++dynamicStateCount;
    }
    if (_useDynamicDepthTest) { // +5 -> 8
      dynamicState[dynamicStateCount]   = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT;
      ++dynamicStateCount;
    }
    if (_useDynamicStencilTest) { // +5 -> 13
      dynamicState[dynamicStateCount]   = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_OP_EXT;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT;
      ++dynamicStateCount;
    }
    if (_viewportsDesc.pViewports == nullptr || _useDynamicViewportCount) { // +1 -> 15
      dynamicState[dynamicStateCount] = VK_DYNAMIC_STATE_VIEWPORT;
      ++dynamicStateCount;
    }
    if (_viewportsDesc.pScissors == nullptr || _useDynamicViewportCount) { // +1 -> 16
      dynamicState[dynamicStateCount] = VK_DYNAMIC_STATE_SCISSOR;
      ++dynamicStateCount;
    }
    if (_useDynamicViewportCount) { // +2 -> 17
      dynamicState[dynamicStateCount]   = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT;
      dynamicState[++dynamicStateCount] = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT;
      ++dynamicStateCount;
    }
    static_assert(__P_MAX_DYNAMIC_STATE_COUNT >= 17, "GraphicsPipeline.Builder: dynamicState array too small");
    return dynamicStateCount;
  }

  namespace pandora {
    namespace video {
      namespace vulkan {
        template <typename _Type>
        struct __TempState final {
          inline __TempState(const _Type** descLink) noexcept : _descriptorLink(descLink) {}
          inline ~__TempState() noexcept { *_descriptorLink = nullptr; }
          inline void bind() noexcept { *_descriptorLink = &value; }

          _Type value{};
        private:
          const _Type** _descriptorLink = nullptr;
        };
      }
    }
  }

  // ---

  // Build a graphics pipeline (based on current params)
  GraphicsPipeline GraphicsPipeline::Builder::build(VkPipelineCache parentCache) { // throws
    // configure dynamic states
    VkDynamicState dynamicStateFlags[__P_MAX_DYNAMIC_STATE_COUNT];
    uint32_t dynamicStateFlagsCount = _setDynamicState(dynamicStateFlags);

    __TempState<VkPipelineDynamicStateCreateInfo> dynamicStateDesc(&_descriptor.pDynamicState);
    if (dynamicStateFlagsCount) {
      dynamicStateDesc.value.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      dynamicStateDesc.value.dynamicStateCount = dynamicStateFlagsCount;
      dynamicStateDesc.value.pDynamicStates = dynamicStateFlags;
      dynamicStateDesc.bind();
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
    return GraphicsPipeline(this->_descriptor, this->_renderer, parentCache);
  }



  /*// -- GraphicsPipeline::Builder - creation -- --------------------------------

  // Build a graphics pipeline (based on current params)
  GraphicsPipeline GraphicsPipeline::Builder::create(std::shared_ptr<Renderer> renderer,
                                                     GraphicsPipeline::RenderTargetDescription renderTargets[], size_t renderTargetCount) {
    if (renderer == nullptr || renderTargetCount == 0 || renderTargets == nullptr)
      throw std::invalid_argument("GraphicsPipeline: renderer or renderTargets is NULL");

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

    // create graphics pipeline
    return GraphicsPipeline(renderer, this->_graphicsPipeline, this->_parentCache);
  }*/

#endif
