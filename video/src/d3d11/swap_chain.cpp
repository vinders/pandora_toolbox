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

# if defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION >= 114
#   include <d3d11_4.h>
# elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 113
#   include <d3d11_3.h>
# elif defined(_VIDEO_D3D11_VERSION) && _VIDEO_D3D11_VERSION == 110
#   include <d3d11.h>
# else
#   include <d3d11_1.h>
# endif
# include <DirectXMath.h>
# include <DirectXColors.h>

  using namespace pandora::video::d3d11;

  SwapChain::SwapChain(Renderer* device, const SwapChain::Config& settings, uint32_t width, uint32_t height, 
                       Handle swapChain, bool isSwapChain11_1) {
    memcpy((void*)&_settings, (void*)&settings, sizeof(SwapChain::Config));
    this->_swapChain = swapChain;
    this->_isSwapChain11_1 = isSwapChain11_1;

    // TODO Renderer: store/remove render targets created/destroyed by SwapChain (+SwapChain destructor callback)
  }
  
  // ---
  
  // Destroy swap-chain + invalidate all associated buffers
  SwapChain::~SwapChain() noexcept {
    if (this->_swapChain != nullptr) {
      if (this->_isSwapChain11_1)
        ((IDXGISwapChain1*)this->_swapChain)->Release();
      else
        ((IDXGISwapChain*)this->_swapChain)->Release();
    }
  }

  SwapChain::SwapChain(SwapChain&& rhs) noexcept : _swapChain(rhs._swapChain), _isSwapChain11_1(rhs._isSwapChain11_1) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(SwapChain::Config));
    rhs._swapChain = nullptr;
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(SwapChain::Config));
    this->_swapChain = rhs._swapChain;
    this->_isSwapChain11_1 = rhs._isSwapChain11_1;
    rhs._swapChain = nullptr;
    return *this;
  }
  
  // ---
          
  // Change back-buffer(s) size + invalidate all associated buffers
  void SwapChain::resize(uint32_t width, uint32_t height) {
    
  }
         
  // Color space:
  // DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709             = 0, // SDR/sRGB
  // DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709             = 1, // HDR/scRGB avec rgba16_f
  // DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020          = 12, // HDR10/BT2100 avec rgb10a2_unorm -> IDXGISwapChain3::SetColorSpace1(...)

#endif
