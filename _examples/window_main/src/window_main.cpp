/*******************************************************************************
Description : Example - window creation + main loop
*******************************************************************************/
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <video/message_box.h>
#include <video/window.h>

#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../../_img/test_win32/resources.h"
# define _SYSTEM_STR(str) L"" str

#else
# define _SYSTEM_STR(str) str
# if defined(__ANDROID__)
#   include <android/log.h>
#   include <system/api/android_app.h>
# endif
#endif

using namespace pandora::video;

bool g_hasDoubleClicked = false;
bool g_isBlackBackground = true;


// create main window
std::unique_ptr<Window> createWindow() { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
# else
    auto mainIcon = WindowResource::buildIconFromFile("logo_big.png");
# endif
  
  Window::Builder builder;
  return builder.setDisplayMode(WindowType::window, WindowBehavior::globalContext, ResizeMode::resizable|ResizeMode::homothety)
         .setSize(800, 600)
         .setIcon(mainIcon)
         .setBackgroundColor(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0)))
         .create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Example Window"));
}

// ---

// change background color
void toggleBackgroundColor(Window& window) {
  g_isBlackBackground ^= true;
  window.setBackgroundColorBrush((g_isBlackBackground)
                                ? WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0))
                                : WindowResource::buildColorBrush(WindowResource::rgbColor(255,255,255)) );
}


// -- handlers -- --------------------------------------------------------------

// window event handler
bool onWindowEvent(WindowEvent event, uint32_t flag, int32_t posX, int32_t posY, uint32_t size, uint64_ptr data) {
  switch (event) {
    case WindowEvent::windowClosed: {
      auto reply = MessageBox::show("Confirmation", "Are you sure you want to exit?", 
                                    MessageBox::ActionType::yesNo, MessageBox::IconType::question);
      if (reply == MessageBox::Result::action2) // "no" button
        return true; // cancel close event
      break;
    }
  }
  return false;
}

// mouse event handler
bool onMouseEvent(MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t activeKeys) {
  switch (event) {
    case MouseEvent::buttonDouble: // double-click -> report it to main loop
      g_hasDoubleClicked = true; 
      break;
  }
  return false;
}


// -- main loop -- -------------------------------------------------------------

void mainAppLoop() {
  try {
    auto window = createWindow();
    window->setMinClientAreaSize(400, 300);
    window->setWindowHandler(&onWindowEvent);
    window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible);
    window->show();
    
    while (Window::pollEvents()) {
      bool isRefreshed = false;

      // input + logic management
      if (g_hasDoubleClicked) {
        g_hasDoubleClicked = false; // unset flag
        toggleBackgroundColor(*window);
        isRefreshed = true;
      }
      
      // display
      if (isRefreshed)
        window->clearClientArea(); // repaint background
    }
  }
  catch (const std::exception& exc) {
    MessageBox::show("Fatal error", exc.what(), MessageBox::ActionType::ok, MessageBox::IconType::error);
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