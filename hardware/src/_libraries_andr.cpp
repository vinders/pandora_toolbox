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
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cstdint>
# include <cstring>
# include <string>
# include <stdexcept>
# include <system/api/android_app.h>
# include <android/native_window.h>
# include "hardware/_private/_libraries_andr.h"

  using namespace pandora::hardware;
  using pandora::system::AndroidApp;
  
  
// -- native Android session (for one thread) -- -------------------------------

  // open java session
  AndroidJavaSession::AndroidJavaSession(int minJavaVersion) {
    auto& app = AndroidApp::instance().state(); // throws
    
    if (app.activity->vm->GetEnv((void**) &_jenv, minJavaVersion) == JNI_EDETACHED || _jenv == nullptr) {
      app.activity->vm->AttachCurrentThread(&_jenv, nullptr);
      if (_jenv == nullptr)
        throw std::runtime_error(__P_EXCEPTION_PREFIX "failed to obtain java access for current thread...");
      
      _isAttached = true;
    }
  }
  
  // close java session
  AndroidJavaSession::~AndroidJavaSession() noexcept {
    if (AndroidApp::instance().isInitialized() && _isAttached) {
      try {
        _jenv = nullptr;
        AndroidApp::instance().state().activity->vm->DetachCurrentThread();
      }
      catch (...) { fprintf(stderr, __P_EXCEPTION_PREFIX "failed to detach current thread..."); }
    }
  }

  AndroidJavaSession::AndroidJavaSession(AndroidJavaSession&& rhs) : _jenv(rhs._jenv), _isAttached(rhs._isAttached) {
    rhs._isAttached = false;
    rhs._jenv = nullptr;
  }
  AndroidJavaSession& AndroidJavaSession::operator=(AndroidJavaSession&& rhs) {
    this->_jenv = rhs._jenv;
    this->_isAttached = rhs._isAttached;
    rhs._isAttached = false;
    rhs._jenv = nullptr;
    return *this;
  }

  
