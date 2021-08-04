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
# include "hardware/_private/_display_monitor_impl_uikit.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;
  
  
  // -- bindings --
  
  // move "ios" DisplayArea bindings to cross-platform data containers
  static inline void _moveDisplayArea(const DisplayArea_uikit& src, DisplayArea& dest) {
    dest.x = src.x;
    dest.y = src.y;
    dest.width = src.width;
    dest.height = src.height;
  }
  // fill "ios" DisplayArea bindings from cross-platform data containers
  static inline void _fillDisplayArea(const DisplayArea& src, DisplayArea_uikit& dest) {
    dest.x = src.x;
    dest.y = src.y;
    dest.width = src.width;
    dest.height = src.height;
  }
  
  // move "ios" DisplayMode bindings to cross-platform data containers
  static void _moveDisplayMode(const DisplayMode_uikit& src, DisplayMode& dest) {
    dest.width = src.width;
    dest.height = src.height;
    dest.bitDepth = src.bitDepth;
    dest.refreshRate = src.refreshRate;
  }
  // fill "ios" DisplayMode bindings from cross-platform data containers
  static inline void _fillDisplayMode(const DisplayMode& src, DisplayMode_uikit& dest) {
    dest.width = src.width;
    dest.height = src.height;
    dest.bitDepth = src.bitDepth;
    dest.refreshRate = src.refreshRate;
  }
  
  // move "ios" Attributes bindings to cross-platform data containers
  static void _moveAttributes(MonitorAttributes_uikit& src, uint32_t& outUnitNumber, DisplayMonitor::Attributes& dest) {
    outUnitNumber = src.index;
    dest.id = std::to_string(src.index);
    if (src.description != nullptr) {
      try { dest.description = src.description; } catch (...) {} // description not mandatory -> if alloc failure, do not report
      free(src.description);
      src.description = nullptr;
    }
    else {
      dest.description = std::string("Screen ") + dest.id;
    }
    
    _moveDisplayArea(src.screenArea, dest.screenArea);
    _moveDisplayArea(src.workArea, dest.workArea);
    dest.isPrimary = src.isPrimary;
  }
  

// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    MonitorAttributes_uikit attributes;
    this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_uikit(&attributes);
    if (this->_handle)
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
  }
  
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    MonitorAttributes_uikit attributes;
    if (this->_handle && __getMonitor_uikit(this->_handle, &attributes)) {
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    }
    else {
      if (usePrimaryAsDefault) {
        this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_uikit(&attributes);
        if (this->_handle)
          _moveAttributes(attributes, this->_unitNumber, this->_attributes);
      }
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    uint32_t index = 0;
    try {
      index = (uint32_t)std::stoul(id);
    }
    catch (...) { 
      if (!usePrimaryAsDefault)
        throw;
    }
    
    MonitorAttributes_uikit attributes;
    this->_handle = __getMonitorByIndex_uikit(index, &attributes);
    if (this->_handle) {
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    }
    else {
      if (usePrimaryAsDefault) {
        this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_uikit(&attributes);
        if (this->_handle)
          _moveAttributes(attributes, this->_unitNumber, this->_attributes);
      }
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    MonitorAttributes_uikit attributes;
    this->_handle = __getMonitorByIndex_uikit(index, &attributes);
    if (this->_handle) {
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    }
    else {
      if (usePrimaryAsDefault) {
        this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_uikit(&attributes);
        if (this->_handle)
          _moveAttributes(attributes, this->_unitNumber, this->_attributes);
      }
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }
  
  // ---

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitors;
    
    uint32_t length = __countMonitorHandles_uikit();
    if (length > 0) {
      for (uint32_t i = 0; i < length; ++i) {
        try {
          monitors.emplace_back(false, i);
        }
        catch (std::bad_alloc) { throw; }
        catch (...) {} // ignore if "not found"
      }
    }
    return monitors;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return ""; // not supported
  }
  

// -- display modes -- ---------------------------------------------------------

  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    this->_handle = __getMonitorHandle_uikit(this->_unitNumber);
    DisplayMode mode;
    
    DisplayMode_uikit displayModeIos;
    if (__getDisplayMode_uikit(this->_unitNumber, &displayModeIos)) {
      _moveDisplayMode(displayModeIos, mode);
    }
    else {
      double scale = __getScaling_uikit(this->_handle);
      mode.width = (uint32_t)((double)(this->_attributes.screenArea.width) * scale + 0.5000001); // points to pixels
      mode.height = (uint32_t)((double)(this->_attributes.screenArea.height) * scale + 0.5000001);
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode) {
    this->_handle = __getMonitorHandle_uikit(this->_unitNumber);
    
    DisplayMode_uikit desiredModeIos;
    _fillDisplayMode(mode, desiredModeIos);
    return (__setDisplayMode_uikit(this->_unitNumber, &desiredModeIos)) ? true : false;
  }
  
  bool DisplayMonitor::setDefaultDisplayMode() {
    this->_handle = __getMonitorHandle_uikit(this->_unitNumber);
    return (__setDefaultDisplayMode_uikit(this->_unitNumber)) ? true : false;
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    this->_handle = __getMonitorHandle_uikit(this->_unitNumber);
    std::vector<DisplayMode> modes;
    
    DisplayMode_uikit* modesIos = nullptr;
    uint32_t length = 0;
    if (__listDisplayModes_uikit(this->_unitNumber, &modesIos, &length) && modesIos != nullptr) {
      for (int i = 0; i < length; ++i) {
        DisplayMode cur;
        _moveDisplayMode(modesIos[i], cur);
        modes.push_back(std::move(cur));
      }
      free(modesIos);
    }
    return modes;
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    return isEnabled; // always enabled
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle windowHandle) const noexcept {
    this->_handle = __getMonitorHandle_uikit(this->_unitNumber);
    double scale = __getScaling_uikit(this->_handle);
    outDpiX = outDpiY = static_cast<uint32_t>(__P_HARDWARE_UIKIT_BASE_DPI * scale + 0.5000001);
  }
  
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle windowHandle) const noexcept {
    this->_handle = __getMonitorHandle_uikit(this->_unitNumber);
    double scale = __getScaling_uikit(this->_handle);
    outScaleX = outScaleY = (float)scale;
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, bool hasMenu, uint32_t, uint32_t) const noexcept {
    DisplayArea_uikit clientAreaIos;
    DisplayArea_uikit windowAreaIos;
    _fillDisplayArea(clientArea, clientAreaIos);
    __clientAreaToWindowArea_uikit(&clientAreaIos, (IosAppHandle)windowHandle, &windowAreaIos);
    
    DisplayArea windowArea;
    _moveDisplayArea(windowAreaIos, windowArea);
    return windowArea;
  }
#endif
