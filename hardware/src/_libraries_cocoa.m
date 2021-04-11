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
# import <Cocoa/Cocoa.h>
# import <ApplicationServices/ApplicationServices.h>

# import "hardware/_private/_libraries_cocoa.h"

  // -- utilities --
  
  Bool LibrariesCocoa_readScreenDpi(CocoaScreenHandle screen, uint32_t* outDpiX, uint32_t* outDpiY) {
    @autoreleasepool {
      if (!screen)
        return Bool_FALSE;
      
      @try {
        NSDictionary *description = [(NSScreen*)screen deviceDescription];
        NSSize pixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize physicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

        float dpi = (static_cast<float>(pixelSize.width) / static_cast<float>(physicalSize.width))*25.4f;
        *outDpiX = *outDpiY = static_cast<uint32_t>(dpi + 0.5f); // round
        return Bool_TRUE;
      }
      @catch (NSException*) { return Bool_FALSE; }
    }
  }
  
  Bool LibrariesCocoa_readScreenScaling(CocoaScreenHandle screen, float* outScaleX, float* outScaleY) {
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


  // -- library management --
  
  CocoaLibraryData* __LibrariesCocoa_data = nil;
  
  Bool __LibrariesCocoa_init() {
    @autoreleasepool {
      @try {
        if (!__LibrariesCocoa_data) {
          __LibrariesCocoa_data = [[CocoaLibraryData alloc] init];
          __LibrariesCocoa_data.isAppReady = Bool_FALSE;
        }
        if (NSApp)
          __LibrariesCocoa_data.isAppReady = Bool_TRUE;
        [NSApplication sharedApplication];
      }
      @catch (NSException*) { return Bool_FALSE; }
      return Bool_TRUE;
    }
  }

  void __LibrariesCocoa_shutdown() {
    @autoreleasepool {
      [__LibrariesCocoa_data release];
      __LibrariesCocoa_data = nil;
    }
  }
  
  CocoaLibraryData* __LibrariesCocoa_getData() {
    return __LibrariesCocoa_data;
  }
#endif
