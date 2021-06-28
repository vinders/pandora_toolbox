/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory/light_string.h>
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
      enum class Result : uint32_t {
        failure = 0, ///< error during message box creation
        action1 = 1, ///< user action #1 (example: "yes" for ActionType "yesNoCancel"    / "button1" for custom actions)
        action2 = 2, ///< user action #2 (example: "no" for ActionType "yesNoCancel"     / "button2" for custom actions)
        action3 = 3, ///< user action #3 (example: "cancel" for ActionType "yesNoCancel" / "button3" for custom actions)
      };
      
      // ---

      /// @brief Show modal message box (wait for user action)
      /// @param caption    Title of the message box
      /// @param message    Text content of the message box
      /// @param actions    Available user actions (buttons)
      /// @param icon       Optional symbol to display in message box (not used on linux systems)
      /// @param isTopMost  Make modal dialog appear on top of all windows
      /// @param parent     Parent window blocked by dialog (optional)
      /// @returns Action chosen by user (or Result::failure if the dialog could not be created)
      static Result show(const char* caption, const char* message, ActionType actions = ActionType::ok, 
                         IconType icon = IconType::none, bool isTopMost = false, WindowHandle parent = (WindowHandle)0) noexcept;
#     if defined(_WINDOWS)
        static Result show(const wchar_t* caption, const wchar_t* message, ActionType actions = ActionType::ok, 
                           IconType icon = IconType::none, bool isTopMost = false, WindowHandle parent = (WindowHandle)0) noexcept;
#     endif

      /// @brief Show modal message box with custom button labels (wait for user action)
      /// @param caption    Title of the message box
      /// @param message    Text content of the message box
      /// @param icon       Optional symbol to display in message box (not used on linux systems)
      /// @param button1    Label of button 1
      /// @param button2    Label of button 2 (optional -> do not set if you only need 1 button)
      /// @param button3    Label of button 3 (optional -> do not set if you only need 2 buttons)
      /// @param isTopMost  Make modal dialog appear on top of all windows
      /// @param parent     Parent window blocked by dialog (optional)
      /// @returns Action chosen by user (or Result::failure if the dialog could not be created)
      static Result show(const char* caption, const char* message, IconType icon, 
                         const char* button1, const char* button2 = nullptr, const char* button3 = nullptr,
                         bool isTopMost = false, WindowHandle parent = (WindowHandle)0) noexcept;
#     if defined(_WINDOWS)
        static Result show(const wchar_t* caption, const wchar_t* message, IconType icon, 
                           const wchar_t* button1, const wchar_t* button2 = nullptr, const wchar_t* button3 = nullptr,
                           bool isTopMost = false, WindowHandle parent = (WindowHandle)0) noexcept;
#     endif

      /// @brief Get last error message (in case of Result::failure)
      /// @returns Last error (if available) or empty string
      static pandora::memory::LightString getLastError() noexcept;
    };
  }
}
