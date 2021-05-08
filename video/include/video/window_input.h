/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
description : window input events & handlers
types : WindowEventHandler, KeyboardEventHandler, MouseEventHandler, TouchEventHandler
enums : KeyboardEvent, KeyTransition, MouseEvent, MouseButton, TouchEvent
methods : virtualKeyToChar, mouseWheelDelta, isMouseButtonPressed
keycodes: see <video/window_keycodes.h>
*******************************************************************************/
#pragma once

#include <cstdint>
#include <type_traits>
#ifdef _WINDOWS
# include <wchar.h>
#endif

namespace pandora {
  namespace video {
    // -- keyboard events --
    
    /// @brief Keyboard event types
    enum class KeyboardEvent : uint32_t {
      none         = 0u, ///< no event
      keyDown      = 1u, ///< any key is pressed (returning true will prevent associated charInput event)
      keyUp        = 2u, ///< any key is released
      altKeyDown   = 3u, ///< any key is pressed with ALT key active (returning true will prevent associated charInput event)
      altKeyUp     = 4u, ///< any key is released with ALT key active
      activateMenu = 5u, ///< system key pressed to activate menu bar (F10 on Windows)
      charInput    = 6u  ///< after keyDown/altKeyDown (if handler did not return true), the associated textual character is generated,
                         ///  using shift/alt-gr/accent (only if the key is pressed and corresponds to an actual character).
    };
    /// @brief Key transition types
    enum class KeyTransition : uint32_t {
      same         = 0u, ///< the key was already pressed
      down         = 1u, ///< the key has been pressed
      up           = 2u  ///< the key has been released
    };
    
    /// @brief Convert virtual key to associated character on current system (will not use shift/alt/accent modifiers!)
    /// @returns Basic unicode character value (or 0 if not a character key)
#   ifdef _WINDOWS
      wchar_t virtualKeyToChar(uint32_t keyCode) noexcept;
#   else
      uint32_t virtualKeyToChar(uint32_t keyCode) noexcept;
#   endif

    /// @brief Keyboard event handling function pointer (press/release):
    ///        * event:      keyboard event type
    ///        * keyCode:    - virtual key code (keyChange/altKeyChange/activateMenu);
    ///                        NB: use <video/window_keycodes.h> definitions to identify key codes.
    ///                      - character value (charInput).
    ///        * value:      - type of transition (keyChange/altKeyChange/activateMenu) -> cast to 'KeyTransition'.
    ///                      - number of repeats (charInput).
    ///        * return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using KeyboardEventHandler = bool (*)(KeyboardEvent event, uint32_t keyCode, uint32_t value);
    
    
    // -- mouse events --
    
    /// @brief Mouse event types
    enum class MouseEvent : uint32_t {
      none         = 0u, ///< no event
      mouseMove    = 1u, ///< mouse move detected in client area (no index) -- not received in CursorMode::hiddenRaw
      mouseLeave   = 2u, ///< mouse leaves client area (no index/activeKeys)
      buttonDown   = 3u, ///< button pressed:      index = button index (can be cast to 'MouseButton')
      buttonUp     = 4u, ///< button released:     index = button index (can be cast to 'MouseButton')
      mouseWheelV  = 5u, ///< mouse wheel vertical move:   index = distance in multiples of 'mouseWheelDelta()' (forward > 0, backwards < 0)
      mouseWheelH  = 6u, ///< mouse wheel horizontal move: index = distance in multiples of 'mouseWheelDelta()' (right > 0, left < 0)
      rawMotion    = 7u  ///< raw mouse X/Y delta -- only received in CursorMode::hiddenRaw
    };
    int32_t mouseWheelDelta() noexcept; ///< Distance unit for mouse wheel movements
    
    /// @brief Mouse button identifiers + shift/control modifiers for mouse events
    /// @warning internal dev: keep values aligned with button indexes returned by Window impl
    enum class MouseButton : uint32_t {
      left    = 0u,
      middle  = 1u,
      right   = 2u,
      button4 = 3u,
      button5 = 4u
    };
    bool isMouseButtonPressed(uint8_t activeKeys, MouseButton button) noexcept; ///< Verify button status from event bit-map
    
    /// @brief Mouse event handling function pointer (click/move/wheel/...):
    ///        * event:  mouse event type
    ///        * x:      X-coord of the mouse pointer (relative to client area top-left corner)
    ///        * y:      Y-coord of the mouse pointer (relative to client area top-left corner)
    ///        * index:  value specific to event type (see MouseEvent enum)
    ///        * activeKeys: bit-map with mouse keys that are currently pressed -> call 'isMouseButtonPressed(activeKeys, <MouseButton>)'
    ///        * return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using MouseEventHandler = bool (*)(MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t activeKeys);


    // -- touch-screen events --
    
