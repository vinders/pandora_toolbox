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
Description : Display monitor - Android implementation
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cstdint>
# include <string>
# include <stdexcept>
# include <vector>
# include <system/api/android_app.h>
# include "hardware/_private/_libraries_andr.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;
  using pandora::system::AndroidApp;


// -- monitor attributes - id/area/description/primary -- ----------------------

  namespace attributes {
    // read primary screen area + density (screen position/size + work area + DPI/scale)
    static inline void _readScreenAreaDensity(AndroidJavaSession& jenv, AndroidBindings& bindings, 
                                              jobject& activity, jobject& windowManager, jobject& display, 
                                              DisplayMonitor::Attributes& outAttr, DisplayMonitor::Density& outDensity) {
      // WindowMetrics (API level >= 30)
      if (bindings._windowMetrics.isAvailable) {
        bindings.readDisplayDensity_wm(jenv, activity, display, outDensity);
        bindings.readDisplayWorkArea_wm(jenv, windowManager, outAttr.workArea);
      }
      // DisplayMetrics (API level < 30)
      else { 
        bindings.readDisplayDensity_dm(jenv, display, outDensity);
        bindings.readDisplayWorkArea_dm(jenv, display, outAttr.workArea);
      }
      
      // work area: pixels to points
      outAttr.workArea.x = static_cast<int32_t>( ((double)outAttr.workArea.x / outDensity.scale) + 0.500001);
      outAttr.workArea.y = static_cast<int32_t>( ((double)outAttr.workArea.y / outDensity.scale) + 0.500001);
      outAttr.workArea.width = static_cast<int32_t>( ((double)outAttr.workArea.width / outDensity.scale) + 0.500001);
      outAttr.workArea.height = static_cast<int32_t>( ((double)outAttr.workArea.height / outDensity.scale) + 0.500001);

      // screen area (points)
      outAttr.screenArea.x = 0;
      outAttr.screenArea.y = 0;
      if (bindings._config.screenWidthDpValue > 0 && bindings._config.screenHeightDpValue > 0) {
        outAttr.screenArea.width = bindings._config.screenWidthDpValue;
        outAttr.screenArea.height = bindings._config.screenHeightDpValue;
      }
      else {
        outAttr.screenArea.width = outAttr.workArea.width + outAttr.workArea.x;
        outAttr.screenArea.height = outAttr.workArea.height + outAttr.workArea.y;
      }
    }

    // read attributes and display modes of primary monitor
    static DisplayMonitor::Handle readPrimary(AndroidJavaSession& jenv, DisplayMonitor::Attributes& outAttr, 
                                              DisplayMonitor::Density& outDensity) {
      AndroidBindings& bindings = AndroidBindings::globalInstance();
      bindings.bindDisplayClasses(jenv);
      jobject activity = AndroidApp::instance().state().activity->clazz;
      
      jobject windowManager = jenv.env().CallObjectMethod(activity, bindings._activity.getWindowManager); // windowManager = app.getWindowManager()
      _P_THROW_ON_ACCESS_FAILURE(jenv, "window manager", windowManager);
      jobject display = (bindings._windowMetrics.isAvailable) 
                      ? jenv.env().CallObjectMethod(activity, bindings._activity.getDisplay) // API level >= 30
                      : jenv.env().CallObjectMethod(windowManager, bindings._windowManager.getDefaultDisplay); // API level < 30
      _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
      
      // read ID + primary
      int displayId = jenv.env().CallIntMethod(display, bindings._display.getDisplayId); // displayId = display.getDisplayId()
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "display.getDisplayId");
      if (displayId <= DEFAULT_DISPLAY) {
        displayId = DEFAULT_DISPLAY; // 0
        outAttr.id = "0";
        outAttr.isPrimary = true;
      }
      else {
        outAttr.id = std::to_string(displayId);
        outAttr.isPrimary = false;
      }
      
      // read screen area + work area + density/DPI
      ::attributes::_readScreenAreaDensity(jenv, bindings, activity, windowManager, display, outAttr, outDensity);

      // read description
      jstring displayName = static_cast<jstring>(jenv.env().CallObjectMethod(display, bindings._display.getName)); // displayName = display.getName()
      if (jenv.env().ExceptionCheck() || !AndroidBindings::jstringToString(jenv, displayName, outAttr.description)) {
        jenv.env().ExceptionClear();
        outAttr.description = std::string("Screen ") + outAttr.id;
      }
      return (DisplayMonitor::Handle)AndroidBindings::displayIdToHandle(displayId);
    }
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    AndroidJavaSession env;
    this->_handle = attributes::readPrimary(env, this->_attributes, this->_density);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault) {
    AndroidJavaSession env;
    this->_handle = attributes::readPrimary(env, this->_attributes, this->_density);
    if (!usePrimaryAsDefault && this->_handle != monitorHandle)
      throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    AndroidJavaSession env;
    this->_handle = attributes::readPrimary(env, this->_attributes, this->_density);
    if (!usePrimaryAsDefault && this->_attributes.id != id)
      throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    if (!usePrimaryAsDefault && index != 0) // currently only default screen supported (if available)
      throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    
    AndroidJavaSession env;
    this->_handle = attributes::readPrimary(env, this->_attributes, this->_density);
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitors;
    try {
      monitors.emplace_back(); // currently only default screen supported (if available)
    }
    catch (const std::bad_alloc&) { throw; }
    catch (...) {} // ignore if display not available
    return monitors;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return ""; // not supported
  }
  

