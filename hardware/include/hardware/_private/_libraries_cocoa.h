/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifndef __OBJC__
# pragma once
#endif

#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <stddef.h>
# include <stdint.h>
  
# ifndef __OBJC__
    extern "C" {
# endif
      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0
      typedef void* ScreenHandle;

      // properties of LibrariesCocoa
      struct LibrariesCocoa_data {
        Bool isAppReady;
      };
      
      Bool LibrariesCocoa_init();
      void LibrariesCocoa_shutdown();
      LibrariesCocoa_data* LibrariesCocoa_getData();

      Bool LibrariesCocoa_readScreenDpi(ScreenHandle screen, uint32_t* outDpiX, uint32_t* outDpiY);
      Bool LibrariesCocoa_readScreenScaling(ScreenHandle screen, float* outScaleX, float* outScaleY);
      Bool LibrariesCocoa_refreshHandleForMonitor(ScreenHandle* screen, uint32_t unitNumber);

# ifndef __OBJC__
    }
    namespace pandora {
      namespace hardware {
        struct LibrariesCocoa final {
          LibrariesCocoa() = default;
          ~LibrariesCocoa() noexcept { LibrariesCocoa_shutdown(); }
          
          // get properties of instance
          inline LibrariesCocoa_data& data() noexcept { return *LibrariesCocoa_getData(); }
          
          // read current DPI value of a screen
          static inline bool readScreenDpi(ScreenHandle screen, uint32_t& outDpiX, uint32_t& outDpiY) noexcept { return (bool)LibrariesCocoa_readScreenDpi(screen, &outDpiX, &outDpiY); }
          // read current scaling factor of a screen
          static inline bool readScreenScaling(ScreenHandle screen, float& outScaleX, float& outScaleY) noexcept { return (bool)LibrariesCocoa_readScreenScaling(screen, &outScaleX, &outScaleY); }
          // get up-to-date handle of a screen
          static inline bool refreshHandleForMonitor(ScreenHandle& screenOut, uint32_t unitNumber) noexcept { return (bool)LibrariesCocoa_refreshHandleForMonitor(&screenOut, unitNumber); }
          
          // get global instance
          static inline LibrariesCocoa* instance() noexcept {
            static LibrariesCocoa _libs;
            static bool _isInit = false;
            return (_isInit || (_isInit = (bool)LibrariesCocoa_init())) ? &_libs : nullptr;
          }
        };
      }
    }
# endif
#endif
