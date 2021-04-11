/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <stdint.h>
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

        float dpi = ((float)pixelSize.width / (float)physicalSize.width)*25.4f;
        *outDpiX = *outDpiY = (uint32_t)(dpi + 0.5001f); // round
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

        *outScaleX = (float)pixels.size.width / (float)points.size.width;
        *outScaleY = (float)pixels.size.height / (float)points.size.height;
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
  
  struct CocoaLibraryData* __LibrariesCocoa_data = NULL;
  
  Bool __LibrariesCocoa_init() {
    @autoreleasepool {
      if (__LibrariesCocoa_data != NULL)
        return Bool_TRUE;
    
      @try {
        if (!__LibrariesCocoa_data) {
          __LibrariesCocoa_data = malloc(sizeof(struct CocoaLibraryData));
          if (__LibrariesCocoa_data == NULL)
            return Bool_FALSE;
            
          __LibrariesCocoa_data->isAppReady = Bool_FALSE;
        }
        if (NSApp)
          __LibrariesCocoa_data->isAppReady = Bool_TRUE;
        [NSApplication sharedApplication];
      }
      @catch (NSException*) { return Bool_FALSE; }
      return Bool_TRUE;
    }
  }

  void __LibrariesCocoa_shutdown() {
    @autoreleasepool {
      if (__LibrariesCocoa_data != NULL) {
        free(__LibrariesCocoa_data);
        __LibrariesCocoa_data = NULL;
      }
    }
  }
  
  struct CocoaLibraryData* __LibrariesCocoa_getData() {
    return __LibrariesCocoa_data;
  }
#endif
