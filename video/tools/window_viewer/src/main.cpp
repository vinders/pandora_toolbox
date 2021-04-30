/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Window viewer - utility to display window, dialog, or modal message-box
*******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1
#include <cstdio>
#include <cstddef>
#include <string>
#include <array>
#include <video/message_box.h>

#if defined(__ANDROID__)
# include <stdexcept>
# include <android/log.h>
# include <system/api/android_app.h>
# define printf(...) __android_log_print(ANDROID_LOG_INFO, "-", __VA_ARGS__)
# ifndef LOGE
#   define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , ">", __VA_ARGS__)
# endif
#endif

using namespace pandora::video;

// Clear all traces in console
void clearScreen() {
# ifdef _WINDOWS
    system("cls");
# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
    if (system("clear") == -1)
      printf("\n____________________________________________________________\n");
# endif
}

// Display menu (first entry should be return/exit command)
template <size_t _Size>
inline void printMenu(const std::array<std::string, _Size>& items) {
  for (int i = 1; i < static_cast<int>(_Size); ++i)
    printf("> %d - %s\n", i, items[i].c_str());
  printf("> 0 - %s\n\n", items[0].c_str());
}

// Get numeric user input
inline int readNumericInput(int minValue, int maxValue) noexcept {
  int val = -1;
  bool isValid = false;
  printf("Enter a value (%d-%d, or 0) :\n> ", minValue, maxValue);

  do {
    fflush(stdin);
    isValid = (scanf("%d", &val) > 0 && (val == 0 || (val >= minValue && val <= maxValue)) );
    if (!isValid)
      printf("Invalid value. Please try again (%d-%d or 0) :\n> ", minValue, maxValue);
  } while (!isValid);

  while (getchar() != '\n'); // clear buffer
  return val;
}

// ---

// Display message box
uint32_t viewMessageBox(uint32_t numberOfActions, bool useCustomLabels) {
  if (!useCustomLabels) {
    switch (numberOfActions) {
      case 1: return (uint32_t)MessageBox::show("Hello World!", "Simple message-box with 1 button...", 
                                                MessageBox::ActionType::ok, MessageBox::IconType::none);
      case 2: return (uint32_t)MessageBox::show("Choice box", "Simple message-box with 2 buttons.", 
                                                MessageBox::ActionType::okCancel, MessageBox::IconType::question);
      case 3: return (uint32_t)MessageBox::show("Error box", "Simple message-box with 3 buttons!", 
                                                MessageBox::ActionType::abortRetryIgnore, MessageBox::IconType::error);
    }
  }
  else {
    switch (numberOfActions) {
      case 1: return (uint32_t)MessageBox::show("Hello World!", "Custom message-box with 1 button...", 
                                                MessageBox::IconType::none, "Apply");
      case 2: return (uint32_t)MessageBox::show("Choice box", "Custom message-box with 2 buttons.", 
                                                MessageBox::IconType::info, "Accept", "Deny");
      case 3: return (uint32_t)MessageBox::show("Error box", "Custom message-box with 3 buttons!\nPlease select a button.", 
                                                MessageBox::IconType::warning, "Save", "Ignore", "Cancel");
    }
  }
  return 0;
}

// ---

#if defined(__ANDROID__)
  // Window viewer - entry point for Android
  void android_main(struct android_app* state) {
    try {
      pandora::system::AndroidApp::instance().init(state);
      printf("-- user choice: %u", viewMessageBox(1, false));
      printf("-- user choice: %u", viewMessageBox(2, false));
      printf("-- user choice: %u", viewMessageBox(3, false));
      printf("-- user choice: %u", viewMessageBox(1, true));
      printf("-- user choice: %u", viewMessageBox(2, true));
      printf("-- user choice: %u", viewMessageBox(3, true));
    }
    catch (const std::exception& exc) { LOGE("%s", exc.what()); }
  }
  
#else
  // Window viewer - main menu
  int main() {
    bool isRunning = true;
    uint32_t lastAction = 0;
    while (isRunning) {
      clearScreen();
      printf("Window viewer: show window / dialog / message-box\n_________________________________________________\n");
      if (lastAction)
        printf("-- last user choice: button %u", lastAction);

      printf("\nWindow type :\n");
      printMenu<7>({ "Exit...", "MessageBox: OK", "MessageBox: OK-Cancel", "MessageBox: Abort-Retry-Ignore", 
                                "MessageBox: 1 custom action", "MessageBox: 2 custom actions", "MessageBox: 3 custom actions" });
      int option = readNumericInput(0, 6);
      switch (option) {
        case 1: lastAction = viewMessageBox(1, false); break;
        case 2: lastAction = viewMessageBox(2, false); break;
        case 3: lastAction = viewMessageBox(3, false); break;
        case 4: lastAction = viewMessageBox(1, true); break;
        case 5: lastAction = viewMessageBox(2, true); break;
        case 6: lastAction = viewMessageBox(3, true); break;
        case 0:
        default: isRunning = false; break;
      }
    }
    return 0;
  }
#endif
