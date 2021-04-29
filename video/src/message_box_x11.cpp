/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Xorg implementation (Linux/Unix/BSD)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cassert>
# include <cstdint>
# include "video/message_box.h"

  using namespace pandora::video;

  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    return MessageBox::Result::action1;
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string LastError() { return ""; }
  
  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    return MessageBox::Result::action1;
  }

#endif
