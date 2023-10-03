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

// note: this file depends on '__refreshDxgiFactory(...)' declared in renderer.cpp


// -- color spaces -- ----------------------------------------------------------

# define __P_DEFAULT_COLORSPACE_SRGB  DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709
# define __P_SWAPCHAIN_FORMAT_SUPPORT (D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DISPLAY)

// -- color management --

  // Verify if a buffer format is supported to create swap-chains and render targets
  bool DisplaySurface::isFormatSupported(DataFormat bufferFormat) const noexcept {
    UINT support = 0;
    if (this->_renderer != nullptr
    && SUCCEEDED(this->_renderer->device()->CheckFormatSupport(_getDataFormatComponents(bufferFormat), &support)))
      return ((support & __P_SWAPCHAIN_FORMAT_SUPPORT) == __P_SWAPCHAIN_FORMAT_SUPPORT);

    return (bufferFormat == DataFormat::rgba8_unorm || bufferFormat == DataFormat::rgba8_sRGB);
  }

# if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
    // Find color space for a buffer format
    static __forceinline DXGI_COLOR_SPACE_TYPE __getColorSpace(DXGI_FORMAT backBufferFormat) noexcept {
      switch (backBufferFormat) {
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
          return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020; // HDR-10
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16_FLOAT:
          return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709; // HDR-scRGB
        default:
          return __P_DEFAULT_COLORSPACE_SRGB; // SDR-sRGB
      }
    }
