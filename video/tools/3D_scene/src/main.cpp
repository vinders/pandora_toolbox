/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
3D scene - video/3D rendering test utility
*******************************************************************************/
#include <cstdint>
#include <thread>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <video/message_box.h>
#include <video/window_keycodes.h>
#include <video/window.h>
#include "window_builder.h"
#include "menu_manager.h"
#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
#endif

#define __P_FREQUENCY 60.0

using namespace pandora::video;

struct {
  scene::Options settings;
  std::unique_ptr<scene::MenuManager> menuManager = nullptr;
  std::unique_ptr<Window> window = nullptr;
  PixelSize screenSize{ 0,0 };
  float frequency = (float)__P_FREQUENCY;
  bool isMouseLeftDown = false;
} g_windowState;


// -- command handlers -- ------------------------------------------------------

void onApiChange(bool isRendererChanged) {
  //...
}
void onVsyncChange(bool useVsync) {
  //...
}
void onFilterChange() {
  //...
}


// -- window handlers -- -------------------------------------------------------

// window/hardware event handler
bool onWindowEvent(Window*, WindowEvent event, uint32_t status, int32_t posX, int32_t, void*) {
  switch (event) {
    case WindowEvent::windowClosed: { // close -> confirmation
      auto reply = MessageBox::show("Confirmation", "Are you sure you want to exit?", 
                                    MessageBox::ActionType::yesNo, MessageBox::IconType::question, true);
      if (reply == MessageBox::Result::action2) // "no" button
        return true; // cancel close event
      break;
    }
    case WindowEvent::menuCommand: {
      if (status == 0)
        g_windowState.menuManager->onMenuCommand(posX);
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
      g_windowState.screenSize.width = sizeX;
      g_windowState.screenSize.height = sizeY;
      scene::updateWindowCaption(*sender, sizeX, sizeY, g_windowState.frequency);

      //resize render view: ...
      break;
    case PositionEvent::sizePositionTrack: 
      g_windowState.screenSize.width = sizeX;
      g_windowState.screenSize.height = sizeY;
      scene::updateWindowCaption(*sender, sizeX, sizeY, g_windowState.frequency);
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
    default: break;
  }
  return false;
}

// mouse event handler
bool onMouseEvent(Window* sender, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t) {
  switch (event) {
    case MouseEvent::buttonDown:
      if ((MouseButton)index == MouseButton::left && !g_windowState.isMouseLeftDown) {
        sender->setCursorMode(Window::CursorMode::hiddenRaw);
        g_windowState.isMouseLeftDown = true;
      }
      break;
    case MouseEvent::buttonUp:
      if ((MouseButton)index == MouseButton::left && g_windowState.isMouseLeftDown) {
        sender->setCursorMode(Window::CursorMode::visible);
        g_windowState.isMouseLeftDown = false;
      }
      break;
    case MouseEvent::rawMotion:
      //rotate cam: (g_windowState.settings.mouseSensitivity+3)*x;
      //rotate cam: (g_windowState.settings.mouseSensitivity+3)*y;
      break;
    default: break;
  }
  return false;
}


// -- main loop -- -------------------------------------------------------------

#if defined(_WINDOWS)
  int APIENTRY WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)appInstance);
#else
int main() {
#endif
  try {
    g_windowState.menuManager = std::unique_ptr<scene::MenuManager>(new scene::MenuManager(g_windowState.settings));
    g_windowState.menuManager->apiChangeHandler = &onApiChange;
    g_windowState.menuManager->vsyncChangeHandler = &onVsyncChange;
    g_windowState.menuManager->filterChangeHandler = &onFilterChange;
    
    g_windowState.window = scene::createWindow(g_windowState.menuManager->resource());
    g_windowState.window->setWindowHandler(&onWindowEvent);
    g_windowState.window->setPositionHandler(&onPositionEvent);
    g_windowState.window->setKeyboardHandler(&onKeyboardEvent);
    g_windowState.window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible);
    g_windowState.window->show();
    
    uint32_t loopCount = 0;
    auto timePoint = std::chrono::steady_clock::now();
    while (Window::pollEvents()) {
      //...
      
      // calculate refresh frequency
      if (++loopCount >= ((uint32_t)__P_FREQUENCY)) {
        loopCount = 0;
        auto newTime = std::chrono::steady_clock::now();
        g_windowState.frequency = static_cast<float>(__P_FREQUENCY*1000000000.0 / (double)((newTime - timePoint).count()));
        timePoint = newTime;
        scene::updateWindowCaption(*g_windowState.window, g_windowState.screenSize.width, g_windowState.screenSize.height, g_windowState.frequency);
      }
      // vsync / sleep
      if (g_windowState.settings.useVsync)
        /*TMP*/std::this_thread::sleep_for(std::chrono::milliseconds(10LL));
      else
        std::this_thread::sleep_for(std::chrono::milliseconds(950LL/(int64_t)__P_FREQUENCY)); // 60Hz (NB: 950 = 1000ms - average processing time)
    }
  }
  catch (const std::exception& exc) {
    MessageBox::show("Fatal error", exc.what(), MessageBox::ActionType::ok, MessageBox::IconType::error, true);
    exit(-1);
  }
  return 0;
}
