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
Description : Display monitor - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <cstdint>
# include <cstdlib>
# include <string>
# include <stdexcept>
# include <vector>
# include "hardware/_private/_libraries_cocoa.h"
# include "hardware/_private/_display_monitor_impl_cocoa.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;
  
  
  // -- bindings --
  
  // move "cocoa" DisplayArea bindings to cross-platform data containers
  static inline void _moveDisplayArea(const DisplayArea_cocoa& src, DisplayArea& dest) {
    dest.x = src.x;
    dest.y = src.y;
    dest.width = src.width;
    dest.height = src.height;
  }
  // fill "cocoa" DisplayArea bindings from cross-platform data containers
  static inline void _fillDisplayArea(const DisplayArea& src, DisplayArea_cocoa& dest) {
    dest.x = src.x;
    dest.y = src.y;
    dest.width = src.width;
    dest.height = src.height;
  }
  
  // move "cocoa" DisplayMode bindings to cross-platform data containers
  static void _moveDisplayMode(const DisplayMode_cocoa& src, DisplayMode& dest) {
    dest.width = src.width;
    dest.height = src.height;
    dest.bitDepth = src.bitDepth;
    dest.refreshRate = src.refreshRate;
  }
  // fill "cocoa" DisplayMode bindings from cross-platform data containers
  static inline void _fillDisplayMode(const DisplayMode& src, DisplayMode_cocoa& dest) {
    dest.width = src.width;
    dest.height = src.height;
    dest.bitDepth = src.bitDepth;
    dest.refreshRate = src.refreshRate;
  }
  
  // move "cocoa" Attributes bindings to cross-platform data containers
  static void _moveAttributes(MonitorAttributes_cocoa& src, uint32_t unitNumber, DisplayMonitor::Attributes& dest) {
    dest.id = std::to_string(unitNumber); // use unit number instead of displayId (not sensitive to graphics switching)
    if (src.description != nullptr) {
      try { dest.description = src.description; } catch (...) {} // description not mandatory -> if alloc failure, do not report
      free(src.description);
      src.description = nullptr;
    }
    else {
      dest.description = std::string("Generic monitor ") + dest.id;
    }
    
    _moveDisplayArea(src.screenArea, dest.screenArea);
    _moveDisplayArea(src.workArea, dest.workArea);
    dest.isPrimary = src.isPrimary;
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    if (LibrariesCocoa::instance() == nullptr)
      throw std::invalid_argument("DisplayMonitor: cocoa libraries can't be used.");
    
    MonitorAttributes_cocoa attributes;
    this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_cocoa(&(this->_unitNumber), &attributes);
    if (this->_handle)
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    else
      this->_attributes.isPrimary = true;
  }
  
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (LibrariesCocoa::instance() == nullptr)
      throw std::invalid_argument("DisplayMonitor: cocoa libraries can't be used.");
    
    MonitorAttributes_cocoa attributes;
    if (this->_handle && __getMonitor_cocoa((CocoaScreenHandle)this->_handle, &(this->_unitNumber), &attributes)) {
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    } 
    else { // failure
      if (!usePrimaryAsDefault)
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
      
      this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_cocoa(&(this->_unitNumber), &attributes);
      if (this->_handle)
        _moveAttributes(attributes, this->_unitNumber, this->_attributes);
      else
        this->_attributes.isPrimary = true;
    }
  }
  
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& unitId, bool usePrimaryAsDefault) {
    if (LibrariesCocoa::instance() == nullptr)
      throw std::invalid_argument("DisplayMonitor: cocoa libraries can't be used.");
    
    MonitorAttributes_cocoa attributes;
    try {
      this->_unitNumber = (uint32_t)std::stoul(unitId);
      this->_handle = (DisplayMonitor::Handle)__getMonitorByUnit_cocoa(this->_unitNumber, &attributes);
    }
    catch (...) { // ID is not a number -> invalid
      if (!usePrimaryAsDefault)
        throw;
    }

    if (this->_handle) {
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    }
    else { // failure
      if (!usePrimaryAsDefault)
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
      
      this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_cocoa(&(this->_unitNumber), &attributes);
      if (this->_handle)
        _moveAttributes(attributes, this->_unitNumber, this->_attributes);
      else
        this->_attributes.isPrimary = true;
    }
  }
  
  DisplayMonitor::DisplayMonitor(uint32_t displayId) {
    MonitorAttributes_cocoa attributes;
    this->_handle = (DisplayMonitor::Handle)__getMonitorById_cocoa((CocoaDisplayId)displayId, &(this->_unitNumber), &attributes);
    if (this->_handle)
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    else // failure
      throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
  }
  
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    if (LibrariesCocoa::instance() == nullptr)
      throw std::invalid_argument("DisplayMonitor: cocoa libraries can't be used.");
    MonitorAttributes_cocoa attributes;
    
    CocoaDisplayId* ids = nullptr;
    uint32_t length = 0;
    if (__listMonitorIds_cocoa(&ids, &length) && ids != nullptr) {
      if (index < length)
        this->_handle = (DisplayMonitor::Handle)__getMonitorById_cocoa(__getMonitorIdFromList_cocoa(ids, index), &(this->_unitNumber), &attributes);
      free(ids);
    }
    
    if (this->_handle) {
      _moveAttributes(attributes, this->_unitNumber, this->_attributes);
    }
    else { // failure
      if (!usePrimaryAsDefault)
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
      
      this->_handle = (DisplayMonitor::Handle)__getPrimaryMonitor_cocoa(&(this->_unitNumber), &attributes);
      if (this->_handle)
        _moveAttributes(attributes, this->_unitNumber, this->_attributes);
      else
        this->_attributes.isPrimary = true;
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    if (LibrariesCocoa::instance() == nullptr)
      throw std::invalid_argument("DisplayMonitor: cocoa libraries can't be used.");
    std::vector<DisplayMonitor> monitorList;

    CocoaDisplayId* ids = nullptr;
    uint32_t length = 0;
    if (__listMonitorIds_cocoa(&ids, &length) && ids != nullptr) {
      for (uint32_t i = 0; i < length; ++i) {
        try {
          DisplayMonitor monitor(static_cast<uint32_t>(__getMonitorIdFromList_cocoa(ids, i))); // use getter (because of alloc type cast)
          monitorList.push_back(std::move(monitor));
        }
        catch (const std::bad_alloc&) { free(ids); throw; }
        catch (...) {} // ignore invalid_argument
      }
      free(ids);
    }
    return monitorList;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const { // not supported for MacOS
    return ""; // no reliable way that would work on both Intel & Apple Silicon CPUs 
               //                                 + with both PCIe & embedded GPUs
  }


// -- display modes -- ---------------------------------------------------------

  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    
    DisplayMode mode;
    DisplayMode_cocoa modeCocoa;
    if (this->_handle && __getDisplayMode_cocoa(displayId, &modeCocoa) ) {
      _moveDisplayMode(modeCocoa, mode);
    }
    else {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode) {
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    
    DisplayMode_cocoa modeCocoa;
    _fillDisplayMode(mode, modeCocoa);
    return (this->_handle && __setDisplayMode_cocoa(displayId, this->_unitNumber, &modeCocoa) );
  }
  
  bool DisplayMonitor::setDefaultDisplayMode() {
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    
    return (this->_handle && __setDefaultDisplayMode_cocoa(displayId, this->_unitNumber) );
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    std::vector<DisplayMode> modes;
    
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    if (!this->_handle)
      return modes;
    
    DisplayMode_cocoa* modesCocoa = nullptr;
    uint32_t length = 0;
    __listDisplayModes_cocoa(displayId, &modesCocoa, &length);
    if (modesCocoa != nullptr) {
      try {
        for (int i = 0; i < length; ++i) {
          modes.emplace_back();
          _moveDisplayMode(modesCocoa[i], modes.back());
        }
      }
      catch (...) { free(modesCocoa); throw; }
      free(modesCocoa);
    }
    return modes;
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    return isEnabled; // always enabled: on->true (can't be disabled: off->false)
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle) const noexcept {
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    
    if (!this->_handle || !LibrariesCocoa_readScreenDpi((CocoaScreenHandle)this->_handle, &outDpiX, &outDpiY)) { 
      outDpiX = outDpiY = static_cast<uint32_t>(__P_HARDWARE_COCOA_DEFAULT_DPI);
    }
  }
  
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle) const noexcept {
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    
    if (!this->_handle || !LibrariesCocoa_readScreenScaling((CocoaScreenHandle)this->_handle, &outScaleX, &outScaleY)) { 
      outScaleX = outScaleY = __P_HARDWARE_COCOA_DEFAULT_SCALE;
    }
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, 
                                                            bool hasMenu, uint32_t hasBorders, uint32_t hasCaption) const noexcept {
    // refresh handle (fix automatic graphics switching)
    CocoaDisplayId displayId = 0;
    this->_handle = (DisplayMonitor::Handle)__getMonitorHandle_cocoa(this->_unitNumber, &displayId);
    
    if (this->_handle) { 
      DisplayArea windowArea;
      DisplayArea_cocoa windowAreaCocoa;
      
      DisplayArea_cocoa clientAreaCocoa;
      _fillDisplayArea(clientArea, clientAreaCocoa);
      __clientAreaToWindowArea_cocoa((CocoaScreenHandle)this->_handle, &clientAreaCocoa, (CocoaScreenHandle)windowHandle, 
                                     hasMenu, hasBorders, hasCaption, &windowAreaCocoa);

      _moveDisplayArea(windowAreaCocoa, windowArea);
      return windowArea;
    }
    return clientArea;
  }
#endif
