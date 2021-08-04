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
