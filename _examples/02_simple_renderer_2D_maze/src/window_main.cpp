/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - window creation + main loop
*******************************************************************************/
#include <cstdint>
#include <thread>
#include <chrono>
#include <memory>
#include <stdexcept>

#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../../_img/test_win32/resources.h" // --> replace with your own resource file
# define _SYSTEM_STR(str) L"" str
#else
# define _SYSTEM_STR(str) str
#endif
#include <video/message_box.h>
#include <video/window_keycodes.h>
#include <video/window.h>
#include "scene.h"

using namespace pandora::video;

RendererContext* g_renderer = nullptr;
Scene* g_currentScene = nullptr;
bool g_isVisible = true;


// create main window
std::unique_ptr<Window> createWindow() { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
# else
    auto mainIcon = WindowResource::buildIconFromPackage("logo_big.png");
# endif
  
  Window::Builder builder;
  return builder.setDisplayMode(WindowType::window, WindowBehavior::globalContext,
                                ResizeMode::resizable|ResizeMode::homothety)
         .setSize(__BASE_WINDOW_WIDTH, __BASE_WINDOW_HEIGHT) // --> to use entire screen (in fullscreen mode), do not call
         .setPosition(Window::Builder::centeredPosition(),
                      Window::Builder::centeredPosition()) // --> useless in fullscreen mode
         .setIcon(mainIcon)
         .setBackgroundColor(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0)))
         .create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Simple Renderer"));
}

// ---

// renderer re-creation after 'device lost' or state update error
void reCreateRendererContext(Window* parentWindow, uint32_t width, uint32_t height) {
  if (g_currentScene)
    g_currentScene->release();
  g_renderer->release();
  *g_renderer = RendererContext(pandora::hardware::DisplayMonitor{}, parentWindow->handle(), width, height,
                                pandora::video::RefreshRate{}, g_renderer->hasVsync());
  if (g_currentScene)
    g_currentScene->initResources(*g_renderer, width, height);
  // --> note: on failure, a message-box could be used to ask the user what to do (in fullscreen, don't forget to minimize first)
}


// -- handlers -- --------------------------------------------------------------

// window/hardware event handler --> should never throw!
bool onWindowEvent(Window*, WindowEvent event, uint32_t status, int32_t, int32_t, void*) {
  switch (event) {
    case WindowEvent::windowClosed: { // close -> confirmation
      auto reply = MessageBox::show("Confirmation", "Are you sure you want to exit?", 
                                    MessageBox::ActionType::yesNo, MessageBox::IconType::question, true);
      if (reply == MessageBox::Result::action2) // "no" button
        return true; // cancel close event
      break;
    }
    case WindowEvent::stateChanged:
      g_isVisible = ((WindowActivity)status != WindowActivity::hidden);
      if (g_isVisible && g_currentScene)
        g_currentScene->refreshScreen();
      break;
    default: break;
  }
  return false;
}

// size/position event handler --> should never throw!
bool onPositionEvent(Window*, PositionEvent event, int32_t, int32_t, uint32_t, uint32_t) {
  switch (event) {
    case PositionEvent::sizePositionChanged: {
      if (g_currentScene)
        g_currentScene->refreshScreen();
      break;
    }
    default: break;
  }
  return false;
}

// keyboard event handler --> should never throw!
bool onKeyboardEvent(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t) {
  switch (event) {
    case KeyboardEvent::keyDown: {
      switch (keyCode) {
        case _P_VK_ESC: // ESC pressed -> close
          if (sender->displayMode() == WindowType::fullscreen)
            sender->show(Window::VisibilityCommand::minimize); // minimize: do not show msg-box in fullscreen!
          Window::sendCloseEvent(sender->handle());
          break;
        default: // user input -> forward to game scene
          if (g_currentScene)
            g_currentScene->storeUserInput(keyCode, true);
          break;
      }
      break;
    }
    case KeyboardEvent::keyUp: {
      if (g_currentScene)
        g_currentScene->storeUserInput(keyCode, false);
      break;
    }
    default: break;
  }
  return false;
}

// mouse event handler --> should never throw!
bool onMouseEvent(Window*, MouseEvent event, int32_t, int32_t, int32_t, uint8_t) {
  switch (event) {
    default: break;
  }
  return false;
}


// -- main loop -- -------------------------------------------------------------

inline void mainAppLoop() {
  try {
    // create window
    auto window = createWindow();
    window->setMinClientAreaSize(400, 300);
    window->setWindowHandler(&onWindowEvent);
    window->setPositionHandler(&onPositionEvent);
    window->setKeyboardHandler(&onKeyboardEvent);
    window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible);
    window->show();

    // create renderer
    uint32_t clientWidth = window->getClientSize().width;
    uint32_t clientHeight = window->getClientSize().height;
    RendererContext renderer(pandora::hardware::DisplayMonitor{}, window->handle(),
                             clientWidth, clientHeight, pandora::video::RefreshRate{}, true);
    g_renderer = &renderer;
    // --> for this demo, the default/primary monitor is used, with a default rate (60Hz).
    //     In fullscreen, it's better to use the refresh rate associated with the desired DisplayMode.

    // create rendering scene
    { // --> scope
      Scene defaultScene(renderer, clientWidth, clientHeight);
      g_currentScene = &defaultScene;

      while (Window::pollEvents()) {
        if (g_isVisible) {
          defaultScene.processGameLogic(); // business logic

          if (defaultScene.isUpdated()) {
            try {
              renderer.beginDrawing();
              defaultScene.render();  // draw entities in framebuffer
              renderer.swapBuffers(); // display frame
            }
            catch (...) { // device lost -> recreate renderer (on failure, exit)
              reCreateRendererContext(window.get(), clientWidth, clientHeight);
            }

            // game finished -> start new level
            if (defaultScene.isFinished()) {
              MessageBox::show("Congratulations!", "Press OK to start a new level.",
                               MessageBox::ActionType::ok, MessageBox::IconType::info, true);
              defaultScene.restartScene(renderer, clientWidth, clientHeight);
            }
          }
        }
        else { // window is hidden -> no rendering
          renderer.skipFrame();
          std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
      }
      defaultScene.release();
    } // end of scope --> make sure resources are freed before the renderer
  }
  catch (const std::exception& exc) {
    MessageBox::flushEvents();
    MessageBox::show("Fatal error", exc.what(), MessageBox::ActionType::ok, MessageBox::IconType::error, true);
    exit(-1);
  }
}


// -- entry point -- -----------------------------------------------------------

#if defined(_WINDOWS)
  int APIENTRY WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)appInstance);
    mainAppLoop();
    return 0;
  }
#else
  int main() {
    mainAppLoop();
    return 0;
  }
#endif