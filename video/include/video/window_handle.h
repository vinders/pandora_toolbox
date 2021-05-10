/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && !defined(__APPLE__)
# include <cstdint>
#endif

namespace pandora {
  namespace video {
#   if defined(_WINDOWS) || defined(__APPLE__)
      using WindowHandle = void*;         ///< Handle to native system window
      using WindowResourceHandle = void*; ///< Handle to native window resource (icon, cursor, color brush, ...)
      using MenuHandle = void*;           ///< Handle to native menu instance
#   else
      using WindowHandle = uint32_t;      ///< Handle to native system window
      using WindowResourceHandle = void*; ///< Handle to native window resource (icon, cursor, color brush, ...)
      using MenuHandle = void*;           ///< Handle to native menu instance
#   endif
  }
}
