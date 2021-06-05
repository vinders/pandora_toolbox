/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <memory>
#include <video/window.h>
#include "options.h"
#include "menu_manager.h"

namespace scene {
  class Program {
  public:
    static std::unique_ptr<Program> createProgram(std::shared_ptr<MenuManager> menu, pandora::video::Window& window);

    Program() = default;
    virtual ~Program() noexcept = default;

    virtual void onFilterChange() = 0;
    virtual void onSizeChange(uint32_t width, uint32_t height) = 0;
    virtual void onViewportChange() = 0;

    virtual void renderFrame() = 0;
  };
}
