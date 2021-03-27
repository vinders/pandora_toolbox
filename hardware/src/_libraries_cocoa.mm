/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <cstdlib>
# include <cstring>
# include "hardware/_private/_libraries_cocoa.h"

  pandora::hardware::LibrariesCocoa pandora::hardware::LibrariesCocoa::_libs{};

  using namespace pandora::hardware;


  // -- utilities --
  
  bool LibrariesCocoa::readScreenDpi(NSScreen* screen, uint32_t& outDpiX, uint32_t& outDpiY) noexcept {
    @autoreleasepool {
      if (!screen)
        return false;
      
      @try {
        NSDictionary *description = [screen deviceDescription];
        NSSize pixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize physicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

        float dpi = static_cast<float>(pixelSize.width) / static_cast<float>(physicalSize.width))*25.4f;
        outDpiX = outDpiY = static_cast<uint32_t>(dpi + 0.5f); // round
        return true;
      }
      @catch (NSException*) { return false; }
    }
  }
  bool LibrariesCocoa::readScreenScaling(NSScreen* screen, float& outScaleX, float& outScaleY) noexcept {
    @autoreleasepool {
      if (!screen)
        return false;

      @try {
        NSRect points = [screen frame];
        NSRect pixels = [screen convertRectToBacking:points];

        outScaleX = static_cast<float>(pixels.size.width) / static_cast<float>(points.size.width);
        outScaleY = static_cast<float>(pixels.size.height) / static_cast<float>(points.size.height);
        return true;
      }
      @catch (NSException*) { return false; }
    }
  }

  // -- init --

  bool LibrariesCocoa::init() noexcept {
    @autoreleasepool {
      if (this->_isInit)
        return true;
      
      @try {
        if (NSApp)
          isAppLaunchFinished = true;
        [NSApplication sharedApplication];
      }
      @catch (NSException*) { return false; }

      this->_isInit = true;
      return true;
    }
  }

  // -- shutdown --

  void LibrariesCocoa::shutdown() noexcept {
    @autoreleasepool {
      this->_isInit = false;
    }
  }
#endif
