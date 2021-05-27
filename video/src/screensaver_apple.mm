/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
