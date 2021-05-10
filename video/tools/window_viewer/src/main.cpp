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
#include <video/window_keycodes.h>
#include <video/window.h>

#if defined(__ANDROID__)
# include <stdexcept>
# include <android/log.h>
# include <system/api/android_app.h>
# define printf(...) __android_log_print(ANDROID_LOG_INFO, "-", __VA_ARGS__)
# ifndef LOGE
#   define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , ">", __VA_ARGS__)
# endif
#endif

#ifdef _WINDOWS
# include <system/api/windows_api.h>
# include "../../../../_img/test_win32/resources.h"
# define _SYSTEM_STR(str) L"" str
# define _SYSTEM_sprintf(...) _snwprintf(__VA_ARGS__)
#else
# define _SYSTEM_STR(str) str
# define _SYSTEM_sprintf(...) snprintf(__VA_ARGS__)
#endif

#define _DEFAULT_WINDOW_WIDTH 800
#define _DEFAULT_WINDOW_HEIGHT 600

using namespace pandora::video;


// -- console helpers -- -------------------------------------------------------

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


// -- window helpers -- --------------------------------------------------------

// create main window
std::unique_ptr<Window> createWindow(WindowType mode, WindowBehavior behavior, 
                                     ResizeMode resize, WindowResource::Color background,
                                     bool hasCustomCursor) { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
# else
    auto mainIcon = WindowResource::buildIconFromPackage("logo_big.png");
# endif

  std::shared_ptr<WindowResource> cursor = nullptr;
  if (hasCustomCursor) {
#   ifdef _WINDOWS
      cursor = WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR));
#   else
      cursor = WindowResource::buildCursorFromPackage("base_cur.png");
#   endif
  }
  
  Window::Builder builder;
  return builder.setDisplayMode(mode, behavior, resize)
         .setSize(_DEFAULT_WINDOW_WIDTH, _DEFAULT_WINDOW_HEIGHT)
         .setPosition(Window::Builder::centeredPosition(), Window::Builder::centeredPosition())
         .setIcon(mainIcon)
         .setCursor(cursor)
         .setBackgroundColor(WindowResource::buildColorBrush(background))
         .create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Example Window"));
}

// test data
bool g_isRefreshed = false;
bool g_hasScrollbars = false;
uint32_t g_lastSizeX = _DEFAULT_WINDOW_WIDTH;
uint32_t g_lastSizeY = _DEFAULT_WINDOW_HEIGHT;
uint32_t g_lastCursorPosX = 0;
uint32_t g_lastCursorPosY = 0;
uint32_t g_lastCharInput = (uint32_t)'-';
bool g_hasClicked = false;

// window/hardware event handler
bool onWindowEvent(Window*, WindowEvent event, uint32_t, int32_t, int32_t, void*) {
  switch (event) {
    case WindowEvent::windowClosed: {
      auto reply = MessageBox::show("Confirmation", "Are you sure you want to exit?", 
                                    MessageBox::ActionType::yesNo, MessageBox::IconType::question, true);
      if (reply == MessageBox::Result::action2) // "no" button
        return true; // cancel close event
      break;
    }
    default: break;
  }
  return false;
}
// size/position event handler
bool onPositionEvent(Window* sender, PositionEvent event, int32_t, int32_t, uint32_t sizeX, uint32_t sizeY) {
  switch (event) {
    case PositionEvent::sizePositionChanged: 
      g_lastSizeX = sizeX; 
      g_lastSizeY = sizeY; 
      if (g_hasScrollbars)
        sender->setScrollbarRange((uint16_t)sender->getScrollPositionH(), (uint16_t)sender->getScrollPositionV(), 
                                  (uint16_t)sizeX*2, (uint16_t)sizeY*2);
      g_isRefreshed = true;
      break;
    default: break;
  }
  return false;
}
// keyboard event handler
bool onKeyboardEvent(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t) {
  switch (event) {
    case KeyboardEvent::keyDown:
      if (keyCode == _P_VK_ESC)
        Window::sendCloseEvent(sender->handle());
      break;
    case KeyboardEvent::charInput:
      g_lastCharInput = keyCode;
      g_isRefreshed = true; 
      break;
    default: break;
  }
  return false;
}
// mouse event handler
bool onMouseEvent(Window*, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t) {
  switch (event) {
    case MouseEvent::mouseMove:
      g_lastCursorPosX = x;
      g_lastCursorPosY = y;
      g_isRefreshed = true;
      break;
    case MouseEvent::buttonDown:
      if ((MouseButton)index == MouseButton::left) {
        g_hasClicked = true;
        g_isRefreshed = true;
      }
      break;
    default: break;
  }
  return false;
}