// -- display modes -- ---------------------------------------------------------

  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    try {
      AndroidJavaSession jenv;
      AndroidBindings& bindings = AndroidBindings::globalInstance(); // already bound in DisplayMonitor constructor
      jobject activity = AndroidApp::instance().state().activity->clazz;
      
      if (bindings._windowMetrics.isAvailable) { // API level >= 30
        jobject display = jenv.env().CallObjectMethod(activity, bindings._activity.getDisplay); // display = activity.getDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.readCurrentDisplayMode(jenv, display, mode);
      }
      else { // API level < 30
        jobject windowManager = jenv.env().CallObjectMethod(activity, bindings._activity.getWindowManager); // windowManager = app.getWindowManager()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "window manager", windowManager);
        jobject display = jenv.env().CallObjectMethod(windowManager, bindings._windowManager.getDefaultDisplay); // display = windowManager.getDefaultDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.readCurrentDisplayMode(jenv, display, mode);
      }
    }
    catch (...) { // on error, fill default mode
      mode.width = this->_attributes.screenArea.width * this->_density.scale;
      mode.height = this->_attributes.screenArea.height * this->_density.scale;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode) {
    try {
      AndroidJavaSession jenv;
      AndroidBindings& bindings = AndroidBindings::globalInstance(); // already bound in DisplayMonitor constructor
      jobject activity = AndroidApp::instance().state().activity->clazz;
      
      if (bindings._windowMetrics.isAvailable) { // API level >= 30
        jobject display = jenv.env().CallObjectMethod(activity, bindings._activity.getDisplay); // display = activity.getDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.setDisplayMode(jenv, activity, display, mode, this->_density);
      }
      else { // API level < 30
        jobject windowManager = jenv.env().CallObjectMethod(activity, bindings._activity.getWindowManager); // windowManager = app.getWindowManager()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "window manager", windowManager);
        jobject display = jenv.env().CallObjectMethod(windowManager, bindings._windowManager.getDefaultDisplay); // display = windowManager.getDefaultDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.setDisplayMode(jenv, activity, display, mode, this->_density);
      }
      return true;
    }
    catch (...) { return false; }
  }
  
  bool DisplayMonitor::setDefaultDisplayMode() {
    try {
      AndroidJavaSession jenv;
      AndroidBindings& bindings = AndroidBindings::globalInstance(); // already bound in DisplayMonitor constructor
      jobject activity = AndroidApp::instance().state().activity->clazz;
      
      if (bindings._windowMetrics.isAvailable) { // API level >= 30
        jobject display = jenv.env().CallObjectMethod(activity, bindings._activity.getDisplay); // display = activity.getDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.restoreDisplayMode(jenv, activity, display, this->_density);
      }
      else { // API level < 30
        jobject windowManager = jenv.env().CallObjectMethod(activity, bindings._activity.getWindowManager); // windowManager = app.getWindowManager()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "window manager", windowManager);
        jobject display = jenv.env().CallObjectMethod(windowManager, bindings._windowManager.getDefaultDisplay); // display = windowManager.getDefaultDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.restoreDisplayMode(jenv, activity, display, this->_density);
      }
      return true;
    }
    catch (...) { return false; }
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    std::vector<DisplayMode> modes;
    try {
      AndroidJavaSession jenv;
      AndroidBindings& bindings = AndroidBindings::globalInstance(); // already bound in DisplayMonitor constructor
      jobject activity = AndroidApp::instance().state().activity->clazz;
      
      if (bindings._windowMetrics.isAvailable) { // API level >= 30
        jobject display = jenv.env().CallObjectMethod(activity, bindings._activity.getDisplay); // display = activity.getDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.readDisplayModes(jenv, display, modes);
      }
      else { // API level < 30
        jobject windowManager = jenv.env().CallObjectMethod(activity, bindings._activity.getWindowManager); // windowManager = app.getWindowManager()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "window manager", windowManager);
        jobject display = jenv.env().CallObjectMethod(windowManager, bindings._windowManager.getDefaultDisplay); // display = windowManager.getDefaultDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.readDisplayModes(jenv, display, modes);
      }
    }
    catch (...) { // on error, fill default mode
      modes.clear();
      modes.emplace_back(getDisplayMode());
    }
    return modes;
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    return isEnabled; // always enabled
  }

  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle) const noexcept {
    outDpiX = this->_density.dpiX;
    outDpiY = this->_density.dpiY;
  }
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle) const noexcept {
    outScaleX = outScaleY = this->_density.scale;
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle, 
                                                            bool, uint32_t, uint32_t) const noexcept {
    try {
      DisplayArea windowArea;
      AndroidJavaSession jenv;
      AndroidBindings& bindings = AndroidBindings::globalInstance(); // already bound in DisplayMonitor constructor
      jobject activity = AndroidApp::instance().state().activity->clazz;
      
      jobject windowManager = jenv.env().CallObjectMethod(activity, bindings._activity.getWindowManager); // windowManager = app.getWindowManager()
      _P_THROW_ON_ACCESS_FAILURE(jenv, "window manager", windowManager);

      // WindowMetrics (API level >= 30)
      if (bindings._windowMetrics.isAvailable) {
        bindings.readWindowArea_wm(jenv, windowManager, windowArea);
      }
      // DisplayMetrics (API level < 30)
      else { 
        jobject display = jenv.env().CallObjectMethod(windowManager, bindings._windowManager.getDefaultDisplay); // display = windowManager.getDefaultDisplay()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "display", display);
        bindings.readWindowArea_dm(jenv, display, windowArea);
      }
      return windowArea;
    }
    catch (...) { return clientArea; }
  }
#endif
