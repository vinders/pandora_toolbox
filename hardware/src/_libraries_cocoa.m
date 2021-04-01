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

# include <IOKit/graphics/IOGraphicsLib.h>
# include <ApplicationServices/ApplicationServices.h>

# define __P_LIBRARIES_COCOA_OBJC 1
# include "hardware/_private/_libraries_cocoa.h"

  // -- utilities --
  
  (Bool) LibrariesCocoa_readScreenDpi:(ScreenHandle)screen :(uint32_t*)outDpiX :(uint32_t*)outDpiY) {
    @autoreleasepool {
      if (!screen)
        return Bool_FALSE;
      
      @try {
        NSDictionary *description = [(NSScreen*)screen deviceDescription];
        NSSize pixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize physicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

        float dpi = static_cast<float>(pixelSize.width) / static_cast<float>(physicalSize.width))*25.4f;
        *outDpiX = *outDpiY = static_cast<uint32_t>(dpi + 0.5f); // round
        return Bool_TRUE;
      }
      @catch (NSException*) { return Bool_FALSE; }
    }
  }
  (Bool) LibrariesCocoa_readScreenScaling:(ScreenHandle)screen :(float*)outScaleX :(float*)outScaleY) {
    @autoreleasepool {
      if (!screen)
        return Bool_FALSE;

      @try {
        NSRect points = [(NSScreen*)screen frame];
        NSRect pixels = [(NSScreen*)screen convertRectToBacking:points];

        *outScaleX = static_cast<float>(pixels.size.width) / static_cast<float>(points.size.width);
        *outScaleY = static_cast<float>(pixels.size.height) / static_cast<float>(points.size.height);
        return Bool_TRUE;
      }
      @catch (NSException*) { return Bool_FALSE; }
    }
  }
  (Bool) LibrariesCocoa_refreshHandleForMonitor:(ScreenHandle*)outScreen :(uint32_t)unitNumber) {
    @try {
      for (NSScreen* screen in [NSScreen screens]) {
        NSNumber* displayNb = [screen deviceDescription][@"NSScreenNumber"];

        // use screen number instead of display ID -> that way, it also works with automatic graphics switching
        if (CGDisplayUnitNumber([displayNb unsignedIntValue]) == unitNumber) {
          *((NSScreen*)outScreen) = screen;
          return Bool_TRUE;
        }
      }
      return Bool_FALSE;
    }
    @catch (NSException*) { return Bool_FALSE; }
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
  - (void)applicationDidFinishLaunching:(NSNotification *)notification { __LibrariesCocoa_isAppLaunchFinished = Bool_TRUE; [NSApp stop:nil]; }
  - (void)applicationDidHide:(NSNotification *)notification { __restoreVideoMode(...); }
  @end
  */
  /*
  __LibrariesCocoa_delegate = [[ApplicationDelegate alloc] init];
  if (_glfw.ns.delegate == nil)
    return Bool_FALSE;
  [NSApp setDelegate:__LibrariesCocoa_delegate];
  */
  
  Bool __LibrariesCocoa_isInit = Bool_FALSE;
  LibrariesCocoa_data* __LibrariesCocoa_data = nil;
  
  (LibrariesCocoa_data*) LibrariesCocoa_getData {
    return __LibrariesCocoa_data;
  }
  
  // -- init --
  
  (Bool) LibrariesCocoa_init {
    @autoreleasepool {
      if (__LibrariesCocoa_isInit)
        return Bool_TRUE;

      @try {
        if (!__LibrariesCocoa_data) {
          __LibrariesCocoa_data = [[LibrariesCocoa_data alloc] init];
          __LibrariesCocoa_data.isAppReady = Bool_FALSE;
        }
        if (NSApp)
          __LibrariesCocoa_data.isAppReady = Bool_TRUE;
        [NSApplication sharedApplication];
      }
      @catch (NSException*) { return Bool_FALSE; }

      __LibrariesCocoa_isInit = Bool_TRUE;
      return Bool_TRUE;
    }
  }
  
  // -- shutdown --
  
  (void) LibrariesCocoa_shutdown {
    @autoreleasepool {
      if (__LibrariesCocoa_isInit) {
        [__LibrariesCocoa_data release];
        __LibrariesCocoa_data = nil;
        __LibrariesCocoa_isInit = Bool_FALSE;
      }
    }
  }
#endif
