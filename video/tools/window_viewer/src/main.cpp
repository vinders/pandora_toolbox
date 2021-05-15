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
#define _REFRESH_RATE 60

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
    isValid = (scanf("%d", &val) > 0 && (val == 0 || (val >= minValue && val <= maxValue)) );
    if (!isValid) {
      while(getchar() != '\n');
      printf("Invalid value. Please try again (%d-%d or 0) :\n> ", minValue, maxValue);
    }
  } while (!isValid);

  while (getchar() != '\n'); // clear buffer
  return val;
}


// -- window helpers -- --------------------------------------------------------

enum class FeatureMode : uint32_t {
  none = 0,
  inputKeyInCaption,
  fullscreenToggle,
  cursorChangeOnClick,
  colorChangeOnClick,
  rainbowBackground
};
enum class CursorMode : uint32_t {
  normal = 0,
  custom,
  hidden,
  rawMotion
};
struct WindowParams {
  WindowHandle parent = (WindowHandle)0;
  WindowType mode = WindowType::window;
  ResizeMode resize = ResizeMode::fixed;
  FeatureMode feat = FeatureMode::none;
  CursorMode cursorMode = CursorMode::normal;
  bool isCentered = true;
  bool isScrollable = false;
};

// build icon
std::shared_ptr<WindowResource> buildWindowIcon(bool usePackage) {
  if (usePackage) {
#   ifdef _WINDOWS
      return WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
#   else
      return WindowResource::buildIconFromPackage("logo_big.png");
#   endif
  }
  else
    return WindowResource::buildIcon(SystemIcon::app);
}
// build cursor
std::shared_ptr<WindowResource> buildWindowCursor(bool usePackage) {
  if (usePackage) {
#   ifdef _WINDOWS
      return WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR));
#   else
    return WindowResource::buildCursorFromPackage("base_cur.png");
#   endif
  }
  else
    return WindowResource::buildCursor(SystemCursor::pointer);
}
// get display behavior
WindowBehavior getBehavior(bool isScrollable) {
  return isScrollable ? (WindowBehavior::scrollV | WindowBehavior::scrollH) : WindowBehavior::none;
}

// create main window
std::unique_ptr<Window> createWindow(const WindowParams& params, WindowResource::Color background) { // throws on failure
  std::shared_ptr<WindowResource> mainIcon = buildWindowIcon(true);
  std::shared_ptr<WindowResource> cursor = nullptr;
  if (params.cursorMode == CursorMode::custom)
      cursor = buildWindowCursor(true);
  int32_t position = params.isCentered ? Window::Builder::centeredPosition() : Window::Builder::defaultPosition();
  
  Window::Builder builder;
  builder.setDisplayMode(params.mode, getBehavior(params.isScrollable), params.resize)
         .setRefreshRate(_REFRESH_RATE)
         .setSize(_DEFAULT_WINDOW_WIDTH, _DEFAULT_WINDOW_HEIGHT)
         .setPosition(position, position)
         .setIcon(mainIcon)
         .setCursor(cursor)
         .setBackgroundColor(WindowResource::buildColorBrush(background));
  if (params.parent == (WindowHandle)0 || params.mode == WindowType::dialog)
    return builder.create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Example Window"), params.parent);
  else
    return builder.update(params.parent, false);
}


// -- window events -- ---------------------------------------------------------

struct {
  uint32_t lastSizeX;
  uint32_t lastSizeY;
  uint32_t lastCursorPosX;
  uint32_t lastCursorPosY;
  uint32_t lastCharInput;
  bool hasScrollbars;
  bool isRefreshed;
  bool hasClicked;
  
