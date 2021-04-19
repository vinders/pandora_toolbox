/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
# import "hardware/_private/_display_monitor_impl_ios.h"

# define __P_DEFAULT_STATUS_BAR_SIZE 20.0


// -- monitor attributes - id/area/description/primary -- ----------------------

  // read all attributes of a monitor handle
  void __readAttributes_ios(IosScreenHandle handle, uint32_t index, struct MonitorAttributes_ios* outAttr) {
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
      
#     if defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
      Bool isWorkAreaValid = Bool_FALSE;
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
        
      if (!isWorkAreaValid) {
#     endif
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
#     if defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
      }
#     endif
    }
  }
  
  // get scaling factor
  double __getScaling_ios(IosScreenHandle screen) {
    return (screen) ? [(UIScreen*)screen scale] : 1.0;
  }
  
  
// -- get display monitors (handle + attributes) -- ----------------------------
  
  // get screen handle based on unit number (fix handle in case of automatic graphics switching)
  IosScreenHandle __getMonitorHandle_ios(uint32_t index) {
    @try {
      if ([[UIScreen screens] count] > index)
        return (IosScreenHandle)[[UIScreen screens] objectAtIndex:index];
      return (IosScreenHandle)NULL;
    }
    @catch (NSException*) { return (IosScreenHandle)NULL; }
  }
  
  // read handle/attributes of primary/default monitor
  IosScreenHandle __getPrimaryMonitor_ios(struct MonitorAttributes_ios* outAttr) {
    UIScreen* handle = [UIScreen mainScreen];
    if (handle)
      __readAttributes_ios((IosScreenHandle)handle, 0, outAttr);
    return (IosScreenHandle)handle;
  }
  
  // get attributes of a monitor handle
  Bool __getMonitor_ios(IosScreenHandle screen, struct MonitorAttributes_ios* outAttr) {
    if (screen) {
      uint32_t index = 0;
      for (UIScreen* existingScreen in [UIScreen screens]) {
        if ((UIScreen*)screen == existingScreen) {
          __readAttributes_ios(screen, index, outAttr);
          return Bool_TRUE;
        }
        ++index;
      }
    }
    return Bool_FALSE;
  }
  
  // get handle/attributes of any monitor by index
  IosScreenHandle __getMonitorByIndex_ios(uint32_t index, struct MonitorAttributes_ios* outAttr) {
    if ([[UIScreen screens] count] > index) {
      UIScreen* screen = [[UIScreen screens] objectAtIndex:index];
      __readAttributes_ios((IosScreenHandle)screen, index, outAttr);
      return (IosScreenHandle)screen;
    }
    return (IosScreenHandle)NULL;
  }
  
  // ---
  
  // count all active monitors
  uint32_t __countMonitorHandles_ios() {
    @try {
      uint32_t displayCount = [[UIScreen screens] count];
      return displayCount;
    }
    @catch (NSException*) { return 0; }
  }
  
  
// -- display modes -- ---------------------------------------------------------

  // get current display resolution/depth/rate of a monitor
  Bool __getDisplayMode_ios(uint32_t index, struct DisplayMode_ios* out) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*) __getMonitorHandle_ios(index);
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
  Bool __setDisplayMode_ios(uint32_t index, const struct DisplayMode_ios* mode) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*) __getMonitorHandle_ios(index);
      if (screen != NULL && [[screen availableModes] count] > 0) {
        for (UIScreenMode* modeInfo in [screen availableModes]) {
          if (mode->width == modeInfo.size.width && mode->height == modeInfo.size.height) {
            screen->setCurrentMode(modeInfo);
            return Bool_TRUE;
          }
        }
      }
      return Bool_FALSE;
    }
  }
  
  // reset display resolution/depth/rate of a monitor to default values
  Bool __setDefaultDisplayMode_ios(uint32_t index) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*) __getMonitorHandle_ios(index);
      if (screen != NULL && [screen preferredMode]) {
        screen->setCurrentMode([screen preferredMode]);
        return Bool_TRUE;
      }
      return Bool_FALSE;
    }
  }
  
  // ---
  
  // list all display modes of a monitor
  Bool __listDisplayModes_ios(uint32_t index, struct DisplayMode_ios** outModes, uint32_t* outLength) {
    @autoreleasepool {
      UIScreen* screen = (UIScreen*) __getMonitorHandle_ios(index);
      if (screen != NULL && [[screen availableModes] count] > 0) {
        *outLength = (uint32_t)[[screen availableModes] count];
        *outModes = (struct DisplayMode_ios*)calloc(*outLength, sizeof(struct DisplayMode_ios));
        if (*outModes == NULL) {
          *outLength = 0;
          return Bool_FALSE;
        }
        
        uint32_t refreshRate = (uint32_t)((double)[screen maximumFramesPerSecond] * 1000.0 + 0.500001);
        uint32_t index = 0;
        for (UIScreenMode* modeInfo in [screen availableModes]) {
          outModes[index].width = modeInfo.size.width;
          outModes[index].height = modeInfo.size.height;
          outModes[index].bitDepth = 32;
          outModes[index].refreshRate = refreshRate;
          ++index;
        }
        return Bool_TRUE;
      }
      return Bool_FALSE;
    }
  }
  
  
// -- metrics -- ---------------------------------------------------------------
  
  // client area to window area (DPI adjusted)
  void __clientAreaToWindowArea_ios(const struct DisplayArea_ios* clientArea, IosAppHandle windowHandle, 
                                    Bool hasStatusBar, struct DisplayArea_ios* outWindowArea) {
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
      uint32_t offset = (hasStatusBar) ? __P_DEFAULT_STATUS_BAR_SIZE : 0;
      if (clientArea->width <= clientArea->height) {
        outWindowArea->x = clientArea->x;
        outWindowArea->y = clientArea->y + offset;
        outWindowArea->width = clientArea->width;
        outWindowArea->height = clientArea->height - offset;
      }
      else {
        outWindowArea->x = clientArea->x + offset;
        outWindowArea->y = clientArea->y;
        outWindowArea->width = clientArea->width - offset;
        outWindowArea->height = clientArea->height;
      }
    }
  }

#endif
