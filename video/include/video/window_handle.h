/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>

namespace pandora {
  namespace video {
#     if defined(_WINDOWS) || defined(__APPLE__)
        using WindowHandle = void*;
#     else
        using WindowHandle = uint32_t;
#     endif
  }
}
