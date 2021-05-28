/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <memory>
#include <hardware/display_monitor.h>
#include <video/window.h>

#define _P_DEFAULT_WIDTH 1280
#define _P_DEFAULT_HEIGHT 800
#define _P_MIN_WIDTH 512
#define _P_MIN_HEIGHT 320
#define _P_RATIO 16.0/10.0

namespace scene {
  // Find appropriate window client size
  pandora::video::PixelSize computeWindowClientSize(std::shared_ptr<pandora::hardware::DisplayMonitor> primaryMonitor);
  
  // -- main menu --
  
  // Create main menu-bar
  std::shared_ptr<pandora::video::WindowResource> createMainMenu();

  // Menu handlers
  void onMenuCommand(pandora::video::WindowResource& menu, int32_t id) noexcept;
  void onMenuKey(pandora::video::WindowResource& menu, int32_t charCode) noexcept;

  //TODO classe gestion menu
  //- contient handle menu
  //- contient onMenuCommand et onMenuKey
  //- contient des handlers à enregistrer (pour réagir aux changements du menu)
}
