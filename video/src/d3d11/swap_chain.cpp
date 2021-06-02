/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cassert>
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

# include "video/d3d11/_private/_d3d_resource.h"
# include <DirectXMath.h>
# include <DirectXColors.h>

  using namespace pandora::video::d3d11;

  SwapChain::SwapChain(Renderer* device, const SwapChain::Config& settings, uint32_t width, uint32_t height, 
                       Handle swapChain, bool isSwapChain11_1)
  : _device(device), _swapChain(swapChain), _isSwapChain11_1(isSwapChain11_1) {
    assert(device != nullptr);
    assert(width > 0 && height > 0);
    memcpy((void*)&_settings, (void*)&settings, sizeof(SwapChain::Config));

    // TODO Renderer: store/remove render targets created/destroyed by SwapChain
  }
  
  // ---
  
  // Destroy swap-chain + invalidate all associated buffers
  SwapChain::~SwapChain() noexcept {
    if (this->_swapChain != nullptr) {
      // TODO: inform Renderer of SwapChain destruction
      //...
      
      if (this->_isSwapChain11_1)
        ((IDXGISwapChain1*)this->_swapChain)->Release();
      else
        ((IDXGISwapChain*)this->_swapChain)->Release();
    }
  }

  SwapChain::SwapChain(SwapChain&& rhs) noexcept 
  : _device(rhs._device), _swapChain(rhs._swapChain), _isSwapChain11_1(rhs._isSwapChain11_1) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(SwapChain::Config));
    rhs._swapChain = nullptr;
  }
  SwapChain& SwapChain::operator=(SwapChain&& rhs) noexcept {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(SwapChain::Config));
    this->_device = rhs._device;
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
