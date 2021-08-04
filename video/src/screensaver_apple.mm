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
#if defined(__APPLE__)
# include <TargetConditionals.h>
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   import <UIKit/UIKit.h>
# else
#   import <IOKit/pwr_mgt/IOPMLib.h>
# endif
# include "video/screensaver.h"
#endif


#if defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE // iOS
  bool pandora::video::disableScreenSaver(pandora::video::WindowHandle) noexcept { 
    if ([UIApplication sharedApplication]) {
      [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
      return true;
    }
    return false;
  }
  bool pandora::video::restoreScreenSaver() noexcept {
    if ([UIApplication sharedApplication]) {
      [[UIApplication sharedApplication] setIdleTimerDisabled: NO];
      return true;
    }
    return false;
  }
  
  void pandora::video::notifyScreenActivity(pandora::video::WindowHandle) noexcept {
    if ([UIApplication sharedApplication]) {
      [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
      [[UIApplication sharedApplication] setIdleTimerDisabled: NO];
    }
  }
  
  
#elif defined(__APPLE__) // OSX
  static IOPMAssertionID g_noSleepAssertionId = 0;
  static bool g_isSleepDisabled = false;

  bool pandora::video::disableScreenSaver(pandora::video::WindowHandle) noexcept {
    if (g_isSleepDisabled)
      return true;
    
    try {
      IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, 
                                                     CFSTR("Game/video running"), &g_noSleepAssertionId);
      g_isSleepDisabled = (success == kIOReturnSuccess);
      return g_isSleepDisabled;
    } 
    catch (...) { return false; }
  }

  bool pandora::video::restoreScreenSaver() noexcept {
    if (g_isSleepDisabled) {
      try {
        if (IOPMAssertionRelease(g_noSleepAssertionId) != kIOReturnSuccess)
          return false;
        g_isSleepDisabled = false;
        g_noSleepAssertionId = 0;
      } 
      catch (...) { return false; }
    }
    return true; 
  }
  
  void pandora::video::notifyScreenActivity(pandora::video::WindowHandle) noexcept {
    IOPMAssertionID assertionId = 0;
    IOReturn success = IOPMAssertionDeclareUserActivity(CFSTR("Game/video activity"), kIOPMUserActiveLocal, &assertionId);
    if (success == kIOReturnSuccess)
      IOPMAssertionRelease(assertionId);
  }
#endif
