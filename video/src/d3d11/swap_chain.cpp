/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <string>
# include <stdexcept>
# include "video/d3d11/renderer.h"
# include "video/d3d11/swap_chain.h"

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include <system/api/windows_api.h>
# include "video/d3d11/api/d3d_11.h"
# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
#   include <dxgi1_6.h>
# elif defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#   include <dxgi1_5.h>
# else
#   include <dxgi1_2.h>
# endif
# include "video/d3d11/_private/_d3d_resource.h"

  using namespace pandora::video::d3d11;


// -- color management -- ------------------------------------------------------

# if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
    // Find color space for a buffer format
    static inline DXGI_COLOR_SPACE_TYPE __getColorSpace(DXGI_FORMAT backBufferFormat) noexcept {
      switch (backBufferFormat) {
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
          return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020; // HDR-10
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32_SINT:
          return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709; // HDR-scRGB
        default: break;
      }
      return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709; // SDR-sRGB
    }
# endif
  
  // Set swap-chain color space
  // returns: color spaces supported (true) or not
  static bool __setColorSpace(IDXGISwapChain* swapChain, DXGI_FORMAT backBufferFormat, bool isHdrPreferred, DXGI_COLOR_SPACE_TYPE& outColorSpace) { // throws
    // verify HDR support
    outColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709; // SDR-sRGB
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#     if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
        if (isHdrPreferred) {
          D3dResource<IDXGIOutput> output;
          if (SUCCEEDED(swapChain->GetContainingOutput(output.address())) && output) {
            DXGI_OUTPUT_DESC1 outputInfo;
            auto outputV6 = D3dResource<IDXGIOutput6>::tryFromInterface(output.get());
            if (outputV6 && SUCCEEDED(outputV6->GetDesc1(&outputInfo))) {
              
              if (outputInfo.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
              ||  outputInfo.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709) {
                outColorSpace = __getColorSpace(backBufferFormat);
              }
            }
          }
        }
#     endif

      // apply color space
      auto swapChainV3 = D3dResource<IDXGISwapChain3>::tryFromInterface((IDXGISwapChain*)swapChain);
      if (swapChainV3) {
        UINT colorSpaceSupport = 0;
        if (SUCCEEDED(swapChainV3->CheckColorSpaceSupport(outColorSpace, &colorSpaceSupport))
        && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) {

          auto result = swapChainV3->SetColorSpace1(outColorSpace);
          if (FAILED(result))
            throwError(result, "SwapChain: failed to apply color space");
          return true;
        }
      }
#   endif
    return false;
  }


