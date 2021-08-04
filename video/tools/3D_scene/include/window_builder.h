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
