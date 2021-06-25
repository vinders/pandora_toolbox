/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "video/render_options.h"

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include "video/d3d11/api/d3d_11.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // Internal swap-chain config values
        struct _SwapChainConfig {
          uint32_t width;            // pixel size
          uint32_t height;           // pixel size
          uint32_t frameBufferCount; // front buffer + back buffers
          
          DXGI_FORMAT swapChainFormat;
          DXGI_FORMAT backBufferFormat;
          DXGI_USAGE bufferUsageMode;
          DXGI_COLOR_SPACE_TYPE colorSpace;
          bool isHdrPreferred;
          
          SwapChainOutputFlag flags; // advanced settings
          bool useFlipSwap;          // flip-swap enabled (more efficient, but many limitations)
        };
      }
    }
  }

# undef NODRAWTEXT
# undef NOGDI
# undef NOBITMAP
# undef NOMCX
# undef NOSERVICE
#endif