// -- swap-chain creation -- ---------------------------------------------------

  // Verify if buffer format is supported by flip-swap (-> outIsFlipSwapAllowed) + return swap-chain format to use
  static inline DXGI_FORMAT __verifyFlipSwapFormat(DXGI_FORMAT backBufferFormat, bool& outIsFlipSwapAllowed) noexcept {
#   if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
      switch (backBufferFormat) {
        case DXGI_FORMAT_R16G16B16A16_FLOAT: 
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UNORM:   outIsFlipSwapAllowed = true; return backBufferFormat;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: outIsFlipSwapAllowed = true; return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: outIsFlipSwapAllowed = true; return DXGI_FORMAT_B8G8R8A8_UNORM;
        default: break;
      }
#   endif
    outIsFlipSwapAllowed = false;
    return backBufferFormat;
  }

  // Convert portable params to Direct3D swap-chain params
  void SwapChain::_convertSwapChainParams(const Renderer& renderer, const pandora::video::SwapChainParams& params, 
                                          _SwapChainConfig& outConfig) noexcept {
    // convert buffer formats
    outConfig.backBufferFormat = (params.backBufferFormat() != pandora::video::ComponentFormat::custom)
                                 ? Renderer::toDxgiFormat(params.backBufferFormat())
                                 : params.customBackBufferFormat();
    outConfig.frameBufferCount = params.frameBufferCount();
    outConfig.isHdrPreferred = params.isHdrPreferred();
    
    // multisample mode
    if (params.multisampleCount() > 1u && renderer._isMultisampleSupported(params.multisampleCount(), 
                                                      outConfig.backBufferFormat, outConfig.msaaQualityLevel)) {
      outConfig.msaaSampleNumber = params.multisampleCount();
    }
    else {
      outConfig.msaaSampleNumber = 1u;
      outConfig.msaaQualityLevel = 1u;
    }
    
    // verify if flip-swap is applicable
    outConfig.useFlipSwap = (renderer.isFlipSwapAvailable() 
                        && params.renderTargetMode() == SwapChainTargetMode::uniqueOutput // only one render target view
                        && outConfig.msaaSampleNumber <= 1u // no MSAA (not allowed in flip-swap mode)
                        && (renderer.isTearingAvailable()   // tearing OFF or supported with flip-swap
                           || (params.outputFlags() & SwapChainOutputFlag::variableRefresh) == SwapChainOutputFlag::none) );
    outConfig.swapChainFormat = (outConfig.useFlipSwap)     // supported color format
                              ? (int32_t)__verifyFlipSwapFormat((DXGI_FORMAT)outConfig.backBufferFormat, outConfig.useFlipSwap)
                              : outConfig.backBufferFormat;
    
    // buffer usage
    outConfig.bufferUsageMode = ((params.outputFlags() & SwapChainOutputFlag::shaderInput) == true)
                              ? (int32_t)(DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT)
                              : (int32_t)DXGI_USAGE_RENDER_TARGET_OUTPUT;
    outConfig.flags = params.outputFlags();
    if ((outConfig.flags & SwapChainOutputFlag::localOutput) == true && !renderer.isLocalDisplayRestrictionAvailable())
      outConfig.flags &= ~(SwapChainOutputFlag::localOutput);
  }
  
  // ---

  // Create rendering swap-chain for existing renderer
  SwapChain::SwapChain(std::shared_ptr<Renderer> renderer, const pandora::video::SwapChainParams& params, 
                       pandora::video::WindowHandle window, uint32_t width, uint32_t height)
    : _renderer(std::move(renderer)) { // throws
    if (this->_renderer == nullptr)
      throw std::invalid_argument("SwapChain: renderer must not be NULL");
    if (window == nullptr)
      throw std::invalid_argument("SwapChain: window handle must not be NULL");
    if (width == 0 || height == 0)
      throw std::invalid_argument("SwapChain: invalid width/height: values must not be 0");
    
    // build swap-chain
    memset((void*)&this->_settings, 0, sizeof(_SwapChainConfig));
    this->_settings.width = width;
    this->_settings.height = height;
    this->_presentFlags = ((this->_settings.flags & SwapChainOutputFlag::variableRefresh) == true) ? DXGI_PRESENT_ALLOW_TEARING : 0;
    _convertSwapChainParams(*(this->_renderer), params, this->_settings);
    
    this->_swapChain = this->_renderer->_createSwapChain(this->_settings, window, params.rateNumerator(),
                                                         params.rateDenominator(), this->_swapChainLevel); // throws

    // find + set color space value (if supported)
    DXGI_COLOR_SPACE_TYPE colorSpace;
    __setColorSpace((IDXGISwapChain*)this->_swapChain, (DXGI_FORMAT)this->_settings.backBufferFormat,
                    this->_settings.isHdrPreferred, colorSpace);
    this->_settings.colorSpace = (int32_t)colorSpace;

    _createSwapChainTargetView(); // create render-target-view
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      auto contextV1 = D3dResource<ID3D11DeviceContext1>::tryFromInterface((ID3D11DeviceContext*)this->_renderer->context());
      this->_deviceContext11_1 = contextV1.extract();
#   endif
  }

