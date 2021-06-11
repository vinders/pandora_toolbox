/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "video/component_format.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // Internal depth/stencil buffer config values
        struct _DepthStencilBufferConfig {
          uint32_t width;  // pixel size
          uint32_t height; // pixel size
          pandora::video::ComponentFormat format;
          uint32_t msaaSampleNumber; // MSAA x1/2/4/8
        };
      }
    }
  }
  
#endif
