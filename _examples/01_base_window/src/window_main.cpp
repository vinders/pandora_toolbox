/*******************************************************************************
Description : Example - window creation + main loop
*******************************************************************************/
#include <cstdint>
#include <thread>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <video/message_box.h>
#include <video/window_keycodes.h>
#include <video/window.h>

#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../../_img/test_win32/resources.h" // --> replace with your own resource file
# define _SYSTEM_STR(str) L"" str

#else
# define _SYSTEM_STR(str) str
# if defined(__ANDROID__)
#   include <android/log.h>
#   include <system/api/android_app.h>
# endif
#endif

using namespace pandora::video;

bool g_hasClicked = false; // --> only for example
bool g_isBlackBackground = true; // --> only for example


// create main window
std::unique_ptr<Window> createWindow() { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
# else
    auto mainIcon = WindowResource::buildIconFromPackage("logo_big.png");
# endif
  
  Window::Builder builder;
  return builder.setDisplayMode(WindowType::window, WindowBehavior::globalContext, ResizeMode::resizable|ResizeMode::homothety)
         .setSize(800, 600)
         .setPosition(Window::Builder::centeredPosition(), Window::Builder::centeredPosition())
         .setIcon(mainIcon)
         .setBackgroundColor(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0)))
         .create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Example Window"));
}

// ---

// change background color
// --> only for example
void toggleBackgroundColor(Window& window) {
  g_isBlackBackground ^= true;
  window.setBackgroundColorBrush((g_isBlackBackground)
                                ? WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0))
                                : WindowResource::buildColorBrush(WindowResource::rgbColor(255,255,255)) );
}


// -- handlers -- --------------------------------------------------------------

// window/hardware event handler --> should never throw!
bool onWindowEvent(Window* sender, WindowEvent event, uint32_t status, int32_t posX, int32_t posY, void* data) {
  switch (event) {
    case WindowEvent::windowClosed: { // close -> confirmation
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

// size/position event handler --> should never throw!
bool onPositionEvent(Window* sender, PositionEvent event, int32_t posX, int32_t posY, uint32_t sizeX, uint32_t sizeY) {
  switch (event) {
    case PositionEvent::sizePositionChanged: {
      // --> adapt your renderer to sizeX/sizeY...
      break;
    }
    default: break;
  }
  return false;
}

// keyboard event handler --> should never throw!
bool onKeyboardEvent(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t change) {
  switch (event) {
    case KeyboardEvent::keyDown: {
      if (keyCode == _P_VK_ESC) { // ESC pressed -> close
        if (sender->displayMode() == WindowType::fullscreen)
          sender->show(Window::VisibilityCommand::minimize); // do not show message-box in fullscreen
        Window::sendCloseEvent(sender->handle());
      }
      break;
    }
    default: break;
  }
  return false;
}

// mouse event handler --> should never throw!
bool onMouseEvent(Window* sender, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t activeKeys) {
  switch (event) {
    case MouseEvent::buttonDown: { // click -> report user action
      if ((MouseButton)index == MouseButton::left)
        g_hasClicked = true; // --> only for example
      break;
    }
    default: break;
  }
  return false;
}


// -- main loop -- -------------------------------------------------------------

inline void mainAppLoop() {
  try {
    auto window = createWindow();
    window->setMinClientAreaSize(400, 300);
    
    window->setWindowHandler(&onWindowEvent);
    window->setPositionHandler(&onPositionEvent);
    window->setKeyboardHandler(&onKeyboardEvent);
    window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible);
    window->show();
    
    while (Window::pollEvents()) {
      bool isRefreshed = false;

      // input + logic management
      if (g_hasClicked) { // --> only for example
        g_hasClicked = false; // unset flag
        toggleBackgroundColor(*window);
        isRefreshed = true;
      }
      
      // display
      if (isRefreshed)
        window->clearClientArea(); // repaint background --> only for example
      std::this_thread::sleep_for(std::chrono::microseconds(16666LL)); // 60Hz
    }
  }
  catch (const std::exception& exc) {
    MessageBox::flushEvents();
    MessageBox::show("Fatal error", exc.what(), MessageBox::ActionType::ok, MessageBox::IconType::error, true);
    exit(-1);
  }
}


// -- entry point -- -----------------------------------------------------------

#if defined(_WINDOWS)
  int APIENTRY WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE, _In_ LPSTR cmdLine, _In_ int cmdShow) {
    pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)appInstance);
    mainAppLoop();
    return 0;
  }
#elif defined(__ANDROID__)
  void android_main(struct android_app* state) {
    pandora::system::AndroidApp::instance().init(state);
    mainAppLoop();
  }
#else
  int main() {
    mainAppLoop();
    return 0;
  }
#endif