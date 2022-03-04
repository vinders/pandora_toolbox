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

#define _CAM_SENSITIVITY 0.25f // --> this should be a user setting

RendererContext* g_renderer = nullptr;
Scene* g_currentScene = nullptr;
uint32_t g_lastWidth = 0;
uint32_t g_lastHeight = 0;
bool g_isVisible = true;
bool g_isMouseButtonDown = false;


// create main window
std::unique_ptr<Window> createWindow() { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
    auto cursor = WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR));
# else
    auto mainIcon = WindowResource::buildIconFromPackage("logo_big.png");
    auto cursor = WindowResource::buildCursorFromPackage("base_cur.png");
# endif
  
  Window::Builder builder;
  return builder.setDisplayMode(WindowType::window, WindowBehavior::globalContext, ResizeMode::resizable|ResizeMode::homothety)
         .setSize(800, 600) // --> to use entire screen (in fullscreen mode), do not call setSize
         .setPosition(Window::Builder::centeredPosition(), Window::Builder::centeredPosition()) // --> useless in fullscreen mode
         .setIcon(mainIcon)
         .setCursor(cursor)
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
                                pandora::video::RefreshRate{}, g_renderer->isAntiAliasingEnabled(),
                                g_renderer->isAnisotropicSampler(), g_renderer->hasVsync());
  if (g_currentScene)
    g_currentScene->init(*g_renderer, width, height, g_renderer->antiAliasingSamples());
  // --> note: on failure, a message-box could be used to ask the user what to do (in fullscreen, don't forget to minimize first)
}


// -- handlers -- --------------------------------------------------------------

// window/hardware event handler --> should never throw!
bool onWindowEvent(Window* sender, WindowEvent event, uint32_t status, int32_t, int32_t, void*) {
  switch (event) {
    case WindowEvent::windowClosed: { // close -> confirmation
      // stop capturing cursor before displaying message-box
      auto cursorMode = sender->getCursorMode();
      sender->setCursorMode(Window::CursorMode::visible);

      auto reply = MessageBox::show("Confirmation", "Are you sure you want to exit?", 
                                    MessageBox::ActionType::yesNo, MessageBox::IconType::question, true);
      if (reply == MessageBox::Result::action2) { // "no" button
        sender->setCursorMode(cursorMode); // restore cursor capture
        return true; // cancel close event
      }
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
bool onPositionEvent(Window* sender, PositionEvent event, int32_t, int32_t, uint32_t sizeX, uint32_t sizeY) {
  switch (event) {
    case PositionEvent::sizePositionChanged: {
      if (g_renderer && (g_lastWidth != sizeX || g_lastHeight != sizeY)) {
        // adapt framebuffer size
        try {
          g_renderer->resize(sender->handle(), sizeX, sizeY);
          if (g_currentScene)
            g_currentScene->resizeScreen(sizeX, sizeY);
        }
        catch (...) { // device lost -> recreate renderer (on failure, exit)
          reCreateRendererContext(sender, sizeX, sizeY);
        }
        // adapt camera size
        if (g_currentScene)
          g_currentScene->setCamera(sizeX, sizeY, _CAM_SENSITIVITY);
        g_lastWidth = sizeX;
        g_lastHeight = sizeY;
      }
      else if (g_currentScene)
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
        case _P_VK_F9:  // F9 -> toggle sampler (trilinear/anisotropic) -- default: anisotropic
          if (g_renderer) {
            g_renderer->toggleSampler();
            g_currentScene->refreshScreen();
          }
          break;
        case _P_VK_F8: // F8 -> toggle MSAA (on/off) -- default: off
          if (g_renderer)
            try {
              g_renderer->toggleAntiAliasing();
              if (g_currentScene)
                g_currentScene->rebuildPipelines(g_lastWidth, g_lastHeight, g_renderer->antiAliasingSamples());
            }
            catch (...) { // failure -> recreate renderer (on failure, exit)
              reCreateRendererContext(sender, g_lastWidth, g_lastHeight);
            }
          break;
      }
      break;
    }
    default: break;
  }
  return false;
}

// mouse event handler --> should never throw!
bool onMouseEvent(Window* sender, MouseEvent event, int32_t x, int32_t y, int32_t index, uint8_t) {
  switch (event) {
    case MouseEvent::buttonDown: {
      if ((MouseButton)index == MouseButton::left && !g_isMouseButtonDown) {
        g_isMouseButtonDown = true;
        sender->setCursorMode(Window::CursorMode::hiddenRaw);
      }
      break;
    }
    case MouseEvent::buttonUp: {
      if ((MouseButton)index == MouseButton::left && g_isMouseButtonDown) {
        sender->setCursorMode(Window::CursorMode::visible);
        g_isMouseButtonDown = false;
      }
      break;
    }
    case MouseEvent::rawMotion: { // captured mouse move -> update camera view
      if (g_currentScene)
        g_currentScene->moveCamera(x, y);
      break;
    }
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
    g_lastWidth = window->getClientSize().width;
    g_lastHeight = window->getClientSize().height;
    RendererContext renderer(pandora::hardware::DisplayMonitor{}, window->handle(), g_lastWidth, g_lastHeight,
                             pandora::video::RefreshRate{}, false, true, true);
    g_renderer = &renderer;
    // --> for this demo, the default/primary monitor is used, with a default rate (60Hz).
    //     In fullscreen, it's better to use the refresh rate associated with the desired DisplayMode.

    // create rendering scene
    { // --> scope
      Scene defaultScene(renderer, g_lastWidth, g_lastHeight, renderer.antiAliasingSamples(), _CAM_SENSITIVITY);
      g_currentScene = &defaultScene;
      g_isMouseButtonDown = false;

      while (Window::pollEvents()) {
        if (g_isVisible && defaultScene.isUpdated()) {
          try {
            renderer.beginDrawing();
            defaultScene.render3D(); // draw 3D entities in framebuffer
            renderer.setDrawMode2D();
            defaultScene.render2D(); // draw 2D entities in framebuffer
            renderer.swapBuffers(); // display frame
          }
          catch (...) { // device lost -> recreate renderer
            reCreateRendererContext(window.get(), g_lastWidth, g_lastHeight);
          }
        }
        else { // window is hidden -> no rendering
          renderer.skipFrame();
          std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
      }
      defaultScene.release();
    }// end of scope --> make sure resources are freed before the renderer
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