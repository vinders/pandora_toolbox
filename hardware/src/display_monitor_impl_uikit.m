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
--------------------------------------------------------------------------------
Description : Display monitor - iOS implementation
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE

# include <stddef.h>
# include <string.h>
# import <UIKit/UIApplication.h>
# import <UIKit/UIScreen.h>
# import <UIKit/UIScreenMode.h>
# import <UIKit/UIView.h>
# import "hardware/_private/_display_monitor_impl_uikit.h"

# define __P_DEFAULT_STATUS_BAR_SIZE 20.0


// -- monitor attributes - id/area/description/primary -- ----------------------

  // read all attributes of a monitor handle
  void __readAttributes_uikit(IosScreenHandle handle, uint32_t index, struct MonitorAttributes_uikit* outAttr) {
    UIScreen* screen = (UIScreen*)handle;
    
    outAttr->index = index;
    if ([screen description])
      outAttr->description = strdup([[screen description] UTF8String]);
    outAttr->isPrimary = (index == 0);
    
    @autoreleasepool {
      CGRect boundaries = [screen bounds];
      outAttr->screenArea.x = boundaries.origin.x;
      outAttr->screenArea.y = boundaries.origin.y;
      outAttr->screenArea.width = boundaries.size.width;
      outAttr->screenArea.height = boundaries.size.height;
      
      Bool isWorkAreaValid = Bool_FALSE;
      if (@available(iOS 11, *)) {
        @try {
          if ([[UIApplication sharedApplication] keyWindow]) {
            UIEdgeInsets insets = [[[UIApplication sharedApplication] keyWindow] safeAreaInsets];
            uint32_t paddingX = insets.left + insets.right;
            uint32_t paddingY = insets.top + insets.bottom;
            if (paddingX > 0 || paddingY > 0) {
              outAttr->workArea.x = outAttr->screenArea.x + insets.left;
              outAttr->workArea.y = outAttr->screenArea.y + insets.top;
              outAttr->workArea.width = outAttr->screenArea.width - paddingX;
              outAttr->workArea.height = outAttr->screenArea.height - paddingY;
              isWorkAreaValid = Bool_TRUE;
            }
          }
        }
        @catch (NSException*) { isWorkAreaValid = Bool_FALSE; }
      }
        
      if (!isWorkAreaValid) {
        if (outAttr->screenArea.width <= outAttr->screenArea.height) {
          outAttr->workArea.x = outAttr->screenArea.x;
          outAttr->workArea.y = outAttr->screenArea.y + __P_DEFAULT_STATUS_BAR_SIZE;
          outAttr->workArea.width = outAttr->screenArea.width;
          outAttr->workArea.height = outAttr->screenArea.height - __P_DEFAULT_STATUS_BAR_SIZE;
        }
        else {
          outAttr->workArea.x = outAttr->screenArea.x + __P_DEFAULT_STATUS_BAR_SIZE;
          outAttr->workArea.y = outAttr->screenArea.y;
          outAttr->workArea.width = outAttr->screenArea.width - __P_DEFAULT_STATUS_BAR_SIZE;
          outAttr->workArea.height = outAttr->screenArea.height;
        }
      }
    }
  }
  
  // get scaling factor
  double __getScaling_uikit(IosScreenHandle screen) {
    return (screen) ? [(UIScreen*)screen scale] : 1.0;
  }
  
  
// -- get display monitors (handle + attributes) -- ----------------------------
  
  // get screen handle based on unit number (fix handle in case of automatic graphics switching)
  IosScreenHandle __getMonitorHandle_uikit(uint32_t index) {
    @try {
      if ([[UIScreen screens] count] > index)
        return (IosScreenHandle)[[UIScreen screens] objectAtIndex:index];
      return (IosScreenHandle)NULL;
    }
    @catch (NSException*) { return (IosScreenHandle)NULL; }
  }
  
  // read handle/attributes of primary/default monitor
  IosScreenHandle __getPrimaryMonitor_uikit(struct MonitorAttributes_uikit* outAttr) {
    UIScreen* handle = [UIScreen mainScreen];
    if (handle)
      __readAttributes_uikit((IosScreenHandle)handle, 0, outAttr);
    return (IosScreenHandle)handle;
  }
  
  // get attributes of a monitor handle
  Bool __getMonitor_uikit(IosScreenHandle screen, struct MonitorAttributes_uikit* outAttr) {
    if (screen) {
      uint32_t index = 0;
      for (UIScreen* existingScreen in [UIScreen screens]) {
        if ((UIScreen*)screen == existingScreen) {
          __readAttributes_uikit(screen, index, outAttr);
          return Bool_TRUE;
        }
        ++index;
      }
    }
    return Bool_FALSE;
  }
  
  // get handle/attributes of any monitor by index
  IosScreenHandle __getMonitorByIndex_uikit(uint32_t index, struct MonitorAttributes_uikit* outAttr) {
    if ([[UIScreen screens] count] > index) {
      UIScreen* screen = [[UIScreen screens] objectAtIndex:index];
      __readAttributes_uikit((IosScreenHandle)screen, index, outAttr);
      return (IosScreenHandle)screen;
    }
    return (IosScreenHandle)NULL;
  }
  
  // ---
  
  // count all active monitors
  uint32_t __countMonitorHandles_uikit() {
    @try {
      uint32_t displayCount = (uint32_t) [[UIScreen screens] count];
      return displayCount;
    }
    @catch (NSException*) { return 0; }
  }
  
  
