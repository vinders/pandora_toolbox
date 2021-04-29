/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <cassert>
# include <cstdint>
# include <mutex>
# include <system/api/windows_api.h>
# include "video/message_box.h"

# ifdef MessageBox
#   undef MessageBox // fix win32 API conflicts
#  endif
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

  using namespace pandora::video;
  
  static std::mutex __modalDialogLock; // prevent new dialogs from using hooks defined for previous ones
  

// -- simple message box -- ----------------------------------------------------

  // convert portable dialog actions to native type flags
  static UINT _toNativeAction(MessageBox::ActionType actions, int outActionFlags[3]) noexcept {
    UINT flags = 0;
    switch (actions) {
      case MessageBox::ActionType::ok:
        flags = (flags | MB_OK); 
        outActionFlags[0] = IDOK;
        break;
      case MessageBox::ActionType::okCancel:
        flags = (flags | MB_OKCANCEL); 
        outActionFlags[0] = IDOK; outActionFlags[1] = IDCANCEL; 
        break;
      case MessageBox::ActionType::retryCancel:
        flags = (flags | MB_RETRYCANCEL); 
        outActionFlags[0] = IDRETRY; outActionFlags[1] = IDCANCEL; 
        break;
      case MessageBox::ActionType::yesNo:
        flags = (flags | MB_YESNO); 
        outActionFlags[0] = IDYES; outActionFlags[1] = IDNO; 
        break;
      case MessageBox::ActionType::yesNoCancel:
        flags = (flags | MB_YESNOCANCEL); 
        outActionFlags[0] = IDYES; outActionFlags[1] = IDNO; outActionFlags[2] = IDCANCEL; 
        break;
      case MessageBox::ActionType::abortRetryIgnore:
        flags = (flags | MB_ABORTRETRYIGNORE); 
        outActionFlags[0] = IDABORT; outActionFlags[1] = IDRETRY; outActionFlags[2] = IDIGNORE; 
        break;
      default: 
        flags = MB_OK; outActionFlags[0] = IDOK; break;
    }
    return flags;
  }

  // convert portable dialog style to native type flag
  static UINT _toNativeStyle(MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    UINT flags = (parent != (WindowHandle)0) ? MB_APPLMODAL : MB_TASKMODAL;
    switch (icon) {
      case MessageBox::IconType::info:     flags = (flags | MB_ICONINFORMATION); break;
      case MessageBox::IconType::question: flags = (flags | MB_ICONQUESTION); break;
      case MessageBox::IconType::warning:  flags = (flags | MB_ICONWARNING); break;
      case MessageBox::IconType::error:    flags = (flags | MB_ICONERROR); break;
    }
    if (isTopMost)
      flags = (flags | MB_SETFOREGROUND | MB_TOPMOST);
    return flags;
  }
  
  // convert native result ID to portable dialog result
  static MessageBox::Result _toDialogResult(int resultId, int actionFlags[3]) noexcept {
    if (resultId == actionFlags[0]) return MessageBox::Result::action1;
    if (resultId == actionFlags[1]) return MessageBox::Result::action2;
    if (resultId == actionFlags[2]) return MessageBox::Result::action3;
    return MessageBox::Result::failure;
  }

  // ---

  // show modal message box - narrow characters
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                            MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    int actionFlags[3] = { 0 };
    
    std::lock_guard<std::mutex> guard(__modalDialogLock);
    int resultId = MessageBoxA((HWND)parent, message, caption, 
                               (_toNativeAction(actions, actionFlags) | _toNativeStyle(icon, isTopMost, parent)) );
    return _toDialogResult(resultId, actionFlags);
  }
  
  // show modal message box - wide characters
  MessageBox::Result MessageBox::show(const wchar_t* caption, const wchar_t* message, MessageBox::ActionType actions, 
                                            MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    int actionFlags[3] = { 0 };
    
    std::lock_guard<std::mutex> guard(__modalDialogLock);
    int resultId = MessageBoxW((HWND)parent, message, caption, 
                               (_toNativeAction(actions, actionFlags) | _toNativeStyle(icon, isTopMost, parent)) );
    return _toDialogResult(resultId, actionFlags);
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string LastError() {
    DWORD errorNb = ::GetLastError();
    if (errorNb == 0)
      return ""; // no error found
    
    LPSTR buffer = nullptr;
    size_t length = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   nullptr, errorNb, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, nullptr);
    if (buffer == nullptr)
      return "";

    try {
      std::string message(buffer, length);
      LocalFree(buffer);
      return message;
    }
    catch (const std::bad_alloc&) { LocalFree(buffer); throw; } // no leaks
  }
  
  
