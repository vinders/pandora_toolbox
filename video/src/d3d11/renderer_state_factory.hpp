/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- renderer state factory - depth/stencil -- --------------------------------

  // Convert portable depth/stencil comparison enum to Direct3D comparison enum
  static D3D11_COMPARISON_FUNC __toDepthComparison(DepthComparison compare) noexcept {
    switch (compare) {
      case DepthComparison::never:        return D3D11_COMPARISON_NEVER;
      case DepthComparison::less:         return D3D11_COMPARISON_LESS;
      case DepthComparison::lessEqual:    return D3D11_COMPARISON_LESS_EQUAL;
      case DepthComparison::equal:        return D3D11_COMPARISON_EQUAL;
      case DepthComparison::notEqual:     return D3D11_COMPARISON_NOT_EQUAL;
      case DepthComparison::greaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
      case DepthComparison::greater:      return D3D11_COMPARISON_GREATER;
      case DepthComparison::always:       return D3D11_COMPARISON_ALWAYS;
      default: return D3D11_COMPARISON_ALWAYS;
    }
  }
  // Convert portable depth/stencil operation enum to Direct3D operation enum
  static D3D11_STENCIL_OP __toDepthStencilOperation(DepthStencilOperation op) noexcept {
    switch (op) {
      case DepthStencilOperation::keep:           return D3D11_STENCIL_OP_KEEP;
      case DepthStencilOperation::setZero:        return D3D11_STENCIL_OP_ZERO;
      case DepthStencilOperation::replace:        return D3D11_STENCIL_OP_REPLACE;
      case DepthStencilOperation::invert:         return D3D11_STENCIL_OP_INVERT;
      case DepthStencilOperation::incrementClamp: return D3D11_STENCIL_OP_INCR_SAT;
      case DepthStencilOperation::decrementClamp: return D3D11_STENCIL_OP_DECR_SAT;
      case DepthStencilOperation::incrementWrap:  return D3D11_STENCIL_OP_INCR;
      case DepthStencilOperation::decrementWrap:  return D3D11_STENCIL_OP_DECR;
      default: return D3D11_STENCIL_OP_KEEP;
    }
  }
  
  // ---

  // Create depth test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createDepthTestState(const DepthOperationGroup& frontFaceOp, 
                                                   const DepthOperationGroup& backFaceOp,
                                                   DepthComparison depthTest, bool writeMaskAll) { // throws
    D3D11_DEPTH_STENCIL_DESC depthStDescriptor;
    ZeroMemory(&depthStDescriptor, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // depth test
    depthStDescriptor.DepthEnable = true;
    depthStDescriptor.DepthWriteMask = writeMaskAll ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStDescriptor.DepthFunc = __toDepthComparison(depthTest);
    // stencil test
    depthStDescriptor.StencilEnable = false;
    depthStDescriptor.StencilReadMask = 0xFF;
    depthStDescriptor.StencilWriteMask = 0xFF;
    // front-facing/back-facing operations
    depthStDescriptor.FrontFace.StencilFailOp = depthStDescriptor.FrontFace.StencilDepthFailOp = __toDepthStencilOperation(frontFaceOp.failureOp);
    depthStDescriptor.FrontFace.StencilPassOp = __toDepthStencilOperation(frontFaceOp.passOp);
    depthStDescriptor.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStDescriptor.BackFace.StencilFailOp = depthStDescriptor.BackFace.StencilDepthFailOp = __toDepthStencilOperation(backFaceOp.failureOp);
    depthStDescriptor.BackFace.StencilPassOp = __toDepthStencilOperation(backFaceOp.passOp);
    depthStDescriptor.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&depthStDescriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: depth state failure");
    return DepthStencilState(stateData);
  }
  
  // Create stencil test state (disable depth test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createStencilTestState(const DepthStencilOperationGroup& frontFaceOp, 
                                                     const DepthStencilOperationGroup& backFaceOp, 
                                                     uint8_t readMask, uint8_t writeMask) { // throws
    D3D11_DEPTH_STENCIL_DESC depthStDescriptor;
    ZeroMemory(&depthStDescriptor, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // depth test
    depthStDescriptor.DepthEnable = false;
    depthStDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStDescriptor.DepthFunc = D3D11_COMPARISON_ALWAYS;
    // stencil test
    depthStDescriptor.StencilEnable = true;
    depthStDescriptor.StencilReadMask = (UINT8)readMask;
    depthStDescriptor.StencilWriteMask = (UINT8)writeMask;
    // front-facing/back-facing operations
    depthStDescriptor.FrontFace.StencilFailOp = __toDepthStencilOperation(frontFaceOp.failureOp);
    depthStDescriptor.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStDescriptor.FrontFace.StencilPassOp = __toDepthStencilOperation(frontFaceOp.passOp);
    depthStDescriptor.FrontFace.StencilFunc = __toDepthComparison(frontFaceOp.stencilTest);
    depthStDescriptor.BackFace.StencilFailOp = __toDepthStencilOperation(backFaceOp.failureOp);
    depthStDescriptor.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStDescriptor.BackFace.StencilPassOp = __toDepthStencilOperation(backFaceOp.passOp);
    depthStDescriptor.BackFace.StencilFunc = __toDepthComparison(backFaceOp.stencilTest);

    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&depthStDescriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: stencil state failure");
    return DepthStencilState(stateData);
  }
  
  // Create depth/stencil test state (disable stencil test) - can be used to set depth/stencil test mode when needed (setDepthStencilState)
  DepthStencilState RendererStateFactory::createDepthStencilTestState(const DepthStencilOperationGroup& frontFaceOp, 
                                                          const DepthStencilOperationGroup& backFaceOp, 
                                                          DepthComparison depthTest, bool depthWriteMaskAll, 
                                                          uint8_t stencilReadMask, uint8_t stencilWriteMask) { // throws
    D3D11_DEPTH_STENCIL_DESC depthStDescriptor;
    ZeroMemory(&depthStDescriptor, sizeof(D3D11_DEPTH_STENCIL_DESC));
    
    // depth test
    depthStDescriptor.DepthEnable = true;
    depthStDescriptor.DepthWriteMask = depthWriteMaskAll ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStDescriptor.DepthFunc = __toDepthComparison(depthTest);
    // stencil test
    depthStDescriptor.StencilEnable = true;
    depthStDescriptor.StencilReadMask = (UINT8)stencilReadMask;
    depthStDescriptor.StencilWriteMask = (UINT8)stencilWriteMask;
    // front-facing/back-facing operations
    depthStDescriptor.FrontFace.StencilFailOp = __toDepthStencilOperation(frontFaceOp.failureOp);
    depthStDescriptor.FrontFace.StencilDepthFailOp = __toDepthStencilOperation(frontFaceOp.depthFailureOp);
    depthStDescriptor.FrontFace.StencilPassOp = __toDepthStencilOperation(frontFaceOp.passOp);
    depthStDescriptor.FrontFace.StencilFunc = __toDepthComparison(frontFaceOp.stencilTest);
    depthStDescriptor.BackFace.StencilFailOp = __toDepthStencilOperation(backFaceOp.failureOp);
    depthStDescriptor.BackFace.StencilDepthFailOp = __toDepthStencilOperation(backFaceOp.depthFailureOp);
    depthStDescriptor.BackFace.StencilPassOp = __toDepthStencilOperation(backFaceOp.passOp);
    depthStDescriptor.BackFace.StencilFunc = __toDepthComparison(backFaceOp.stencilTest);

    ID3D11DepthStencilState* stateData;
    auto result = this->_device->CreateDepthStencilState(&depthStDescriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: depth/stencil failure");
    return DepthStencilState(stateData);
  }


// -- renderer state factory - rasterizer -- -----------------------------------

  // Create rasterizer mode state - can be used to change rasterizer state when needed (setRasterizerState)
  RasterizerState RendererStateFactory::createRasterizerState(CullMode culling, bool isFrontClockwise, 
                                                  const pandora::video::DepthBias& depth,
                                                  bool scissorClipping) { // throws
    D3D11_RASTERIZER_DESC rasterizerState;
    ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
    
    switch (culling) {
      case CullMode::wireFrame: rasterizerState.FillMode = D3D11_FILL_WIREFRAME; rasterizerState.CullMode = D3D11_CULL_NONE; break;
      case CullMode::cullBack:  rasterizerState.FillMode = D3D11_FILL_SOLID; rasterizerState.CullMode = D3D11_CULL_BACK; break;
      case CullMode::cullFront: rasterizerState.FillMode = D3D11_FILL_SOLID; rasterizerState.CullMode = D3D11_CULL_FRONT; break;
      default: rasterizerState.FillMode = D3D11_FILL_SOLID; rasterizerState.CullMode = D3D11_CULL_NONE; break;
    }
    rasterizerState.FrontCounterClockwise = isFrontClockwise ? FALSE : TRUE;
    
    rasterizerState.DepthBias = (INT)depth.depthBias;
    rasterizerState.DepthBiasClamp = (FLOAT)depth.depthBiasClamp;
    rasterizerState.SlopeScaledDepthBias = (FLOAT)depth.depthBiasSlopeScale;
    rasterizerState.DepthClipEnable = depth.isClipped ? TRUE : FALSE;

    rasterizerState.MultisampleEnable = FALSE;
    rasterizerState.AntialiasedLineEnable = FALSE;
    rasterizerState.ScissorEnable = scissorClipping ? TRUE : FALSE;

    ID3D11RasterizerState* stateData = nullptr;
    auto result = this->_device->CreateRasterizerState(&rasterizerState, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: raster state failure");
    return RasterizerState(stateData);
  }


// -- renderer state factory - blending -- -------------------------------------

  // Convert portable grouped blend factors to Direct3D blend factors
  static void __toBlendFactor(BlendFactor factor, D3D11_BLEND& outColorBlend, D3D11_BLEND& outAlphaBlend) noexcept {
    switch (factor) {
      case BlendFactor::zero:           outColorBlend = outAlphaBlend = D3D11_BLEND_ZERO; break;
      case BlendFactor::one:            outColorBlend = outAlphaBlend = D3D11_BLEND_ONE; break;
      case BlendFactor::sourceColor:     outColorBlend = D3D11_BLEND_SRC_COLOR;      outAlphaBlend = D3D11_BLEND_SRC_ALPHA; break;
      case BlendFactor::sourceInvColor:  outColorBlend = D3D11_BLEND_INV_SRC_COLOR;  outAlphaBlend = D3D11_BLEND_INV_SRC_ALPHA; break;
      case BlendFactor::destColor:       outColorBlend = D3D11_BLEND_DEST_COLOR;     outAlphaBlend = D3D11_BLEND_DEST_ALPHA; break;
      case BlendFactor::destInvColor:    outColorBlend = D3D11_BLEND_INV_DEST_COLOR; outAlphaBlend = D3D11_BLEND_INV_DEST_ALPHA; break;
      case BlendFactor::dualSrcColor:    outColorBlend = D3D11_BLEND_SRC1_COLOR;     outAlphaBlend = D3D11_BLEND_SRC1_ALPHA; break;
      case BlendFactor::dualSrcInvColor: outColorBlend = D3D11_BLEND_INV_SRC1_COLOR; outAlphaBlend = D3D11_BLEND_INV_SRC1_ALPHA; break;
      case BlendFactor::sourceAlpha:    outColorBlend = outAlphaBlend = D3D11_BLEND_SRC_ALPHA; break;
      case BlendFactor::sourceInvAlpha: outColorBlend = outAlphaBlend = D3D11_BLEND_INV_SRC_ALPHA; break;
      case BlendFactor::destAlpha:      outColorBlend = outAlphaBlend = D3D11_BLEND_DEST_ALPHA; break;
      case BlendFactor::destInvAlpha:   outColorBlend = outAlphaBlend = D3D11_BLEND_INV_DEST_ALPHA; break;
      case BlendFactor::sourceAlphaSat: outColorBlend = outAlphaBlend = D3D11_BLEND_SRC_ALPHA_SAT; break;
      case BlendFactor::dualSrcAlpha:    outColorBlend = outAlphaBlend = D3D11_BLEND_SRC1_ALPHA; break;
      case BlendFactor::dualSrcInvAlpha: outColorBlend = outAlphaBlend = D3D11_BLEND_INV_SRC1_ALPHA; break;
      case BlendFactor::constantColor:    outColorBlend = outAlphaBlend = D3D11_BLEND_BLEND_FACTOR; break;
      case BlendFactor::constantInvColor: outColorBlend = outAlphaBlend = D3D11_BLEND_INV_BLEND_FACTOR; break;
      default: outColorBlend = outAlphaBlend = D3D11_BLEND_ZERO; break;
    }
  }
  // Convert portable separate blend factors to Direct3D blend factors
  static D3D11_BLEND __toBlendFactor(BlendFactor factor, bool isAlpha) noexcept {
    switch (factor) {
      case BlendFactor::zero:           return D3D11_BLEND_ZERO;
      case BlendFactor::one:            return D3D11_BLEND_ONE;
      case BlendFactor::sourceColor:    return isAlpha ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_SRC_COLOR;
      case BlendFactor::sourceInvColor: return isAlpha ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_INV_SRC_COLOR;
      case BlendFactor::sourceAlpha:    return D3D11_BLEND_SRC_ALPHA;
      case BlendFactor::sourceInvAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
      case BlendFactor::destColor:      return isAlpha ? D3D11_BLEND_DEST_ALPHA : D3D11_BLEND_DEST_COLOR;
      case BlendFactor::destInvColor:   return isAlpha ? D3D11_BLEND_INV_DEST_ALPHA : D3D11_BLEND_INV_DEST_COLOR;
      case BlendFactor::destAlpha:      return D3D11_BLEND_DEST_ALPHA;
      case BlendFactor::destInvAlpha:   return D3D11_BLEND_INV_DEST_ALPHA;
      case BlendFactor::sourceAlphaSat: return D3D11_BLEND_SRC_ALPHA_SAT;
      case BlendFactor::dualSrcColor:    return isAlpha ? D3D11_BLEND_SRC1_ALPHA : D3D11_BLEND_SRC1_COLOR;
      case BlendFactor::dualSrcInvColor: return isAlpha ? D3D11_BLEND_INV_SRC1_ALPHA : D3D11_BLEND_INV_SRC1_COLOR;
      case BlendFactor::dualSrcAlpha:    return D3D11_BLEND_SRC1_ALPHA;
      case BlendFactor::dualSrcInvAlpha: return D3D11_BLEND_INV_SRC1_ALPHA;
      case BlendFactor::constantColor:    return D3D11_BLEND_BLEND_FACTOR;
      case BlendFactor::constantInvColor: return D3D11_BLEND_INV_BLEND_FACTOR;
      default: return D3D11_BLEND_ZERO;
    }
  }
  // Convert portable blend operators to Direct3D blend operators
  static D3D11_BLEND_OP __toBlendOperator(BlendOperator op) noexcept {
    switch (op) {
      case BlendOperator::add:         return D3D11_BLEND_OP_ADD;
      case BlendOperator::subtract:    return D3D11_BLEND_OP_SUBTRACT;
      case BlendOperator::revSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
      case BlendOperator::minimum:     return D3D11_BLEND_OP_MIN;
      case BlendOperator::maximum:     return D3D11_BLEND_OP_MAX;
      default: return D3D11_BLEND_OP_ADD;
    }
  }
  
  // Fill grouped blend state params
  static inline void __fillBlendStateParams(BlendFactor sourceFactor, BlendFactor destFactor, 
                                            BlendOperator op, D3D11_RENDER_TARGET_BLEND_DESC& outDescriptor) noexcept {
    outDescriptor.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (op != BlendOperator::none) {
      outDescriptor.BlendEnable = TRUE;
      __toBlendFactor(sourceFactor, outDescriptor.SrcBlend, outDescriptor.SrcBlendAlpha);
      __toBlendFactor(destFactor, outDescriptor.DestBlend, outDescriptor.DestBlendAlpha);
      outDescriptor.BlendOp = outDescriptor.BlendOpAlpha = __toBlendOperator(op);
    }
    else {
      outDescriptor.BlendEnable = FALSE;
      outDescriptor.SrcBlend = outDescriptor.SrcBlendAlpha = D3D11_BLEND_ONE;
      outDescriptor.DestBlend = outDescriptor.DestBlendAlpha = D3D11_BLEND_ZERO;
      outDescriptor.BlendOp = outDescriptor.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }
  }
  // Fill separate blend state params
  static inline void __fillBlendStateParams(BlendFactor sourceColorFactor, BlendFactor destColorFactor, BlendOperator colorOp, 
                                            BlendFactor sourceAlphaFactor, BlendFactor destAlphaFactor, BlendOperator alphaOp, 
                                            D3D11_RENDER_TARGET_BLEND_DESC& outDescriptor) noexcept {
    outDescriptor.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (colorOp != BlendOperator::none && alphaOp != BlendOperator::none) {
      outDescriptor.BlendEnable = TRUE;
      outDescriptor.SrcBlend = __toBlendFactor(sourceColorFactor, false);
      outDescriptor.SrcBlendAlpha = __toBlendFactor(sourceAlphaFactor, true);
      outDescriptor.DestBlend = __toBlendFactor(destColorFactor, false);
      outDescriptor.DestBlendAlpha = __toBlendFactor(destAlphaFactor, true);
      outDescriptor.BlendOp = __toBlendOperator(colorOp);
      outDescriptor.BlendOpAlpha = __toBlendOperator(alphaOp);
    }
    else {
      outDescriptor.BlendEnable = FALSE;
      outDescriptor.SrcBlend = outDescriptor.SrcBlendAlpha = D3D11_BLEND_ONE;
      outDescriptor.DestBlend = outDescriptor.DestBlendAlpha = D3D11_BLEND_ZERO;
      outDescriptor.BlendOp = outDescriptor.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }
  }
  
  // ---

  // Create general blend state (common to all render-targets)
  BlendState RendererStateFactory::createBlendState(BlendFactor sourceFactor, BlendFactor destFactor, BlendOperator op) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    __fillBlendStateParams(sourceFactor, destFactor, op, descriptor.RenderTarget[0]);
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: blend state failure");
    return BlendState(stateData);
  }
  // Create general blend state (common to all render-targets) - separate color/alpha params
  BlendState RendererStateFactory::createBlendState(BlendFactor sourceColorFactor, BlendFactor destColorFactor, BlendOperator colorOp, 
                                                    BlendFactor sourceAlphaFactor, BlendFactor destAlphaFactor, BlendOperator alphaOp) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    __fillBlendStateParams(sourceColorFactor, destColorFactor, colorOp, sourceAlphaFactor, destAlphaFactor, alphaOp, descriptor.RenderTarget[0]);
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: split blend failure");
    return BlendState(stateData);
  }

  // Create blend state with different grouped params per render-target (up to 'Renderer::maxRenderTargets()' targets (usually 8))
  BlendState RendererStateFactory::createBlendStatePerTarget(const TargetBlendingParams* perTargetParams, size_t arrayLength) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    descriptor.IndependentBlendEnable = TRUE;
    for (size_t i = 0; i < arrayLength; ++i, ++perTargetParams) {
      __fillBlendStateParams(perTargetParams->sourceFactor, perTargetParams->destFactor, perTargetParams->op, descriptor.RenderTarget[i]);
    }
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: per-target blend failure");
    return BlendState(stateData);
  }
  // Create blend state with different color/alpha params per render-target (up to 'Renderer::maxRenderTargets()' targets (usually 8))
  BlendState RendererStateFactory::createBlendStatePerTarget(const TargetBlendingSplitParams* perTargetParams, size_t arrayLength) { // throws
    D3D11_BLEND_DESC descriptor{};
    ZeroMemory(&descriptor, sizeof(D3D11_BLEND_DESC));
    descriptor.IndependentBlendEnable = TRUE;
    for (size_t i = 0; i < arrayLength; ++i, ++perTargetParams) {
      __fillBlendStateParams(perTargetParams->sourceColorFactor, perTargetParams->destColorFactor, perTargetParams->colorOp, 
                             perTargetParams->sourceAlphaFactor, perTargetParams->destAlphaFactor, perTargetParams->alphaOp, 
                             descriptor.RenderTarget[i]);
    }
    
    ID3D11BlendState* stateData = nullptr;
    HRESULT result = this->_device->CreateBlendState(&descriptor, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: per-target split blend failure");
    return BlendState(stateData);
  }


// -- renderer state factory - sampler -- --------------------------------------
  
  // Convert portable filter types to Direct3D filter type
  static D3D11_FILTER __toFilterType(MinificationFilter minFilter, MagnificationFilter magFilter) noexcept {
    bool isMagLinear = (magFilter == MagnificationFilter::linear);
    switch (minFilter) {
      case MinificationFilter::nearest_mipNearest:
      case MinificationFilter::nearest: return isMagLinear ? D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
      case MinificationFilter::linear_mipNearest:
      case MinificationFilter::linear:  return isMagLinear ? D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
      case MinificationFilter::nearest_mipLinear: return isMagLinear ? D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
      case MinificationFilter::linear_mipLinear:  return isMagLinear ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
      default: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    }
  }
  // Convert portable filter types to Direct3D filter type with comparison
  static D3D11_FILTER __toFilterComparedType(MinificationFilter minFilter, MagnificationFilter magFilter) noexcept {
    bool isMagLinear = (magFilter == MagnificationFilter::linear);
    switch (minFilter) {
      case MinificationFilter::nearest_mipNearest:
      case MinificationFilter::nearest:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
      case MinificationFilter::linear_mipNearest:
      case MinificationFilter::linear:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
      case MinificationFilter::nearest_mipLinear:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
      case MinificationFilter::linear_mipLinear:
        return isMagLinear ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
      default: return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    }
  }
  // Convert portable texture-addressing to Direct3D addressing enum
  static D3D11_TEXTURE_ADDRESS_MODE __toFilterTextureAddress(TextureAddressMode mode) noexcept {
    switch (mode) {
      case TextureAddressMode::border: return D3D11_TEXTURE_ADDRESS_BORDER;
      case TextureAddressMode::clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
      case TextureAddressMode::repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
      case TextureAddressMode::repeatMirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
      case TextureAddressMode::mirrorClamp:  return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
      default: return D3D11_TEXTURE_ADDRESS_WRAP;
    }
  }
  
  // Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createFilter(MinificationFilter minFilter, MagnificationFilter magFilter, 
                                                 const TextureAddressMode texAddressUVW[3], float lodMin, 
                                                 float lodMax, float lodBias, const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = __toFilterType(minFilter, magFilter);
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler failure");
    return FilterState(stateData);
  }
  // Create sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createComparedFilter(MinificationFilter minFilter, MagnificationFilter magFilter,
                                                         const TextureAddressMode texAddressUVW[3], DepthComparison compare, 
                                                         float lodMin, float lodMax, float lodBias, const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = __toFilterComparedType(minFilter, magFilter);
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = __toDepthComparison(compare);
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler-comp failure");
    return FilterState(stateData);
  }
  
  // Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createAnisotropicFilter(uint32_t maxAnisotropy, const TextureAddressMode texAddressUVW[3],
                                                            float lodMin, float lodMax, float lodBias, const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = (maxAnisotropy <= (uint32_t)D3D11_MAX_MAXANISOTROPY) ? maxAnisotropy : D3D11_MAX_MAXANISOTROPY;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: anisotropic sampler failure");
    return FilterState(stateData);
  }
  // Create anisotropic sampler filter state - can be used to change sampler filter state when needed (setFilterState)
  FilterState RendererStateFactory::createComparedAnisotropicFilter(uint32_t maxAnisotropy, const TextureAddressMode texAddressUVW[3],
                                                                    DepthComparison compare, float lodMin, float lodMax, float lodBias, 
                                                                    const FLOAT borderColor[4]) {
    D3D11_SAMPLER_DESC samplerDesc{};
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
    samplerDesc.AddressU = __toFilterTextureAddress(texAddressUVW[0]);
    samplerDesc.AddressV = __toFilterTextureAddress(texAddressUVW[1]);
    samplerDesc.AddressW = __toFilterTextureAddress(texAddressUVW[2]);
    samplerDesc.MipLODBias = lodBias;
    samplerDesc.MinLOD = lodMin;
    samplerDesc.MaxLOD = lodMax;
    samplerDesc.MaxAnisotropy = (maxAnisotropy <= (uint32_t)D3D11_MAX_MAXANISOTROPY) ? maxAnisotropy : D3D11_MAX_MAXANISOTROPY;
    samplerDesc.ComparisonFunc = __toDepthComparison(compare);
    memcpy((void*)samplerDesc.BorderColor, (borderColor != nullptr) ? &borderColor[0] : __defaultBlackColor, 4u*sizeof(float));

    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&samplerDesc, &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: anisotropic sampler-comp failure");
    return FilterState(stateData);
  }

#endif
