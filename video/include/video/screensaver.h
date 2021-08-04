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
