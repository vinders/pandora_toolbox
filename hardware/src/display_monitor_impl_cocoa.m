/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)

# include <stddef.h>
# include <string.h>
# import <Cocoa/Cocoa.h>
# import <ApplicationServices/ApplicationServices.h>
# if !defined(__aarch64__) && (defined(__x86_64__) || defined(__x86_64) || defined(__i686__) || defined(__i386__) || defined(__amd64__))
#   import <IOKit/graphics/IOGraphicsLib.h>
#   define __P_APPLE_IOKIT 1
# endif
# import "hardware/_private/_display_monitor_impl_cocoa.h"


// -- monitor attributes - id/area/description/primary -- ----------------------
  
# ifdef __P_APPLE_IOKIT
    // read brand/description string of monitor (intel compatible)
    static char* __readDeviceDescription_iokit(CGDirectDisplayID displayID) {
      io_service_t service;
      CFDictionaryRef dictionary;
      
      io_iterator_t it;
      if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IODisplayConnect"), &it) == 0)
        return NULL; // may happen if headless system
      
      char* monitorName = NULL;
      while ((service = IOIteratorNext(it)) != 0) {
        dictionary = IODisplayCreateInfoDictionary(service, kIODisplayOnlyPreferredName);
        
        CFNumberRef vendorIdRef = CFDictionaryGetValue(dictionary, CFSTR(kDisplayVendorID));
        CFNumberRef productIdRef = CFDictionaryGetValue(dictionary, CFSTR(kDisplayProductID));
        if (vendorIdRef && productIdRef) {
          unsigned int vendorId = 0, productId = 0;
          CFNumberGetValue(vendorIdRef, kCFNumberIntType, &vendorId);
          CFNumberGetValue(productIdRef, kCFNumberIntType, &productId);
          if (CGDisplayVendorNumber(displayID) == vendorId && CGDisplayModelNumber(displayID) == productId) {
            
            CFDictionaryRef nameList = CFDictionaryGetValue(dictionary, CFSTR(kDisplayProductName));
            CFStringRef nameRef;
            if (nameList && CFDictionaryGetValueIfPresent(names, CFSTR("en_US"), (const void**)&nameRef)) {
              const CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(nameRef), kCFStringEncodingUTF8);
              monitorName = calloc(length + 1, sizeof(char));
              CFStringGetCString(nameRef, name, length, kCFStringEncodingUTF8);
            }
          }
        }
        CFRelease(dictionary);
      }
      IOObjectRelease(it);
      return monitorName;
    }
# endif
  
  // read brand/description string of monitor
  char* __readDeviceDescription_cocoa(CocoaScreenHandle screen, CocoaDisplayId displayId) {
    // Apple Silicon doesn't support IOKit -> use selector if supported
    if (screen && [(NSScreen*)screen respondsToSelector:@selector(localizedName)]) {
      NSString* name = [(NSScreen*)screen valueForKey:@"localizedName"];
      if (name)
        return strdup([name UTF8String]);
    }
#   ifdef __P_APPLE_IOKIT
      return __readDeviceDescription_iokit((CGDirectDisplayID)displayId); // IOKit
#   else
      return NULL;
#   endif
  }
  
  // read screen area of a monitor (screen position/size + work area)
  void __readScreenArea_cocoa(CocoaScreenHandle screen, CocoaDisplayId displayId, struct DisplayArea_cocoa* outScreenArea, struct DisplayArea_cocoa* outWorkArea) {
    @autoreleasepool {
      NSDictionary *description = [(NSScreen*)screen deviceDescription];
      NSSize screenSize = [[description objectForKey:NSDeviceSize] sizeValue];
      
      const CGRect screenBounds = CGDisplayBounds((CGDirectDisplayID)displayId);
      outScreenArea->x = screenBounds.origin.x;
      outScreenArea->y = screenBounds.origin.y;
      outScreenArea->width = screenSize.width;
      outScreenArea->height = screenSize.height;
      
      const NSRect frameRect = [(NSScreen*)screen visibleFrame];
      outWorkArea->x = frameRect.origin.x;
      outWorkArea->y = CGDisplayBounds(CGMainDisplayID()).size.height - (frameRect.origin.y + frameRect.size.height);
      outWorkArea->width = frameRect.size.width;
      outWorkArea->height = frameRect.size.height;
    }
  }
  
  // ---
  
  // read all attributes of a monitor handle
  void __readAttributes_cocoa(CocoaScreenHandle screen, Bool checkPrimary, uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr) {
    NSNumber* screenNumber = [(NSScreen*)screen deviceDescription][@"NSScreenNumber"];
    *outUnitNumber = CGDisplayUnitNumber([screenNumber unsignedIntValue]);
    
    outAttr->id = [screenNumber unsignedIntValue];
    outAttr->description = __readDeviceDescription_cocoa(screen, outAttr->id);
    __readScreenArea_cocoa(screen, outAttr->id, &(outAttr->screenArea), &(outAttr->workArea));
    
    if (checkPrimary && [[NSScreen screens] count] > 0) {
      NSScreen* primaryScreen = [[NSScreen screens] objectAtIndex:0];
      uint32_t primaryUnit = CGDisplayUnitNumber([[primaryScreen deviceDescription][@"NSScreenNumber"] unsignedIntValue]);
      outAttr->isPrimary = (primaryUnit == *outUnitNumber) ? Bool_TRUE : Bool_FALSE;
    }
  }
  
  
