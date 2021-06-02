/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window events & handlers - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <cassert>
# include <cstdint>
# include <system/api/windows_api.h>
# include "video/window_events.h"
# include "video/window_keycodes.h"

  
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
