/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "../window_handle.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class Renderer;

        /// @class SwapChain
        /// @brief Direct3D rendering swap-chain (framebuffers)
        class SwapChain final {
        public:
          /// @brief Swap-chain running config
          struct Config {
            int32_t backBufferFormat;  // DXGI_FORMAT
            int32_t depthBufferFormat; // DXGI_FORMAT
            int32_t bufferUsageMode;   // DXGI_USAGE
            int32_t scalingMode;       // DXGI_SCALING
            int32_t alphaBlending;     // DXGI_ALPHA_MODE
            
            uint32_t msaaSampleNumber; // MSAA x1/2/4/8
            uint32_t msaaQualityLevel; // MSAA max quality
            uint32_t drawingBufferNumber;// front buffer + back buffers
            
            SwapChainOutputFlag flags; // advanced settings
            bool useFlipSwap;          // flip-swap enabled
          };
          using Handle = void*; // IDXGISwapChain*/IDXGISwapChain1*
          

          /// @brief Destroy swap-chain + invalidate all associated buffers
          ~SwapChain() noexcept;
          
          SwapChain(const SwapChain&) = delete;
          SwapChain(SwapChain&& rhs) noexcept;
          SwapChain& operator=(const SwapChain&) = delete;
          SwapChain& operator=(SwapChain&&) noexcept;
          
          /// @brief Change back-buffer(s) size + invalidate all associated buffers
          /// @throws exception on failure
          void resize(uint32_t width, uint32_t height);
          
        private:
          SwapChain(Renderer* device, const Config& settings, uint32_t width, uint32_t height, 
                    Handle swapChain, bool isSwapChain11_1); // throws
          friend class Renderer;
          
        private:
          Config _settings{ 0,0,0,0,0, 1,1,2, SwapChainOutputFlag::none,false };
          Renderer* _device = nullptr;
          Handle _swapChain = nullptr;
          bool _isSwapChain11_1 = false;
        };
      }
    }
  }

#endif