  void reset(bool isScrollable) noexcept {
    lastSizeX = _DEFAULT_WINDOW_WIDTH;
    lastSizeY = _DEFAULT_WINDOW_HEIGHT;
    lastCursorPosX = lastCursorPosY = 0;
    lastCharInput = (uint32_t)'-';
    hasScrollbars = isScrollable;
    isRefreshed = false;
    hasClicked = false;
  }
} g_events;

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
      g_events.lastSizeX = sizeX; 
      g_events.lastSizeY = sizeY; 
      if (g_events.hasScrollbars)
        sender->setScrollbarRange((uint16_t)sender->getScrollPositionH(), (uint16_t)sender->getScrollPositionV(), 
                                  (uint16_t)sizeX*2, (uint16_t)sizeY*2);
      g_events.isRefreshed = true;
      break;
    default: break;
  }
  return false;
}
// keyboard event handler
bool onKeyboardEvent(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t) {
  switch (event) {
    case KeyboardEvent::keyDown:
      if (keyCode == _P_VK_ESC) {
        if (sender->displayMode() == WindowType::fullscreen)
          sender->show(Window::VisibilityCommand::minimize);
        Window::sendCloseEvent(sender->handle());
      }
      break;
    case KeyboardEvent::charInput:
      g_events.lastCharInput = keyCode;
      g_events.isRefreshed = true; 
      break;
    default: break;
  }
  return false;
}
// mouse event handler
bool onMouseEvent(Window*, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t) {
  switch (event) {
    case MouseEvent::mouseMove:
      g_events.lastCursorPosX = x;
      g_events.lastCursorPosY = y;
      g_events.isRefreshed = true;
      break;
    case MouseEvent::buttonDown:
      if ((MouseButton)index == MouseButton::left) {
        g_events.hasClicked = true;
        g_events.isRefreshed = true;
      }
      break;
    case MouseEvent::rawMotion:
      g_events.lastCursorPosX += x;
      g_events.lastCursorPosY += y;
      break;
    default: break;
  }
  return false;
}


// -- message box viewer -- ----------------------------------------------------

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


// -- window viewer -- ---------------------------------------------------------

struct BackgroundColor {
  uint32_t r, g, b;
};

