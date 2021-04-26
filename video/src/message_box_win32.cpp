/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <cassert>
# include <cstdint>
# include <system/api/windows_api.h>
# include "video/message_box.h"

# ifdef MessageBox
#   undef MessageBox // fix conflicts with win32 API
#  endif

  using namespace pandora::video;
  

  // convert portable enums to native type flags
  static UINT _toNativeType(MessageBox::ActionType actions, MessageBox::IconType icon, bool isTopMost, bool hasHandle) noexcept {
    UINT flags = (hasHandle) ? MB_APPLMODAL : MB_TASKMODAL;
    switch (actions) {
      case MessageBox::ActionType::ok:               flags = (flags | MB_OK); break;
      case MessageBox::ActionType::okCancel:         flags = (flags | MB_OKCANCEL); break;
      case MessageBox::ActionType::retryCancel:      flags = (flags | MB_RETRYCANCEL); break;
      case MessageBox::ActionType::yesNo:            flags = (flags | MB_YESNO); break;
      case MessageBox::ActionType::yesNoCancel:      flags = (flags | MB_YESNOCANCEL); break;
      case MessageBox::ActionType::abortRetryIgnore: flags = (flags | MB_ABORTRETRYIGNORE); break;
      default: flags = MB_OK; break;
    }
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
  static MessageBox::DialogResult _toDialogResult(int resultId) noexcept {
    switch (resultId) {
      case IDOK:     return MessageBox::DialogResult::ok;
      case IDCANCEL: return MessageBox::DialogResult::cancel;
      case IDYES:    return MessageBox::DialogResult::yes;
      case IDNO:     return MessageBox::DialogResult::no;
      case IDABORT:  return MessageBox::DialogResult::abort;
      case IDRETRY:
      case IDTRYAGAIN: return MessageBox::DialogResult::retry;
      case IDIGNORE:
      case IDCONTINUE: return MessageBox::DialogResult::ignore;
      case 0:
      default: return MessageBox::DialogResult::failure;
    }
  }
  
  // ---

  // show modal message box - narrow characters
  MessageBox::DialogResult MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                            MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    int resultId = MessageBoxA((HWND)parent, message, caption, _toNativeType(actions, icon, isTopMost, (parent != (WindowHandle)0)) );
    return _toDialogResult(resultId);
  }
  
  // show modal message box - wide characters
  MessageBox::DialogResult MessageBox::show(const wchar_t* caption, const wchar_t* message, MessageBox::ActionType actions, 
                                            MessageBox::IconType icon, bool isTopMost, WindowHandle parent) noexcept {
    int resultId = MessageBoxW((HWND)parent, message, caption, _toNativeType(actions, icon, isTopMost, (parent != (WindowHandle)0)) );
    return _toDialogResult(resultId);
  }
  
  // ---
  
  // get last error message (in case of DialogResult::failure)
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
    catch (const std::bad_alloc&) { LocalFree(buffer); throw; }
  }

#endif
