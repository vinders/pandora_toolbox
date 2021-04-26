/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - UIKit implementation (iOS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
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
