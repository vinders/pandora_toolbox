/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- renderer state factory - rasterizer -- -----------------------------------

  RasterizerParams::RasterizerParams() noexcept {
    ZeroMemory(&_params, sizeof(D3D11_RASTERIZER_DESC));
    _params.CullMode = D3D11_CULL_BACK;
    _params.FillMode = D3D11_FILL_SOLID;
    _params.DepthClipEnable = TRUE;
  }
  
  RasterizerParams::RasterizerParams(CullMode cull, FillMode fill, bool isFrontClockwise, bool depthClipping, bool scissorClipping) noexcept {
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

  // Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
  RasterizerState RendererStateFactory::createRasterizerState(const RasterizerParams& params) { // throws
    ID3D11RasterizerState* stateData = nullptr;
    auto result = this->_device->CreateRasterizerState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: raster state error");
    return RasterizerState(stateData);
  }


// -- renderer state factory - depth/stencil -- --------------------------------

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

  // Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createDepthStencilTestState(const DepthStencilParams& params) { // throws
    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: depth state error");
    return DepthStencilState(stateData);
  }
  

// -- renderer state factory - blending -- -------------------------------------

  BlendParams::BlendParams(BlendFactor srcColorFactor, BlendFactor destColorFactor, BlendOp colorBlendOp,
                           BlendFactor srcAlphaFactor, BlendFactor destAlphaFactor, BlendOp alphaBlendOp) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_BLEND_DESC));
    _params.RenderTarget->BlendEnable = TRUE;
    _params.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    _params.RenderTarget->SrcBlend  = (D3D11_BLEND)srcColorFactor;
    _params.RenderTarget->DestBlend = (D3D11_BLEND)destColorFactor;
    _params.RenderTarget->BlendOp = (D3D11_BLEND_OP)colorBlendOp;
    _params.RenderTarget->SrcBlendAlpha  = (D3D11_BLEND)srcAlphaFactor;
    _params.RenderTarget->DestBlendAlpha = (D3D11_BLEND)destAlphaFactor;
    _params.RenderTarget->BlendOpAlpha = (D3D11_BLEND_OP)alphaBlendOp;
  }
  
  // ---
  
  // Create general blend state (common to all render-targets)
  BlendState RendererStateFactory::createBlendState(const BlendParams& params) { // throws
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: blend error");
    return BlendState(stateData);
  }

  // Create blend state with different color/alpha params per render-target
  BlendState RendererStateFactory::createBlendStatePerTarget(const BlendTargetParams* perTargetParams, size_t arrayLength) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    descriptor.IndependentBlendEnable = TRUE;
    if (arrayLength > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)
      arrayLength = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    
    auto* last = arrayLength ? &(descriptor.RenderTarget[arrayLength - 1]) : nullptr;
    for (auto* it = &(descriptor.RenderTarget[0]); it <= last; ++it, ++perTargetParams) {
      it->BlendEnable = perTargetParams->isEnabled;
      it->RenderTargetWriteMask = (UINT8)perTargetParams->targetWriteMask;
      
      it->SrcBlend  = (D3D11_BLEND)perTargetParams->srcColorFactor;
      it->DestBlend = (D3D11_BLEND)perTargetParams->destColorFactor;
      it->BlendOp = (D3D11_BLEND_OP)perTargetParams->colorBlendOp;
      it->SrcBlendAlpha  = (D3D11_BLEND)perTargetParams->srcAlphaFactor;
      it->DestBlendAlpha = (D3D11_BLEND)perTargetParams->destAlphaFactor;
      it->BlendOpAlpha = (D3D11_BLEND_OP)perTargetParams->alphaBlendOp;
    }
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: per-target blend error");
    return BlendState(stateData);
  }


// -- renderer state factory - sampler -- --------------------------------------

  FilterParams::FilterParams() noexcept {
    ZeroMemory(&_params, sizeof(D3D11_SAMPLER_DESC));
    _params.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    _params.AddressU = _params.AddressV = _params.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    _params.MaxAnisotropy = 1;
    _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    _params.MinLOD = 1.f;
  }
  
  void FilterParams::_init(const TextureWrap textureWrapUVW[3], float lodMin, float lodMax) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_SAMPLER_DESC));
    textureWrap(textureWrapUVW);
    _params.MinLOD = lodMin;
    _params.MaxLOD = lodMax;
  }
  
  FilterParams& FilterParams::borderColor(const ColorChannel rgba[4]) noexcept {
    if (rgba)
      memcpy(&_params.BorderColor[0], &rgba[0], 4u*sizeof(ColorChannel));
    else
      memset(&_params.BorderColor[0], 0, 4u*sizeof(ColorChannel));
    return *this;
  }

  // ---

  // Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createFilter(const FilterParams& params) {
    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler error");
    return FilterState(stateData);
  }

#endif