# endif
  
  // Set swap-chain color space
  // returns: color spaces supported (true) or not
  static __forceinline void __setColorSpace(IDXGISwapChain* swapChain, DXGI_FORMAT backBufferFormat) { // throws
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      DXGI_COLOR_SPACE_TYPE colorSpace = __getColorSpace(backBufferFormat);

      auto swapChainV3 = SharedResource<IDXGISwapChain3>::tryFromInterface((IDXGISwapChain*)swapChain);
      if (swapChainV3) {
        UINT colorSpaceSupport = 0;
        if (SUCCEEDED(swapChainV3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
        && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) {

          auto result = swapChainV3->SetColorSpace1(colorSpace);
          if (FAILED(result) && colorSpace != __P_DEFAULT_COLORSPACE_SRGB)
            throwError(result, "SwapChain: color space error");
          return;
        }
      }
#   endif
  }


// -- swap-chain creation -- ---------------------------------------------------

  // Get swap-chain format (based on buffer format + flip-swap request) + detect flip-swap support for format
  // inOutUseFlipSwap: [in] = request flip-swap or not  /  [out] = requested and supported for format
  static inline DXGI_FORMAT __getSwapChainFormat(DXGI_FORMAT backBufferFormat, bool& inOutUseFlipSwap) noexcept {
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      if (inOutUseFlipSwap) {
        switch (backBufferFormat) {
          case DXGI_FORMAT_R16G16B16A16_FLOAT: 
          case DXGI_FORMAT_R8G8B8A8_UNORM:
          case DXGI_FORMAT_B8G8R8A8_UNORM:
          case DXGI_FORMAT_R10G10B10A2_UNORM:   inOutUseFlipSwap = true; return backBufferFormat;
          case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: inOutUseFlipSwap = true; return DXGI_FORMAT_R8G8B8A8_UNORM;
          case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: inOutUseFlipSwap = true; return DXGI_FORMAT_B8G8R8A8_UNORM;
          default: inOutUseFlipSwap = false; break;
        }
      }
#   endif
    return backBufferFormat;
  }
  
  // ---
  
  // Create DXGI swap-chain resource for existing renderer
  void SwapChain::_createSwapChain(pandora::video::WindowHandle window, const SwapChain::RefreshRate& rate) { // throws
    if (this->_renderer == nullptr)
      throw std::invalid_argument("SwapChain: NULL renderer");
    __refreshDxgiFactory(&(_renderer->_dxgiFactory));

    // build swap-chain
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      if (this->_renderer->_dxgiLevel >= 2u) { // Direct3D 11.1+
        bool useFlipSwap = ( (this->_flags & (SwapChain::OutputFlag::disableFlipSwap)) == false // * flip-swap not disabled + unique render-target view
                          && this->_renderer->_isFlipSwapAvailable()                            // * supported by device/API/system
                          && ((this->_flags & SwapChain::OutputFlag::variableRefresh) == false  // * no tearing or support with flip-swap
                            || this->_renderer->isTearingAvailable()) );

        DXGI_SWAP_CHAIN_DESC1 descriptor = {};
        ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC1));
        descriptor.Width = _width();
        descriptor.Height = _height();
        descriptor.Format = __getSwapChainFormat(this->_backBufferFormat, useFlipSwap); // * verify color format supported with flip-swap (or unset useFlipSwap)
        descriptor.BufferCount = (UINT)this->_framebufferCount;
        descriptor.BufferUsage = ((this->_flags & SwapChain::OutputFlag::shaderInput) == true)
                               ? (DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT) : DXGI_USAGE_RENDER_TARGET_OUTPUT;
        descriptor.Scaling = DXGI_SCALING_STRETCH;
        descriptor.SampleDesc.Count = 1;
        descriptor.Stereo = ((this->_flags & SwapChain::OutputFlag::stereo) == true) ? TRUE : FALSE;
        
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscnDescriptor = {};
        ZeroMemory(&fullscnDescriptor, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
        fullscnDescriptor.RefreshRate.Numerator = (UINT)rate.numerator();
        fullscnDescriptor.RefreshRate.Denominator = (UINT)rate.denominator();
        
        if (useFlipSwap) {
          descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
          if ((this->_flags & SwapChain::OutputFlag::variableRefresh) == true) {
            descriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            this->_tearingSwapFlags = (this->_vsyncInterval == 0) ? (UINT)DXGI_PRESENT_ALLOW_TEARING : 0; // no screen tearing with vsync
          }
          if (descriptor.Stereo == FALSE)
            fullscnDescriptor.Windowed = TRUE;
        }
        else {
          this->_flags |= SwapChain::OutputFlag::disableFlipSwap; // mark as disabled
          descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
          if ((this->_flags & (SwapChain::OutputFlag::variableRefresh|SwapChain::OutputFlag::stereo)) == false)
            fullscnDescriptor.Windowed = TRUE;
        }

        auto dxgiFactoryV2 = SharedResource<IDXGIFactory2>::fromInterface((IDXGIFactory1*)this->_renderer->_dxgiFactory, "SwapChain: DXGI access error");
        auto deviceV1 = SharedResource<ID3D11Device1>::fromInterface(this->_renderer->_device, "SwapChain: device access error");
        auto result = dxgiFactoryV2->CreateSwapChainForHwnd(deviceV1.get(), (HWND)window, &descriptor, &fullscnDescriptor, nullptr, (IDXGISwapChain1**)&_swapChain);
        if (FAILED(result) || this->_swapChain == nullptr)
          throwError(result, "Renderer: swap-chain not created");

#       if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
          this->_deviceContext11_1 = this->_renderer->_context11_1;
#       endif
        if (this->_renderer->_context11_1 == nullptr)
          this->_flags |= SwapChain::OutputFlag::swapNoDiscard;
      }
      else
#   endif
    { // Direct3D 11.0
      this->_flags |= (SwapChain::OutputFlag::disableFlipSwap | SwapChain::OutputFlag::swapNoDiscard); // no flip-swap & no discard in 11.0
    
      DXGI_SWAP_CHAIN_DESC descriptor = {};
      ZeroMemory(&descriptor, sizeof(DXGI_SWAP_CHAIN_DESC));
      descriptor.BufferDesc.Width = _width();
      descriptor.BufferDesc.Height = _height();
      descriptor.BufferDesc.Format = this->_backBufferFormat;
      descriptor.BufferCount = (UINT)this->_framebufferCount;
      descriptor.BufferUsage = ((this->_flags & SwapChain::OutputFlag::shaderInput) == true)
                             ? (DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT) : DXGI_USAGE_RENDER_TARGET_OUTPUT;
      descriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
      descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      descriptor.SampleDesc.Count = 1;
      descriptor.OutputWindow = (HWND)window;
      descriptor.BufferDesc.RefreshRate.Numerator = (UINT)rate.numerator();
      descriptor.BufferDesc.RefreshRate.Denominator = (UINT)rate.denominator();
      if ((this->_flags & SwapChain::OutputFlag::variableRefresh) == false)
        descriptor.Windowed = TRUE;

      auto result = ((IDXGIFactory1*)this->_renderer->_dxgiFactory)->CreateSwapChain(this->_renderer->_device, &descriptor, (IDXGISwapChain**)&_swapChain);
      if (FAILED(result) || this->_swapChain == nullptr)
        throwError(result, "Renderer: swap-chain not created");
    }
    
    ((IDXGIFactory1*)this->_renderer->_dxgiFactory)->MakeWindowAssociation((HWND)window, DXGI_MWA_NO_ALT_ENTER);// no DXGI response to ALT+ENTER
  }
  
  // ---
  
  // Create/refresh swap-chain render-target view + color space
  void SwapChain::_createOrRefreshTargetView() { // throws
    // find + set color space value (if supported)
    __setColorSpace((IDXGISwapChain*)this->_swapChain, this->_backBufferFormat);

    // create render-target view
    if (this->_renderTargetView == nullptr) {
      auto targetResult = ((IDXGISwapChain*)this->_swapChain)->GetBuffer(0, IID_PPV_ARGS(&(this->_renderTarget)));
      if (FAILED(targetResult) || this->_renderTarget == nullptr) {
        throwError(targetResult, "SwapChain: render-target error"); return;
      }
      
      CD3D11_RENDER_TARGET_VIEW_DESC viewDescriptor(D3D11_RTV_DIMENSION_TEXTURE2D, this->_backBufferFormat);
      targetResult = this->_renderer->device()->CreateRenderTargetView(this->_renderTarget, &viewDescriptor, &(this->_renderTargetView));
      if (FAILED(targetResult) || this->_renderTargetView == nullptr)
        throwError(targetResult, "SwapChain: view not created");
    }
  }


