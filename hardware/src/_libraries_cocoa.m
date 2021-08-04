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

        double dpi = ((double)pixelSize.width / (double)physicalSize.width)*25.4;
        *outDpiX = *outDpiY = (uint32_t)(dpi + 0.5000001); // round
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
          __LibrariesCocoa_data->originalModes = NULL;
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
        // deallocate "original mode" linked-list
        struct DisplayModeBackup* modeIt = __LibrariesCocoa_data->originalModes;
        while (modeIt != NULL) {
          struct DisplayModeBackup* next = modeIt->next;
          if (modeIt->mode)
            CGDisplayModeRelease(modeIt->mode);
          
          free(modeIt);
          modeIt = next;
        }
        
        free(__LibrariesCocoa_data);
        __LibrariesCocoa_data = NULL;
      }
    }
  }
  
  // -- library data --
  
  struct CocoaLibraryData* __LibrariesCocoa_getData() {
    return __LibrariesCocoa_data;
  }
  
  struct DisplayModeBackup* __LibrariesCocoa_findOriginalMode(uint32_t unitNumber) {
    struct DisplayModeBackup* entry = (__LibrariesCocoa_data) ? __LibrariesCocoa_data->originalModes : NULL;
    while (entry != NULL && entry->unitNumber != unitNumber)
      entry = entry->next;
    return entry;
  }
  
  struct DisplayModeBackup* __LibrariesCocoa_createDetachedOriginalMode(uint32_t unitNumber, CocoaDisplayModeRef mode) {
    struct DisplayModeBackup* entry = NULL;
    if (mode) {
      entry = calloc(1, sizeof(struct DisplayModeBackup));
      if (entry != NULL) {
        entry->unitNumber = unitNumber;
        entry->mode = mode;
        entry->next = NULL;
      }
      else
        CGDisplayModeRelease((CGDisplayModeRef)mode);
    }
    return entry;
  }
  
  void __LibrariesCocoa_appendOriginalMode(struct DisplayModeBackup* newEntry) {
    if (__LibrariesCocoa_data != NULL) {
      if (__LibrariesCocoa_data->originalModes != NULL) { // existing entries -> add after last
        struct DisplayModeBackup* parent = __LibrariesCocoa_data->originalModes;
        while (parent->next != NULL)
          parent = parent->next;
        parent->next = newEntry;
      }
      else // empty -> add first entry
        __LibrariesCocoa_getData()->originalModes = newEntry;
    }
  }
  
  void __LibrariesCocoa_eraseOriginalMode(uint32_t unitNumber) {
    struct DisplayModeBackup* parent = NULL;
    struct DisplayModeBackup* entry = (__LibrariesCocoa_data) ? __LibrariesCocoa_data->originalModes : NULL;
    while (entry != NULL && entry->unitNumber != unitNumber) {
      parent = entry;
      entry = entry->next;
    }
    
    if (entry != NULL) {
      if (parent != NULL)
        parent->next = entry->next;
      else
        __LibrariesCocoa_data->originalModes = entry->next;
      
      if (entry->mode)
        CGDisplayModeRelease((CGDisplayModeRef)entry->mode);
      free(entry);
    }
  }
#endif
