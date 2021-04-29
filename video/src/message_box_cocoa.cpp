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

# define __P_MAX_LABEL_LENGTH 8
# define __P_ASSERT_LENGTH(n) static_assert((n) < __P_MAX_LABEL_LENGTH, "MessageBox (cocoa): action value too long");

  using namespace pandora::video;
  
  static pandora::thread::SpinLock __lastErrorLock;
  static std::string __lastError;
  
  
// -- simple message box -- ----------------------------------------------------

  // convert portable actions type to list of button labels
  // note: reverse order of buttons (usual order on macOS)
  static inline uint32_t __toNativeActions(MessageBox::ActionType actions, char** outBtns) noexcept {
    // warning: values must not exceed: __P_MAX_LABEL_LENGTH (including trailing zero)
    switch (actions) {
      case MessageBox::ActionType::ok:
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        return 1;
      case MessageBox::ActionType::okCancel:
        memcpy((void*)outBtns[1], (void*)"OK", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        memcpy((void*)outBtns[0], (void*)"Cancel", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 2;
      case MessageBox::ActionType::retryCancel:
        memcpy((void*)outBtns[1], (void*)"Retry", 6*sizeof(char)); __P_ASSERT_LENGTH(5);
        memcpy((void*)outBtns[0], (void*)"Cancel", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 2;
      case MessageBox::ActionType::yesNo:
        memcpy((void*)outBtns[1], (void*)"Yes", 4*sizeof(char)); __P_ASSERT_LENGTH(3);
        memcpy((void*)outBtns[0], (void*)"No", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        return 2;
      case MessageBox::ActionType::yesNoCancel:
        memcpy((void*)outBtns[2], (void*)"Yes", 4*sizeof(char)); __P_ASSERT_LENGTH(3);
        memcpy((void*)outBtns[1], (void*)"No", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        memcpy((void*)outBtns[0], (void*)"Cancel", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 3;
      case MessageBox::ActionType::abortRetryIgnore:
        memcpy((void*)outBtns[2], (void*)"Abort", 6*sizeof(char)); __P_ASSERT_LENGTH(5);
        memcpy((void*)outBtns[1], (void*)"Retry", 6*sizeof(char)); __P_ASSERT_LENGTH(5);
        memcpy((void*)outBtns[0], (void*)"Ignore", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 3;
      default: 
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
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
    char _stringAlloc[3*__P_MAX_LABEL_LENGTH] = { 0 };
    char* buttons[3] = { &_stringAlloc[0], &_stringAlloc[__P_MAX_LABEL_LENGTH], &_stringAlloc[2*__P_MAX_LABEL_LENGTH] };
    uint32_t length = __toNativeActions(actions, &buttons[0]);
    
    char* error = nullptr;
    uint32_t result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), const_cast<const char**>(&buttons[0]), length, 
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
                                           const char** placeholders, const char** outButtons) noexcept {
    // 3 buttons
    if (button3) { 
      outButtons[0] = button3;
      outButtons[1] = (button2) ? button2 : placeholders[1];
      outButtons[2] = (button1) ? button1 : placeholders[0];
      return 3;
    }
    // 2 buttons
    else if (button2) { 
      outButtons[0] = button2;
      outButtons[1] = (button1) ? button1 : placeholders[0];
      return 2;
    }
    // 1 button
    else { 
      outButtons[0] = (button1) ? button1 : placeholders[0];
      return 1;
    }
  }
  
  // ---
  
  // show modal message box with custom button labels
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle) noexcept {
    const char* placeholders[2] = { "OK", "No" }; // used if missing labels before last button (ok / okCancel / yesNoCancel)
    const char* buttons[3] = { nullptr };
    uint32_t length = __toNativeActions(button1, button2, button3, placeholders, &buttons[0]);
      
    char* error = nullptr;
    uint32_t result = __showMessageBox_cocoa(caption, message, __toNativeIcon(icon), &buttons[0], length, 
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
