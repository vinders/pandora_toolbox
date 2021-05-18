/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cstdint>
# include <string>
# include <vector>
# include <jni.h>
# include "hardware/display_monitor.h"

  namespace pandora {
    namespace hardware {
      // native Android session (for one thread)
      // warning: must not be shared between threads
      class AndroidJavaSession final {
      public:
        // open java session
        // throws if AndroidApp not initialized or if session can't be open
        AndroidJavaSession(int minJavaVersion = JNI_VERSION_1_6);
        // close java session
        ~AndroidJavaSession() noexcept;
        
        AndroidJavaSession(const AndroidJavaSession&) = delete;
        AndroidJavaSession(AndroidJavaSession&& rhs);
        AndroidJavaSession& operator=(const AndroidJavaSession&) = delete;
        AndroidJavaSession& operator=(AndroidJavaSession&& rhs);
        
        // verify if the thread is managed by the JVM (true) or if it was created in C++ (false)
        inline bool isManagedThread() const noexcept { return !_isAttached; }
        
        // get java environment access
        inline JNIEnv& env() noexcept { return *_jenv; }
        inline const JNIEnv& env() const noexcept { return *_jenv; }
        
      private:
        JNIEnv* _jenv = nullptr;
        bool _isAttached = true;
      };


      // -- JNI bindings -- ----------------------------------------------------
      
      // android/JNI bindings (access to android API)
      // remarks: can be instanciated multiple times (for bindings only useful once)
      //          or used as global instance (for bindings that will serve multiple times)
      // warnings: will throw if some bindings fail
      class AndroidBindings {
      public:
        AndroidBindings() = default;
        AndroidBindings(const AndroidBindings&) = delete;
        AndroidBindings(AndroidBindings&&) = delete;
        AndroidBindings& operator=(const AndroidBindings&) = delete;
        AndroidBindings& operator=(AndroidBindings&&) = delete;
        ~AndroidBindings() noexcept;
        
        static AndroidBindings& globalInstance() noexcept {
          static AndroidBindings _globalInstance;
          return _globalInstance;
        }
        
        
        // -- java/c++ data converters --
        
        // convert java display ID to portable "handle"
        static inline uint64_t displayIdToHandle(int id) noexcept { return static_cast<uint64_t>(id + 1); } // ID 0 is valid, but handle 0 == empty
        // convert portable "handle" to java display ID
        static inline int handleToDisplayId(uint64_t handle) noexcept { return static_cast<int>(handle - 1); }
        
        // convert java string to c++ string
        // returns: success
        static bool jstringToString(AndroidJavaSession& jenv, jstring& value, std::string& outData);
        // convert java Display.Mode to c++ DisplayMode
        // throws on object access failure
        void jobjectToDisplayMode(AndroidJavaSession& jenv, jobject& displayMode, DisplayMode& outData);
        
        
        // -- display utilities --
        
        // must be called once (per instance) before using any of the methods in this category
        // remarks: can be called multiple times (will not re-bind what's already bound)
        // throws on failure
        void bindDisplayClasses(AndroidJavaSession& jenv);

        // read DPI/density of a display (also fills config.screenWidthDpValue/screenHeightDpValue)
        void readDisplayDensity_wm(AndroidJavaSession& jenv, jobject& activity, jobject& display, DisplayMonitor::Density& outDensity); // WindowMetrics (API level >= 30)
        void readDisplayDensity_dm(AndroidJavaSession& jenv, jobject& display, DisplayMonitor::Density& outDensity); // DisplayMetrics (API level < 30)
        
        // read work area of a display
        void readDisplayWorkArea_wm(AndroidJavaSession& jenv, jobject& windowManager, DisplayArea& outWorkArea); // WindowMetrics (API level >= 30)
        void readDisplayWorkArea_dm(AndroidJavaSession& jenv, jobject& display, DisplayArea& outWorkArea); // DisplayMetrics (API level < 30)
        
        // get size/position of current window
        void readWindowArea_wm(AndroidJavaSession& jenv, jobject& windowManager, DisplayArea& outWindowArea); // WindowMetrics (API level >= 30)
        void readWindowArea_dm(AndroidJavaSession& jenv, jobject& display, DisplayArea& outWindowArea) { // DisplayMetrics (API level < 30)
          return readDisplayWorkArea_dm(jenv, display, outWindowArea);
        }
        
        // get current display mode of a display
        void readCurrentDisplayMode(AndroidJavaSession& jenv, jobject& display, DisplayMode& outMode);
        // get available display modes for a display
        // remarks: the first item contains current display mode
        void readDisplayModes(AndroidJavaSession& jenv, jobject& display, std::vector<DisplayMode>& outModes);
        
        // change display mode of a display
        void setDisplayMode(AndroidJavaSession& jenv, jobject& activity, jobject& display, const DisplayMode& mode, DisplayMonitor::Density& inOutDensity);
        // restore default display mode of a display
        void restoreDisplayMode(AndroidJavaSession& jenv, jobject& activity, jobject& display, DisplayMonitor::Density& inOutDensity);
        
        
        // must be called once (per instance) before using '_window' variable
        // remarks: can be called multiple times (will not re-bind what's already bound)
        // throws on failure
        void bindWindowClass(AndroidJavaSession& jenv);
        

        // -- bindings --
        
        // android/app/NativeActivity
        struct jNativeActivity {
          jclass definition;
          jmethodID getDisplay; // only if WindowMetrics available
          jmethodID getResources;
          jmethodID getWindowManager;
          jmethodID getWindow;
          bool isCached = false;
        };
        //android/content/res/Resources
        struct jResources {
          jclass definition;
          jmethodID getConfiguration;
          bool isCached = false;
        };
        //android/content/res/Configuration
        struct jConfiguration {
          jclass definition;
          jfieldID densityDpi;
          jfieldID screenWidthDp;
          jfieldID screenHeightDp;
          int screenWidthDpValue = 0;
          int screenHeightDpValue = 0;
          bool isCached = false;
        };
        // android/view/WindowManager
        struct jWindowManager {
          jclass definition;
          jmethodID getDefaultDisplay; // only if DisplayMetrics available
          jmethodID getCurrentWindowMetrics; // only if WindowMetrics available
          jmethodID getMaximumWindowMetrics; // only if WindowMetrics available
          bool isCached = false;
        };
        // android/view/Display
#       define DEFAULT_DISPLAY 0
        struct jDisplay {
          jclass definition;
          jmethodID getDisplayId;
          jmethodID getName;
          jmethodID getMode;
          jmethodID getSupportedModes;
          jmethodID getRectSize; // only if DisplayMetrics available
          jmethodID getMetrics;  // only if DisplayMetrics available
          bool isCached = false;
        };
        // android/view/Display.Mode
        struct jDisplayMode {
          jclass definition;
          jmethodID getModeId;
          jmethodID getPhysicalHeight;
          jmethodID getPhysicalWidth;
          jmethodID getRefreshRate;
          double defaultScale = 1.0;
          int defaultMode = -1;
          bool isCached = false;
        };
        // android/graphics/Rect
        struct jRect {
          jclass definition;
          jmethodID Rect;
          jfieldID left;
          jfieldID right;
          jfieldID top;
          jfieldID bottom;
          bool isCached = false;
        };
        // android/view/Window
        struct jWindow {
          jclass definition;
          jmethodID getAttributes;
          jmethodID setAttributes;
          jmethodID addFlags;
          jmethodID clearFlags;
          bool isCached = false;
        };
        // android/view/WindowManager.LayoutParams
        struct jWindowLayoutParams {
          jclass definition;
          jfieldID preferredDisplayModeId;
          bool isCached = false;
        };
        // android/view/WindowMetrics (API level >= 30)
        struct jWindowMetrics {
          jclass definition;
          jmethodID getBounds;
          bool isAvailable = false;
          bool isCached = false;
        };
        // android/util/DisplayMetrics (API level < 30)
        struct jDisplayMetrics {
          jclass definition;
          jmethodID DisplayMetrics;
          jfieldID density;
          jfieldID xdpi;
          jfieldID ydpi;
          jfieldID widthPixels;
          jfieldID heightPixels;
          bool isAvailable = false;
          bool isCached = false;
        };
      
        jNativeActivity     _activity;
        jResources          _resources;
        jConfiguration      _config;
        jWindowManager      _windowManager;
        jDisplay            _display;
        jDisplayMode        _displayMode;
        jRect               _rect;
        jWindow             _window;
        jWindowLayoutParams _windowLayoutParams;
        jWindowMetrics      _windowMetrics;
        jDisplayMetrics     _displayMetrics;
      };
      
#     define __P_EXCEPTION_PREFIX "android: "
#     define __P_THROW_ON_JAVA_EXCEPTION(jni, title, op) \
          if (jni.env().ExceptionCheck()) { \
            jni.env().ExceptionClear(); \
            throw std::runtime_error(__P_EXCEPTION_PREFIX "native " title " " op " failure"); \
          }
#     define __P_THROW_ON_FAILURE(jni, title, op, value) \
          __P_THROW_ON_JAVA_EXCEPTION(jni, title, op) \
          if (!(value)) \
            throw std::runtime_error(__P_EXCEPTION_PREFIX "native " title " " op " failure");

      // exception management helpers
#     define _P_THROW_ON_BINDING_FAILURE(jni, title, value) __P_THROW_ON_FAILURE(jni, title, "binding", value)
#     define _P_THROW_ON_ACCESS_FAILURE(jni, title, value)  __P_THROW_ON_FAILURE(jni, title, "access", value)
#     define _P_THROW_ON_FIELD_ACCESS_FAILURE(jni, title)   __P_THROW_ON_JAVA_EXCEPTION(jni, title, "access")
    }
  }
#endif