// -- JNI bindings -- ----------------------------------------------------------
// docs: https://developer.android.com/reference/packages

  AndroidBindings::~AndroidBindings() noexcept {
    try {
      AndroidJavaSession jenv;
      if (this->_activity.isCached)       jenv.env().DeleteGlobalRef(this->_activity.definition);
      if (this->_resources.isCached)      jenv.env().DeleteGlobalRef(this->_resources.definition);
      if (this->_config.isCached)         jenv.env().DeleteGlobalRef(this->_config.definition);
      if (this->_windowManager.isCached)  jenv.env().DeleteGlobalRef(this->_windowManager.definition);
      if (this->_display.isCached)        jenv.env().DeleteGlobalRef(this->_display.definition);
      if (this->_displayMode.isCached)    jenv.env().DeleteGlobalRef(this->_displayMode.definition);
      if (this->_rect.isCached)           jenv.env().DeleteGlobalRef(this->_rect.definition);
      if (this->_windowMetrics.isCached)  jenv.env().DeleteGlobalRef(this->_windowMetrics.definition);
      if (this->_displayMetrics.isCached) jenv.env().DeleteGlobalRef(this->_displayMetrics.definition);
      jenv.env().ExceptionClear();
    }
    catch (...) {}
  }
  

  // -- java/c++ data converters --

  // convert java string to c++ string
  bool AndroidBindings::jstringToString(AndroidJavaSession& jenv, jstring& value, std::string& outData) {
    const jsize length = jenv.env().GetStringLength(value);
    if (jenv.env().ExceptionCheck())
      return false;
    
    if (length > 0) {
      char buffer[1024];
      memset((void*)buffer, 0, sizeof(buffer));
      jenv.env().GetStringUTFRegion(value, 0, (length < 1024) ? length : 1023, buffer);
      if (jenv.env().ExceptionCheck())
        return false;
      
      outData = buffer;
    }
    else
      outData = "";
    return true;
  }
  
  // convert java Display.Mode to c++ DisplayMode
  void AndroidBindings::jobjectToDisplayMode(AndroidJavaSession& jenv, jobject& displayMode, DisplayMode& outData) {
    int modePxWidth = jenv.env().CallIntMethod(displayMode, this->_displayMode.getPhysicalWidth); // modePxWidth = displayMode.getPhysicalWidth()
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getPhysicalWidth");
    int modePxHeight = jenv.env().CallIntMethod(displayMode, this->_displayMode.getPhysicalHeight); // modePxHeight = displayMode.getPhysicalHeight()
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getPhysicalHeight");
    double modeRate = static_cast<double>(jenv.env().CallFloatMethod(displayMode, this->_displayMode.getRefreshRate)); // modeRate = displayMode.getRefreshRate()
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getRefreshRate");
    
    outData.width = (modePxWidth > 0) ? static_cast<uint32_t>(modePxWidth) : 0;
    outData.height = (modePxHeight > 0) ? static_cast<uint32_t>(modePxHeight) : 0;
    outData.bitDepth = 32;
    outData.refreshRate = (modeRate > 0.0) ? static_cast<uint32_t>(modeRate*1000.0 + 0.500001) : 0;
  }
  
  
  // -- bindings --

  // android/app/NativeActivity
  static void __bindNativeActivityClass(AndroidJavaSession& jenv, bool isWindowMetricsAvailable, AndroidBindings::jNativeActivity& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/app/NativeActivity");
    _P_THROW_ON_BINDING_FAILURE(jenv, "activity", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "activity", out.definition);

    try {
      if (isWindowMetricsAvailable) {
        out.getDisplay = jenv.env().GetMethodID(out.definition, "getDisplay", "()Landroid/view/Display;");
        _P_THROW_ON_BINDING_FAILURE(jenv, "activity method", out.getDisplay);
      }
      out.getResources = jenv.env().GetMethodID(out.definition, "getResources", "()Landroid/content/res/Resources;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "activity method", out.getResources);
      out.getWindowManager = jenv.env().GetMethodID(out.definition, "getWindowManager", "()Landroid/view/WindowManager;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "activity method", out.getWindowManager);
      out.getWindow = jenv.env().GetMethodID(out.definition, "getWindow", "()Landroid/view/Window;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "activity method", out.getWindow);
      
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  //android/content/res/Resources
  static void __bindResourcesClass(AndroidJavaSession& jenv, AndroidBindings::jResources& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/content/res/Resources");
    _P_THROW_ON_BINDING_FAILURE(jenv, "resources", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "resources", out.definition);
    
    try {
      out.getConfiguration = jenv.env().GetMethodID(out.definition, "getConfiguration", "()Landroid/content/res/Configuration;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "resources method", out.getConfiguration);
      
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  //android/content/res/Configuration
  static void __bindConfigurationClass(AndroidJavaSession& jenv, AndroidBindings::jConfiguration& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/content/res/Configuration");
    _P_THROW_ON_BINDING_FAILURE(jenv, "config", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "config", out.definition);
    
    try {
      out.densityDpi = jenv.env().GetFieldID(out.definition, "densityDpi", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "config field", out.densityDpi);
      out.screenWidthDp = jenv.env().GetFieldID(out.definition, "screenWidthDp", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "config field", out.screenWidthDp);
      out.screenHeightDp = jenv.env().GetFieldID(out.definition, "screenHeightDp", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "config field", out.screenHeightDp);
      out.screenWidthDpValue = out.screenHeightDpValue = 0;
    
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/view/WindowManager
  static void __bindWindowManagerClass(AndroidJavaSession& jenv, bool isWindowMetricsAvailable, AndroidBindings::jWindowManager& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/view/WindowManager");
    _P_THROW_ON_BINDING_FAILURE(jenv, "window", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "window", out.definition);
    
    try {
      if (isWindowMetricsAvailable) {
        out.getCurrentWindowMetrics = jenv.env().GetMethodID(out.definition, "getCurrentWindowMetrics", "()Landroid/view/WindowMetrics;");
        _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.getCurrentWindowMetrics);
        out.getMaximumWindowMetrics = jenv.env().GetMethodID(out.definition, "getMaximumWindowMetrics", "()Landroid/view/WindowMetrics;");
        _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.getMaximumWindowMetrics);
      }
      else {
        out.getDefaultDisplay = jenv.env().GetMethodID(out.definition, "getDefaultDisplay", "()Landroid/view/Display;");
        _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.getDefaultDisplay);
      }
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/view/Display
  static void __bindDisplayClass(AndroidJavaSession& jenv, bool isDisplayMetricsAvailable, AndroidBindings::jDisplay& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/view/Display");
    _P_THROW_ON_BINDING_FAILURE(jenv, "display", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "display", out.definition);
    
    try {
      out.getDisplayId = jenv.env().GetMethodID(out.definition, "getDisplayId", "()I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display method", out.getDisplayId);
      out.getName = jenv.env().GetMethodID(out.definition, "getName", "()Ljava/lang/String;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display method", out.getName);
      out.getMode = jenv.env().GetMethodID(out.definition, "getMode", "()Landroid/view/Display$Mode;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display method", out.getMode);
      out.getSupportedModes = jenv.env().GetMethodID(out.definition, "getSupportedModes", "()[Landroid/view/Display$Mode;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display method", out.getSupportedModes);
      if (isDisplayMetricsAvailable) {
        out.getRectSize = jenv.env().GetMethodID(out.definition, "getRectSize", "(Landroid/graphics/Rect;)V");
        _P_THROW_ON_BINDING_FAILURE(jenv, "display method", out.getRectSize);
        out.getMetrics = jenv.env().GetMethodID(out.definition, "getMetrics", "(Landroid/util/DisplayMetrics;)V");
        _P_THROW_ON_BINDING_FAILURE(jenv, "display method", out.getMetrics);
      }
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/view/Display.Mode
  static void __bindDisplayModeClass(AndroidJavaSession& jenv, AndroidBindings::jDisplayMode& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/view/Display$Mode");
    _P_THROW_ON_BINDING_FAILURE(jenv, "display mode", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "display mode", out.definition);
    
    try {
      out.getModeId = jenv.env().GetMethodID(out.definition, "getModeId", "()I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display mode method", out.getModeId);
      out.getPhysicalHeight = jenv.env().GetMethodID(out.definition, "getPhysicalHeight", "()I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display mode method", out.getPhysicalHeight);
      out.getPhysicalWidth = jenv.env().GetMethodID(out.definition, "getPhysicalWidth", "()I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display mode method", out.getPhysicalWidth);
      out.getRefreshRate = jenv.env().GetMethodID(out.definition, "getRefreshRate", "()I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "display mode method", out.getRefreshRate);
      
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/graphics/Rect
  static void __bindRectClass(AndroidJavaSession& jenv, AndroidBindings::jRect& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/graphics/Rect");
    _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle", out.definition);
    
    try {
      out.Rect = jenv.env().GetMethodID(out.definition, "<init>", "()V");
      _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle method", out.Rect);
      out.left = jenv.env().GetFieldID(out.definition, "left", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle field", out.left);
      out.right = jenv.env().GetFieldID(out.definition, "right", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle field", out.right);
      out.top = jenv.env().GetFieldID(out.definition, "top", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle field", out.top);
      out.bottom = jenv.env().GetFieldID(out.definition, "bottom", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "rectangle field", out.bottom);
      
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/view/Window
  static void __bindWindowClass(AndroidJavaSession& jenv, AndroidBindings::jWindow& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/view/Window");
    _P_THROW_ON_BINDING_FAILURE(jenv, "window", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "window", out.definition);
    
    try {
      out.getAttributes = jenv.env().GetMethodID(out.definition, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
      _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.getAttributes);
      out.setAttributes = jenv.env().GetMethodID(out.definition, "setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V");
      _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.setAttributes);
      out.addFlags = jenv.env().GetMethodID(out.definition, "addFlags", "(I)V");
      _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.addFlags);
      out.clearFlags = jenv.env().GetMethodID(out.definition, "clearFlags", "(I)V");
      _P_THROW_ON_BINDING_FAILURE(jenv, "window method", out.clearFlags);
      
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/view/WindowManager.LayoutParams
  static void __bindWindowLayoutParamsClass(AndroidJavaSession& jenv, AndroidBindings::jWindowLayoutParams& out) {
    if (out.isCached)
      return;
    jclass localClass = jenv.env().FindClass("android/view/WindowManager$LayoutParams");
    _P_THROW_ON_BINDING_FAILURE(jenv, "layout params", localClass);
    out.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "layout params", out.definition);
    
    try {
      out.preferredDisplayModeId = jenv.env().GetFieldID(out.definition, "preferredDisplayModeId", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "layout params field", out.preferredDisplayModeId);
      
      out.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  // android/view/WindowMetrics - android/util/DisplayMetrics
  static void __bindMetricsClass(AndroidJavaSession& jenv, AndroidBindings& out) {
    if (out._windowMetrics.isCached || out._displayMetrics.isCached)
      return;
    out._windowMetrics.isAvailable = false;
    out._displayMetrics.isAvailable = false;
    
    // WindowMetrics if android API level >= 30
    jclass localWmClass = jenv.env().FindClass("android/view/WindowMetrics");
    if (!jenv.env().ExceptionCheck() && out._windowMetrics.definition) {
      out._windowMetrics.getBounds = jenv.env().GetMethodID(out._windowMetrics.definition, "getBounds", "()Landroid/graphics/Rect;");
      if (!jenv.env().ExceptionCheck() && out._windowMetrics.getBounds) {
        out._windowMetrics.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localWmClass));
        _P_THROW_ON_BINDING_FAILURE(jenv, "metrics", out._windowMetrics.definition);
        
        out._windowMetrics.isAvailable = true;
        out._windowMetrics.isCached = true;
        return;
      }
    }
    jenv.env().ExceptionClear(); // if not supported, clear exceptions
    
    // DisplayMetrics if android API level < 30
    jclass localDmClass = jenv.env().FindClass("android/util/DisplayMetrics");
    _P_THROW_ON_BINDING_FAILURE(jenv, "metrics", localDmClass);
    out._displayMetrics.definition = reinterpret_cast<jclass>(jenv.env().NewGlobalRef(localDmClass));
    _P_THROW_ON_BINDING_FAILURE(jenv, "metrics", out._displayMetrics.definition);
    try {
      out._displayMetrics.DisplayMetrics = jenv.env().GetMethodID(out._displayMetrics.definition, "<init>", "()V");
      _P_THROW_ON_BINDING_FAILURE(jenv, "metrics method", out._displayMetrics.DisplayMetrics);
      out._displayMetrics.density = jenv.env().GetFieldID(out._displayMetrics.definition, "density", "F");
      _P_THROW_ON_BINDING_FAILURE(jenv, "metrics field", out._displayMetrics.density);
      out._displayMetrics.xdpi = jenv.env().GetFieldID(out._displayMetrics.definition, "xdpi", "F");
      _P_THROW_ON_BINDING_FAILURE(jenv, "metrics field", out._displayMetrics.xdpi);
      out._displayMetrics.ydpi = jenv.env().GetFieldID(out._displayMetrics.definition, "ydpi", "F");
      _P_THROW_ON_BINDING_FAILURE(jenv, "metrics field", out._displayMetrics.ydpi);
      out._displayMetrics.widthPixels = jenv.env().GetFieldID(out._displayMetrics.definition, "widthPixels", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "metrics field", out._displayMetrics.widthPixels);
      out._displayMetrics.heightPixels = jenv.env().GetFieldID(out._displayMetrics.definition, "heightPixels", "I");
      _P_THROW_ON_BINDING_FAILURE(jenv, "metrics field", out._displayMetrics.heightPixels);
      
      out._displayMetrics.isAvailable = true;
      out._displayMetrics.isCached = true;
    }
    catch (...) { jenv.env().DeleteGlobalRef(out._displayMetrics.definition); jenv.env().ExceptionClear(); throw; }
  }
  
  
// -- display utilities -- -----------------------------------------------------

  // must be called once (per instance) before using any of the methods in this category
  void AndroidBindings::bindDisplayClasses(AndroidJavaSession& jenv) {
    __bindMetricsClass(jenv, *this); // call first, to fill windowMetrics/displayMetrics.isAvailable (throws)
    
    __bindNativeActivityClass(jenv, this->_windowMetrics.isAvailable, this->_activity); // throws
    __bindResourcesClass(jenv, this->_resources); // throws
    __bindConfigurationClass(jenv, this->_config); // throws
    __bindWindowManagerClass(jenv, this->_windowMetrics.isAvailable, this->_windowManager); // throws
    __bindDisplayClass(jenv, this->_displayMetrics.isAvailable, this->_display); // throws
    __bindDisplayModeClass(jenv, this->_displayMode); // throws
    __bindRectClass(jenv, this->_rect); // throws
  }
  // must be called once (per instance) before using '_window' variable
  void AndroidBindings::bindWindowClass(AndroidJavaSession& jenv) {
    __bindWindowClass(jenv, this->_window);
  }
  
  // ---

  // read DPI/density of a display (also fills config.screenWidthDpValue/screenHeightDpValue) - WindowMetrics (API level >= 30)
  void AndroidBindings::readDisplayDensity_wm(AndroidJavaSession& jenv, jobject& activity, jobject& display, DisplayMonitor::Density& outDensity) {
    jobject resources = jenv.env().CallObjectMethod(activity, this->_activity.getResources); // resources = activity.getResources()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "resources", resources);
    jobject config = jenv.env().CallObjectMethod(resources, this->_resources.getConfiguration); // config = resources.getConfiguration()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "config", config);

    this->_config.screenWidthDpValue = jenv.env().GetIntField(config, this->_config.screenWidthDp);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "config.screenWidthDp");
    this->_config.screenHeightDpValue = jenv.env().GetIntField(config, this->_config.screenHeightDp);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "config.screenHeightDp");
    
    int densityDpi = jenv.env().GetIntField(config, this->_config.densityDpi);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "config.densityDpi");
    
    if (this->_config.screenWidthDpValue > 0) {
      jobject mode = jenv.env().CallObjectMethod(display, this->_display.getMode); // mode = display.getMode()
      _P_THROW_ON_ACCESS_FAILURE(jenv, "display.getMode", mode);
      int currentPxWidth = jenv.env().CallIntMethod(mode, this->_displayMode.getPhysicalWidth); 
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getPhysicalWidth");
      
      outDensity.scale = (currentPxWidth > 0) ? (double)currentPxWidth / (double)this->_config.screenWidthDpValue : 1.0;
    }
    else
      outDensity.scale = 1.0;
    
    if (densityDpi > 0) {
      outDensity.dpiX = outDensity.dpiY = static_cast<uint32_t>(densityDpi);
      if (outDensity.scale == 1.0)
        outDensity.scale = (double)outDensity.dpiX / 160.0;
    }
    else
      outDensity.dpiX = outDensity.dpiY = static_cast<uint32_t>(160.0*outDensity.scale + 0.5);
  }
  
  // read DPI/density of a display (also fills config.screenWidthDpValue/screenHeightDpValue) - DisplayMetrics (API level < 30)
  void AndroidBindings::readDisplayDensity_dm(AndroidJavaSession& jenv, jobject& display, DisplayMonitor::Density& outDensity) {
    jobject metrics = jenv.env().NewObject(this->_displayMetrics.definition, this->_displayMetrics.DisplayMetrics); // new
    _P_THROW_ON_ACCESS_FAILURE(jenv, "metrics.init", metrics);
    jenv.env().CallVoidMethod(display, this->_display.getMetrics, metrics); // display.getMetrics(metrics)
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "display.getMetrics");
    
    outDensity.scale = (double)jenv.env().GetFloatField(metrics, this->_displayMetrics.density);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.density");
    outDensity.dpiX = (uint32_t)jenv.env().GetFloatField(metrics, this->_displayMetrics.xdpi);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.xdpi");
    outDensity.dpiY = (uint32_t)jenv.env().GetFloatField(metrics, this->_displayMetrics.ydpi);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.ydpi");
    if (outDensity.scale <= 0.0)
      outDensity.scale = 1.0;
    if (outDensity.dpiX <= 0 || outDensity.dpiY <= 0)
      outDensity.dpiX = outDensity.dpiY = static_cast<uint32_t>(160.0*outDensity.scale + 0.5);
    
    if (outDensity.scale != 1.0) {
      this->_config.screenWidthDpValue = static_cast<double>(jenv.env().GetIntField(metrics, this->_displayMetrics.widthPixels)) / outDensity.scale;
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.widthPixels");
      this->_config.screenHeightDpValue = static_cast<double>(jenv.env().GetIntField(metrics, this->_displayMetrics.heightPixels)) / outDensity.scale;
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.heightPixels");
    }
  }
  
  // ---
  
  // read work area of a display - WindowMetrics (API level >= 30)
  void AndroidBindings::readDisplayWorkArea_wm(AndroidJavaSession& jenv, jobject& windowManager, DisplayArea& outWorkArea) {
    jobject workArea = jenv.env().CallObjectMethod(windowManager, this->_windowManager.getMaximumWindowMetrics); // workArea = windowManager.getMaximumWindowMetrics()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "window.getMaximumWindowMetrics", workArea);
    jobject workAreaBounds = jenv.env().CallObjectMethod(workArea, this->_windowMetrics.getBounds); //workAreaBounds = workArea.getBounds()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "metrics.getBounds", workAreaBounds);
    
    outWorkArea.x = (int32_t)jenv.env().GetIntField(workAreaBounds, this->_rect.left);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.left");
    outWorkArea.y = (int32_t)jenv.env().GetIntField(workAreaBounds, this->_rect.top);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.top");
    outWorkArea.width = static_cast<uint32_t>(jenv.env().GetIntField(workAreaBounds, this->_rect.right) - outWorkArea.x);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.right");
    outWorkArea.height = static_cast<uint32_t>(jenv.env().GetIntField(workAreaBounds, this->_rect.bottom) - outWorkArea.y);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.bottom");
  }
  
  // read work area of a display - DisplayMetrics (API level < 30)
  void AndroidBindings::readDisplayWorkArea_dm(AndroidJavaSession& jenv, jobject& display, DisplayArea& outWorkArea) {
    jobject workAreaRect = jenv.env().NewObject(this->_rect.definition, this->_rect.Rect); // new
    _P_THROW_ON_ACCESS_FAILURE(jenv, "rectangle.init", workAreaRect);
    jenv.env().CallVoidMethod(display, this->_display.getRectSize, workAreaRect); // display.getRectSize(workArea)
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "display.getRectSize");
    
    outWorkArea.x = (int32_t)jenv.env().GetIntField(workAreaRect, this->_rect.left);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.left");
    outWorkArea.y = (int32_t)jenv.env().GetIntField(workAreaRect, this->_rect.top);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.top");
    outWorkArea.width = static_cast<uint32_t>(jenv.env().GetIntField(workAreaRect, this->_rect.right) - outWorkArea.x);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.right");
    outWorkArea.height = static_cast<uint32_t>(jenv.env().GetIntField(workAreaRect, this->_rect.bottom) - outWorkArea.y);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.bottom");
  }
  
  // ---
  
  // get size/position of current window - WindowMetrics (API level >= 30)
  void AndroidBindings::readWindowArea_wm(AndroidJavaSession& jenv, jobject& windowManager, DisplayArea& outWindowArea) {
    jobject metrics = jenv.env().CallObjectMethod(windowManager, this->_windowManager.getCurrentWindowMetrics); // metrics = windowManager.getCurrentWindowMetrics()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "window.getCurrentWindowMetrics", metrics);
    jobject windowBounds = jenv.env().CallObjectMethod(metrics, this->_windowMetrics.getBounds); //windowBounds = metrics.getBounds()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "metrics.getBounds", windowBounds);
    
    outWindowArea.x = (int32_t)jenv.env().GetIntField(windowBounds, this->_rect.left);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.left");
    outWindowArea.y = (int32_t)jenv.env().GetIntField(windowBounds, this->_rect.top);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.top");
    outWindowArea.width = static_cast<uint32_t>(jenv.env().GetIntField(windowBounds, this->_rect.right) - outWindowArea.x);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.right");
    outWindowArea.height = static_cast<uint32_t>(jenv.env().GetIntField(windowBounds, this->_rect.bottom) - outWindowArea.y);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "metrics.bounds.bottom");
  }
  
  // ---
  
  // get current display mode of a display
  void AndroidBindings::readCurrentDisplayMode(AndroidJavaSession& jenv, jobject& display, DisplayMode& outMode) {
    jobject mode = jenv.env().CallObjectMethod(display, this->_display.getMode); // mode = display.getMode()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "display.getMode", mode);
    jobjectToDisplayMode(jenv, mode, outMode);
  }
  
  // get available display modes for a display
  void AndroidBindings::readDisplayModes(AndroidJavaSession& jenv, jobject& display, std::vector<DisplayMode>& outModes) {
    // read current mode
    jobject mode = jenv.env().CallObjectMethod(display, this->_display.getMode); // mode = display.getMode()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "display.getMode", mode);
    
    DisplayMode currentMode;
    jobjectToDisplayMode(jenv, mode, currentMode);
    outModes.emplace_back(std::move(currentMode));

    // read other modes
    jobjectArray supportedModes = static_cast<jobjectArray>(jenv.env().CallObjectMethod(display, this->_display.getSupportedModes));
    _P_THROW_ON_ACCESS_FAILURE(jenv, "display.getSupportedModes", supportedModes);
    int supportedModesCount = jenv.env().GetArrayLength(supportedModes);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getSupportedModes().count");
    
    for (int i = 0; i < supportedModesCount; ++i) {
      jobject mode = jenv.env().GetObjectArrayElement(supportedModes, i);
      _P_THROW_ON_ACCESS_FAILURE(jenv, "displayMode.getSupportedModes()[i]", mode);
      
      DisplayMode newMode;
      jobjectToDisplayMode(jenv, mode, newMode);
      
      bool isExisting = false;
      for (auto& it: outModes) {
        if (it.width == newMode.width && it.height == newMode.height && it.refreshRate == newMode.refreshRate) {
          isExisting = true;
          break;
        }
      }
      if (!isExisting)
        outModes.emplace_back(std::move(newMode));
    }
  }

  // ---
  
  static inline int __findDisplayModeId(AndroidJavaSession& jenv, AndroidBindings& bindings, jobject& display, const DisplayMode& desiredMode, uint32_t targetRate) {
    int closestModeId = -1;
    uint32_t closestRate = 0;
    
    // supportedModes = display.getSupportedModes()
    jobjectArray supportedModes = static_cast<jobjectArray>(jenv.env().CallObjectMethod(display, bindings._display.getSupportedModes));
    _P_THROW_ON_ACCESS_FAILURE(jenv, "display.getSupportedModes", supportedModes);
    int supportedModesCount = jenv.env().GetArrayLength(supportedModes);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getSupportedModes().count");
    
    for (int i = 0; i < supportedModesCount; ++i) {
      jobject mode = jenv.env().GetObjectArrayElement(supportedModes, i);
      _P_THROW_ON_ACCESS_FAILURE(jenv, "displayMode.getSupportedModes()[i]", mode);
      DisplayMode curMode;
      bindings.jobjectToDisplayMode(jenv, mode, curMode);
      
      if (curMode.width == desiredMode.width && curMode.height == desiredMode.height) {
        int modeId = jenv.env().CallIntMethod(mode, bindings._displayMode.getModeId); // modeId = displayMode.getModeId()
        _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getModeId");
        
        if (curMode.refreshRate == targetRate) {
          closestModeId = modeId;
          break;
        }
        else if (closestModeId == -1) { // same size, different rate, and no previous match
          closestModeId = modeId;
          closestRate = curMode.refreshRate;
        }
        else { // same size, different rate, previous match -> get closest possible rate
          uint32_t closestOffset = (closestRate >= targetRate) 
                                ? closestRate - targetRate 
                                : targetRate - closestRate;
          uint32_t currentOffset = (curMode.refreshRate >= targetRate) 
                                ? curMode.refreshRate - targetRate 
                                : targetRate - curMode.refreshRate;
          if (currentOffset < closestOffset) {
            closestModeId = modeId;
            closestRate = curMode.refreshRate;
          }
        }
      }
    }
    return closestModeId;
  }
  
  // change display mode of a display
  void AndroidBindings::setDisplayMode(AndroidJavaSession& jenv, jobject& activity, jobject& display,
                                       const DisplayMode& desiredMode, DisplayMonitor::Density& inOutDensity) {
    __bindWindowClass(jenv, this->_window);
    __bindWindowLayoutParamsClass(jenv, this->_windowLayoutParams);
    jobject currentMode = jenv.env().CallObjectMethod(display, this->_display.getMode); // mode = display.getMode()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "display.getMode", currentMode);
    
    // store original mode (if no change occurred yet)
    uint32_t previousWidth = 0;
    if (this->_displayMode.defaultMode == -1) {
      int currentModeId = jenv.env().CallIntMethod(currentMode, this->_displayMode.getModeId); // currentModeId = displayMode.getModeId()
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getModeId");
      int pxWidth = jenv.env().CallIntMethod(currentMode, this->_displayMode.getPhysicalWidth); // pxWidth = displayMode.getPhysicalWidth()
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getPhysicalWidth");

      previousWidth = (pxWidth > 0) ? static_cast<uint32_t>(pxWidth) : 0;
      this->_displayMode.defaultMode = currentModeId;
      this->_displayMode.defaultScale = inOutDensity.scale;
    }
    
    // find target mode ID (keep current refresh rate, if not defined)
    uint32_t targetRate = desiredMode.refreshRate;
    if (targetRate == 0) {
      double modeRate = static_cast<double>(jenv.env().CallFloatMethod(currentMode, this->_displayMode.getRefreshRate));
      _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "displayMode.getRefreshRate");
      if (modeRate > 0.0) 
        targetRate = static_cast<uint32_t>(modeRate*1000.0 + 0.500001);
    }
    int targetModeId = __findDisplayModeId(jenv, *this, display, desiredMode, targetRate);
    if (targetModeId == -1)
      throw std::invalid_argument(__P_EXCEPTION_PREFIX "display mode not available");
    
    // change display mode
    jobject window = jenv.env().CallObjectMethod(activity, this->_activity.getWindow); // window = activity.getWindow()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "activity.getWindow", window);
    jobject attributes = jenv.env().CallObjectMethod(window, this->_window.getAttributes); // attributes = window.getAttributes()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "window.getAttributes", attributes);
    
    jenv.env().SetIntField(attributes, this->_windowLayoutParams.preferredDisplayModeId, targetModeId);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "layout-params.preferredDisplayModeId");
    
    jenv.env().CallVoidMethod(window, this->_window.setAttributes, attributes); // window.setAttributes(attributes)
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "window.setAttributes");
    
    // update density
    double factor = (previousWidth > 0) ? ((double)desiredMode.width / (double)previousWidth) : 1.0;
    inOutDensity.scale *= factor;
    inOutDensity.dpiX = static_cast<uint32_t>((double)inOutDensity.dpiX * factor + 0.5);
    inOutDensity.dpiY = static_cast<uint32_t>((double)inOutDensity.dpiY * factor + 0.5);
  }
  
  // restore default display mode of a display
  void AndroidBindings::restoreDisplayMode(AndroidJavaSession& jenv, jobject& activity, jobject& display, 
                                           DisplayMonitor::Density& inOutDensity) {
    if (this->_displayMode.defaultMode == -1)
      return; // no previous display change
    __bindWindowClass(jenv, this->_window);
    __bindWindowLayoutParamsClass(jenv, this->_windowLayoutParams);
    
    // change display mode
    jobject window = jenv.env().CallObjectMethod(activity, this->_activity.getWindow); // window = activity.getWindow()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "activity.getWindow", window);
    jobject attributes = jenv.env().CallObjectMethod(window, this->_window.getAttributes); // attributes = window.getAttributes()
    _P_THROW_ON_ACCESS_FAILURE(jenv, "window.getAttributes", attributes);
    
    jenv.env().SetIntField(attributes, this->_windowLayoutParams.preferredDisplayModeId, this->_displayMode.defaultMode);
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "layout-params.preferredDisplayModeId");
    
    jenv.env().CallVoidMethod(window, this->_window.setAttributes, attributes); // window.setAttributes(attributes)
    _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "window.setAttributes");
    
    // update density
    double factor = this->_displayMode.defaultScale / inOutDensity.scale;
    inOutDensity.scale = this->_displayMode.defaultScale;
    inOutDensity.dpiX = static_cast<uint32_t>((double)inOutDensity.dpiX * factor + 0.5);
    inOutDensity.dpiY = static_cast<uint32_t>((double)inOutDensity.dpiY * factor + 0.5);
    this->_displayMode.defaultScale = 1.0;
    this->_displayMode.defaultMode = -1;
  }
#endif