// -- swap-chain destruction/move -- -------------------------------------------

  // Destroy swap-chain
  void SwapChain::_destroy() noexcept {
    if (this->_swapChain != nullptr) {
      try {
        if (this->_renderTargetView) {
          if (this->_renderer)
            this->_renderer->setActiveRenderTarget(nullptr);
          ((ID3D11RenderTargetView*)this->_renderTargetView)->Release();
        }
        if (this->_renderTarget)
          ((ID3D11Texture2D*)this->_renderTarget)->Release();

#       if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
          if (this->_deviceContext11_1)
            ((ID3D11DeviceContext1*)this->_deviceContext11_1)->Release();
#       endif
        
        if (this->_swapChainLevel == Renderer::DeviceLevel::direct3D_11_1) {
          ((IDXGISwapChain1*)this->_swapChain)->SetFullscreenState(FALSE, nullptr); // fullscreen must be OFF, or Release will throw
          ((IDXGISwapChain1*)this->_swapChain)->Release();
        }
        else {
          ((IDXGISwapChain*)this->_swapChain)->SetFullscreenState(FALSE, nullptr); // fullscreen must be OFF, or Release will throw
          ((IDXGISwapChain*)this->_swapChain)->Release();
        }
      } 
      catch (...) {}
    }
  }

  SwapChain::SwapChain(SwapChain&& rhs) noexcept
    : _renderer(std::move(rhs._renderer)),
      _renderTarget(rhs._renderTarget),
      _renderTargetView(rhs._renderTargetView),
      _deviceContext11_1(rhs._deviceContext11_1),
      _swapChain(rhs._swapChain),
      _swapChainLevel(rhs._swapChainLevel),
      _presentFlags(rhs._presentFlags) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_SwapChainConfig));
    rhs._renderer = nullptr;
    rhs._swapChain = rhs._renderTarget = rhs._renderTargetView = rhs._deviceContext11_1 = nullptr;
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    _destroy();
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_SwapChainConfig));
    this->_renderer = std::move(rhs._renderer);
    this->_renderTarget = rhs._renderTarget;
    this->_renderTargetView = rhs._renderTargetView;
    this->_deviceContext11_1 = rhs._deviceContext11_1;
    this->_swapChain = rhs._swapChain;
    this->_swapChainLevel = rhs._swapChainLevel;
    this->_presentFlags = rhs._presentFlags;
    
    rhs._renderer = nullptr;
    rhs._swapChain = rhs._renderTarget = rhs._renderTargetView = rhs._deviceContext11_1 = nullptr;
    return *this;
  }


// -- accessors -- -------------------------------------------------------------

  // Verify if HDR is enabled in current swap-chain (should be verified before using HDR shaders and data)
  bool SwapChain::isHdrEnabled() const noexcept {
#   if defined(NTDDI_WIN10_RS2) && NTDDI_VERSION >= NTDDI_WIN10_RS2
      return (this->_settings.colorSpace == (int32_t)DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
           || this->_settings.colorSpace == (int32_t)DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709);
#   else
      return false;
#   endif
  }