// -- custom message box -- ----------------------------------------------------
  
  static const void* __hookButtons[3] = { nullptr };
  static HHOOK __hookHandle = nullptr;
  static bool __useWideChar = false;
  
  // change button label
  static inline void __setButtonLabel(HWND dialogHandle, int buttonId, const void* label) noexcept {
    if (__useWideChar)
      SetDlgItemTextW(dialogHandle, buttonId, (const wchar_t*)label);
    else
      SetDlgItemTextA(dialogHandle, buttonId, (const char*)label);
  }
  
  // message box hook processor
  static LRESULT CALLBACK __messageBoxProcessor(INT nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HCBT_ACTIVATE) { // dialog is activated -> set button labels
      HWND dialogHandle = (HWND)wParam;

      if (__hookButtons[2] != nullptr) {
        if (GetDlgItem(dialogHandle, IDNO) == nullptr || GetDlgItem(dialogHandle, IDYES) == nullptr || GetDlgItem(dialogHandle, IDCANCEL) == nullptr)
          return CallNextHookEx(__hookHandle, nCode, wParam, lParam); // wrong window ?
          
        if (__hookButtons[0] != nullptr)
          __setButtonLabel(dialogHandle, IDYES, __hookButtons[0]);
        if (__hookButtons[1] != nullptr)
          __setButtonLabel(dialogHandle, IDNO, __hookButtons[1]);
        __setButtonLabel(dialogHandle, IDCANCEL, __hookButtons[2]);
      }
      else {
        if (GetDlgItem(dialogHandle, IDOK) == nullptr || GetDlgItem(dialogHandle, IDCANCEL) == nullptr)
          return CallNextHookEx(__hookHandle, nCode, wParam, lParam); // wrong window ?
        
        if (__hookButtons[0] != nullptr)
          __setButtonLabel(dialogHandle, IDOK, __hookButtons[0]);
        if (__hookButtons[1] != nullptr)
          __setButtonLabel(dialogHandle, IDCANCEL, __hookButtons[1]);
      }
      
      UnhookWindowsHookEx(__hookHandle); // remove hook
      __hookHandle = nullptr;
      return 0;
    }
    else 
      return CallNextHookEx(__hookHandle, nCode, wParam, lParam);
  }
  
  // convert custom dialog actions to native type flags
  template <typename _CharType>
  static UINT _toNativeAction(const _CharType* button1, const _CharType* button2, 
                             const _CharType* button3, int outActionFlags[3]) noexcept {
    UINT flags = 0;
    if (button3) {
      flags = (flags | MB_YESNOCANCEL);
      outActionFlags[0] = IDYES; outActionFlags[1] = IDNO; outActionFlags[2] = IDCANCEL;
    }
    else if (button2) {
      flags = (flags | MB_OKCANCEL);
      outActionFlags[0] = IDOK; outActionFlags[1] = IDCANCEL;
    }
    else {
      flags = (flags | MB_OK);
      outActionFlags[0] = IDOK;
    }
    
    // set hook flags
    __hookButtons[0] = (void*)button1;
    __hookButtons[1] = (void*)button2;
    __hookButtons[2] = (void*)button3;
    __if_constexpr (std::is_same<_CharType, wchar_t>::value) {
      __useWideChar = true;
      __hookHandle = SetWindowsHookExW(WH_CBT, &__messageBoxProcessor, 0, GetCurrentThreadId());
    }
    else {
      __useWideChar = false;
      __hookHandle = SetWindowsHookExA(WH_CBT, &__messageBoxProcessor, 0, GetCurrentThreadId());
    }
    return flags;
  }
  
  // ---
  
  // show modal message box with custom button labels - narrow characters
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType icon,
                                      const char* button1, const char* button2, const char* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    int actionFlags[3] = { 0 };
    
    std::lock_guard<std::mutex> guard(__modalDialogLock);
    int resultId = MessageBoxA((HWND)parent, message, caption, 
                               (_toNativeAction(button1, button2, button3, actionFlags) | _toNativeStyle(icon, isTopMost, parent)) );
    if (__hookHandle)
      UnhookWindowsHookEx(__hookHandle);
    return _toDialogResult(resultId, actionFlags);
  }
  
  // show modal message box with custom button labels - wide characters
  MessageBox::Result MessageBox::show(const wchar_t* caption, const wchar_t* message, MessageBox::IconType icon,
                                      const wchar_t* button1, const wchar_t* button2, const wchar_t* button3,
                                      bool isTopMost, WindowHandle parent) noexcept {
    int actionFlags[3] = { 0 };
    
    std::lock_guard<std::mutex> guard(__modalDialogLock);
    int resultId = MessageBoxW((HWND)parent, message, caption, 
                               (_toNativeAction(button1, button2, button3, actionFlags) | _toNativeStyle(icon, isTopMost, parent)) );
    if (__hookHandle)
      UnhookWindowsHookEx(__hookHandle);
    return _toDialogResult(resultId, actionFlags);
  }

#endif
