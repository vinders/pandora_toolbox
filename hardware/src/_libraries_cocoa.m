/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <stdlib.h>
# include <string.h>
# include <Carbon/Carbon.h>
# import <Cocoa/Cocoa.h>

#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>

# define __P_LIBRARIES_COCOA_OBJC 1
# include "hardware/_private/_libraries_cocoa.h"

  // -- utilities --
  
  (bool) LibrariesCocoa_readScreenDpi:(NSScreen*)screen :(uint32_t*)outDpiX :(uint32_t*)outDpiY) {
    @autoreleasepool {
      if (!screen)
        return false;
      
      @try {
        NSDictionary *description = [screen deviceDescription];
        NSSize pixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize physicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

        float dpi = static_cast<float>(pixelSize.width) / static_cast<float>(physicalSize.width))*25.4f;
        *outDpiX = *outDpiY = static_cast<uint32_t>(dpi + 0.5f); // round
        return true;
      }
      @catch (NSException*) { return false; }
    }
  }
  (bool) LibrariesCocoa_readScreenScaling:(NSScreen*)screen :(float*)outScaleX :(float*)outScaleY) {
    @autoreleasepool {
      if (!screen)
        return false;

      @try {
        NSRect points = [screen frame];
        NSRect pixels = [screen convertRectToBacking:points];

        *outScaleX = static_cast<float>(pixels.size.width) / static_cast<float>(points.size.width);
        *outScaleY = static_cast<float>(pixels.size.height) / static_cast<float>(points.size.height);
        return true;
      }
      @catch (NSException*) { return false; }
    }
  }
  (bool) LibrariesCocoa_refreshHandleForMonitor:(NSScreen**)outScreen :(uint32_t)unitNumber) {
    @try {
      for (NSScreen* screen in [NSScreen screens]) {
        NSNumber* displayNb = [screen deviceDescription][@"NSScreenNumber"];

        // use screen number instead of display ID -> that way, it also works with automatic graphics switching
        if (CGDisplayUnitNumber([displayNb unsignedIntValue]) == unitNumber) {
          *outScreen = screen;
          return true;
        }
      }
      return false;
    }
    @catch (NSException*) { return false; }
  }
  
  // ---------------------------------------------------------------------------
  
  //TODO: 
  /*
  @interface ApplicationDelegate : NSObject <NSApplicationDelegate>
  @end
  @implementation GLFWApplicationDelegate
  - (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender { __closeRequest(); return NSTerminateCancel; }
  - (void)applicationDidChangeScreenParameters:(NSNotification *) notification { __refreshMonitors(); }
  - (void)applicationWillFinishLaunching:(NSNotification *)notification { __manageMenus(); }
  - (void)applicationDidFinishLaunching:(NSNotification *)notification { __LibrariesCocoa_isAppLaunchFinished = true; [NSApp stop:nil]; }
  - (void)applicationDidHide:(NSNotification *)notification { __restoreVideoMode(...); }
  @end
  */
  /*
  __LibrariesCocoa_delegate = [[ApplicationDelegate alloc] init];
  if (_glfw.ns.delegate == nil)
    return false;
  [NSApp setDelegate:__LibrariesCocoa_delegate];
  */
  
  bool __LibrariesCocoa_isInit = false;
  LibrariesCocoa_data* __LibrariesCocoa_data = nil;
  
  (LibrariesCocoa_data*) LibrariesCocoa_getData {
    return __LibrariesCocoa_data;
  }
  
  // -- init --
  
  (bool) LibrariesCocoa_init {
    @autoreleasepool {
      if (__LibrariesCocoa_isInit)
        return true;

      @try {
        if (!__LibrariesCocoa_data) {
          __LibrariesCocoa_data = [[LibrariesCocoa_data alloc] init];
          __LibrariesCocoa_data.isAppReady = false;
        }
        if (NSApp)
          __LibrariesCocoa_data.isAppReady = true;
        [NSApplication sharedApplication];
      }
      @catch (NSException*) { return false; }

      __LibrariesCocoa_isInit = true;
      return true;
    }
  }
  
  // -- shutdown --
  
  (void) LibrariesCocoa_shutdown {
    @autoreleasepool {
      if (__LibrariesCocoa_isInit) {
        [__LibrariesCocoa_data release];
        __LibrariesCocoa_data = nil;
        __LibrariesCocoa_isInit = false;
      }
    }
  }
#endif
