/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - Wayland implementation (Linux)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdint>
# include <cstdlib>
# include <cstring>
# include <string>
# include <stdexcept>
# include <vector>
# include <unistd.h>
# include "hardware/_private/_libraries_wln.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;


// -- monitor attributes - id/area/description/primary -- ----------------------

  namespace attributes {
    // read all attributes of a monitor handle
    static bool read(WaylandOutput& monitor, uint32_t index, DisplayMonitor::Attributes& outAttr) {
      outAttr.id = std::to_string(monitor.id);
      outAttr.description = (monitor.description) ? monitor.description : "Generic monitor";
      
      outAttr.screenArea.x = monitor.x;
      outAttr.screenArea.y = monitor.y;
      LibrariesWayland::instance().getScreenPixelSize(monitor, outAttr.screenArea.width, outAttr.screenArea.height);
      memcpy((void*)&(outAttr.workArea), (void*)&(outAttr.screenArea), sizeof(DisplayArea)); // known limitation: work area same as screen area
      
      outAttr.isPrimary = (index == 0);
      return false;
    }
  }


// -- get display monitors (handle + attributes) -- ----------------------------

  namespace monitors {
    // read handle/attributes of primary/default monitor
    static DisplayMonitor::Handle getPrimary(uint32_t& outId, DisplayMonitor::Attributes& outAttr) {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      DisplayMonitor::Handle handle = (DisplayMonitor::Handle)0;
      if (libs.wl.outputs) {
        attributes::read(*(libs.wl.outputs), 0, outAttr);
        handle = (DisplayMonitor::Handle)libs.wl.outputs->output;
        outId = libs.wl.outputs->id;
      }
      outAttr.isPrimary = true;
      return handle;
    }
    
    // read handle/attributes of monitor by handle
    static bool getByHandle(DisplayMonitor::Handle handle, uint32_t& outId, DisplayMonitor::Attributes& outAttr) {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      uint32_t index = 0;
      for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
        if (cur->output == (wl_output*)handle) {
          attributes::read(*cur, index, outAttr);
          outId = cur->id;
          return true;
        }
        ++index;
      }
      return false;
    }
    
    // read handle/attributes of monitor by ID
    static DisplayMonitor::Handle getById(const DisplayMonitor::DeviceId& deviceId, uint32_t& outId, DisplayMonitor::Attributes& outAttr) {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      uint32_t id = 0;
      try {
        id = std::stoul(deviceId);
      }
      catch (...) { return (DisplayMonitor::Handle)0; }

      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      uint32_t index = 0;
      for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
        if (cur->id == id) {
          attributes::read(*cur, index, outAttr);
          outId = id;
          return (DisplayMonitor::Handle)cur->output;
        }
        ++index;
      }
      return (DisplayMonitor::Handle)0;
    }
    
    // read handle/attributes of monitor by index
    static DisplayMonitor::Handle getByIndex(uint32_t targetIndex, uint32_t& outId, DisplayMonitor::Attributes& outAttr) {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);

      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      uint32_t index = 0;
      WaylandOutput* it = libs.wl.outputs;
      while (it != nullptr && index < targetIndex) {
        it = it->next;
        ++index;
      }
      
      if (it != nullptr) {
        attributes::read(*it, targetIndex, outAttr);
        outId = it->id;
        return (DisplayMonitor::Handle)it->output;
      }
      return (DisplayMonitor::Handle)0;
    }
    
    // ---
    
    // list all active monitors
    static inline bool listMonitors(std::vector<DisplayMonitor>& resultsOut) {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
        try {
          resultsOut.emplace_back((DisplayMonitor::Handle)cur->output, false);
        }
        catch (const std::bad_alloc&) { throw; }
        catch (...) {}
      }
      return false; 
    }
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    this->_handle = monitors::getPrimary(this->_id, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (!this->_handle || !monitors::getByHandle(this->_handle, this->_id, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_id, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = monitors::getById(id, this->_id, this->_attributes);
    if (!this->_handle) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_id, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    this->_handle = monitors::getByIndex(index, this->_id, this->_attributes);
    if (!this->_handle) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_id, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitorList;
    monitors::listMonitors(monitorList);
    return monitorList;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return ""; // not supported
  }
  

// -- display modes -- ---------------------------------------------------------

  namespace monitors {
    // read display resolution/depth/rate of a monitor
    static bool getDisplayMode(uint32_t id, DisplayMode& out) noexcept { 
      try {
        LibrariesWayland& libs = LibrariesWayland::instance();
        wl_display_roundtrip(libs.wl.display);
        
        std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
        for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
          if (cur->id == id) {
            if (cur->modes != nullptr && cur->modesLength > 0) {
              WaylandDisplayMode& modeInfo = cur->modes[cur->currentMode];
              
              out.width = modeInfo.width;
              out.height = modeInfo.height;
              out.bitDepth = 32;
              out.refreshRate = modeInfo.refreshRate;
              return true;
            }
            break;
          }
        }
      } 
      catch (...) {}
      return false;
    }
    
    // ---
  
    // list all display modes of a monitor
    static inline std::vector<DisplayMode> listDisplayModes(uint32_t id) {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      std::vector<DisplayMode> modes;
      
      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
        if (cur->id == id) {
          if (cur->modes != nullptr) {
            for (size_t i = 0; i < cur->modesLength; ++i) {
              WaylandDisplayMode& modeInfo = cur->modes[cur->currentMode];
              DisplayMode data;
              data.width = modeInfo.width;
              data.height = modeInfo.height;
              data.bitDepth = 32;
              data.refreshRate = modeInfo.refreshRate;
              
              bool isAlreadyListed = false;
              for (auto& it : modes) {
                if (it.width == data.width && it.height == data.height && it.bitDepth == data.bitDepth && it.refreshRate == data.refreshRate) {
                  isAlreadyListed = true;
                  break;
                }
              }
              if (!isAlreadyListed)
                modes.emplace_back(std::move(data));
            }
          }
          break;
        }
      }
      return modes;
    }
  }
  
  // ---
  
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!monitors::getDisplayMode(this->_id, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) {
    return false; // not supported by wayland
  }
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) {
    return false; // not supported by wayland... again...
  }
  
  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    return monitors::listDisplayModes(this->_id);
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    return isEnabled; // always DPI aware
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle) const noexcept {
    try {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
        if (cur->id == this->_id) {
          outDpiX = outDpiY = libs.getMonitorDpi(*cur);
          return;
        }
      }
    } 
    catch (...) {}
    outDpiX = outDpiY = __P_HARDWARE_WAYLAND_BASE_DPI;
  }
  
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle) const noexcept {
    try {
      LibrariesWayland& libs = LibrariesWayland::instance();
      wl_display_roundtrip(libs.wl.display);
      
      std::lock_guard<std::recursive_mutex> guard(libs.wl.outputsLock);
      for (WaylandOutput* cur = libs.wl.outputs; cur != nullptr; cur = cur->next) {
        if (cur->id == this->_id) {
          outScaleX = outScaleY = static_cast<float>(cur->scaleFactor);
          return;
        }
      }
    } 
    catch (...) {}
    outScaleX = outScaleY = 1.0f;
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle, 
                                                            bool, uint32_t, uint32_t) const noexcept {
    return clientArea; // not supported without conflicting with existing window listeners...
  }
#endif
