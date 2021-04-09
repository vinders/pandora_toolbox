/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstddef>
# include <cstdint>
# include <unistd.h>

  namespace pandora {
    namespace hardware {
      using LibHandle = void*;
      
      // library loader - Wayland
      struct LibrariesWayland final {
        LibrariesWayland() = default;
        ~LibrariesWayland() noexcept { shutdown(); }
        
        // initialize available libraries
        bool init() noexcept;
        // close libraries
        void shutdown() noexcept;
        
        // get global instance
        static inline LibrariesWayland* instance() noexcept {
          return (_libs._isInit || _libs.init()) ? &_libs : nullptr;
        }
      
      private:
        bool _isInit = false;
        static LibrariesWayland _libs;
      };
    }
  }
#endif