// Display window
void viewWindow(const WindowParams& params) {
  try {
    bool hasCursorChanged = false;
    g_events.reset(params.isScrollable);
    BackgroundColor back = (params.feat == FeatureMode::rainbowBackground || params.mode == WindowType::fullscreen)
                         ? BackgroundColor{ 255,0,0 }
                         : BackgroundColor{ 0,0,0 };
    
    auto window = createWindow(params, WindowResource::rgbColor(back.r,back.g,back.b));
    window->setMinClientAreaSize(400, 300);
    if (params.isScrollable)
      window->setScrollbarRange(0, 0, (uint16_t)_DEFAULT_WINDOW_WIDTH*2, (uint16_t)_DEFAULT_WINDOW_HEIGHT*2);

    window->setWindowHandler(&onWindowEvent);
    window->setPositionHandler(&onPositionEvent);
    window->setKeyboardHandler(&onKeyboardEvent);
    switch (params.cursorMode) {
      case CursorMode::hidden:    window->setMouseHandler(&onMouseEvent, Window::CursorMode::hidden); break;
      case CursorMode::rawMotion: window->setMouseHandler(&onMouseEvent, Window::CursorMode::hiddenRaw); break;
      default: window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible); break;
    }
    window->show();

    while (Window::pollEvents()) {
      if (g_events.isRefreshed) {
        // update caption: input char, window size, cursor position
        if (window->displayMode() != WindowType::fullscreen) {
          window_char buffer[64]{ 0 };
          if (params.feat == FeatureMode::inputKeyInCaption)
            _SYSTEM_sprintf(buffer, sizeof(buffer) / sizeof(window_char), 
                            _SYSTEM_STR("Example Window - input:%c - %ux%u [%u-%u]"), (window_char)g_events.lastCharInput,
                            g_events.lastSizeX, g_events.lastSizeY, g_events.lastCursorPosX, g_events.lastCursorPosY);
          else
            _SYSTEM_sprintf(buffer, sizeof(buffer) / sizeof(window_char), 
                            _SYSTEM_STR("Example Window - %ux%u [%u-%u]"),
                            g_events.lastSizeX, g_events.lastSizeY, g_events.lastCursorPosX, g_events.lastCursorPosY);
          window->setCaption((const window_char*)buffer);
        }

        // click event
        if (g_events.hasClicked) {
          switch (params.feat) {
            case FeatureMode::fullscreenToggle: {
              int32_t position = params.isCentered ? Window::Builder::centeredPosition() : Window::Builder::defaultPosition();
              pandora::hardware::DisplayArea clientArea{ position, position, _DEFAULT_WINDOW_WIDTH, _DEFAULT_WINDOW_HEIGHT };
              if (window->displayMode() == WindowType::fullscreen)
                window->setDisplayMode((params.mode != WindowType::fullscreen) ? params.mode : WindowType::window, 
                                       getBehavior(params.isScrollable), params.resize, clientArea);
              else
                window->setDisplayMode(WindowType::fullscreen, getBehavior(params.isScrollable), params.resize, clientArea, _REFRESH_RATE);
              break;
            }
            case FeatureMode::cursorChangeOnClick: {
              hasCursorChanged ^= true;
              switch (params.cursorMode) {
                case CursorMode::normal: window->setCursor(buildWindowCursor(hasCursorChanged)); break;
                case CursorMode::custom: window->setCursor(buildWindowCursor(!hasCursorChanged)); break;
                case CursorMode::hidden: 
                  window->setMouseHandler(&onMouseEvent, hasCursorChanged ? Window::CursorMode::visible : Window::CursorMode::hidden); 
                  break;
                case CursorMode::rawMotion:
                  g_events.lastCursorPosX = g_events.lastCursorPosY = 0; // reset start position
                  window->setMouseHandler(&onMouseEvent, hasCursorChanged ? Window::CursorMode::visible : Window::CursorMode::hiddenRaw); 
                  break;
                default: break;
              }
              break;
            }
            case FeatureMode::colorChangeOnClick: {
              back.r = (back.r) ? 0 : 255;
              window->setBackgroundColorBrush(WindowResource::buildColorBrush(WindowResource::rgbColor(back.r,back.g,back.b)));
              window->clearClientArea();
              break;
            }
            default: break;
          }
          g_events.hasClicked = false;
        }
        g_events.isRefreshed = false;
      }

      // rainbow background update
      if (params.feat == FeatureMode::rainbowBackground) {
        if (back.r > 0 && back.b == 0) { 
          --back.r; ++back.g; 
        }
        else if (back.g > 0) { 
          --back.g; ++back.b; 
        }
        else { 
          --back.b; ++back.r; 
        }
        window->setBackgroundColorBrush(WindowResource::buildColorBrush(WindowResource::rgbColor(back.r,back.g,back.b)));
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
    int option = readNumericInput(1, 6);
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

// window/dialog menu
void menuWindow(WindowType mode, const char* typeName) {
  WindowParams params;
  params.mode = mode;
  std::unique_ptr<Window> parentWindow = nullptr;
  
  bool isRunning = true;
  while (isRunning) {
    clearScreen();
    printf("Window viewer: %s\n_________________________________________________\n", typeName);
    printf("> to exit: ESC or close button\n");

    printf("\nSize mode:\n");
    printMenu<9>({ "Back to main menu...", "Fixed", "ResizableX", "ResizableY", "Resizable", "Homothety", 
                                           "Fixed / scrollbars", "Resizable / scrollbars", "Homothety / scrollbars" });
    int option = readNumericInput(1, 8);
    if (option == 0) { isRunning = false; continue; }
    switch (option) {
      case 1: params.isScrollable = false; params.resize = ResizeMode::fixed; break;
      case 2: params.isScrollable = false; params.resize = ResizeMode::resizableX; break;
      case 3: params.isScrollable = false; params.resize = ResizeMode::resizableY; break;
      case 4: params.isScrollable = false; params.resize = ResizeMode::resizable; break;
      case 5: params.isScrollable = false; params.resize = ResizeMode::resizable|ResizeMode::homothety; break;
      case 6: params.isScrollable = true;  params.resize = ResizeMode::fixed; break;
      case 7: params.isScrollable = true;  params.resize = ResizeMode::resizable; break;
      case 8: params.isScrollable = true;  params.resize = ResizeMode::resizable|ResizeMode::homothety; break;
      default: break;
    }

    printf("\nStyle:\n");
    printMenu<5>({ "Cancel options...", "Default pos", "Centered", "Default pos, parent/original window", "Centered, parent/original window" });
    option = readNumericInput(1, 4);
    if (option == 0)
      continue;
    bool hasParent = false;
    switch (option) {
      case 1: params.isCentered = false; hasParent = false; break;
      case 2: params.isCentered = true;  hasParent = false; break;
      case 3: params.isCentered = false; hasParent = true; break;
      case 4: params.isCentered = true;  hasParent = true; break;
      default: break;
    }
    
    printf("\nCursor style:\n");
    printMenu<5>({ "Cancel options...", "Standard", "Custom", "Hidden", "Mouse capture" });
    option = readNumericInput(1, 4);
    if (option == 0)
      continue;
    switch (option) {
      case 1: params.cursorMode = CursorMode::normal; break;
      case 2: params.cursorMode = CursorMode::custom; break;
      case 3: params.cursorMode = CursorMode::hidden; break;
      case 4: params.cursorMode = CursorMode::rawMotion; break;
      default: break;
    }

    printf("\nFeature:\n");
    printMenu<7>({ "Cancel options...", "Standard", "Last input key in caption", "Toggle window/fullscreen on click", 
                                           "Change cursor on click", "Change color on click", "Rainbow background" });
    option = readNumericInput(1, 6);
    if (option == 0)
      continue;
    switch (option) {
      case 1: params.feat = FeatureMode::none; break;
      case 2: params.feat = FeatureMode::inputKeyInCaption; break;
      case 3: params.feat = FeatureMode::fullscreenToggle; break;
      case 4: params.feat = FeatureMode::cursorChangeOnClick; break;
      case 5: params.feat = FeatureMode::colorChangeOnClick; break;
      case 6: params.feat = FeatureMode::rainbowBackground; break;
      default: break;
    }

    if (hasParent) { // create parent / original window
#     ifdef _WINDOWS
        HMENU menuBar = CreateMenu();
        HMENU subMenu = CreatePopupMenu();
        AppendMenuW(subMenu, MF_STRING, 161, L"&First item");
        AppendMenuW(subMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(subMenu, MF_STRING, 162, L"&Second item");
        AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)subMenu, L"&Menu");
        std::shared_ptr<WindowResource> menu = WindowResource::buildMenu(menuBar);
#     else
        std::shared_ptr<WindowResource> menu = nullptr;
#     endif
    
      Window::Builder builder;
      parentWindow = builder.setDisplayMode(WindowType::window, WindowBehavior::none, ResizeMode::fixed)
                     .setSize(380, 220)
                     .setPosition(Window::Builder::centeredPosition(), Window::Builder::centeredPosition())
                     .setIcon(WindowResource::buildIcon(SystemIcon::info))
                     .setBackgroundColor(WindowResource::buildColorBrush(WindowResource::rgbColor(0,120,160)))
#                    ifdef _WINDOWS
                       .setMenu(menu)
#                    endif
                     .create(_SYSTEM_STR("PARENT"), _SYSTEM_STR("Parent window"));
      parentWindow->show();
      params.parent = parentWindow->handle();
      Window::pollEvents();
    }
    
    viewWindow(params);
    params.parent = (WindowHandle)0;
    parentWindow = nullptr;
  }
}

// ---

#if defined(__ANDROID__)
  // Window viewer - entry point for Android
  void android_main(struct android_app* state) {
    try {
      pandora::system::AndroidApp::instance().init(state);
      printf("-- user choice: %u", viewMessageBox(1, false));
      printf("-- user choice: %u", viewMessageBox(2, true));
      printf("-- user choice: %u", viewMessageBox(3, false));
      viewWindow(WindowType::window, ResizeMode::fixed);
      viewWindow(WindowType::fullscreen, ResizeMode::fixed);
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
      printMenu<7>({ "Exit...", "MessageBox", "Bordered", "Dialog", "Window", "Borderless", "Fullscreen" });
      int option = readNumericInput(1, 6);
      switch (option) {
        case 1: menuMessageBox(); break;
        case 2: menuWindow(WindowType::bordered, "bordered"); break;
        case 3: menuWindow(WindowType::dialog, "dialog"); break;
        case 4: menuWindow(WindowType::window, "window"); break;
        case 5: menuWindow(WindowType::borderless, "borderless"); break;
        case 6: menuWindow(WindowType::fullscreen, "fullscreen"); break;
        case 0:
        default: isRunning = false; break;
      }
    }
    return 0;
  }
#endif
