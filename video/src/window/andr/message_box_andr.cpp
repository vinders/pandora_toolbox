/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - JNI implementation (Android)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cassert>
# include <cstdint>
# include <android/native_window.h>
# include "hardware/_private/_libraries_andr.h"
# include "video/_private/_message_box_common.h"
# include "video/message_box.h"

  using namespace pandora::video;
  using pandora::hardware::AndroidJavaSession;
  
  
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
      __MessageBox::setLastError(exc.what());
      return 0;
    }
  }
  
  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType icon, bool, WindowHandle) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(actions, &buttons[0]);
    uint32_t result = __showJavaMessageBox(caption, message, &buttons[0], length, icon);
    return __MessageBox::toDialogResult(result, length);
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  pandora::memory::LightString MessageBox::getLastError() noexcept { 
    return __MessageBox::getLastError();
  }
  
  
  
// -- custom message box -- ----------------------------------------------------

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(button1, button2, button3, &buttons[0]);
    uint32_t result = __showJavaMessageBox(caption, message, &buttons[0], length, icon);
    return __MessageBox::toDialogResult(result, length);
  }

#endif
