/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - X11 implementation (Linux/BSD)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdint>
# include <cstdlib>
# include <string>
# include <stdexcept>
# include <vector>
# include <unistd.h>
# include "hardware/_private/_libraries_wln.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;


// -- monitor attributes - id/area/description/primary -- ----------------------

  namespace attributes {
    // verify if a monitor is the primary device
    static inline bool _isPrimaryDevice(DisplayMonitor::Handle handle) {
      //...
      return true;
    }
    
    // read brand/description string of adapter
    static inline std::string _readAdapterName(DisplayMonitor::Handle handle) {
      //...
      return "";
    }
    
    // read work area of a monitor (usable position/size)
    static inline void _readScreenArea(DisplayMonitor::Handle handle, 
                                       DisplayArea& outScreenArea, DisplayArea& outWorkArea) {
      //...
    }
    
    // ---
    
    // read all attributes of a monitor handle
    static bool read(DisplayMonitor::Handle handle, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      //...
      return false;
    }
    // find monitor handle for ID + read all attributes (if outAttr not null)
    static DisplayMonitor::Handle readForId(const DisplayMonitor::DeviceId& id, DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      //...
      return false;
    }
  }


// -- get display monitors (handle + attributes) -- ----------------------------

  namespace monitors {
    // read handle/attributes of primary/default monitor
    static DisplayMonitor::Handle getPrimary(DisplayMonitor::Handle& outController, DisplayMonitor::Attributes& outAttr) {
      DisplayMonitor::Handle handle = (DisplayMonitor::Handle)0;
      //...
      outAttr.isPrimary = true;
      return (DisplayMonitor::Handle)handle;
    }
    
    // ---
    
    // list handles of all active monitors
    static inline bool listHandles(std::vector<DisplayMonitor::Handle>& resultsOut) {
      //...
      return false; 
    }
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (!this->_handle || !attributes::read(this->_handle, this->_controller, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = attributes::readForId(id, this->_controller, this->_attributes);
    if (!this->_handle) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    std::vector<DisplayMonitor::Handle> handles;
    if (monitors::listHandles(handles) && index < handles.size())
      this->_handle = handles[index];
    if (!this->_handle || !attributes::read(this->_handle, this->_controller, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_controller, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitorList;

    std::vector<DisplayMonitor::Handle> handles;
    if (monitors::listHandles(handles)) {
      for (auto it : handles) {
        try {
          monitorList.emplace_back(it, false);
        }
        catch (...) {}
      }
    }
    return monitorList;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return attributes::_readAdapterName(this->_handle);
  }
  

// -- display modes -- ---------------------------------------------------------

  namespace monitors {
    // read display resolution/depth/rate of a monitor
    static bool getDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller, DisplayMode& out) noexcept { 
      //...
      return false;
    }

    // set display resolution/depth/rate of a monitor
    static inline bool setDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller, const DisplayMode& desiredMode) { 
      //...
      return false;
    }
    
    // reset display resolution/depth/rate of a monitor to default values
    static inline bool setDefaultDisplayMode(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller) { 
      //...
      return false;
    }
    
    // ---
  
    // list all display modes of a monitor
    static inline std::vector<DisplayMode> listDisplayModes(DisplayMonitor::Handle handle, DisplayMonitor::Handle controller) {
      std::vector<DisplayMode> modes;
      //...
      return modes;
    }
  }
  
  // ---
  
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!monitors::getDisplayMode(this->_handle, this->_controller, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) {
    if (monitors::setDisplayMode(this->_handle, this->_controller, mode)) {
      if (refreshAttributes) {
        if (!attributes::read(this->_handle, this->_controller, this->_attributes)) {
          this->_attributes.screenArea.width = mode.width;
          this->_attributes.screenArea.height = mode.height;
        }
      }
      return true;
    }
    return false;
  }
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) {
    if (monitors::setDefaultDisplayMode(this->_handle, this->_controller)) {
      if (refreshAttributes)
        attributes::read(this->_handle, this->_controller, this->_attributes);
      return true;
    }
    return false;
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    return monitors::listDisplayModes(this->_handle, this->_controller);
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    //...
    return true;
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle) const noexcept {
    //...
    outDpiX = outDpiY = 72;
  }
  
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle) const noexcept {
    //...
    outScaleX = outScaleY = 1.0f;
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, 
                                                            bool hasMenu, uint32_t hasBorders, uint32_t hasCaption) const noexcept {
    //...
    return clientArea;
  }
#endif