// -- swap-chain destruction/move -- -------------------------------------------

  // Destroy swap-chain
  void SwapChain::release() noexcept {
    if (this->_swapChain != nullptr) {
      try {
        if (this->_renderTargetView != nullptr) {
          this->_renderer->setActiveRenderTarget(nullptr);
          this->_renderer->context()->Flush();
          
          this->_renderTargetView->Release();
          this->_renderTargetView = nullptr;
        }
        if (this->_renderTarget != nullptr) {
          this->_renderTarget->Release();
          this->_renderTarget = nullptr;
        }
      } 
      catch (...) {}
        
      try {
        if (this->_renderer->_dxgiLevel >= 2u) {
          ((IDXGISwapChain1*)this->_swapChain)->SetFullscreenState(FALSE, nullptr); // fullscreen must be OFF, or Release will throw
          ((IDXGISwapChain1*)this->_swapChain)->Release();
        }
        else {
          ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(FALSE, nullptr); // fullscreen must be OFF, or Release will throw
          ((IDXGISwapChain*)this->_swapChain)->Release();
        }
      } 
      catch (...) {}
      this->_swapChain = nullptr;
      this->_renderer = nullptr;
    }
  }

  SwapChain::SwapChain(SwapChain&& rhs) noexcept
    : _swapChain(rhs._swapChain),
      _tearingSwapFlags(rhs._tearingSwapFlags),
      _flags(rhs._flags),
      _pixelSize(rhs._pixelSize),
      _framebufferCount(rhs._framebufferCount),
      _backBufferFormat(rhs._backBufferFormat),
      _renderer(rhs._renderer),
      _renderTarget(rhs._renderTarget),
      _renderTargetView(rhs._renderTargetView)
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        , _deviceContext11_1(rhs._deviceContext11_1)
#     endif
  {
    rhs._swapChain = nullptr;
    rhs._renderer = nullptr;
    rhs._renderTarget = nullptr;
    rhs._renderTargetView = nullptr;
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    release();
    this->_swapChain = rhs._swapChain;
    this->_tearingSwapFlags = rhs._tearingSwapFlags;
    this->_flags = rhs._flags;
    this->_pixelSize = rhs._pixelSize;
    this->_framebufferCount = rhs._framebufferCount;
    this->_backBufferFormat = rhs._backBufferFormat;
    this->_renderer = rhs._renderer;
    this->_renderTarget = rhs._renderTarget;
    this->_renderTargetView = rhs._renderTargetView;
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      this->_deviceContext11_1 = rhs._deviceContext11_1;
#   endif
    rhs._swapChain = nullptr;
    rhs._renderer = nullptr;
    rhs._renderTarget = nullptr;
    rhs._renderTargetView = nullptr;
    return *this;
  }


