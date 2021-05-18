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

namespace pandora {
  namespace video {
    /// @brief Disable screensaver for the duration of the program
    /// @remarks - argument 'windowSurface' is only required for Wayland (not X11) on Linux.
    ///          - all other platforms don't need to provide any argument.
    bool disableScreenSaver(void* windowSurface = nullptr) noexcept;
    /// @brief Restore normal screensaver behavior (after disabling it)
    bool restoreScreenSaver() noexcept;
    
    /// @brief Ping activity signal (to reset screensaver/sleep timer)
    /// @remarks - should be called periodically (only if disableScreensaver() was not called)
    ///          - argument 'windowSurface' is only required for Wayland (not X11) on Linux.
    ///          - all other platforms don't need to provide any argument.
    void notifyScreenActivity(void* windowSurface = nullptr) noexcept;
  }
}
