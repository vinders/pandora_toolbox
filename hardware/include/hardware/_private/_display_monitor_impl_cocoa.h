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
#ifndef _DISPLAY_MONITOR_COCOA_IMPL_H
# define _DISPLAY_MONITOR_COCOA_IMPL_H 1

# if !defined(_WINDOWS) && defined(__APPLE__)
#   include <TargetConditionals.h>
#   if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
#     ifndef __OBJC__
        extern "C" {
#     endif
#     include <stdint.h>
#     include "./_libraries_cocoa.h"
      
      // -- types - obj-C bindings --
      
      struct DisplayArea_cocoa {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
      };
      struct DisplayMode_cocoa {
        uint32_t width;  // px
        uint32_t height; // px
        uint32_t bitDepth; // bits
        uint32_t refreshRate; // mHz
      };
      struct MonitorAttributes_cocoa {
        CocoaDisplayId displayId;
        char* description;
        struct DisplayArea_cocoa screenArea;
        struct DisplayArea_cocoa workArea;
        Bool isPrimary;
      };
      
      
      // -- monitor attributes - id/area/description/primary --

      char* __readDeviceDescription_cocoa(CocoaScreenHandle screen, CocoaDisplayId displayId);
      void __readScreenArea_cocoa(CocoaScreenHandle screen, CocoaDisplayId displayId, struct DisplayArea_cocoa* outScreenArea, struct DisplayArea_cocoa* outWorkArea);
      
      void __readAttributes_cocoa(CocoaScreenHandle screen, Bool checkPrimary, uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr);
      
      
      // -- get display monitors (handle + attributes) --
      
      CocoaScreenHandle __getMonitorHandle_cocoa(uint32_t unitNumber, CocoaDisplayId* outId);
      CocoaScreenHandle __getPrimaryMonitor_cocoa(uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr);
      Bool              __getMonitor_cocoa(CocoaScreenHandle screen, uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr);
      CocoaScreenHandle __getMonitorById_cocoa(CocoaDisplayId displayId, uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr);
      CocoaScreenHandle __getMonitorByUnit_cocoa(uint32_t unitNumber, struct MonitorAttributes_cocoa* outAttr);
      
      Bool __listMonitorIds_cocoa(CocoaDisplayId** outList, uint32_t* outLength);
      CocoaDisplayId __getMonitorIdFromList_cocoa(CocoaDisplayId* list, uint32_t index);
      
      
      // -- display modes --
      
      Bool __getDisplayMode_cocoa(CocoaDisplayId displayId, struct DisplayMode_cocoa* out);
      Bool __setDisplayMode_cocoa(CocoaDisplayId displayId, uint32_t unitNumber, const struct DisplayMode_cocoa* mode);
      Bool __setDefaultDisplayMode_cocoa(CocoaDisplayId displayId, uint32_t unitNumber);
      
      Bool __listDisplayModes_cocoa(CocoaDisplayId displayId, struct DisplayMode_cocoa** outModes, uint32_t* outLength);
      
      
      // -- metrics --
      
      void __clientAreaToWindowArea_cocoa(CocoaScreenHandle screen, const struct DisplayArea_cocoa* clientArea, CocoaWindowHandle windowHandle, 
                                          Bool hasMenu, uint32_t hasBorders, uint32_t hasCaption, struct DisplayArea_cocoa* outWindowArea);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
