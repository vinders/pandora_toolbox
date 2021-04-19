/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifndef __OBJC__
# pragma once
#endif
#ifndef _DISPLAY_MONITOR_IOS_IMPL_H
# define _DISPLAY_MONITOR_IOS_IMPL_H 1

# if !defined(_WINDOWS) && defined(__APPLE__)
#   include <TargetConditionals.h>
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#     ifndef __OBJC__
        extern "C" {
#     endif
#     include <stdint.h>

#     define __P_HARDWARE_IOS_BASE_DPI 150.0

      typedef int Bool;
#     define Bool_TRUE  1
#     define Bool_FALSE 0

      typedef void* IosScreenHandle; // UIScreen*
      typedef void* IosAppHandle;    // UIApplication*
      
      
      // -- types - obj-C bindings --
      
      struct DisplayArea_ios {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
      };
      struct DisplayMode_ios {
        uint32_t width;  // px
        uint32_t height; // px
        uint32_t bitDepth; // bits
        uint32_t refreshRate; // mHz
      };
      struct MonitorAttributes_ios {
        uint32_t index;
        char* description;
        struct DisplayArea_ios screenArea;
        struct DisplayArea_ios workArea;
        Bool isPrimary;
      };
      
      
      // -- monitor attributes - id/area/description/primary --

      void __readAttributes_ios(IosScreenHandle screen, uint32_t index, struct MonitorAttributes_ios* outAttr);
      double __getScaling_ios(IosScreenHandle screen);
      
      
      // -- get display monitors (handle + attributes) --
      
      IosScreenHandle __getMonitorHandle_ios(uint32_t index);
      IosScreenHandle __getPrimaryMonitor_ios(struct MonitorAttributes_ios* outAttr);
      Bool            __getMonitor_ios(IosScreenHandle screen, struct MonitorAttributes_ios* outAttr);
      IosScreenHandle __getMonitorByIndex_ios(uint32_t index, struct MonitorAttributes_ios* outAttr);
      
      uint32_t __countMonitorHandles_ios();
      
      
      // -- display modes --
      
      Bool __getDisplayMode_ios(uint32_t index, struct DisplayMode_ios* out);
      Bool __setDisplayMode_ios(uint32_t index, const struct DisplayMode_ios* mode);
      Bool __setDefaultDisplayMode_ios(uint32_t index);
      
      Bool __listDisplayModes_ios(uint32_t index, struct DisplayMode_ios** outModes, uint32_t* outLength);
      
      
      // -- metrics --
      
      void __clientAreaToWindowArea_ios(const struct DisplayArea_ios* clientArea, IosAppHandle windowHandle, 
                                        Bool hasStatusBar, struct DisplayArea_ios* outWindowArea);

#     ifndef __OBJC__
        } // extern "C"
#     endif
#   endif
# endif

#endif
