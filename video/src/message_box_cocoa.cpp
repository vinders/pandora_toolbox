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
# include <thread/spin_lock.h>
# include "video/_private/_message_box_impl_cocoa.h"
# include "video/message_box.h"

  using namespace pandora::video;
  
  static pandora::thread::SpinLock __lastErrorLock;
  static std::string __lastError;
  
  
// -- simple message box -- ----------------------------------------------------

  // convert portable actions type to list of button labels
  // note: reverse order of buttons (usual order on macOS)
  static inline uint32_t __toNativeActions(MessageBox::ActionType actions, char outBtns[3][8]) noexcept {
    switch (actions) {
      case MessageBox::ActionType::ok:
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char));
        return 1;
      case MessageBox::ActionType::okCancel:
        memcpy((void*)outBtns[1], (void*)"OK", 3*sizeof(char));
        memcpy((void*)outBtns[0], (void*)"Cancel", 7*sizeof(char));
        return 2;
      case MessageBox::ActionType::retryCancel:
        memcpy((void*)outBtns[1], (void*)"Retry", 6*sizeof(char));
        memcpy((void*)outBtns[0], (void*)"Cancel", 7*sizeof(char));
        return 2;
      case MessageBox::ActionType::yesNo:
        memcpy((void*)outBtns[1], (void*)"Yes", 4*sizeof(char));
        memcpy((void*)outBtns[0], (void*)"No", 3*sizeof(char));
        return 2;
      case MessageBox::ActionType::yesNoCancel:
        memcpy((void*)outBtns[2], (void*)"Yes", 4*sizeof(char));
        memcpy((void*)outBtns[1], (void*)"No", 3*sizeof(char));
        memcpy((void*)outBtns[0], (void*)"Cancel", 7*sizeof(char));
        return 3;
      case MessageBox::ActionType::abortRetryIgnore:
        memcpy((void*)outBtns[2], (void*)"Abort", 6*sizeof(char));
        memcpy((void*)outBtns[1], (void*)"Retry", 6*sizeof(char));
        memcpy((void*)outBtns[0], (void*)"Ignore", 7*sizeof(char));
        return 3;
      default: 
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char));
        return 1;
    }
  }
  
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
  
  // convert native result to portable DialogResult
  // note: restore order (reversed on macOS)
  static inline MessageBox::Result __toDialogResult(uint32_t result, uint32_t maxLength) noexcept {
    if (!result)
      return MessageBox::Result::failure;
    
    result = maxLength + 1 - result; // reverse
    switch (result) {
      case 1: return MessageBox::Result::action1;
      case 2: return MessageBox::Result::action2;
      case 3: return MessageBox::Result::action3;
      default: return MessageBox::Result::failure;
    }
  }

  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle) noexcept {
    char buttons[3][8] = {{ 0 }};
    uint32_t length = __toNativeActions(actions, buttons);
    
    char* error = nullptr;
    uint32_t result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), buttons, length, 
                                             isTopMost ? Bool_TRUE : Bool_FALSE, &error);
    if (error) {
      try { 
        std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
        __lastError = error; 
      } catch (...) {} // avoid leak of 'error'
      free(error);
    }
    return __toDialogResult(result, length);
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string LastError() { 
    std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
    return __lastError;
  }
  
  
// -- custom message box -- ----------------------------------------------------

  // convert custom actions to list of button labels (missing labels are replaced with placeholders)
  // note: reverse order of buttons (usual order on macOS)
  static inline uint32_t __toNativeActions(const char* button1, const char* button2, const char* button3, 
                                           char outPlaceholders[2][8], const char* outButtons[3]) noexcept {
    // 3 buttons
    if (button3) { 
      outButtons[0] = button3;
      
      if (button2)
        outButtons[1] = button2;
      else {
        memcpy((void*)outPlaceholders[1], (void*)"No", 3*sizeof(char));
        outButtons[1] = outPlaceholders[1];
      }
      if (button1)
        outButtons[2] = button1;
      else {
        memcpy((void*)outPlaceholders[0], (void*)"Yes", 4*sizeof(char));
        outButtons[2] = outPlaceholders[0];
      }
      return 3;
    }
    // 2 buttons
    else if (button2) { 
      outButtons[0] = button2;
      
      if (button1)
        outButtons[1] = button1;
      else {
        memcpy((void*)outPlaceholders[0], (void*)"OK", 3*sizeof(char));
        outButtons[1] = outPlaceholders[0];
      }
      return 2;
    }
    // 1 button
    else { 
      if (button1)
        outButtons[0] = button1;
      else {
        memcpy((void*)outPlaceholders[0], (void*)"OK", 3*sizeof(char));
        outButtons[0] = outPlaceholders[0];
      }
      return 1;
    }
  }
  
  // ---
  
  // show modal message box with custom button labels
  MessageBox::Result MessageBox::show(const char* caption, const char* message, 
                                      const char* button1, const char* button2, const char* button3,
                                      MessageBox::IconType icon, bool isTopMost, WindowHandle) noexcept {
    char placeholders[2][4] = {{ 0 }};
    const char* buttons[3] = { nullptr };
    uint32_t length = __toNativeActions(button1, button2, button3, placeholders, buttons);
      
    char* error = nullptr;
    uint32_t result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), buttons, length, 
                                             isTopMost ? Bool_TRUE : Bool_FALSE, &error);
    if (error) {
      try { 
        std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
        __lastError = error; 
      } catch (...) {} // avoid leak of 'error'
      free(error);
    }
    return __toDialogResult(result, length);
  }

#endif
