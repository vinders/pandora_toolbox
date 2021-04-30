/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Wayland implementation (Linux)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cassert>
# include <cstdint>
# include <cstdio>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include "video/_private/_message_box_common.h"
# include "video/message_box.h"

  using namespace pandora::video;
  
  // show message box
  uint32_t __showWaylandDialog(const char* caption, const char* message, const char** actions, uint32_t actionsLength) {
    
    return 0;
  }

  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(actions, &buttons[0]);
    uint32_t result = __showWaylandDialog(caption, message, buttons, length);
    return __MessageBox::toDialogResult(result, length);
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string MessageBox::getLastError() {
    return __MessageBox::getLastError();
  }
  
  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(button1, button2, button3, &buttons[0]);
    uint32_t result = __showWaylandDialog(caption, message, buttons, length);
    return __MessageBox::toDialogResult(result, length);
  }

#endif
