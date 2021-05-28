/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include <video/window_resource.h>
#include "option_types.h"

namespace scene {
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
    void (*apiChangeHandler)(RenderingApi) = nullptr;
    void (*mouseSensivHandler)(int32_t) = nullptr;
    void (*aaChangeHandler)(AntiAliasing) = nullptr;
    void (*fxChangeHandler)(VisualEffect) = nullptr;
    void (*screenFilterHandler)(Interpolation) = nullptr;
    void (*textureFilterHandler)(Interpolation) = nullptr;
    void (*spriteFilterHandler)(Interpolation) = nullptr;
    
  private:
    std::shared_ptr<pandora::video::WindowResource> _resource = nullptr;
  };
}
