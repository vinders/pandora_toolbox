/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window events & handlers - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include "video/window_events.h"

  
// -- keyboard events -- ----------------------------------------------------------

  // convert virtual key to associated character on current system
  uint32_t pandora::video::virtualKeyToChar(uint32_t keyCode) noexcept {
    return 0;//TODO
  }


// -- mouse events -- ----------------------------------------------------------

  // distance unit for mouse wheel movements
  int32_t pandora::video::mouseWheelDelta() noexcept { return 120; }
  
  // verify button status from event bit-map
  bool pandora::video::isMouseButtonPressed(uint8_t activeKeys, pandora::video::MouseButton button) noexcept {
    return (activeKeys != 0);//TODO
  }

#endif
