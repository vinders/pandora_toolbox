/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "../render_options.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // Internal swap-chain config values
        struct _SwapChainConfig {
          uint32_t width;            // pixel size
          uint32_t height;           // pixel size
          uint32_t frameBufferCount; // front buffer + back buffers
          
          int32_t swapChainFormat;   // DXGI_FORMAT
          int32_t backBufferFormat;  // DXGI_FORMAT
          int32_t bufferUsageMode;   // DXGI_USAGE
          int32_t colorSpace;        // DXGI_COLOR_SPACE_TYPE
          bool isHdrPreferred;
          
          uint32_t msaaSampleNumber; // MSAA x1/2/4/8
          uint32_t msaaQualityLevel; // MSAA max quality
          SwapChainOutputFlag flags; // advanced settings
          bool useFlipSwap;          // flip-swap enabled (more efficient, but many limitations)
        };
      }
    }
  }
  
#endif