// -- swap-chain operations -- -------------------------------------------------

  // Change presentation mode
  void SwapChain::setPresentMode(pandora::video::PresentMode mode) noexcept {
    if (mode == pandora::video::PresentMode::immediate) {
      this->_vsyncInterval = 0;
      if ((this->_flags & (SwapChain::OutputFlag::variableRefresh
                         | SwapChain::OutputFlag::disableFlipSwap)) == SwapChain::OutputFlag::variableRefresh
      && this->_renderer->_dxgiLevel >= 2u && this->_renderer->isTearingAvailable()) {
        this->_tearingSwapFlags = (UINT)DXGI_PRESENT_ALLOW_TEARING;
      }
    }
    else {
      this->_vsyncInterval = 1;
      this->_tearingSwapFlags = 0;
    }
  }

  // Change back-buffer(s) size + refresh color space
  bool SwapChain::resize(uint32_t width, uint32_t height) { // throws
    uint32_t pixelSize = _toPixelSize(width, height);
    bool isResized = (this->_pixelSize != pixelSize);
  
    if (isResized) {
      // clear previous size-specific context
      this->_renderer->setActiveRenderTarget(nullptr);
      this->_renderTargetView->Release();
      this->_renderTargetView = nullptr; // set to NULL -> tells _createOrRefreshTargetView to re-create it
      this->_renderTarget->Release();
      this->_renderTarget = nullptr;
      ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(FALSE, nullptr);
      this->_renderer->context()->Flush();
      
      // resize swap-chain
      DXGI_SWAP_CHAIN_FLAG flags = (DXGI_SWAP_CHAIN_FLAG)0;
      if ((this->_flags & (SwapChain::OutputFlag::variableRefresh|SwapChain::OutputFlag::disableFlipSwap)) == SwapChain::OutputFlag::variableRefresh)
        flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
      bool useFlipSwap = ((this->_flags & SwapChain::OutputFlag::disableFlipSwap) == false);
      DXGI_FORMAT swapChainFormat = __getSwapChainFormat(this->_backBufferFormat, useFlipSwap);
      
      auto result = ((IDXGISwapChain*)this->_swapChain)->ResizeBuffers(this->_framebufferCount, width, height, swapChainFormat, flags);
      if (FAILED(result)) {
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_HUNG)
          throw std::domain_error("Adapter changed: Renderer/SwapChain must be recreated");
        throwError(result, "SwapChain: resize error");
      }
      this->_pixelSize = pixelSize;
      
      // fullscreen required: tearing without flip-swap / stereo rendering
      if ((this->_flags & SwapChain::OutputFlag::stereo) == true || flags == DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
        ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(TRUE, nullptr);
    }

    // create/refresh render-target-view (if not resized, will only verify color space (in case monitor changed))
    _createOrRefreshTargetView();
    return isResized;
  }
  
  // ---
  
  // Throw appropriate exception for 'swap buffers' error
  static void __processSwapError(HRESULT result) {
    switch (result) {
      // minor issues -> ignore
      case DXGI_ERROR_WAS_STILL_DRAWING:
      case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
      case DXGI_ERROR_NONEXCLUSIVE:
      case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: break;
      // device lost
      case DXGI_ERROR_DEVICE_REMOVED:
      case DXGI_ERROR_DEVICE_RESET: throw std::domain_error("SwapChain: device lost");
      // invalid option / internal error
      default: throwError(result, "SwapChain: internal error"); break;
    }
  }

  // Swap back-buffer(s) and front-buffer, to display drawn content on screen
  void SwapChain::swapBuffers(DepthStencilView depthBuffer) {
    auto result = ((IDXGISwapChain*)this->_swapChain)->Present(this->_vsyncInterval, this->_tearingSwapFlags);
    if (FAILED(result))
      __processSwapError(result);
    __refreshDxgiFactory(&(_renderer->_dxgiFactory));

#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      // discard content of render target + depth/stencil buffer
      if ((this->_flags & SwapChain::OutputFlag::swapNoDiscard) == SwapChain::OutputFlag::none) {
        ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView(this->_renderTargetView);
        if (depthBuffer != nullptr)
          ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView(depthBuffer);
      }
#   endif
  }

#endif
