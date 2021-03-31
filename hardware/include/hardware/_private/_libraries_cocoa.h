/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifndef __P_LIBRARIES_COCOA_OBJC
# pragma once
#endif

#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <stddef.h>
# include <stdint.h>
# include <Carbon/Carbon.h>
# if defined(__OBJC__)
#   import <Cocoa/Cocoa.h>
# else
    typedef void* id;
# endif

# ifndef __P_LIBRARIES_COCOA_OBJC
    extern "C" {
# endif
      struct LibrariesCocoa_data {
        bool isAppReady;
      };
      
      bool LibrariesCocoa_init();
      void LibrariesCocoa_shutdown();
      LibrariesCocoa_data* LibrariesCocoa_getData();

      bool LibrariesCocoa_readScreenDpi(NSScreen* screen, uint32_t* outDpiX, uint32_t* outDpiY);
      bool LibrariesCocoa_readScreenScaling(NSScreen* screen, float* outScaleX, float* outScaleY);
      bool LibrariesCocoa_refreshHandleForMonitor(NSScreen** screen, uint32_t unitNumber);

# ifndef __P_LIBRARIES_COCOA_OBJC
    }
    namespace pandora {
      namespace hardware {
        struct LibrariesCocoa final {
          LibrariesCocoa() = default;
          ~LibrariesCocoa() noexcept { LibrariesCocoa_shutdown(); }
          
          inline LibrariesCocoa_data& data() noexcept { return *LibrariesCocoa_getData(); }
          
          // read current DPI value of a screen
          static inline bool readScreenDpi(NSScreen& screen, uint32_t& outDpiX, uint32_t& outDpiY) noexcept { return LibrariesCocoa_readScreenDpi(&screen, &outDpiX, &outDpiY); }
          // read current scaling factor of a screen
          static inline bool readScreenScaling(NSScreen& screen, float& outScaleX, float& outScaleY) noexcept { return LibrariesCocoa_readScreenScaling(&screen, &outScaleX, &outScaleY); }
          // get up-to-date handle of a screen
          static inline bool refreshHandleForMonitor(NSScreen** screenOut, uint32_t unitNumber) noexcept { return LibrariesCocoa_refreshHandleForMonitor(screenOut, unitNumber); }
          
          // get global instance
          static inline LibrariesCocoa* instance() noexcept {
            return (_isInit || (_isInit = LibrariesCocoa_init())) ? &_libs : nullptr;
          }

        private:
          static bool _isInit = false;
          static LibrariesCocoa _libs;
        };
      }
    }
# endif
#endif
