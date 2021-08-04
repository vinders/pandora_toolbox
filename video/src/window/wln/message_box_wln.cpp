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
Description : Message box - Wayland implementation (Linux)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cassert>
# include <cstdint>
# include <cstdio>
# include "video/_private/_message_box_common.h"
# include "video/message_box.h"

  using namespace pandora::video;
  
  // show message box
  uint32_t __showWaylandDialog(const char* caption, const char* message, const char** actions, uint32_t actionsLength) {
    __MessageBox::setLastError("MessageBox: not supported on Wayland display server. Use X11 instead for MessageBox support on linux.");
    return 0; // currently not supported (wayland API is very poorly documented, and examples are really hard to find)
  }

  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(actions, &buttons[0]);
    uint32_t result = __showWaylandDialog(caption, message, &buttons[0], length);
    return __MessageBox::toDialogResult(result, length);
  }
  
  // ---
  
  // flush system events
  void MessageBox::flushEvents() noexcept {}
  
  // ---
  
  // get last error message (in case of Result::failure)
  pandora::memory::LightString MessageBox::getLastError() noexcept {
    return __MessageBox::getLastError();
  }
  
  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(button1, button2, button3, &buttons[0]);
    uint32_t result = __showWaylandDialog(caption, message, &buttons[0], length);
    return __MessageBox::toDialogResult(result, length);
  }

#endif
