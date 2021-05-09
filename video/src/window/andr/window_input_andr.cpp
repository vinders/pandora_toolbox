/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window input events & handlers - JNI implementation (Android)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include "video/window_input.h"

  
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