// -- viewers -- ---------------------------------------------------------------

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
      default: break;
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
      default: break;
    }
  }
  return 0;
}

// Display window
void viewWindow(WindowType mode, ResizeMode resize, WindowBehavior behavior = WindowBehavior::none, 
                bool isCaptionFromInput = false, bool rainbowBackground = false, bool hasCustomCursor = false) {
  try {
    bool isFullscreen = (mode == WindowType::fullscreen);
    uint8_t r = 255, g = 0, b = 0;
    auto window = createWindow(mode, behavior, resize, rainbowBackground 
                                                       ? WindowResource::rgbColor(r,g,b) 
                                                       : WindowResource::rgbColor(0,0,0), hasCustomCursor);
    window->setMinClientAreaSize(400, 300);
    if ((behavior & WindowBehavior::scrollV) == true) {
      g_hasScrollbars = true;
      window->setScrollbarRange(0, 0, (uint16_t)_DEFAULT_WINDOW_WIDTH*2, (uint16_t)_DEFAULT_WINDOW_HEIGHT*2);
    }
    
    window->setWindowHandler(&onWindowEvent);
    window->setPositionHandler(&onPositionEvent);
    window->setKeyboardHandler(&onKeyboardEvent);
    window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible);
    window->show();
    
    while (Window::pollEvents()) {
      if (g_isRefreshed) {
        // update caption: input char, window size, cursor position
        window_char buffer[64]{ 0 };
        if (isCaptionFromInput)
          _SYSTEM_sprintf(buffer, sizeof(buffer)/sizeof(window_char), _SYSTEM_STR("Example Window - input:%c - %ux%u [%u-%u]"), 
                          (window_char)g_lastCharInput, g_lastSizeX, g_lastSizeY, g_lastCursorPosX, g_lastCursorPosY);
        else
          _SYSTEM_sprintf(buffer, sizeof(buffer)/sizeof(window_char), _SYSTEM_STR("Example Window - %ux%u [%u-%u]"),
                           g_lastSizeX, g_lastSizeY, g_lastCursorPosX, g_lastCursorPosY);
        window->setCaption((const window_char*)buffer);

        // fullscreen mode: toggle window/fullscreen on click
        if (mode == WindowType::fullscreen && g_hasClicked) {
          pandora::hardware::DisplayArea clientArea{ 0, 0, _DEFAULT_WINDOW_WIDTH, _DEFAULT_WINDOW_HEIGHT };
          if (!isFullscreen) {
            clientArea.x = Window::Builder::centeredPosition(); 
            clientArea.y = Window::Builder::centeredPosition();
          }
          window->setDisplayMode(isFullscreen ? WindowType::window : WindowType::fullscreen, behavior, resize, clientArea);

          isFullscreen ^= true;
          g_hasClicked = false;
        }
        g_isRefreshed = false;
      }
      // update background (rainbow animation)
      if (rainbowBackground) {
        if (r > 0 && b == 0) {
          --r; ++g;
        }
        else if (g > 0) {
          --g; ++b;
        }
        else {
          --b; ++r;
        }
        window->setBackgroundColorBrush(WindowResource::buildColorBrush(WindowResource::rgbColor(r,g,b)));
        window->clearClientArea(); // repaint
      }
      std::this_thread::sleep_for(std::chrono::microseconds(16666LL)); // 60Hz
    }
  }
  catch (const std::exception& exc) {
    MessageBox::show("Fatal error", exc.what(), MessageBox::ActionType::ok, MessageBox::IconType::error, true);
  }
}


