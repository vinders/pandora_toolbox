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
# include <cstdlib>
# include <climits>
# include <cmath>
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;
  
  /*//EXAMPLE
#define JNI_ASSERT(jni, cond) { \
  if (!(cond)) {\
    std::stringstream ss; \
    ss << __FILE__ << ":" << __LINE__; \
    throw std::runtime_error(ss.str()); \
  } \
  if (jni->ExceptionCheck()) { \
    std::stringstream ss; \
    ss << __FILE__ << ":" << __LINE__; \
    throw std::runtime_error("Exception: " + ss.str()); \
  } \
}
void print_dpi(android_app* app) {
  JNIEnv* jni;
  app->activity->vm->AttachCurrentThread(&jni, NULL);

  jclass activityClass = jni->FindClass("android/app/NativeActivity");
  JNI_ASSERT(jni, activityClass);

  jmethodID getWindowManager = jni->GetMethodID
                                    ( activityClass
                                    , "getWindowManager"
                                    , "()Landroid/view/WindowManager;"); 
  JNI_ASSERT(jni, getWindowManager);

  jobject wm = jni->CallObjectMethod(app->activity->clazz, getWindowManager);
  JNI_ASSERT(jni, wm);

  jclass windowManagerClass = jni->FindClass("android/view/WindowManager");
  JNI_ASSERT(jni, windowManagerClass);

  jmethodID getDefaultDisplay = jni->GetMethodID( windowManagerClass
                                                , "getDefaultDisplay"
                                                , "()Landroid/view/Display;");
  JNI_ASSERT(jni, getDefaultDisplay);

  jobject display = jni->CallObjectMethod(wm, getDefaultDisplay);
  JNI_ASSERT(jni, display);

  jclass displayClass = jni->FindClass("android/view/Display");
  JNI_ASSERT(jni, displayClass);

  // Check if everything is OK so far, it is, the values it prints
  // are sensible.
  { 
    jmethodID getWidth = jni->GetMethodID(displayClass, "getWidth", "()I");
    JNI_ASSERT(jni, getWidth);

    jmethodID getHeight = jni->GetMethodID(displayClass, "getHeight", "()I");
    JNI_ASSERT(jni, getHeight);

    int width = jni->CallIntMethod(display, getWidth);
    JNI_ASSERT(jni, true);
    log("Width: ", width); // Width: 320

    int height = jni->CallIntMethod(display, getHeight);
    JNI_ASSERT(jni, true);
    log("Height: ", height); // Height: 480
  }

  jclass displayMetricsClass = jni->FindClass("android/util/DisplayMetrics");
  JNI_ASSERT(jni, displayMetricsClass);

  jmethodID displayMetricsConstructor = jni->GetMethodID( displayMetricsClass
                                                        , "<init>", "()V");
  JNI_ASSERT(jni, displayMetricsConstructor);

  jobject displayMetrics = jni->NewObject( displayMetricsClass
                                         , displayMetricsConstructor);
  JNI_ASSERT(jni, displayMetrics);

  jmethodID getMetrics = jni->GetMethodID( displayClass
                                         , "getMetrics"
                                         , "(Landroid/util/DisplayMetrics;)V");
  JNI_ASSERT(jni, getMetrics);

  jni->CallVoidMethod(display, getMetrics, displayMetrics);
  JNI_ASSERT(jni, true);

  {
    jfieldID xdpi_id = jni->GetFieldID(displayMetricsClass, "xdpi", "F");
    JNI_ASSERT(jni, xdpi_id);

    float xdpi = jni->GetFloatField(displayMetrics, xdpi_id);
    JNI_ASSERT(jni, true);

    log("XDPI: ", xdpi); // XDPI: 0
  }

  {
    jfieldID height_id = jni->GetFieldID( displayMetricsClass
                                        , "heightPixels", "I");
    JNI_ASSERT(jni, height_id);

    int height = jni->GetIntField(displayMetrics, height_id);
    JNI_ASSERT(jni, true);

    log("Height: ", height); // Height: 0
  }
  // TODO: Delete objects here.
  app->activity->vm->DetachCurrentThread();
}
*/