// -- operations -- ------------------------------------------------------------

  // Change back-buffer(s) size + refresh color space
  bool SwapChain::resize(uint32_t width, uint32_t height) { // throws
    bool isResized = (width != this->_settings.width || height != this->_settings.height);
    if (isResized) {
      if (width == 0 || height == 0)
        throw std::invalid_argument("SwapChain: invalid width/height: values must not be 0");
      
      // clear previous size-specific context
      this->_renderer->setActiveRenderTarget(nullptr);
      ((ID3D11RenderTargetView*)this->_renderTargetView)->Release();
      this->_renderTargetView = nullptr;
      ((ID3D11Texture2D*)this->_renderTarget)->Release();
      this->_renderTarget = nullptr;
      ((IDXGISwapChain1*)this->_swapChain)->SetFullscreenState(FALSE, nullptr);
      ((ID3D11DeviceContext*)this->_renderer->context())->Flush();
      
      // resize swap-chain
      DXGI_SWAP_CHAIN_FLAG flags = (this->_settings.useFlipSwap && (this->_settings.flags & SwapChainOutputFlag::variableRefresh) == true) 
                                 ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : (DXGI_SWAP_CHAIN_FLAG)0;
      auto result = ((IDXGISwapChain*)this->_swapChain)->ResizeBuffers((UINT)this->_settings.frameBufferCount, (UINT)width, (UINT)height, 
                                                                       (DXGI_FORMAT)this->_settings.swapChainFormat, flags);
      if (FAILED(result)) {
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_HUNG)
          throw std::domain_error("SwapChain: adapter has changed: Renderer and SwapChain must be recreated");
        throwError(result, "SwapChain: resize failure");
      }
      
      this->_settings.width = width;
      this->_settings.height = height;
      if (!this->_settings.useFlipSwap && (this->_settings.flags & SwapChainOutputFlag::variableRefresh) == true)
        ((IDXGISwapChain1*)this->_swapChain)->SetFullscreenState(TRUE, nullptr);
    }

    // find + set color space value (if supported)
    DXGI_COLOR_SPACE_TYPE colorSpace;
    __setColorSpace((IDXGISwapChain*)this->_swapChain, (DXGI_FORMAT)this->_settings.backBufferFormat,
                    this->_settings.isHdrPreferred, colorSpace);
    this->_settings.colorSpace = (int32_t)colorSpace;
    
    if (isResized) // create render-target-view
      _createSwapChainTargetView();
    return isResized;
  }
  
  // ---
  
  // Create swap-chain render-target-view
  void SwapChain::_createSwapChainTargetView() { // throws
    auto targetResult = ((IDXGISwapChain*)this->_swapChain)->GetBuffer(0, IID_PPV_ARGS((ID3D11Texture2D**)&(this->_renderTarget)));
    if (FAILED(targetResult) || this->_renderTarget == nullptr) {
      throwError(targetResult, "SwapChain: could not access render target");
      return;
    }
    
    auto* device = (ID3D11Device*)this->_renderer->device();
    CD3D11_RENDER_TARGET_VIEW_DESC viewDescriptor((this->_settings.msaaSampleNumber <= 1u) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS, 
                                                  (DXGI_FORMAT)this->_settings.backBufferFormat);
    
    targetResult = device->CreateRenderTargetView((ID3D11Texture2D*)this->_renderTarget, &viewDescriptor, 
                                                  (ID3D11RenderTargetView**)(&(this->_renderTargetView)));
    if (FAILED(targetResult) || this->_renderTargetView == nullptr)
      throwError(targetResult, "SwapChain: could not create render target view");
  }
  
  // ---
  
  // Throw appropriate exception for 'swap buffers' error
  void __processSwapBuffersError(HRESULT result) {
    switch (result) {
      // minor issues -> ignore
      case DXGI_ERROR_WAS_STILL_DRAWING:
      case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
      case DXGI_ERROR_NONEXCLUSIVE:
      case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: break;
      // device lost
      case DXGI_ERROR_DEVICE_REMOVED:
      case DXGI_ERROR_DEVICE_RESET: throw std::domain_error("SwapChain: device access lost");
      // invalid option
      case DXGI_ERROR_CANNOT_PROTECT_CONTENT: throw std::invalid_argument("SwapChain: local display restriction is not supported on current device");
      default: throwError(result, "SwapChain: internal error"); break;
    }
  }

  // Swap back-buffer(s) and front-buffer, to display drawn content on screen
  void SwapChain::swapBuffers(bool useVsync) {
    auto result = ((IDXGISwapChain*)this->_swapChain)->Present(useVsync ? 1 : 0, this->_presentFlags);
    if (FAILED(result))
      __processSwapBuffersError(result);
    this->_renderer->_refreshDxgiFactory();
  }
  
  // Swap back-buffer(s) and front-buffer, to display drawn content on screen + discard render-target content after it
  void SwapChain::swapBuffersDiscard(bool useVsync, Renderer::DepthStencilViewHandle depthBuffer) {
    auto result = ((IDXGISwapChain*)this->_swapChain)->Present(useVsync ? 1 : 0, this->_presentFlags);
    if (FAILED(result))
      __processSwapBuffersError(result);
    this->_renderer->_refreshDxgiFactory();
    
#   if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
      // discard content of render target + depth/stencil buffer
      if (this->_deviceContext11_1) {
        ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView((ID3D11RenderTargetView*)this->_renderTargetView);
        if (depthBuffer != nullptr)
          ((ID3D11DeviceContext1*)this->_deviceContext11_1)->DiscardView((ID3D11DepthStencilView*)depthBuffer);
      }
#   endif
  }

#endif
