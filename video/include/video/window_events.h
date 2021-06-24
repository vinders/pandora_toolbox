/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
- description : window events & handlers (system/hardware/input events)
- types : WindowEventHandler, KeyboardEventHandler, MouseEventHandler, TouchEventHandler
- enums : KeyboardEvent, KeyTransition, MouseEvent, MouseButton, TouchEvent
- functions : virtualKeyToChar, mouseWheelDelta, isMouseButtonPressed
- keycodes: see <video/window_keycodes.h>
*******************************************************************************/
#pragma once

#include <cstdint>
#ifdef _WINDOWS
# include <wchar.h>
#endif

namespace pandora {
  namespace video {
    class Window;
    
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
    ///        * sender:  event origin
    ///        * event:   keyboard event type
    ///        * keyCode: - virtual key code (keyChange/altKeyChange/activateMenu);
    ///                     NB: use <video/window_keycodes.h> definitions to identify key codes.
    ///                   - character value (charInput).
    ///        * change:  - type of transition (keyChange/altKeyChange/activateMenu) -> cast to 'KeyTransition'.
    ///                   - number of repeats (charInput).
    ///        * return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using KeyboardEventHandler = bool (*)(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t change);
    
    
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
      rawMotion    = 7u  ///< raw mouse X/Y delta (normalized) -- only received in CursorMode::hiddenRaw
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
    
    /// @brief Mouse or touch-screen event handling function pointer (click/move/wheel/...):
    ///        * sender: event origin
    ///        * event:  mouse event type
    ///        * x:      X-coord of the mouse pointer (relative to client area top-left corner) / delta-X (rawMotion)
    ///        * y:      Y-coord of the mouse pointer (relative to client area top-left corner) / delta-Y (rawMotion)
    ///        * index:  value specific to event type (see MouseEvent enum)
    ///        * activeKeys: bit-map with mouse keys that are currently pressed -> call 'isMouseButtonPressed(activeKeys, <MouseButton>)'
    ///        * return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using MouseEventHandler = bool (*)(Window* sender, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t activeKeys);
    
    
    // -- general window/hardware events --
    
    /// @brief Window/hardware event types
    enum class WindowEvent : uint32_t {
      none                   = 0u, ///< no event
      windowClosed           = 1u, ///< request for closing window (return true to prevent it, or false to allow it).
      stateChanged           = 2u, ///< window state has changed (visibility/activity):
                                   ///  - status = cast to 'WindowActivity'.
      suspendResume          = 3u, ///< system suspend/resume operation:
                                   ///  - status = boolean: 1 (suspend) or 0 (resume).
      deviceInterfaceChange  = 4u, ///< device interface added/removed (gamepad/joystick/...):
                                   ///  - status = boolean: 1 (added) or 0 (removed).
      menuCommand            = 5u, ///< menu item selected: 
                                   ///  - status = boolean: 1 (accelerator key) or 0 (menu command item);
                                   ///  - posX = if menu command item: menu identifier;
                                   ///  - posY = if accelerator key: character code.
      monitorChanged         = 6u, ///< window DPI has changed or window has moved to a different monitor: 
                                   ///  - status = content X scaling factor multiplied by 100 (percentage);
                                   ///  - posX/posY = suggested adjusted client area width/height;
                                   ///  - data = native monitor handle (if monitor has changed) or NULL (if same monitor).
      inputLangChanged       = 7u, ///< keyboard input language has changed for current window:
                                   ///  - status = locale code page;
                                   ///  - posX/poxY = locale primary/sublang ID;
                                   ///  - data = native language handle/ID.
      dropFiles              = 8u  ///< end of a drag & drop operation:
                                   ///  - status = length of 'data' (number of file paths);
                                   ///  - posX/posY = drop position X/Y, relative to client area;
                                   ///  - data = array of file paths (cast to 'wchar_t**' on Windows, cast to 'char**' on other systems) - do NOT free buffer!
    };
    /// @brief Window visibility/activity
    enum class WindowActivity : uint32_t {
      hidden       = 0u, ///< hidden or minimized/inactive
      inactive     = 1u, ///< visible/inactive
      active       = 2u, ///< visible/active
    };

    /// @brief Window/hardware event handling function pointer (close/activate/suspend/drop/...):
    ///        * sender:    event origin
    ///        * event:     window/hardware event type
    ///        * status:    event status -or- info about other params (see WindowEvent enum)
    ///        * posX/posY: horizontal/vertical position -or- primary/secondary ID, specific to event type (see WindowEvent enum)
    ///        * data:      handle/pointer, specific to event type (see WindowEvent enum)
    ///        * bool return:  true to prevent default system processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using WindowEventHandler = bool (*)(Window* sender, WindowEvent event, uint32_t status, int32_t posX, int32_t posY, void* data);
    
    
    // -- window position/size events --
    
    /// @brief Window position/size event types
    enum class PositionEvent : uint32_t {
      none                   = 0u, ///< no event
      sizePositionTrack      = 1u, ///< window size/position is being changed (resizing/moving):
                                   ///  - posX/posY = tracked client-area position X/Y (absolute);
                                   ///  - sizeX/sizeY = tracked client-area width/height.
      sizePositionChanged    = 2u  ///< window size/position changed (resized/maximized/restored/moved):
                                   ///  - posX/posY = new client-area position X/Y (absolute);
                                   ///  - sizeX/sizeY = new client-area width/height.
    };

    /// @brief Window size/position event handling function pointer (move/resize/maximize/...):
    ///        * sender:      event origin
    ///        * event:       window size/position event type
    ///        * posX/posY:   horizontal/vertical positions (see PositionEvent enum)
    ///        * sizeX/sizeY: horizontal/vertical sizes or limits (see PositionEvent enum)
    ///        * bool return:  true to prevent default size processing, false to allow normal processing
    /// @warning Handler should NOT throw exceptions -> use try/catch in it
    using PositionEventHandler = bool (*)(Window* sender, PositionEvent event, int32_t posX, int32_t posY, uint32_t sizeX, uint32_t sizeY);
  }
}
