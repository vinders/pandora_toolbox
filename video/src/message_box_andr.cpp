/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - JNI implementation (Android)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cassert>
# include <cstdint>
# include <mutex>
# include <thread/spin_lock.h>
# include <android/native_window.h>
# include "hardware/_private/_libraries_andr.h"
# include "video/message_box.h"

# define __P_MAX_LABEL_LENGTH 8
# define __P_ASSERT_LENGTH(n) static_assert((n) < __P_MAX_LABEL_LENGTH, "MessageBox (android): action value too long");

  using namespace pandora::video;
  using pandora::hardware::AndroidJavaSession;
  
  static pandora::thread::SpinLock __lastErrorLock;
  static std::string __lastError;
  
  
// -- simple message box -- ----------------------------------------------------
  
# define ICON_NONE 0
# define ICON_INFO 1
# define ICON_QUESTION 2
# define ICON_WARNING 3
# define ICON_ERROR 4

  // convert portable icon enum to java binding
  static inline int __toNativeIcon(MessageBox::IconType icon) noexcept {
    switch (icon) {
      case MessageBox::IconType::info: return ICON_INFO;
      case MessageBox::IconType::question: return ICON_QUESTION;
      case MessageBox::IconType::warning: return ICON_WARNING;
      case MessageBox::IconType::error: return ICON_ERROR;
      default: return ICON_NONE;
    }
  }
  
  // convert portable actions type to list of button labels
  static inline uint32_t __toNativeActions(MessageBox::ActionType actions, char** outBtns) noexcept {
    // warning: values must not exceed: __P_MAX_LABEL_LENGTH (including trailing zero)
    switch (actions) {
      case MessageBox::ActionType::ok:
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        return 1;
      case MessageBox::ActionType::okCancel:
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        memcpy((void*)outBtns[1], (void*)"Cancel", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 2;
      case MessageBox::ActionType::retryCancel:
        memcpy((void*)outBtns[0], (void*)"Retry", 6*sizeof(char)); __P_ASSERT_LENGTH(5);
        memcpy((void*)outBtns[1], (void*)"Cancel", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 2;
      case MessageBox::ActionType::yesNo:
        memcpy((void*)outBtns[0], (void*)"Yes", 4*sizeof(char)); __P_ASSERT_LENGTH(3);
        memcpy((void*)outBtns[1], (void*)"No", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        return 2;
      case MessageBox::ActionType::yesNoCancel:
        memcpy((void*)outBtns[0], (void*)"Yes", 4*sizeof(char)); __P_ASSERT_LENGTH(3);
        memcpy((void*)outBtns[1], (void*)"No", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        memcpy((void*)outBtns[2], (void*)"Cancel", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 3;
      case MessageBox::ActionType::abortRetryIgnore:
        memcpy((void*)outBtns[0], (void*)"Abort", 6*sizeof(char)); __P_ASSERT_LENGTH(5);
        memcpy((void*)outBtns[1], (void*)"Retry", 6*sizeof(char)); __P_ASSERT_LENGTH(5);
        memcpy((void*)outBtns[2], (void*)"Ignore", 7*sizeof(char)); __P_ASSERT_LENGTH(6);
        return 3;
      default: 
        memcpy((void*)outBtns[0], (void*)"OK", 3*sizeof(char)); __P_ASSERT_LENGTH(2);
        return 1;
    }
  }
  
  // convert native result to portable dialog Result
  static MessageBox::Result __toDialogResult(uint32_t result) noexcept {
    switch (result) {
      case 1: return MessageBox::Result::action1;
      case 2: return MessageBox::Result::action2;
      case 3: return MessageBox::Result::action3;
      default: return MessageBox::Result::failure;
    }
  }

  // ---
  
  // show modal message box from java module
  uint32_t __showJavaMessageBox(const char* caption, const char* message, 
                                const char** actions, uint32_t length, MessageBox::IconType icon) noexcept {
    try {
      AndroidJavaSession jenv;
      
      // get context
      jclass activityThreadClass = jenv.env().FindClass("android/app/ActivityThread");
      _P_THROW_ON_BINDING_FAILURE(jenv, "activityThread", activityThreadClass);
      jmethodID currentActivityThread = jenv.env().GetStaticMethodID(activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "activityThread method", currentActivityThread);
      jobject activityThread = jenv.env().CallStaticObjectMethod(activityThreadClass, currentActivityThread);
      _P_THROW_ON_ACCESS_FAILURE(jenv, "activityThread", activityThread);
      jmethodID getApplication = jenv.env().GetMethodID(activityThreadClass, "getApplication", "()Landroid/app/Application;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "activityThread method", getApplication);
      jobject context = jenv.env().CallObjectMethod(activityThread, getApplication);
      _P_THROW_ON_ACCESS_FAILURE(jenv, "context", context);
      
      // MessageBox definition
      jclass messageBoxClass = jenv.env().FindClass("pandora/video/MessageBox");
      _P_THROW_ON_BINDING_FAILURE(jenv, "messageBox", messageBoxClass);
      jmethodID messageBoxConstructor = jenv.env().GetMethodID(messageBoxClass, "<init>", "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;I[Ljava/lang/String;)V");
      _P_THROW_ON_BINDING_FAILURE(jenv, "messageBox method", messageBoxConstructor);
      jmethodID showMessageBox = jenv.env().GetMethodID(messageBoxClass, "show", "()I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "messageBox.show method", showMessageBox);
      
      // prepare params
      int iconInt = __toNativeIcon(icon);
      jstring captionStr = jenv.env().NewStringUTF((caption != nullptr) ? caption : "Error");
      jstring messageStr = jenv.env().NewStringUTF((message != nullptr) ? message : "");
      
      jclass stringClass = jenv.env().FindClass("Ljava/lang/String;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "String[]", stringClass);
      jobjectArray actionsArray = jenv.env().NewObjectArray(length, stringClass, nullptr);
      _P_THROW_ON_ACCESS_FAILURE(jenv, "String[]", actionsArray);
      for (uint32_t i = 0; i < length; ++i) {
        jstring actionStr = jenv.env().NewStringUTF(actions[i]);
        jenv.env().SetObjectArrayElement(actionsArray, i, actionStr);
        _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "String[].SetArrayElement");
      }
      
      // show modal message box + wait for user action
      jobject messageBox = jenv.env().NewObject(messageBoxClass, messageBoxConstructor, context, captionStr, messageStr, iconInt, actionsArray); // new
      _P_THROW_ON_ACCESS_FAILURE(jenv, "MessageBox.init", messageBox);
      int result = jenv.env().CallIntMethod(messageBox, showMessageBox); // result = messageBox.show();
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "MessageBox.show");

      return static_cast<uint32_t>(result);
    }
    catch (const std::exception& exc) {
      try { 
        std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
        __lastError = exc.what(); 
      } catch (...) {}
      return 0;
    }
  }
  
  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool, WindowHandle) noexcept {
    char _stringAlloc[3*__P_MAX_LABEL_LENGTH] = { 0 };
    char* buttons[3] = { &_stringAlloc[0], &_stringAlloc[__P_MAX_LABEL_LENGTH], &_stringAlloc[2*__P_MAX_LABEL_LENGTH] };
    uint32_t length = __toNativeActions(actions, &buttons[0]);
    
    return __toDialogResult(__showJavaMessageBox(caption, message, const_cast<const char**>(&buttons[0]), length, icon));
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string LastError() { 
    std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
    return __lastError;
  }
  
  
  
// -- custom message box -- ----------------------------------------------------

  // convert custom actions to list of button labels (missing labels are replaced with placeholders)
  static inline uint32_t __toNativeActions(const char* button1, const char* button2, const char* button3, 
                                           const char** placeholders, const char** outButtons) noexcept {
    // 3 buttons
    if (button3) { 
      outButtons[0] = (button1) ? button1 : placeholders[0];
      outButtons[1] = (button2) ? button2 : placeholders[1];
      outButtons[2] = button3;
      return 3;
    }
    // 2 buttons
    else if (button2) { 
      outButtons[0] = (button1) ? button1 : placeholders[0];
      outButtons[1] = button2;
      return 2;
    }
    // 1 button
    else { 
      outButtons[0] = (button1) ? button1 : placeholders[0];
      return 1;
    }
  }
  
  // ---
  
  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    const char* placeholders[2] = { "OK", "No" }; // used if missing labels before last button (ok / okCancel / yesNoCancel)
    const char* buttons[3] = { nullptr };
    uint32_t length = __toNativeActions(button1, button2, button3, placeholders, &buttons[0]);
    
    return __toDialogResult(__showJavaMessageBox(caption, message, const_cast<const char**>(&buttons[0]), length, icon));
  }

#endif
