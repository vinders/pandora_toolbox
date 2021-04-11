/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifndef __OBJC__
# pragma once
#endif
#ifndef _LIBRARIES_COCOA_H
# define _LIBRARIES_COCOA_H 1

# if !defined(_WINDOWS) && defined(__APPLE__)
#   include <TargetConditionals.h>
#   if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
#     ifndef __OBJC__
        extern "C" {
#     endif
#     include <stdint.h>

#     define __P_HARDWARE_COCOA_DEFAULT_DPI 110.0f
#     define __P_HARDWARE_COCOA_DEFAULT_SCALE 1.27f

      // -- types --
  
      typedef void* CocoaScreenHandle;
      typedef uint32_t CocoaDisplayId;
      
      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0

      // properties of LibrariesCocoa
      struct CocoaLibraryData {
        Bool isAppReady;
      };
      
      
      // -- library management (private) --
      
      Bool __LibrariesCocoa_init();
      void __LibrariesCocoa_shutdown();
      CocoaLibraryData* __LibrariesCocoa_getData();


      // -- utilities --

      Bool LibrariesCocoa_readScreenDpi(CocoaScreenHandle screen, uint32_t* outDpiX, uint32_t* outDpiY);
      Bool LibrariesCocoa_readScreenScaling(CocoaScreenHandle screen, float* outScaleX, float* outScaleY);

#     ifndef __OBJC__
        } // extern "C"
        
        namespace pandora {
          namespace hardware {
            struct LibrariesCocoa final {
              LibrariesCocoa() = default;
              ~LibrariesCocoa() noexcept { __LibrariesCocoa_shutdown(); }
              
              // get properties of instance
              inline CocoaLibraryData& data() noexcept { return *__LibrariesCocoa_getData(); }

              // get global instance
              static LibrariesCocoa* instance() noexcept {
                static LibrariesCocoa _libs;
                static bool _isInit = false;
                return (_isInit || (_isInit = (bool)__LibrariesCocoa_init())) ? &_libs : nullptr;
              }
            };
          }
        }
#     endif
#   endif
# endif
#endif
