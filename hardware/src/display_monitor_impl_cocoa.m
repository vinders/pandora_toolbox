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
            if (nameList && CFDictionaryGetValueIfPresent(nameList, CFSTR("en_US"), (const void**)&nameRef)) {
              const CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(nameRef), kCFStringEncodingUTF8);
              monitorName = calloc(length + 1, sizeof(char));
              CFStringGetCString(nameRef, monitorName, length, kCFStringEncodingUTF8);
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
  CocoaScreenHandle __getMonitorHandle_cocoa(uint32_t unitNumber, CocoaDisplayId* outId) {
    @try {
      for (NSScreen* screen in [NSScreen screens]) {
        NSNumber* displayNb = [screen deviceDescription][@"NSScreenNumber"];
        if (CGDisplayUnitNumber([displayNb unsignedIntValue]) == unitNumber) {
          *outId = [displayNb unsignedIntValue];
          return (CocoaScreenHandle)screen;
        }
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
    *outUnitNumber = (uint32_t)CGDisplayUnitNumber((CGDirectDisplayID)displayId);
    return __getMonitorByUnit_cocoa(*outUnitNumber, outAttr);
  }
  
  // get handle/attributes of any monitor by unit number
  CocoaScreenHandle __getMonitorByUnit_cocoa(uint32_t unitNumber, struct MonitorAttributes_cocoa* outAttr) {
    uint32_t screenIndex = 0;
    NSScreen* screen = NULL;
    for (screen in [NSScreen screens]) {
      // use unit number to support automatic graphics switching
      NSNumber* screenNumber = [screen deviceDescription][@"NSScreenNumber"];
      if (CGDisplayUnitNumber([screenNumber unsignedIntValue]) == unitNumber) { 
        outAttr->id = (CocoaDisplayId)[screenNumber unsignedIntValue];
        outAttr->description = __readDeviceDescription_cocoa(screen, outAttr->id);
        __readScreenArea_cocoa(screen, outAttr->id, &(outAttr->screenArea), &(outAttr->workArea));
        outAttr->isPrimary = (screenIndex == 0) ? Bool_TRUE : Bool_FALSE;
        return (CocoaScreenHandle)screen;
      }
      ++screenIndex;
    }
    return (CocoaScreenHandle)NULL;
  }
  
  // ---
  
  // list IDs of all active monitors
  Bool __listMonitorIds_cocoa(CocoaDisplayId** outList, uint32_t* outLength) {
    uint32_t displayCount = 0;
    if (CGGetOnlineDisplayList(0, NULL, &displayCount) != kCGErrorSuccess)
      return Bool_FALSE;
    if (displayCount == 0) {
      *outLength = 0;
      return Bool_TRUE;
    }
    
    CGDirectDisplayID* displays = calloc(displayCount, sizeof(CGDirectDisplayID));
    if (displays) {
      if (CGGetOnlineDisplayList(displayCount, displays, &displayCount) == kCGErrorSuccess) {
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

# ifdef __P_APPLE_IOKIT
    // read refresh rate from IORegistry (if not readable from ModeRef)
    static double __readRefreshRate__ioreg(CGDirectDisplayID displayId) {
      double refreshRate = 0.0; // undefinedRefreshRate
      
      io_iterator_t it;
      if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOFramebuffer"), &it) == 0) {
        io_service_t service;
        while ((service = IOIteratorNext(it)) != 0) {
          CFNumberRef indexRef = IORegistryEntryCreateCFProperty(service, CFSTR("IOFramebufferOpenGLIndex"), 
                                                                 kCFAllocatorDefault, kNilOptions);
          if (indexRef) {
            uint32_t index = 0;
            CFNumberGetValue(indexRef, kCFNumberIntType, &index);
            CFRelease(indexRef);
            
            if (CGOpenGLDisplayMaskToDisplayID(1 << index) == displayId) {
              uint32_t clock = 0;
              CFNumberRef clockRef = IORegistryEntryCreateCFProperty(service, CFSTR("IOFBCurrentPixelClock"),
                                                                     kCFAllocatorDefault, kNilOptions);
              if (clockRef) {
                CFNumberGetValue(clockRef, kCFNumberIntType, &clock);
                CFRelease(clockRef);
              }

              uint32_t count = 0;
              CFNumberRef countRef = IORegistryEntryCreateCFProperty(service, CFSTR("IOFBCurrentPixelCount"),
                                                                     kCFAllocatorDefault, kNilOptions);
              if (countRef) {
                CFNumberGetValue(countRef, kCFNumberIntType, &count);
                CFRelease(countRef);
              }

              if (clock > 0 && count > 0)
                refreshRate = (double)clock / (double)count;
              break;
            }
          }
        }
        IOObjectRelease(it);
      }
      return refreshRate;
    }
# endif

  // extract display resolution/depth/rate from native display mode
  static void __readFromCGDisplayMode(CocoaDisplayId displayId, CGDisplayModeRef modeRef, struct DisplayMode_cocoa* out) {
    out->width = (int32_t)CGDisplayModeGetWidth(modeRef);
    out->height = (int32_t)CGDisplayModeGetHeight(modeRef);
    
    out->bitDepth = 32;
#   if defined(MAC_OS_X_VERSION_MAX_ALLOWED) && MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
      CFStringRef pxFormat = CGDisplayModeCopyPixelEncoding(modeRef);
      if (CFStringCompare(pxFormat, CFSTR(IO16BitDirectPixels), 0) == 0)
        out->bitDepth = 16;
      CFRelease(pxFormat);
#   endif

    out->refreshRate = (uint32_t)CGDisplayModeGetRefreshRate(modeRef);
#   ifdef __P_APPLE_IOKIT
      if (out->refreshRate == 0)
        out->refreshRate = (uint32_t)__readRefreshRate__ioreg(displayId); // floor -> distinguish 59.94 from 60
#   endif
  }
  
  // verify display mode validity
  static Bool __isDisplayModeValid_cocoa(CGDisplayModeRef modeRef) {
    uint32_t flags = CGDisplayModeGetIOFlags(modeRef);
    if ( (flags & (kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeInterlacedFlag | kDisplayModeStretchedFlag)) 
         != (kDisplayModeValidFlag | kDisplayModeSafeFlag) ) {
      return Bool_FALSE;
    }
    
#   if defined(MAC_OS_X_VERSION_MAX_ALLOWED) && MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
      Bool isValid = Bool_TRUE;
      CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);
      if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) != 0 && CFStringCompare(format, CFSTR(IO32BitDirectPixels), 0) != 0)
        isValid = Bool_FALSE;
      
      CFRelease(format);
      return isValid;
#   else
      return Bool_TRUE;
#   endif
  }
  
  // find cocoa display mode reference
  static CGDisplayModeRef __findCGDisplayModeRef(CFArrayRef allModes, CocoaDisplayId displayId, const struct DisplayMode_cocoa* targetMode) {
    CGDisplayModeRef result = NULL;

    CFIndex modesCount = CFArrayGetCount(allModes);
    for (CFIndex i = 0; i < modesCount; ++i) {
      CGDisplayModeRef modeRef = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
      
      // if mode is valid, compare width/height
      if (__isDisplayModeValid_cocoa(modeRef) && targetMode->width == (int32_t)CGDisplayModeGetWidth(modeRef)
                                              && targetMode->height == (int32_t)CGDisplayModeGetHeight(modeRef)) {
        // compare bit depth
        uint32_t bitDepth = 32;
#       if defined(MAC_OS_X_VERSION_MAX_ALLOWED) && MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
          CFStringRef pxFormat = CGDisplayModeCopyPixelEncoding(modeRef);
          if (CFStringCompare(pxFormat, CFSTR(IO16BitDirectPixels), 0) == 0)
            bitDepth = 16;
          CFRelease(pxFormat);
#       endif
        if (targetMode->bitDepth != bitDepth)
          continue;
        
        // if rate is not undefined, compare it
        if (targetMode->refreshRate > 0) {
          uint32_t refreshRate = (uint32_t)CGDisplayModeGetRefreshRate(modeRef);
#         ifdef __P_APPLE_IOKIT
          if (refreshRate == 0)
            refreshRate = (uint32_t)__readRefreshRate__ioreg(displayId); // floor -> distinguish 59.94 from 60
#         endif
          if (refreshRate > 0 && targetMode->refreshRate != refreshRate)
            continue;
        }
        
        result = modeRef;
        break;
      }
    }
    return result;
  }
  
  // apply cocoa display mode on target unit
  static Bool __applyDisplayModeWithFade_cocoa(CocoaDisplayId displayId, CGDisplayModeRef mode) {
    if (mode == NULL)
      return Bool_FALSE;
    
    // reserve fade effect + fade in
    CGDisplayFadeReservationToken fadeToken = kCGDisplayFadeReservationInvalidToken;
    if (CGAcquireDisplayFadeReservation(5, &fadeToken) == kCGErrorSuccess)
      CGDisplayFade(fadeToken, 0.3, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, TRUE);
    
    // change display mode
    Bool isSuccess = (CGDisplaySetDisplayMode(displayId, mode, NULL) == kCGErrorSuccess);
    
    // fade out + release fade effect
    if (fadeToken != kCGDisplayFadeReservationInvalidToken) {
      CGDisplayFade(fadeToken, 0.5, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
      CGReleaseDisplayFadeReservation(fadeToken);
    }
    return isSuccess;
  }

  // ---
  
  // get current display resolution/depth/rate of a monitor
  Bool __getDisplayMode_cocoa(CocoaDisplayId displayId, struct DisplayMode_cocoa* out) {
    @autoreleasepool {
      CGDisplayModeRef modeRef = CGDisplayCopyDisplayMode((CGDirectDisplayID)displayId);
      if (modeRef == NULL)
        return Bool_FALSE;
      
      __readFromCGDisplayMode(displayId, modeRef, out);
      
      CGDisplayModeRelease(modeRef);
      return Bool_TRUE;
    }
  }
  
  // set display resolution/depth/rate of a monitor
  Bool __setDisplayMode_cocoa(CocoaDisplayId displayId, uint32_t unitNumber, const struct DisplayMode_cocoa* targetMode) {
    @autoreleasepool {
      // if no backup exists, pre-allocate new entry (on alloc failure, exit)
      struct DisplayModeBackup* backupEntry = NULL;
      if (__LibrariesCocoa_findOriginalMode(unitNumber) == NULL) {
        backupEntry = __LibrariesCocoa_createDetachedOriginalMode(unitNumber, (CocoaDisplayModeRef)CGDisplayCopyDisplayMode((CGDirectDisplayID)displayId));
        if (backupEntry == NULL)
          return Bool_FALSE;
      }

      // find + apply display mode
      CFArrayRef allModes = CGDisplayCopyAllDisplayModes(displayId, NULL);
      if (allModes != NULL) {
        Bool isSuccess = Bool_FALSE;
        
        CGDisplayModeRef newModeRef = __findCGDisplayModeRef(allModes, displayId, targetMode);
        if (newModeRef != NULL) {
          isSuccess = __applyDisplayModeWithFade_cocoa(displayId, newModeRef);
          if (isSuccess && backupEntry != NULL) // store backup mode if not yet stored (to allow __setDefaultDisplayMode_cocoa)
            __LibrariesCocoa_appendOriginalMode(backupEntry);
        }
        CFRelease(allModes);
        return isSuccess;
      }
      
      if (backupEntry != NULL) { // error -> release data
        CGDisplayModeRelease((CGDisplayModeRef)backupEntry->mode);
        free(backupEntry);
      }
      return Bool_FALSE;
    }
  }
  
  // reset display resolution/depth/rate of a monitor to default values
  Bool __setDefaultDisplayMode_cocoa(CocoaDisplayId displayId, uint32_t unitNumber) {
    @autoreleasepool {
      // find entry in backup list
      struct DisplayModeBackup* backupEntry = __LibrariesCocoa_findOriginalMode(unitNumber);
      if (backupEntry != NULL) {
        if (__applyDisplayModeWithFade_cocoa(displayId, backupEntry->mode)) { // restore
          __LibrariesCocoa_eraseOriginalMode(unitNumber); // on success, remove entry from backup list
          return Bool_TRUE;
        }
      }
      return Bool_FALSE; // not found -> no previous display change (already original mode)
    }
  }
  
  // ---
  
  // list all display modes of a monitor
  Bool __listDisplayModes_cocoa(CocoaDisplayId displayId, struct DisplayMode_cocoa** outModes, uint32_t* outLength) {
    @autoreleasepool {
      CFArrayRef modes = CGDisplayCopyAllDisplayModes(displayId, NULL);
      if (modes == NULL) {
        *outLength = 0;
        return Bool_FALSE;
      }

      CFIndex modesCount = CFArrayGetCount(modes);
      *outLength = (uint32_t)modesCount;
      *outModes = calloc(*outLength, sizeof(struct DisplayMode_cocoa));
      if (*outModes == NULL) {
        *outLength = 0;
        return Bool_FALSE;
      }
      
      for (CFIndex i = 0, curIndex = 0; i < modesCount; ++i, ++curIndex) {
        CGDisplayModeRef modeRef = (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, i);
        if (__isDisplayModeValid_cocoa(modeRef)) {
          __readFromCGDisplayMode(displayId, modeRef, &(*outModes)[(uint32_t)curIndex]);
          
          // check if identical mode already listed
          for (CFIndex prev = 0; prev < curIndex; ++prev) {
            struct DisplayMode_cocoa* prevMode = &(*outModes)[(uint32_t)prev];
            struct DisplayMode_cocoa* curMode = &(*outModes)[(uint32_t)curIndex];
            
            if (prevMode->width == curMode->width && prevMode->height == curMode->height 
            && prevMode->bitDepth == curMode->bitDepth && prevMode->refreshRate == curMode->refreshRate) {
              --curIndex;
              *outLength = (*outLength) - 1;
              break;
            }
          }
        }
      }
      CFRelease(modes);
      return Bool_TRUE;
    }
  }
  
  
// -- metrics -- ---------------------------------------------------------------
  
  // client area to window area (DPI adjusted)
  void __clientAreaToWindowArea_cocoa(CocoaScreenHandle screen, const struct DisplayArea_cocoa* clientArea, CocoaWindowHandle windowHandle, 
                                      Bool hasMenu, uint32_t hasBorders, uint32_t hasCaption, struct DisplayArea_cocoa* outWindowArea) {
    // get window size
    if (windowHandle) {
      NSSize windowSize = [[(NSWindow*)windowHandle contentView] frame].size;
      
      outWindowArea->width = windowSize.width;
      if (outWindowArea->width < clientArea->width)
        outWindowArea->width = clientArea->width;
      
      outWindowArea->height = windowSize.height;
      if (outWindowArea->height < clientArea->height)
        outWindowArea->height = clientArea->height;
    }
    else { // no handle -> guess value from flags
      // "default" arbitrary values
      uint32_t borderSize = (hasBorders) ? 1 : 0;
      uint32_t captionSize = (hasCaption) ? 44 : 0;
      if (hasMenu)
        captionSize += 46;
      
      NSWindow* parent = [[NSApplication sharedApplication] mainWindow];
      if (parent != NULL) { // try to read actual values from main window, if available
        if (hasBorders) {
          int32_t decorationX = parent.frame.size.width - [parent contentRectForFrameRect: parent.frame].size.width;
          if (decorationX > 0)
            borderSize = (uint32_t)(decorationX) >> 1;
        }
        if (hasCaption) {
          int32_t decorationY = parent.frame.size.height - [parent contentRectForFrameRect: parent.frame].size.height;
          if (decorationY > borderSize)
            captionSize = (uint32_t)(decorationY - borderSize);
        }
      }
      
      outWindowArea->width = clientArea->width + (borderSize << 1);
      outWindowArea->height = (hasCaption) ? clientArea->height + captionSize + borderSize : clientArea->height + (borderSize << 1);
    }
    
    // update position
    outWindowArea->x = clientArea->x - (int32_t)((outWindowArea->width - clientArea->width) >> 1);
    outWindowArea->y = clientArea->y - (int32_t)((outWindowArea->height - clientArea->height) >> 1);
  }

#endif
