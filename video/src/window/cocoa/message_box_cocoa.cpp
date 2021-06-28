/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <cassert>
# include <cstdint>
# include "video/_private/_message_box_impl_cocoa.h"
# include "video/_private/_message_box_common.h"
# include "video/message_box.h"

  using namespace pandora::video;
  
  
// -- simple message box -- ----------------------------------------------------
  
  // convert portable icon to native icon flag
  static inline enum CocoaBoxIconId __toNativeIcon(MessageBox::IconType icon) noexcept {
    switch (icon) {
      case MessageBox::IconType::info:     return COCOA_BOX_ICON_INFO;
      case MessageBox::IconType::question: return COCOA_BOX_ICON_QUESTION;
      case MessageBox::IconType::warning:  return COCOA_BOX_ICON_WARNING;
      case MessageBox::IconType::error:    return COCOA_BOX_ICON_ERROR;
      default: return COCOA_BOX_ICON_NONE;
    }
  }

  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(actions, &buttons[0]);
    
    char* error = nullptr;
    uint32_t result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), &buttons[0], length, 
                                             isTopMost ? Bool_TRUE : Bool_FALSE, &error);
    if (error) {
      __MessageBox::setLastError(error);
      free(error);
    }
    return __MessageBox::toDialogResult(result, length);
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  pandora::memory::LightString MessageBox::getLastError() { 
    return __MessageBox::getLastError();
  }
  
  
// -- custom message box -- ----------------------------------------------------

  // show modal message box with custom button labels
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(button1, button2, button3, &buttons[0]);
      
    char* error = nullptr;
    uint32_t result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), &buttons[0], length, 
                                             isTopMost ? Bool_TRUE : Bool_FALSE, &error);
    if (error) {
      __MessageBox::setLastError(error);
      free(error);
    }
    return __MessageBox::toDialogResult(result, length);
  }

#endif
