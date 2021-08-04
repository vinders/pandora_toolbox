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

#include <cstdint>
#include <memory>
#include <video/window_resource.h>
#include "options.h"

namespace scene {
  enum class ApiChangeType : uint32_t {
    rendererChange = 0, // Direct3D/OpenGL/OpenGLES
    vsyncChange = 1,    // set vsync on/off
    viewportChange = 2, // single/multiple swap-chains
    monitorChange = 3   // content scale change or HDR on/off
  };

  // menu builder + manager
  class MenuManager final {
  public:
    // create menu-bar
    MenuManager();
    ~MenuManager() = default;
    
    std::shared_ptr<pandora::video::WindowResource> resource() noexcept { return _resource; }
    const Options& settings() const noexcept { return _settings; }
    
    // menu event management
    void onMenuCommand(int32_t id);
    
    // register event handlers (after menu changes)
    void (*apiChangeHandler)(ApiChangeType) = nullptr;
    void (*filterChangeHandler)() = nullptr;
    
  private:
    std::shared_ptr<pandora::video::WindowResource> _resource = nullptr;
    Options _settings;
  };
}