// -- get display monitors (handle + attributes) -- ----------------------------
  
  // get screen handle based on unit number (fix handle in case of automatic graphics switching)
  CocoaScreenHandle __getMonitorHandle_cocoa(uint32_t unitNumber) {
    @try {
      for (NSScreen* screen in [NSScreen screens]) {
        NSNumber* displayNb = [screen deviceDescription][@"NSScreenNumber"];
        if (CGDisplayUnitNumber([displayNb unsignedIntValue]) == unitNumber)
          return (CocoaScreenHandle)screen;
      }
      return (CocoaScreenHandle)NULL;
    }
    @catch (NSException*) { return (CocoaScreenHandle)NULL; }
  }
  
  // read handle/attributes of primary/default monitor
  CocoaScreenHandle __getPrimaryMonitor_cocoa(uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr) {
    NSScreen* primaryScreen = NULL;
    
    if ([[NSScreen screens] count] > 0) {
      primaryScreen = [[NSScreen screens] objectAtIndex:0];
      __readAttributes_cocoa((CocoaScreenHandle)primaryScreen, Bool_FALSE, outUnitNumber, outAttr);
    }
    outAttr->isPrimary = Bool_TRUE;
    return (CocoaScreenHandle)primaryScreen; 
  }
  
  // get attributes of a monitor handle
  Bool __getMonitor_cocoa(CocoaScreenHandle screen, uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr) {
    if (screen) {
      for (NSScreen* existingScreen in [NSScreen screens]) {
        if ((NSScreen*)screen == existingScreen) {
          __readAttributes_cocoa(screen, Bool_TRUE, outUnitNumber, outAttr);
          return Bool_TRUE;
        }
      }
    }
    return Bool_FALSE;
  }
  
  // get handle/attributes of any monitor by ID
  CocoaScreenHandle __getMonitorById_cocoa(CocoaDisplayId displayId, uint32_t* outUnitNumber, struct MonitorAttributes_cocoa* outAttr) {
    const uint32_t unitNumber = CGDisplayUnitNumber((CGDirectDisplayID)displayId);
    
    uint32_t screenIndex = 0;
    NSScreen* screen = NULL;
    for (screen in [NSScreen screens]) {
      // use unit number to support automatic graphics switching
      NSNumber* screenNumber = [screen deviceDescription][@"NSScreenNumber"];
      if (CGDisplayUnitNumber([screenNumber unsignedIntValue]) == unitNumber) { 
        *outUnitNumber = unitNumber;
        
        outAttr->id = displayId;
        outAttr->description = __readDeviceDescription_cocoa(screen, outAttr->displayId);
        __readScreenArea_cocoa(screen, outAttr->displayId, &(outAttr->screenArea), &(outAttr->workArea));
        outAttr->isPrimary = (screenIndex == 0) ? Bool_TRUE : Bool_FALSE;
        return (CocoaScreenHandle)screen;
      }
      ++screenIndex;
    }
    return (CocoaScreenHandle)NULL; 
  }
  
  // list IDs of all active monitors
  Bool __listMonitorIds_cocoa(CocoaDisplayId** outList, uint32_t* outLength) {
    uint32_t displayCount = 0;
    if (CGGetOnlineDisplayList(0, NULL, &displayCount) != CGError.success)
      return Bool_FALSE;
    if (displayCount == 0) {
      *outLength = 0;
      return Bool_TRUE;
    }
    
    CGDirectDisplayID* displays = calloc(displayCount, sizeof(CGDirectDisplayID));
    if (displays) {
      if (CGGetOnlineDisplayList(displayCount, displays, &displayCount) == CGError.success) {
        *outList = (CocoaDisplayId*)displays;
        *outLength = displayCount;
        return Bool_TRUE;
      }
      else
        free(displays);
    }
    return Bool_FALSE;
  }
  // get ID at position in list
  CocoaDisplayId __getMonitorIdFromList_cocoa(CocoaDisplayId* list, uint32_t index) {
    return ((CGDirectDisplayID*)list)[index];
  }
  
  