    /// @brief Touch-screen event types
    enum class TouchEvent : uint32_t {
      none         = 0u, ///< no event
      touch        = 1u, ///< screen is touched:  index = number of contact points
      release      = 2u, ///< screen is released: index = remaining number of contact points
      move         = 3u  ///< touch position changed: index = contact point index
    };
    
    /// @brief Touch event handling function pointer (touch/move/...):
    ///        * x:      X-coord (relative to client area top-left corner)
    ///        * y:      Y-coord (relative to client area top-left corner)
    ///        * index:  value specific to event type (see TouchEvent enum)
    ///        * return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using TouchEventHandler = bool (*)(TouchEvent event, uint32_t x, uint32_t y, uint32_t index);
    
    
    // -- general window events --
    
    /// @brief Window/hardware event types
    enum class WindowEvent : uint32_t {
      none                   = 0u, ///< no event
      windowClosed           = 1u, ///< request for closing window (return true to prevent it, or false to allow it).
      stateChanged           = 2u, ///< window state has changed (visibility/activity):
                                   ///  - flag = cast to 'WindowVisibleActive'.
      suspendResume          = 3u, ///< system suspend/resume operation:
                                   ///  - flag = boolean: 1 (suspend) or 0 (resume).
      deviceInterfaceChange  = 4u, ///< device interface added/removed (gamepad/joystick/...):
                                   ///  - flag = boolean: 1 (added) or 0 (removed).
      sizePositionTrack      = 5u, ///< window size/position is being changed (resizing/moving):
                                   ///  - posX = tracked client position X (absolute);
                                   ///  - posY = tracked client position Y (absolute);
                                   ///  - size = tracked client width;
                                   ///  - data = tracked client height;
      sizePositionChanged    = 6u, ///< window size/position changed (resized/maximized/restored/moved):
                                   ///  - posX = new client position X (absolute);
                                   ///  - posY = new client position Y (absolute);
                                   ///  - size = new client width;
                                   ///  - data = new client height;
      scrollPositionVTrack   = 7u, ///< the vertical scrollbar slider is being moved:
                                   ///  - posY (or posX) = current scroll position.
      scrollPositionHTrack   = 8u, ///< the horizontal scrollbar slider is being moved:
                                   ///  - posX (or posY) = current scroll position.
      scrollPositionVChanged = 9u, ///< the vertical scrollbar slider has been moved and released:
                                   ///  - posY (or posX) = new scroll position.
      scrollPositionHChanged = 10u,///< the horizontal scrollbar slider has been moved and released:
                                   ///  - posX (or posY) = new scroll position.
      menuSelected           = 11u,///< menu item selected: 
                                   ///  - flag = boolean: 1 (submenu) or 0 (command);
                                   ///  - size = index of item if submenu / command ID if menu command;
                                   ///  - data = only if submenu: handle to native menu/submenu (cast to 'MenuHandle').
      dpiChanged             = 12u,///< window DPI has changed: 
                                   ///  - flag = boolean: 1 (monitor changed) or 0 (same monitor);
                                   ///  - posX = suggested adjusted client area width;
                                   ///  - posY = suggested adjusted client area height;
                                   ///  - size = content scaling factor multiplied by 100 (percentage);
                                   ///  - data = monitor DPI value.
      inputLangChanged       = 13u,///< keyboard input language has changed for current window:
                                   ///  - flag = locale code page
                                   ///  - posX = locale primary ID
                                   ///  - posY = locale sub-lang ID
                                   ///  - data = native language handle/ID
      dropFiles              = 14u ///< end of a drag & drop operation:
                                   ///  - posX = drop position X, relative to client area;
                                   ///  - posY = drop position Y, relative to client area;
                                   ///  - size = number of file paths in 'data'
                                   ///  - data = array of file paths (cast to 'wchar_t**' on Windows, cast to 'char**' on other systems) - do NOT free buffer !
    };
    /// @brief Window visibility/activity
    enum class WindowVisibleActive : uint32_t {
      hidden       = 0u, ///< hidden or minimized/inactive
      inactive     = 1u, ///< visible/inactive
      active       = 2u, ///< visible/active
    };
    
    typedef std::conditional<sizeof(uint64_t)>=sizeof(uintptr_t), uint64_t, uintptr_t>::type uint64_ptr;
    
    /// @brief Window event handling function pointer (close/maximize/scroll/...):
    ///        * event: window/hardware event type
    ///        * flag: event info, specific to event type (see WindowEvent enum)
    ///        * posX: horizontal position, specific to event type (see WindowEvent enum)
    ///        * posY: vertical position, specific to event type (see WindowEvent enum)
    ///        * size: count/size, specific to event type (see WindowEvent enum)
    ///        * data: handle/pointer/sizeY, specific to event type (see WindowEvent enum)
    ///        * bool return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using WindowEventHandler = bool (*)(WindowEvent event, uint32_t flag, int32_t posX, int32_t posY, uint32_t size, uint64_ptr data);
  }
}
