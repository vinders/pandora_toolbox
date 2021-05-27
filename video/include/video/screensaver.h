/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : turn on/off system screensaver/sleep
Functions : disableScreenSaver, restoreScreenSaver, notifyScreenActivity
*******************************************************************************/
#ifndef __OBJC__
# pragma once
#endif
#include "./window_handle.h"

namespace pandora {
  namespace video {
    /// @brief Disable screensaver for the duration of the program
    /// @remarks - argument 'windowHandle' is only required for Wayland (not X11) on Linux.
    ///          - all other platforms don't need to provide any argument.
    /// @warning All screensaver operations should be managed by the same thread!
    bool disableScreenSaver(WindowHandle windowHandle = (WindowHandle)0) noexcept;
    /// @brief Restore normal screensaver behavior (after disabling it)
    /// @warning To be effective on all platforms, restoreScreenSaver() must be called by the same thread as disableScreenSaver() !
    bool restoreScreenSaver() noexcept;
    
    /// @brief Ping activity signal (to reset screensaver/sleep timer)
    /// @remarks - should be called periodically (only if disableScreensaver() was not called)
    ///          - argument 'windowHandle' is only required for Wayland (not X11) on Linux.
    ///          - all other platforms don't need to provide any argument.
    void notifyScreenActivity(WindowHandle windowHandle = (WindowHandle)0) noexcept;
  }
}
