/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
      typedef void* CocoaWindowHandle;
      typedef uint32_t CocoaDisplayId;
#     ifdef __OBJC__
        typedef CGDisplayModeRef CocoaDisplayModeRef;
#     else
        typedef void* CocoaDisplayModeRef;
#     endif
      
      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0

      // "original modes" linked-list item
      struct DisplayModeBackup {
        uint32_t unitNumber;
        CocoaDisplayModeRef mode;
        struct DisplayModeBackup* next;
      };

      // properties of LibrariesCocoa
      struct CocoaLibraryData {
        Bool isAppReady;
        struct DisplayModeBackup* originalModes;
      };
      
      
      // -- library management (private) --
      
      Bool __LibrariesCocoa_init();
      void __LibrariesCocoa_shutdown();
      
      
      // -- library data --
      
      struct CocoaLibraryData* __LibrariesCocoa_getData();
      
      struct DisplayModeBackup* __LibrariesCocoa_findOriginalMode(uint32_t unitNumber);
      struct DisplayModeBackup* __LibrariesCocoa_createDetachedOriginalMode(uint32_t unitNumber, CocoaDisplayModeRef mode);
      void __LibrariesCocoa_appendOriginalMode(struct DisplayModeBackup* newEntry);
      void __LibrariesCocoa_eraseOriginalMode(uint32_t unitNumber);


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
