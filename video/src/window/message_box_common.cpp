/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
Description : Message box - common implementation helpers
*******************************************************************************/
#if !defined(_WINDOWS)
# include <mutex>
# include <thread/spin_lock.h>
# include "video/_private/_message_box_common.h"

  using namespace pandora::video;


// -- error messages -- --------------------------------------------------------

  static pandora::thread::SpinLock __lastErrorLock;
  static pandora::memory::LightString __lastError;
  
  // set last error message (in case of Result::failure)
  void pandora::video::__MessageBox::setLastError(const char* error) noexcept { 
    std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
    __lastError = error; 
    // do not throw on alloc failure: avoid leaks in calling code (free() called after it) + avoid breaking noexcept callers
  }

  // get copy of last error message (in case of Result::failure)
  pandora::memory::LightString pandora::video::__MessageBox::getLastError() noexcept { 
    std::lock_guard<pandora::thread::SpinLock> guard(__lastErrorLock);
    return __lastError;
  }


// -- action button labels -- --------------------------------------------------

# ifdef __APPLE__
#   define __index(i,maxI) (maxI - i)
# else
#   define __index(i,...) i
# endif

  // default action labels
  static const char* __getLabelOK() noexcept { return "OK"; }
  static const char* __getLabelCancel() noexcept { return "Cancel"; }
  static const char* __getLabelYes() noexcept { return "Yes"; }
  static const char* __getLabelNo() noexcept { return "No"; }
  static const char* __getLabelAbort() noexcept { return "Abort"; }
  static const char* __getLabelRetry() noexcept { return "Retry"; }
  static const char* __getLabelIgnore() noexcept { return "Ignore"; }

  // set action label (or placeholder if label is not defined)
  static inline const char* __setActionLabel(const char* action, const char* placeholder) noexcept {
    return (action != nullptr) ? action : placeholder;
  }

  // convert portable set of actions to list of button labels
  // params: outLabels: must be a pre-allocated 'const char*[3]'
  // returns: number of actions
  // note: on macOS/iOS: will reverse order of buttons (usual order on Apple systems)
  uint32_t pandora::video::__MessageBox::toActionLabels(MessageBox::ActionType actions, const char** outLabels) noexcept {
    switch (actions) {
      case MessageBox::ActionType::ok:
        outLabels[__index(0,0)] = __getLabelOK();
        return 1;
      case MessageBox::ActionType::okCancel:
        outLabels[__index(0,1)] = __getLabelOK();
        outLabels[__index(1,1)] = __getLabelCancel();
        return 2;
      case MessageBox::ActionType::retryCancel:
        outLabels[__index(0,1)] = __getLabelRetry();
        outLabels[__index(1,1)] = __getLabelCancel();
        return 2;
      case MessageBox::ActionType::yesNo:
        outLabels[__index(0,1)] = __getLabelYes();
        outLabels[__index(1,1)] = __getLabelNo();
        return 2;
      case MessageBox::ActionType::yesNoCancel:
        outLabels[__index(0,2)] = __getLabelYes();
        outLabels[__index(1,2)] = __getLabelNo();
        outLabels[__index(2,2)] = __getLabelCancel();
        return 3;
      case MessageBox::ActionType::abortRetryIgnore:
        outLabels[__index(0,2)] = __getLabelAbort();
        outLabels[__index(1,2)] = __getLabelRetry();
        outLabels[__index(2,2)] = __getLabelIgnore();
        return 3;
      default:
        outLabels[__index(0,0)] = __getLabelOK();
        return 1;
    }
  }

  // convert custom actions to list of button labels
  // params: outLabels: must be a pre-allocated 'const char*[3]'
  // returns: number of actions
  // note: on macOS/iOS: will reverse order of buttons (usual order on Apple systems)
  uint32_t pandora::video::__MessageBox::toActionLabels(const char* action1, const char* action2, const char* action3, const char** outLabels) noexcept {
    // 3 buttons
    if (action3) { 
      outLabels[__index(0,2)] = __setActionLabel(action1, __getLabelOK());
      outLabels[__index(1,2)] = __setActionLabel(action2, __getLabelNo());
      outLabels[__index(2,2)] = action3;
      return 3;
    }
    // 2 buttons
    else if (action2) { 
      outLabels[__index(0,1)] = __setActionLabel(action1, __getLabelOK());
      outLabels[__index(1,1)] = action2;
      return 2;
    }
    // 1 button
    else { 
      outLabels[__index(0,0)] = __setActionLabel(action1, __getLabelOK());
      return 1;
    }
  }

  // ---

  // convert action index to portable dialog result
  // params: - index: user action index (1 - 3, or 0 for errors)
  //         - maxActions: number of possible actions (1 - 3)
  // note: restore order on macOS/iOS (usual order reversed on Apple systems)
  MessageBox::Result pandora::video::__MessageBox::toDialogResult(uint32_t index, uint32_t maxActions) noexcept {
#   ifdef __APPLE__
      if (index == 0 || index > maxActions)
        return MessageBox::Result::failure;
      index = maxActions + 1 - index; // reverse
#   else
      if (index > maxActions)
        return MessageBox::Result::failure;
#   endif

    switch (index) {
      case 1: return MessageBox::Result::action1;
      case 2: return MessageBox::Result::action2;
      case 3: return MessageBox::Result::action3;
      default: return MessageBox::Result::failure;
    }
  }

#endif
