/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <cstddef>
# include <cstdint>
# include <Carbon/Carbon.h>
# if defined(__OBJC__)
#   import <Cocoa/Cocoa.h>
# else
    typedef void* id;
# endif

  namespace pandora {
    namespace hardware {
      // library loader - Cocoa
      struct LibrariesCocoa final {
        LibrariesCocoa() = default;
        ~LibrariesCocoa() noexcept { shutdown(); }
        
        bool isAppLaunchFinished = false;

        // initialize available libraries
        bool init() noexcept;
        // close libraries
        void shutdown() noexcept;
        // read current DPI value of a screen
        bool readScreenDpi(NSScreen& screen, uint32_t& outDpiX, uint32_t& outDpiY) noexcept;
        // read current scaling factor of a screen
        bool readScreenScaling(NSScreen& screen, float& outScaleX, float& outScaleY) noexcept;
        
        // get global instance
        static inline LibrariesCocoa& instance() noexcept {
          if (!_libs._isInit)
            _libs.init();
          return _libs;
        }

      private:
        bool _isInit = false;
        static LibrariesCocoa _libs;
      };
    }
  }
#endif
