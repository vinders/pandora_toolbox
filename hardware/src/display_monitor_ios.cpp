/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - iOS implementation
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
# include <cstdint>
# include <string>
# include <stdexcept>
# include <vector>
# include <cstdlib>
# include <climits>
# include <cmath>
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;
  
//obj-C:
// CGRect Rect=[[UIScreen mainScreen] bounds];
  
/*//swift:
var screenWidth: CGFloat {
    if UIInterfaceOrientationIsPortrait(screenOrientation) {
        return UIScreen.mainScreen().bounds.size.width
    } else {
        return UIScreen.mainScreen().bounds.size.height
    }
}
var screenHeight: CGFloat {
    if UIInterfaceOrientationIsPortrait(screenOrientation) {
        return UIScreen.mainScreen().bounds.size.height
    } else {
        return UIScreen.mainScreen().bounds.size.width
    }
}
var screenOrientation: UIInterfaceOrientation {
    return UIApplication.sharedApplication().statusBarOrientation
}
*/


// -- monitor handle & description/attributes -- -------------------------------

  static bool _readDisplayMonitorAttributes(DisplayMonitor::Handle monitorHandle, DisplayMonitor::Attributes& outAttr) { return false; }
  static inline void _readPrimaryDisplayMonitorInfo(DisplayMonitor::Handle& outHandle, DisplayMonitor::Attributes& outAttr) {}
  static inline DisplayMonitor::Handle _getDisplayMonitorById(DisplayMonitor::DeviceId id, DisplayMonitor::Attributes* outAttr) { return 0; }
  static inline bool _listDisplayMonitors(std::vector<DisplayMonitor::Handle>& out) { return false; }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (!this->_handle || !_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = _getDisplayMonitorById(id, &(this->_attributes));
    if (!this->_handle) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    std::vector<DisplayMonitor::Handle> handles;
    if (_listDisplayMonitors(handles) && index < handles.size())
      this->_handle = handles[index];
    if (!this->_handle || !_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitors;

    std::vector<DisplayMonitor::Handle> handles;
    if (_listDisplayMonitors(handles)) {
      for (auto it : handles) {
        try {
          monitors.emplace_back(it, false);
        }
        catch (...) {}
      }
    }

    if (monitors.empty()) { // primary monitor as default
      monitors.emplace_back();
      if (monitors[0]._handle == (DisplayMonitor::Handle)0) // no display monitor
        monitors.clear();
    }
    return monitors;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return "";
  }
  

// -- display modes -- ---------------------------------------------------------

  static inline bool _getMonitorDisplayMode(const DisplayMonitor::DeviceId& id, DisplayMode& out) noexcept { return false; }
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!_getMonitorDisplayMode(this->_attributes.id, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  static inline bool _setMonitorDisplayMode(const DisplayMonitor::DeviceId& id, const DisplayMode& mode) noexcept { return false; }
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) {
    if (_setMonitorDisplayMode(this->_attributes.id, mode)) {
      if (refreshAttributes) {
        if (!_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
          this->_attributes.screenArea.width = mode.width;
          this->_attributes.screenArea.height = mode.height;
        }
      }
      return true;
    }
    return false;
  }
  
  static inline bool _setDefaultMonitorDisplayMode(const DisplayMonitor::DeviceId& id) noexcept { return false; }
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) {
    if (_setDefaultMonitorDisplayMode(this->_attributes.id)) {
      if (refreshAttributes)
        _readDisplayMonitorAttributes(this->_handle, this->_attributes);
      return true;
    }
    return false;
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    std::vector<DisplayMode> modes;
    //...
    return modes;
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    return true;
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle windowHandle) const noexcept {

  }
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle windowHandle) const noexcept {
    //...
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, bool hasMenu, uint32_t, uint32_t) const noexcept {
    return DisplayArea{ 0, 0, 0, 0 };
  }
#endif
