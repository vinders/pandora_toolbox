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
#include <cstdio>
#include <hardware/display_monitor.h>
#include "window_builder.h"
#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# include "../../../../_img/test_win32/resources.h"
#endif

using namespace pandora::video;

// find default window client size
static pandora::video::PixelSize __computeWindowClientSize(std::shared_ptr<pandora::hardware::DisplayMonitor> primaryMonitor) {
  pandora::video::PixelSize windowSize{ _P_DEFAULT_WIDTH, _P_DEFAULT_HEIGHT };
  
  if (primaryMonitor && windowSize.height + 60 > primaryMonitor->attributes().workArea.height) {
    windowSize.height = primaryMonitor->attributes().workArea.height - 64;
    if (windowSize.height < _P_MIN_HEIGHT)
      windowSize.height = _P_MIN_HEIGHT;
    windowSize.width = static_cast<uint32_t>(_P_RATIO * (double)windowSize.height + 0.50001);
  }
  return windowSize;
}

// ---

// create main window
std::unique_ptr<pandora::video::Window> scene::createWindow(std::shared_ptr<pandora::video::WindowResource> menu) { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
    auto cursor = WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR));
# else
    auto mainIcon = WindowResource::buildIconFromPackage("logo_big.png");
    auto cursor = WindowResource::buildCursorFromPackage("base_cur.png");
# endif
  auto primaryMonitor = std::make_shared<pandora::hardware::DisplayMonitor>();
  PixelSize windowSize = __computeWindowClientSize(primaryMonitor);
  
  Window::Builder builder;
  auto window = builder.setDisplayMode(WindowType::window, WindowBehavior::globalContext, ResizeMode::resizable|ResizeMode::homothety)
                       .setParentMonitor(primaryMonitor)
                       .setSize(windowSize.width, windowSize.height)
                       .setPosition(Window::Builder::centeredPosition(), Window::Builder::centeredPosition())
                       .setIcon(mainIcon)
                       .setCursor(cursor)
                       .setBackgroundColor(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0)))
                       .setMenu(menu)
                       .create(_SYSTEM_STR("3D_SCENE_WINDOW0"), _P_WINDOW_CAPTION);
  window->setMinClientAreaSize(_P_MIN_WIDTH, _P_MIN_HEIGHT);
  return window;
}

// format window title
void scene::updateWindowCaption(pandora::video::Window& window, uint32_t sizeX, uint32_t sizeY, float freq) {
  window_char buffer[48];
  _SYSTEM_sprintf(buffer, 48, _P_WINDOW_CAPTION " - %ux%u - %.2fHz", sizeX, sizeY, freq);
  window.setCaption(buffer);
}
