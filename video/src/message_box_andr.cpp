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
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    return MessageBox::Result::action1;
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string LastError() { return ""; }
  
  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, 
                                      const char* button1, const char* button2, const char* button3,
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    return MessageBox::Result::action1;
  }

#endif
