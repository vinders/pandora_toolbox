/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <string>
#include "video/window_handle.h"
#ifdef MessageBox
# undef MessageBox // fix win32 API conflicts
#endif

namespace pandora {
  namespace video {
    /// @class MessageBox
    /// @brief Modal message box (message, warning, confirmation, question...)
    class MessageBox final {
    public:
      MessageBox() = delete;

      /// @brief Message box category (user actions available)
      enum class ActionType : uint32_t {
        ok = 0,
        okCancel,
        retryCancel,
        yesNo,
        yesNoCancel,
        abortRetryIgnore
      };
      /// @brief Symbol displayed in message box
      enum class IconType : uint32_t {
        none = 0,
        info,
        question,
        warning,
        error
      };
      /// @brief User action chosen to close dialog (or failure if an error occurred)
      enum class DialogResult : uint32_t {
        failure = 0,
        ok,
        cancel,
        yes,
        no,
        abort,
        retry,
        ignore
      };

      /// @brief Show modal message box (wait for user action)
      /// @param caption    Title of the message box
      /// @param message    Text content of the message box
      /// @param actions    Available user actions (buttons)
      /// @param icon       Optional symbol to display in message box
      /// @param isTopMost  Make modal dialog appear on top of all windows
      /// @param parent     Parent window blocked by dialog (optional)
      /// @returns Action chosen by user (or DialogResult::failure if the dialog could not be created)
      static DialogResult show(const char* caption, const char* message, ActionType actions = ActionType::ok, 
                               IconType icon = IconType::none, bool isTopMost = false, WindowHandle parent = (WindowHandle)0) noexcept;
#     if defined(_WINDOWS)
        static DialogResult show(const wchar_t* caption, const wchar_t* message, ActionType actions = ActionType::ok, 
                                 IconType icon = IconType::none, bool isTopMost = false, WindowHandle parent = (WindowHandle)0) noexcept;
#     endif

      /// @brief Get last error message (in case of DialogResult::failure)
      /// @returns Last error (if available) or empty string
      static std::string LastError();
    };
  }
}