// -- display modes -- ---------------------------------------------------------
  
  // read display resolution/depth/rate of a monitor
  Bool __getDisplayMode_cocoa(CocoaDisplayId displayId, struct DisplayMode_cocoa* out) {
    @autoreleasepool {
      CGDisplayModeRef modeRef = CGDisplayCopyDisplayMode((CGDirectDisplayID)displayId);

      out->width = (int32_t)CGDisplayModeGetWidth(modeRef);
      out->height = (int32_t)CGDisplayModeGetHeight(modeRef);
      
      out->bitDepth = 32;
#     if defined(MAC_OS_X_VERSION_MAX_ALLOWED) && MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
      CFStringRef pxFormat = CGDisplayModeCopyPixelEncoding(modeRef);
      if (CFStringCompare(pxFormat, CFSTR(IO16BitDirectPixels), 0) == 0)
        out->bitDepth = 16;
      CFRelease(pxFormat);
#     endif

      out->refreshRate = (uint32_t)CGDisplayModeGetRefreshRate(modeRef);
      //if (out->refreshRate == 0)
        //out->refreshRate = (uint32_t)getFallbackRefreshRate(displayID);

      CGDisplayModeRelease(modeRef);
      return Bool_FALSE;
    }
  }
  
  // set display resolution/depth/rate of a monitor
  Bool __setDisplayMode_cocoa(CocoaDisplayId displayId, const struct DisplayMode_cocoa* mode) {
    @autoreleasepool {
      return Bool_FALSE;
    }
  }
  // reset display resolution/depth/rate of a monitor to default values
  Bool __setDefaultDisplayMode_cocoa(CocoaDisplayId displayId) {
    @autoreleasepool {
      return Bool_FALSE;
    }
  }
  
  // ---
  
  // list all display modes of a monitor
  void __listDisplayModes_cocoa(CocoaDisplayId displayId, struct DisplayMode_cocoa** outModes, uint32_t* outLength) {
    @autoreleasepool {
      *outLength = 0;
    }
  }
  
  
// -- metrics -- ---------------------------------------------------------------
  
  // client area to window area (DPI adjusted)
  void __clientAreaToWindowArea_cocoa(CocoaScreenHandle screen, const struct DisplayArea_cocoa* clientArea, CocoaWindowHandle windowHandle, 
                                      Bool hasMenu, uint32_t hasBorders, uint32_t hasCaption, struct DisplayArea_cocoa* outWindowArea) {
    //TODO
    outWindowArea.x = clientArea.x;
    outWindowArea.y = clientArea.y;
    outWindowArea.width = clientArea.width;
    outWindowArea.height = clientArea.height;
  }

#endif
