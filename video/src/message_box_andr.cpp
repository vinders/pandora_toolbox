/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - JNI implementation (Android)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
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
  std::string LastError() { return "" }

#endif
