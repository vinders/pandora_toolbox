/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - Android implementation
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cstdint>
# include <string>
# include <stdexcept>
# include <vector>
//# include "hardware/_private/_libraries_andr.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;


// -- monitor attributes - id/area/description/primary -- ----------------------

  namespace attributes {
    // read all attributes of a monitor handle
    static bool read(DisplayMonitor::Handle handle, DisplayMonitor::Attributes& outAttr) {

      return false;
    }
  }
  
  //TODO - stocker aussi dpi/scale -> les invalider uniquement si on fait setDisplayMode/setDefaultDisplayMode
  //-> on ne réinstancie JNI que pour faire un changement de mode


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    //_readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    // if (!this->_handle || !_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
      // if (usePrimaryAsDefault)
        // _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      // else
        // throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    // }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    // this->_handle = _getDisplayMonitorById(id, &(this->_attributes));
    // if (!this->_handle) {
      // if (usePrimaryAsDefault)
        // _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      // else
        // throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    // }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    // std::vector<DisplayMonitor::Handle> handles;
    // if (_listDisplayMonitors(handles) && index < handles.size())
      // this->_handle = handles[index];
    // if (!this->_handle || !_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
      // if (usePrimaryAsDefault)
        // _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      // else
        // throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    // }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitors;
    
    return monitors;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return "";
  }
  

// -- display modes -- ---------------------------------------------------------

  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (false) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode) {
    return false;
  }
  bool DisplayMonitor::setDefaultDisplayMode() {
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
    return clientArea;
  }
#endif