// -- display modes -- ---------------------------------------------------------

  // get current display resolution/depth/rate of a monitor
  Bool __getDisplayMode_uikit(uint32_t index, struct DisplayMode_uikit* out) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*) __getMonitorHandle_uikit(index);
      if (screen != NULL && [screen currentMode]) {
        UIScreenMode* modeInfo = [screen currentMode];
        out->width = modeInfo.size.width;
        out->height = modeInfo.size.height;
        out->bitDepth = 32;
        out->refreshRate = (uint32_t)((double)[screen maximumFramesPerSecond] * 1000.0 + 0.500001);
        return Bool_TRUE;
      }
      return Bool_FALSE;
    }
  }
  
  // set display resolution/depth/rate of a monitor
  Bool __setDisplayMode_uikit(uint32_t index, const struct DisplayMode_uikit* mode) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*)__getMonitorHandle_uikit(index);
      if (screen != NULL && [[screen availableModes] count] > 0) {
        
        for (UIScreenMode* modeInfo in [screen availableModes]) {
          if (mode->width == modeInfo.size.width && mode->height == modeInfo.size.height) {
            [screen setCurrentMode:modeInfo];
            return Bool_TRUE;
          }
        }
      }
      return Bool_FALSE;
    }
  }
  
  // reset display resolution/depth/rate of a monitor to default values
  Bool __setDefaultDisplayMode_uikit(uint32_t index) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*)__getMonitorHandle_uikit(index);
      if (screen != NULL) {
        
        UIScreenMode* modeInfo = [screen preferredMode];
        if (modeInfo) {
          [screen setCurrentMode:modeInfo];
          return Bool_TRUE;
        }
      }
      return Bool_FALSE;
    }
  }
  
  // ---
  
  // list all display modes of a monitor
  Bool __listDisplayModes_uikit(uint32_t index, struct DisplayMode_uikit** outModes, uint32_t* outLength) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*) __getMonitorHandle_uikit(index);
      if (screen != NULL && [[screen availableModes] count] > 0) {
        *outLength = (uint32_t)[[screen availableModes] count];
        *outModes = (struct DisplayMode_uikit*)calloc(*outLength, sizeof(struct DisplayMode_uikit));
        if (*outModes == NULL) {
          *outLength = 0;
          return Bool_FALSE;
        }
        
        uint32_t refreshRate = (uint32_t)((double)[screen maximumFramesPerSecond] * 1000.0 + 0.500001);
        uint32_t index = 0;
        for (UIScreenMode* modeInfo in [screen availableModes]) {
          (*outModes)[index].width = modeInfo.size.width;
          (*outModes)[index].height = modeInfo.size.height;
          (*outModes)[index].bitDepth = 32;
          (*outModes)[index].refreshRate = refreshRate;
          ++index;
        }
        return Bool_TRUE;
      }
      return Bool_FALSE;
    }
  }
  
  
// -- metrics -- ---------------------------------------------------------------
  
  // client area to window area (DPI adjusted)
  void __clientAreaToWindowArea_uikit(const struct DisplayArea_uikit* clientArea, IosAppHandle windowHandle, 
                                    struct DisplayArea_uikit* outWindowArea) {
    Bool isWindowAreaValid = Bool_FALSE;
    @try {
      UIApplication* app = (UIApplication*)windowHandle;
      if (app == NULL)
        app = [UIApplication sharedApplication];
      
      if (app && [app keyWindow]) {
        CGRect position = [[app keyWindow] frame];
        outWindowArea->x = position.origin.x;
        outWindowArea->y = position.origin.y;
        outWindowArea->width = position.size.width;
        outWindowArea->height = position.size.height;
        isWindowAreaValid = Bool_TRUE;
      }
    }
    @catch (NSException*) { isWindowAreaValid = Bool_FALSE; }
    
    if (!isWindowAreaValid) {
      outWindowArea->x = clientArea->x;
      outWindowArea->y = clientArea->y;
      outWindowArea->width = clientArea->width;
      outWindowArea->height = clientArea->height;
    }
  }

#endif