// -- monitor handle & description/attributes -- -------------------------------

  static bool _readDisplayMonitorAttributes(DisplayMonitor::Handle monitorHandle, DisplayMonitor::Attributes& outAttr) noexcept { 
    ANativeWindow nativeWindow = ANativeWindow_fromSurface(env, surface);
    int width =  ANativeWindow_getWidth(renderEngine->nativeWindow);
    int height = ANativeWindow_getHeight(renderEngine->nativeWindow);
  
  /*
    JNIEnv* jni;
    app->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass activityClass = jni->FindClass("android/app/NativeActivity");
    JNI_ASSERT(jni, activityClass);

    jmethodID getWindowManager = jni->GetMethodID (activityClass, "getWindowManager" , "()Landroid/view/WindowManager;"); 
    JNI_ASSERT(jni, getWindowManager);

    jobject wm = jni->CallObjectMethod(app->activity->clazz, getWindowManager);
    JNI_ASSERT(jni, wm);

    jclass windowManagerClass = jni->FindClass("android/view/WindowManager");
    JNI_ASSERT(jni, windowManagerClass);

    jmethodID getDefaultDisplay = jni->GetMethodID(windowManagerClass, "getDefaultDisplay" , "()Landroid/view/Display;");
    JNI_ASSERT(jni, getDefaultDisplay);

    jobject display = jni->CallObjectMethod(wm, getDefaultDisplay);
    JNI_ASSERT(jni, display);

    jclass displayClass = jni->FindClass("android/view/Display");
    JNI_ASSERT(jni, displayClass);

    jclass displayMetricsClass = jni->FindClass("android/util/DisplayMetrics");
    JNI_ASSERT(jni, displayMetricsClass);

    jmethodID displayMetricsConstructor = jni->GetMethodID(displayMetricsClass, "<init>", "()V");
    JNI_ASSERT(jni, displayMetricsConstructor);

    jobject displayMetrics = jni->NewObject(displayMetricsClass, displayMetricsConstructor);
    JNI_ASSERT(jni, displayMetrics);

    jmethodID getMetrics = jni->GetMethodID(displayClass, "getMetrics", "(Landroid/util/DisplayMetrics;)V");
    JNI_ASSERT(jni, getMetrics);

    jni->CallVoidMethod(display, getMetrics, displayMetrics);
    JNI_ASSERT(jni, true);

    jfieldID xdpi_id = jni->GetFieldID(displayMetricsClass, "xdpi", "F");
    JNI_ASSERT(jni, xdpi_id);

    float xdpi = jni->GetFloatField( displayMetrics, xdpi_id);
    JNI_ASSERT(jni, true);
  */


  
    return true; 
  }
  static inline void _readPrimaryDisplayMonitorInfo(DisplayMonitor::Handle& outHandle, DisplayMonitor::Attributes& outAttr) {}
  static inline DisplayMonitor::Handle _getDisplayMonitorById(DisplayMonitor::DeviceId id, DisplayMonitor::Attributes* outAttr) noexcept { return 0; }
  static inline bool _listDisplayMonitors(std::vector<DisplayMonitor::Handle>& out) { return false; }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() noexcept {
    _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(Handle monitorHandle, bool usePrimaryAsDefault)
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
  DisplayMonitor::DisplayMonitor(uint32_t index, bool usePrimaryAsDefault) {
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

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() noexcept {
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
      if (monitors[0].attributes().screenArea.width == 0) // no display monitor
        monitors.clear();
    }
    return monitors;
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
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) noexcept {
    if (_setMonitorDisplayMode(this->_attributes.id, mode)) {
      if (refreshAttributes) {
        if (this->_attributes.isPrimary) {
          _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
        }
        else if (!_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
          this->_attributes.screenArea.width = mode.width;
          this->_attributes.screenArea.height = mode.height;
        }
      }
      return true;
    }
    return false;
  }
  
  static inline bool _setDefaultMonitorDisplayMode(const DisplayMonitor::DeviceId& id) noexcept { return false; }
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) noexcept {
    if (_setDefaultMonitorDisplayMode(this->_attributes.id)) {
      if (refreshAttributes) {
        if (this->_attributes.isPrimary) {
          _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
        }
        else
          _readDisplayMonitorAttributes(this->_handle, this->_attributes);
      }
      return true;
    }
    return false;
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const noexcept {
    std::vector<DisplayMode> modes;
    //...
    
    if (modes.empty())
      modes.emplace_back(getDisplayMode());
    return modes;
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
    return true;
  }

  void DisplayMonitor::getMonitorDpi(uint32_t outDpiX, uint32_t outDpiY, DisplayMonitor::WindowHandle windowHandle) const noexcept {

  }
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle windowHandle) const noexcept {
    //...
  }


// -- metrics -- ---------------------------------------------------------------

  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, bool hasMenu, uint32_t, uint32_t) const noexcept {
    return DisplayArea{ 0, 0, 0, 0 };
  }
#endif
