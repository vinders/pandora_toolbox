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
# include <mutex>
# include "video/message_box.h"

  using namespace pandora::video;
  
  static std::mutex __lastErrorLock;
  static std::string __lastError;
  
  
  // convert portable action to list of native actions
  static inline void __toNativeActions(CocoaBoxButtonId actions[3], uint32_t outLength) noexcept {
    
    switch (actions) {
      case MessageBox::ActionType::ok:
        outLength = 1; actions[0] = COCOA_BOX_BUTTON_OK; break;
      case MessageBox::ActionType::okCancel:
        outLength = 2; actions[0] = COCOA_BOX_BUTTON_CANCEL; actions[1] = COCOA_BOX_BUTTON_OK; break;
      case MessageBox::ActionType::retryCancel:
        outLength = 2; actions[0] = COCOA_BOX_BUTTON_CANCEL; actions[1] = COCOA_BOX_BUTTON_RETRY; break;
      case MessageBox::ActionType::yesNo:
        outLength = 2; actions[0] = COCOA_BOX_BUTTON_NO; actions[1] = COCOA_BOX_BUTTON_YES; break;
      case MessageBox::ActionType::yesNoCancel:
        outLength = 3; actions[0] = COCOA_BOX_BUTTON_CANCEL; actions[1] = COCOA_BOX_BUTTON_NO; actions[2] = COCOA_BOX_BUTTON_YES; break;
      case MessageBox::ActionType::abortRetryIgnore:
        outLength = 3; actions[0] = COCOA_BOX_BUTTON_ABORT; actions[1] = COCOA_BOX_BUTTON_IGNORE; actions[2] = COCOA_BOX_BUTTON_RETRY; break;
      default: 
        outLength = 1; actions[0] = COCOA_BOX_BUTTON_OK; break;
    }
  }
  
  // convert portable icon to native icon flag
  static inline CocoaBoxIconId __toNativeIcon(MessageBox::IconType icon) noexcept {
    switch (icon) {
      case MessageBox::IconType::info:     return COCOA_BOX_ICON_INFO;
      case MessageBox::IconType::question: return COCOA_BOX_ICON_QUESTION;
      case MessageBox::IconType::warning:  return COCOA_BOX_ICON_WARNING;
      case MessageBox::IconType::error:    return COCOA_BOX_ICON_ERROR;
      default: return COCOA_BOX_ICON_NONE;
    }
  }
  
  // convert native result to portable DialogResult
  static inline MessageBox::DialogResult __toDialogResult(CocoaBoxButtonId result) noexcept {
    switch (result) {
      case COCOA_BOX_BUTTON_OK:     return MessageBox::DialogResult::ok;
      case COCOA_BOX_BUTTON_CANCEL: return MessageBox::DialogResult::cancel;
      case COCOA_BOX_BUTTON_YES:    return MessageBox::DialogResult::yes;
      case COCOA_BOX_BUTTON_NO:     return MessageBox::DialogResult::no;
      case COCOA_BOX_BUTTON_ABORT:  return MessageBox::DialogResult::abort;
      case COCOA_BOX_BUTTON_RETRY:  return MessageBox::DialogResult::retry;
      case COCOA_BOX_BUTTON_IGNORE: return MessageBox::DialogResult::ignore;
      default: return MessageBox::DialogResult::failure;
    }
  }

  // ---

  // show modal message box
  MessageBox::DialogResult MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                            MessageBox::IconType icon, bool isTopMost, WindowHandle) noexcept {
    CocoaBoxButtonId actions[3];
    uint32_t actionsLength = 0;
    __toNativeActions(actions, length);
    
    char* error = nullptr;
    CocoaBoxButtonId result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), 
                                                     actions, length, (isTopMost) ? Bool_TRUE : Bool_FALSE);
    if (error) {
      std::lock_guard<std::mutex> guard(__lastErrorLock);
      try { __lastError = error; } catch (...) {} // avoid leak of 'error' instance
      free(error);
    }
    return __toDialogResult(result);
  }
  
  // ---
  
  // get last error message (in case of DialogResult::failure)
  std::string LastError() { 
    std::lock_guard<std::mutex> guard(__lastErrorLock);
    return __lastError;
  }

#endif
