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
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp

// -- RasterizerParams - rasterizer state params -- ----------------------------

  RasterizerParams::RasterizerParams() noexcept {
    ZeroMemory(&_params, sizeof(D3D11_RASTERIZER_DESC));
    _params.CullMode = D3D11_CULL_BACK;
    _params.FillMode = D3D11_FILL_SOLID;
  }
  
  RasterizerParams::RasterizerParams(CullMode cull, FillMode fill, bool isFrontClockwise,
                                     bool depthClipping, bool scissorClipping) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_RASTERIZER_DESC));
    
    _params.CullMode = (D3D11_CULL_MODE)cull;
    if (fill == FillMode::linesAA) {
      _params.FillMode = D3D11_FILL_WIREFRAME;
      _params.AntialiasedLineEnable = TRUE;
    }
    else
      _params.FillMode = (D3D11_FILL_MODE)fill;
    
    _params.FrontCounterClockwise = isFrontClockwise ? FALSE : TRUE;
    _params.DepthClipEnable = depthClipping ? TRUE : FALSE;
    _params.ScissorEnable = scissorClipping ? TRUE : FALSE;
  }
  
  // ---

  // Compute resource ID based on params -- Reserved for internal use
  RasterizerStateId RasterizerParams::computeId() const noexcept {
    SharedResourceId<4> id;
    id.id()[0] = (((uint32_t)_params.CullMode & 0xFFu) | (((uint32_t)_params.FillMode & 0xFFu) << 8)
                | ((uint32_t)_params.FrontCounterClockwise << 16)
                | ((uint32_t)_params.DepthClipEnable << 17)
                | ((uint32_t)_params.ScissorEnable << 18)
                | ((uint32_t)_params.MultisampleEnable << 19)
                | ((uint32_t)_params.AntialiasedLineEnable << 20) );
    id.id()[1] = static_cast<uint32_t>(_params.DepthBias);
    id.id()[2] = *reinterpret_cast<const uint32_t*>(&_params.DepthBiasClamp);
    id.id()[3] = *reinterpret_cast<const uint32_t*>(&_params.SlopeScaledDepthBias);
    return id;
  }

// -- DepthStencilParams - depth/stencil testing state params -- ---------------

  void DepthStencilParams::_init(BOOL enableDepth, BOOL enableStencil, D3D11_COMPARISON_FUNC depthComp, 
    D3D11_COMPARISON_FUNC frontFaceComp, D3D11_COMPARISON_FUNC backFaceComp) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_DEPTH_STENCIL_DESC));
    _params.DepthEnable = enableDepth;
    _params.DepthFunc = depthComp;
    _params.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

    _params.StencilEnable = enableStencil;
    _params.FrontFace.StencilFunc = frontFaceComp;
    _params.BackFace.StencilFunc = backFaceComp;
    _params.StencilReadMask = _params.StencilWriteMask = 0xFF;
  }

  // ---

  // Compute resource ID based on params -- Reserved for internal use
  DepthStencilStateId DepthStencilParams::computeId() const noexcept {
    SharedResourceId<2> id;
    id.id()[0] = (((uint32_t)_params.DepthFunc & 0xFu) | (((uint32_t)_params.DepthWriteMask & 0x3u) << 4)
                | ((uint32_t)_params.DepthEnable << 6) | ((uint32_t)_params.StencilEnable << 7)
                | (((uint32_t)_params.StencilReadMask & 0xFFu) << 8)
                | (((uint32_t)_params.StencilWriteMask & 0xFFu) << 16) );
    id.id()[1] = ( ((uint32_t)_params.FrontFace.StencilFailOp & 0xFu)
                | (((uint32_t)_params.FrontFace.StencilDepthFailOp & 0xFu) << 4)
                | (((uint32_t)_params.FrontFace.StencilPassOp & 0xFu) << 8)
                | (((uint32_t)_params.FrontFace.StencilFunc & 0xFu) << 12)
                | (((uint32_t)_params.BackFace.StencilFailOp & 0xFu) << 16)
                | (((uint32_t)_params.BackFace.StencilDepthFailOp & 0xFu) << 20)
                | (((uint32_t)_params.BackFace.StencilPassOp & 0xFu) << 24)
                | (((uint32_t)_params.BackFace.StencilFunc & 0xFu) << 28) );
    return id;
  }

