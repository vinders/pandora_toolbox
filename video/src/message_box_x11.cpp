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
  MessageBox::DialogResult MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                            MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    return MessageBox::DialogResult::ok;
  }
  
  // ---
  
  // get last error message (in case of DialogResult::failure)
  std::string LastError() { return ""; }

#endif
