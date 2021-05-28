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
    void (*apiChangeHandler)() = nullptr;
    void (*vsyncChangeHandler)(bool) = nullptr;
    void (*filterChangeHandler)() = nullptr;
    
  private:
    std::shared_ptr<pandora::video::WindowResource> _resource = nullptr;
    Options& _settings;
  };
}
