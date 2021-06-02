/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window events & handlers - X11 implementation (Linux/Unix/BSD)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
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
