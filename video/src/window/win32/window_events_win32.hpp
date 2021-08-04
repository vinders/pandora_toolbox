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
Description : Window events & handlers - Win32 implementation (Windows)
--------------------------------------------------------------------------------
Implementation included in window_win32.cpp
(grouped object improves compiler optimizations + reduces executable size)
*******************************************************************************/
#ifdef _WINDOWS
// includes + namespaces: in window_win32.cpp

  
// -- keyboard events -- ----------------------------------------------------------

  // convert virtual key to associated character on current system
  wchar_t pandora::video::virtualKeyToChar(uint32_t keyCode) noexcept {
    if (keyCode != _P_VK_UNKNOWN) {
      if (keyCode == _P_VK_ENTER_PAD)
        keyCode = VK_RETURN;

      UINT value = (MapVirtualKeyW((UINT)keyCode, MAPVK_VK_TO_CHAR) & 0x7FFFFFFFu); // remove top bit (if "dead key")
      return static_cast<wchar_t>(value);
    }
    return L'\0';
  }


// -- mouse events -- ----------------------------------------------------------

  // distance unit for mouse wheel movements
  int32_t pandora::video::mouseWheelDelta() noexcept { return WHEEL_DELTA; }
  
  // verify button status from event bit-map
  bool pandora::video::isMouseButtonPressed(uint8_t activeKeys, pandora::video::MouseButton button) noexcept {
    const uint8_t keyBindings[] {
      MK_LBUTTON,
      MK_MBUTTON,
      MK_RBUTTON,
      MK_XBUTTON1,
      MK_XBUTTON2
    };
    static_assert(sizeof(keyBindings) / sizeof(uint8_t) == (size_t)pandora::video::MouseButton::button5 + 1,
                  "mouse_input: isMouseButtonPressed: missing keys in array");
    return (activeKeys & keyBindings[static_cast<uint32_t>(button)]);
  }

#endif
