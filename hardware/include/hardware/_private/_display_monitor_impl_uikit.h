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
#ifndef _DISPLAY_MONITOR_UIKIT_IMPL_H
# define _DISPLAY_MONITOR_UIKIT_IMPL_H 1

# if !defined(_WINDOWS) && defined(__APPLE__)
#   include <TargetConditionals.h>
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#     ifndef __OBJC__
        extern "C" {
#     endif
#     include <stdint.h>

#     define __P_HARDWARE_UIKIT_BASE_DPI 150.0

      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0

      typedef void* IosScreenHandle; // UIScreen*
      typedef void* IosAppHandle;    // UIApplication*
      
      
      // -- types - obj-C bindings --
      
      struct DisplayArea_uikit {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
      };
      struct DisplayMode_uikit {
        uint32_t width;  // px
        uint32_t height; // px
        uint32_t bitDepth; // bits
        uint32_t refreshRate; // mHz
      };
      struct MonitorAttributes_uikit {
        uint32_t index;
        char* description;
        struct DisplayArea_uikit screenArea;
        struct DisplayArea_uikit workArea;
        Bool isPrimary;
      };
      
      
      // -- monitor attributes - id/area/description/primary --

      void __readAttributes_uikit(IosScreenHandle screen, uint32_t index, struct MonitorAttributes_uikit* outAttr);
      double __getScaling_uikit(IosScreenHandle screen);
      
      
      // -- get display monitors (handle + attributes) --
      
      IosScreenHandle __getMonitorHandle_uikit(uint32_t index);
      IosScreenHandle __getPrimaryMonitor_uikit(struct MonitorAttributes_uikit* outAttr);
      Bool            __getMonitor_uikit(IosScreenHandle screen, struct MonitorAttributes_uikit* outAttr);
      IosScreenHandle __getMonitorByIndex_uikit(uint32_t index, struct MonitorAttributes_uikit* outAttr);
      
      uint32_t __countMonitorHandles_uikit();
      
      
      // -- display modes --
      
      Bool __getDisplayMode_uikit(uint32_t index, struct DisplayMode_uikit* out);
      Bool __setDisplayMode_uikit(uint32_t index, const struct DisplayMode_uikit* mode);
      Bool __setDefaultDisplayMode_uikit(uint32_t index);
      
      Bool __listDisplayModes_uikit(uint32_t index, struct DisplayMode_uikit** outModes, uint32_t* outLength);
      
      
      // -- metrics --
      
      void __clientAreaToWindowArea_uikit(const struct DisplayArea_uikit* clientArea, IosAppHandle windowHandle, 
                                          struct DisplayArea_uikit* outWindowArea);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
