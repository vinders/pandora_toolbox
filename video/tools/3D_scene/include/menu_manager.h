/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include <video/window_resource.h>
#include "options.h"

namespace scene {
  enum class ApiChangeType : uint32_t {
    rendererChange = 0,   // Direct3D/OpenGL/OpenGLES
    vsyncChange = 1,      // set vsync on/off
    splitScreenChange = 2,// single/multiple swap-chains
    monitorChange = 3,    // content scale change or HDR on/off
    specialChange = 4     // special feature on/off (fire, tv...)
  };

  // menu builder + manager
  class MenuManager final {
  public:
    // create menu-bar
    MenuManager(Options& outSettings);
    ~MenuManager() = default;
    
    std::shared_ptr<pandora::video::WindowResource> resource() noexcept { return _resource; }
    
    // menu event management
    void onMenuCommand(int32_t id);
    
    // register event handlers (after menu changes)
    void (*apiChangeHandler)(ApiChangeType) = nullptr;
    void (*filterChangeHandler)() = nullptr;
    
  private:
    std::shared_ptr<pandora::video::WindowResource> _resource = nullptr;
    Options& _settings;
  };
}