// -- menu -- ------------------------------------------------------------------

// message-box menu
void menuMessageBox() {
  bool isRunning = true;
  uint32_t lastAction = 0;
  while (isRunning) {
    clearScreen();
    printf("Window viewer: message-box\n_________________________________________________\n");
    if (lastAction)
      printf("-- last user choice: button %u", lastAction);

    printf("\nMessageBox type :\n");
    printMenu<7>({ "Back to main menu...", "OK", "OK-Cancel", "Abort-Retry-Ignore", 
                                           "1 custom action", "2 custom actions", "3 custom actions" });
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
}

// dialog menu
void menuDialog() {
  bool isRunning = true;
  while (isRunning) {
    clearScreen();
    printf("Window viewer: dialog\n_________________________________________________\n");

    printf("\nDialog (exit: ESC or close button) :\n");
    printMenu<6>({ "Back to main menu...", "Fixed", "Resizable", "Homothety", "No caption", "Rainbow" });
    int option = readNumericInput(0, 5);
    switch (option) {
      case 1: viewWindow(WindowType::dialog, ResizeMode::fixed); break;
      case 2: viewWindow(WindowType::dialog, ResizeMode::resizable); break;
      case 3: viewWindow(WindowType::dialog, ResizeMode::resizable|ResizeMode::homothety); break;
      case 4: viewWindow(WindowType::bordered, ResizeMode::resizable); break;
      case 5: viewWindow(WindowType::dialog, ResizeMode::resizable|ResizeMode::homothety, 
                         WindowBehavior::none, false, true); break;
      case 0:
      default: isRunning = false; break;
    }
  }
}

// window menu
void menuWindow() {
  bool isRunning = true;
  while (isRunning) {
    clearScreen();
    printf("Window viewer: window\n_________________________________________________\n");

    printf("\nWindow (exit: ESC or close button) :\n");
    printMenu<6>({ "Back to main menu...", "Fixed, caption input", "Resizable, scrollable", "Homothety custom cursor", "Borderless", 
                                           "Fullscreen/window (click to toggle)" });
    int option = readNumericInput(0, 5);
    switch (option) {
      case 1: viewWindow(WindowType::window, ResizeMode::fixed, WindowBehavior::none, true); break;
      case 2: viewWindow(WindowType::window, ResizeMode::resizable, WindowBehavior::scrollV|WindowBehavior::scrollH); break;
      case 3: viewWindow(WindowType::window, ResizeMode::resizable|ResizeMode::homothety, 
                         WindowBehavior::none, false, false, true); break;
      case 4: viewWindow(WindowType::borderless, ResizeMode::fixed); break;
      case 5: viewWindow(WindowType::fullscreen, ResizeMode::resizable|ResizeMode::homothety); break;
      case 0:
      default: isRunning = false; break;
    }
  }
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
#   ifdef _WINDOWS
      pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)GetModuleHandle(nullptr));
#   endif

    bool isRunning = true;
    while (isRunning) {
      clearScreen();
      printf("Window viewer: main menu\n_________________________________________________\n");

      printf("\nWindow type :\n");
      printMenu<4>({ "Exit...", "MessageBox", "Dialog", "Window" });
      int option = readNumericInput(0, 3);
      switch (option) {
        case 1: menuMessageBox(); break;
        case 2: menuDialog(); break;
        case 3: menuWindow(); break;
        case 0:
        default: isRunning = false; break;
      }
    }
    return 0;
  }
#endif