// -- BlendParams - blend state params -- --------------------------------------

  BlendParams::BlendParams(BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                           BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                           ColorComponentFlag mask) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_BLEND_DESC));
    _params.RenderTarget->BlendEnable = TRUE;
    _params.RenderTarget->RenderTargetWriteMask = (UINT8)mask;
    
    _params.RenderTarget->SrcBlend  = (D3D11_BLEND)srcColorFactor;
    _params.RenderTarget->DestBlend = (D3D11_BLEND)destColorFactor;
    _params.RenderTarget->BlendOp = (D3D11_BLEND_OP)colorBlendOp;
    _params.RenderTarget->SrcBlendAlpha  = (D3D11_BLEND)srcAlphaFactor;
    _params.RenderTarget->DestBlendAlpha = (D3D11_BLEND)destAlphaFactor;
    _params.RenderTarget->BlendOpAlpha = (D3D11_BLEND_OP)alphaBlendOp;
  }

  BlendParams& BlendParams::blendConstant(const ColorChannel constantColorRgba[4]) noexcept {
    memcpy(this->_blendConstant, constantColorRgba, sizeof(ColorChannel)*4u);
    return *this;
  }

  // ---

  BlendPerTargetParams::BlendPerTargetParams() noexcept {
    ZeroMemory(&_params, sizeof(D3D11_BLEND_DESC));
    _params.IndependentBlendEnable = TRUE;
  }

  BlendPerTargetParams& BlendPerTargetParams::setTargetBlend(uint32_t targetIndex,
                                              BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                                              BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp,
                                              ColorComponentFlag mask) noexcept {
    assert(targetIndex < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
    auto* target = &(_params.RenderTarget[targetIndex]);
    target->BlendEnable = TRUE;
    target->RenderTargetWriteMask = (UINT8)mask;
    target->SrcBlend  = (D3D11_BLEND)srcColorFactor;
    target->DestBlend = (D3D11_BLEND)destColorFactor;
    target->BlendOp = (D3D11_BLEND_OP)colorBlendOp;
    target->SrcBlendAlpha  = (D3D11_BLEND)srcAlphaFactor;
    target->DestBlendAlpha = (D3D11_BLEND)destAlphaFactor;
    target->BlendOpAlpha = (D3D11_BLEND_OP)alphaBlendOp;
    return *this;
  }

  BlendPerTargetParams& BlendPerTargetParams::blendConstant(const ColorChannel constantColorRgba[4]) noexcept {
    memcpy(this->_blendConstant, constantColorRgba, sizeof(ColorChannel)*4u);
    return *this;
  }

  // ---

  // Compute single-target resource ID based on params -- Reserved for internal use
  BlendStateId BlendParams::computeId() const noexcept {
    SharedResourceId<2> id;
    if (_params.RenderTarget->BlendEnable) {
      id.id()[0] = (((uint32_t)_params.RenderTarget->SrcBlend & 0x1Fu)
                 | (((uint32_t)_params.RenderTarget->DestBlend & 0x1Fu) << 5)
                 | (((uint32_t)_params.RenderTarget->SrcBlendAlpha & 0x1Fu) << 10)
                 | (((uint32_t)_params.RenderTarget->DestBlendAlpha & 0x1Fu) << 15)
                 | (((uint32_t)_params.RenderTarget->BlendOp & 0xFu) << 20)
                 | (((uint32_t)_params.RenderTarget->BlendOpAlpha & 0x7u) << 24) 
                 | (((uint32_t)_params.RenderTarget->RenderTargetWriteMask & 0xFu) << 28)
                 | ((uint32_t)_params.AlphaToCoverageEnable << 27) );
      id.id()[1] = (static_cast<uint32_t>(_blendConstant[0]*128.f)
                 ^ (static_cast<uint32_t>(_blendConstant[1]*128.f) << 8)
                 ^ (static_cast<uint32_t>(_blendConstant[2]*128.f) << 16)
                 ^ (static_cast<uint32_t>(_blendConstant[3]*128.f) << 24) );
    }
    return id;
  }

  // Compute multi-target resource ID based on params -- Reserved for internal use
  BlendStatePerTargetId BlendPerTargetParams::computeId() const noexcept {
    SharedResourceId<D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT+1> id;
    constexpr const size_t lastTarget = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT - 1;

    const D3D11_RENDER_TARGET_BLEND_DESC* srcIt = &_params.RenderTarget[lastTarget];
    for (uint32_t* destIt = &(id.id()[lastTarget]); srcIt >= _params.RenderTarget; --srcIt, --destIt) {
      if (srcIt->BlendEnable) {
        *destIt = (((uint32_t)srcIt->SrcBlend & 0x1Fu)             | (((uint32_t)srcIt->DestBlend & 0x1Fu) << 5)
                | (((uint32_t)srcIt->SrcBlendAlpha & 0x1Fu) << 10) | (((uint32_t)srcIt->DestBlendAlpha & 0x1Fu) << 15)
                | (((uint32_t)srcIt->BlendOp & 0xFu) << 20)        | (((uint32_t)srcIt->BlendOpAlpha & 0x7u) << 24) 
                | (((uint32_t)srcIt->RenderTargetWriteMask & 0xFu) << 28) );
      }
    }
    id.id()[0] |= (uint32_t)_params.AlphaToCoverageEnable << 27;
    id.id()[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = (static_cast<uint32_t>(_blendConstant[0]*128.f)
                                                    ^ (static_cast<uint32_t>(_blendConstant[1]*128.f) << 8)
                                                    ^ (static_cast<uint32_t>(_blendConstant[2]*128.f) << 16)
                                                    ^ (static_cast<uint32_t>(_blendConstant[3]*128.f) << 24) );
    return id;
  }


// -----------------------------------------------------------------------------
// graphics pipeline builder
// -----------------------------------------------------------------------------

// -- GraphicsPipeline -- ------------------------------------------------------

  // Create pipeline object -- reserved for internal use or advanced usage
  GraphicsPipeline::GraphicsPipeline(GraphicsPipeline::Builder& builder)
    : _renderer(builder._renderer) {
    assert(builder._renderer != nullptr);
    if (!builder._params.shaderStages[(size_t)ShaderType::vertex].hasValue())
      throw std::logic_error("GraphicsPipeline: vertex shader required");
    if (!builder._params.rasterizerState.hasValue() || !builder._params.depthStencilState.hasValue() || !builder._params.blendState.hasValue())
      throw std::logic_error("GraphicsPipeline: missing required pipeline state");

    this->_pipeline = std::make_shared<_DxPipelineStages>(builder._params); // copy pipeline states + shader stages

    // copy viewport/scissor descriptors
    this->_pipeline->viewports = pandora::memory::LightVector<D3D11_VIEWPORT>(builder._viewportCount);
    if (builder._viewportCount > 0) {
      D3D11_VIEWPORT* lhs = this->_pipeline->viewports.data();
      const Viewport* rhs = builder._viewports;
      for (D3D11_VIEWPORT* lhsEnd = lhs + (intptr_t)builder._viewportCount; lhs < lhsEnd; ++lhs, ++rhs)
        memcpy(lhs, rhs->descriptor(), sizeof(D3D11_VIEWPORT));
    }
    this->_pipeline->scissorTests = pandora::memory::LightVector<D3D11_RECT>(builder._scissorCount);
    if (builder._scissorCount > 0) {
      D3D11_RECT* lhs = this->_pipeline->scissorTests.data();
      const ScissorRectangle* rhs = builder._scissorTests;
      for (D3D11_RECT* lhsEnd = lhs + (intptr_t)builder._scissorCount; lhs < lhsEnd; ++lhs, ++rhs)
        memcpy(lhs, rhs->descriptor(), sizeof(D3D11_RECT));
    }

    // register pipeline states to cache
    this->_renderer->_addRasterizerState(builder._params.rasterizerCacheId, builder._params.rasterizerState);
    this->_renderer->_addDepthStencilState(builder._params.depthStencilCacheId, builder._params.depthStencilState);
    if (builder._params.blendPerTargetCacheId == nullptr)
      this->_renderer->_addBlendState(builder._params.blendCacheId, builder._params.blendState);
    else
      this->_renderer->_addBlendStatePerTarget(*builder._params.blendPerTargetCacheId, builder._params.blendState);
  }

  // Destroy pipeline object
  void GraphicsPipeline::release() noexcept {
    if (this->_pipeline != nullptr) {
      // unregister pipeline states from cache
      this->_renderer->_removeRasterizerState(this->_pipeline->rasterizerCacheId);
      this->_renderer->_removeDepthStencilState(this->_pipeline->depthStencilCacheId);
      if (this->_pipeline->blendPerTargetCacheId == nullptr)
        this->_renderer->_removeBlendState(this->_pipeline->blendCacheId);
      else
        this->_renderer->_removeBlendStatePerTarget(*_pipeline->blendPerTargetCacheId);

      this->_pipeline = nullptr;
    }
  }


// -- GraphicsPipeline.Builder -- ----------------------------------------------

  uint64_t GraphicsPipeline::Builder::_lastViewportScissorId = 0;

  // Create pipeline builder
  GraphicsPipeline::Builder::Builder(std::shared_ptr<Renderer> renderer) noexcept : _renderer(renderer) { // throws
    if (this->_renderer == nullptr)
      throw std::logic_error("GraphicsPipeline.Builder: renderer is NULL");
  }

  // ---

# ifndef __P_DISABLE_TESSELLATION_STAGE
    // Set vertex patch topology for input stage (required for tessellation shaders)
    GraphicsPipeline::Builder& GraphicsPipeline::Builder::setPatchTopology(uint32_t controlPoints) noexcept {
      if (controlPoints != 0u) {
        --controlPoints;
        if (controlPoints >= 32u)
          controlPoints = 31u;
      }
      this->_params.topology = static_cast<VertexTopology>((int)D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (int)controlPoints);
      return *this;
    }
# endif

  // Bind shader stages to pipeline (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setShaderStages(const Shader shaders[], size_t shaderCount) {
    if (shaderCount) {
      for (const Shader* it = &shaders[shaderCount - 1]; it >= shaders; --it) {
        assert((unsigned int)(it->type()) <= __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX);
        if (it->handle() != nullptr)
          ((ID3D11DeviceChild*)it->handle())->AddRef();
        this->_params.shaderStages[(unsigned int)(it->type())] = (ID3D11DeviceChild*)it->handle();
      }
    }
    return *this;
  }
  // Bind (or replace) shader module for a specific stage (at least vertex+fragment or compute required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::attachShaderStage(const Shader& shaderModule) {
    assert((unsigned int)shaderModule.type() <= __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX);
    if (shaderModule.handle() != nullptr)
      ((ID3D11DeviceChild*)shaderModule.handle())->AddRef();
    this->_params.shaderStages[(unsigned int)shaderModule.type()] = (ID3D11DeviceChild*)shaderModule.handle();
    return *this;
  }

  // ---

  // Bind rasterization state (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerState(const RasterizerParams& state) { // throws
    this->_params.rasterizerState.release();
    this->_params.rasterizerCacheId = state.computeId();

    if (!this->_renderer->_findRasterizerState(this->_params.rasterizerCacheId, this->_params.rasterizerState))
      this->_params.rasterizerState = createRasterizerState(state);
    return *this;
  }
  // Bind depth/stencil test state (required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setDepthStencilState(const DepthStencilParams& state) { // throws
    this->_params.depthStencilState.release();
    this->_params.depthStencilCacheId = state.computeId();
    this->_params.stencilRef = state.stencilReference();

    if (!this->_renderer->_findDepthStencilState(this->_params.depthStencilCacheId, this->_params.depthStencilState))
      this->_params.depthStencilState = createDepthStencilState(state);
    return *this;
  }
  // Bind color/alpha blending state -- common to all render-targets (one of the 2 methods required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setBlendState(const BlendParams& state) { // throws
    this->_params.blendState.release();
    this->_params.blendCacheId = state.computeId();
    this->_params.blendPerTargetCacheId = nullptr;
    memcpy(this->_params.blendConstant, state.blendConstant(), sizeof(ColorChannel)*4u);

    if (!this->_renderer->_findBlendState(this->_params.blendCacheId, this->_params.blendState))
      this->_params.blendState = createBlendState(state);
    return *this;
  }
  // Bind color/alpha blending state -- customized per render-target (one of the 2 methods required)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setBlendState(const BlendPerTargetParams& state) { // throws
    this->_params.blendState.release();
    this->_params.blendCacheId = BlendStateId{};
    this->_params.blendPerTargetCacheId = std::make_shared<BlendStatePerTargetId>(state.computeId());
    memcpy(this->_params.blendConstant, state.blendConstant(), sizeof(ColorChannel)*4u);

    if (!this->_renderer->_findBlendStatePerTarget(*_params.blendPerTargetCacheId, this->_params.blendState))
      this->_params.blendState = createBlendState(state);
    return *this;
  }

  // ---

  // Set viewports and scissor-test rectangles (optional: dynamic viewports if ignored)
  GraphicsPipeline::Builder& GraphicsPipeline::Builder::setViewports(const Viewport viewports[], size_t viewportCount,
                                                        const ScissorRectangle scissorTests[], size_t scissorCount,
                                                        bool useDynamicCount) noexcept {
    assert(useDynamicCount || viewportCount == scissorCount);
    this->_viewports = viewports; this->_viewportCount = viewportCount;
    this->_scissorTests = scissorTests; this->_scissorCount = scissorCount;

    if (viewportCount + scissorCount != 0) {
      this->_params.viewportScissorId = ++_lastViewportScissorId;
      if (_lastViewportScissorId == 0)
        ++_lastViewportScissorId;
    }
    else
      this->_params.viewportScissorId = 0;
    return *this;
  }


  // -- Direct3D pipeline state factory --

  // Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
  RasterizerState GraphicsPipeline::Builder::createRasterizerState(const RasterizerParams& params) { // throws
    ID3D11RasterizerState* stateData = nullptr;
    auto result = this->_renderer->device()->CreateRasterizerState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "GraphicsPipeline: raster state error");
    return RasterizerState(stateData);
  }
  // Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState GraphicsPipeline::Builder::createDepthStencilState(const DepthStencilParams& params) { // throws
    ID3D11DepthStencilState* stateData;
    auto result = this->_renderer->device()->CreateDepthStencilState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "GraphicsPipeline: depth state error");
    return DepthStencilState(stateData);
  }

  // Create general blend state (common to all render-targets)
  BlendState GraphicsPipeline::Builder::createBlendState(const BlendParams& params) { // throws
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_renderer->device()->CreateBlendState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "GraphicsPipeline: blend state error");
    return BlendState(stateData);
  }
  // Create blend state with different color/alpha params per render-target
  BlendState GraphicsPipeline::Builder::createBlendState(const BlendPerTargetParams& params) { // throws
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_renderer->device()->CreateBlendState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "GraphicsPipeline: multi-target blend state error");
    return BlendState(stateData);
  }

#endif
