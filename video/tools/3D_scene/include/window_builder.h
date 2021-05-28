/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <memory>
#include <hardware/display_monitor.h>
#include <video/window.h>

#if defined(_WINDOWS)
# define _SYSTEM_STR(str) L"" str
# define _SYSTEM_sprintf(...) _snwprintf(__VA_ARGS__)
#else
# define _SYSTEM_STR(str) str
# define _SYSTEM_sprintf(...) snprintf(__VA_ARGS__)
#endif

#define _P_DEFAULT_WIDTH 1280
#define _P_DEFAULT_HEIGHT 800
#define _P_MIN_WIDTH 512
#define _P_MIN_HEIGHT 320
#define _P_RATIO 16.0/10.0

#define _P_WINDOW_CAPTION _SYSTEM_STR("3D Scene Renderer")

namespace scene {
  // create main window
  std::unique_ptr<pandora::video::Window> createWindow(std::shared_ptr<pandora::video::WindowResource> menu); // throws on failure
  
  // format window title
  void updateWindowCaption(pandora::video::Window& window, uint32_t sizeX, uint32_t sizeY, float freq);
}
