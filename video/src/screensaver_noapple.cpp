/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(__APPLE__)
# if defined(_WINDOWS)
#   include <system/api/windows_api.h>
# elif defined(__ANDROID__)
#   include <system/api/android_app.h>
#   include <hardware/_private/_libraries_andr.h>
# elif defined(_P_ENABLE_LINUX_WAYLAND)
#   include <hardware/_private/_libraries_wln.h>
# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
#   include <hardware/_private/_libraries_x11.h>
# endif

# include "video/screensaver.h"


# if defined _WINDOWS
    bool pandora::video::disableScreenSaver(pandora::video::WindowHandle) noexcept {
      SetLastError(0);
      return (SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED | ES_CONTINUOUS) != 0 || GetLastError() == 0);
    }
    bool pandora::video::restoreScreenSaver() noexcept {
      SetLastError(0);
      return (SetThreadExecutionState(ES_CONTINUOUS) != 0 || GetLastError() == 0);
    }

    void pandora::video::notifyScreenActivity(pandora::video::WindowHandle) noexcept {
      SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    }


# elif defined(__ANDROID__)
#   define FLAG_KEEP_SCREEN_ON 0x00000080

    bool pandora::video::disableScreenSaver(pandora::video::WindowHandle) noexcept {
      try {
        pandora::hardware::AndroidJavaSession jenv;
        pandora::hardware::AndroidBindings& bindings = pandora::hardware::AndroidBindings::globalInstance();
        bindings.bindDisplayClasses(jenv);
        bindings.bindWindowClass(jenv);
        
        jobject activity = pandora::system::AndroidApp::instance().state().activity->clazz;
        jobject window = jenv.env().CallObjectMethod(activity, bindings._activity.getWindow); // window = activity.getWindow()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "activity.getWindow", window);
        
        jenv.env().CallVoidMethod(window, bindings._window.addFlags, FLAG_KEEP_SCREEN_ON); // window.addFlags(FLAG_KEEP_SCREEN_ON)
        _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "window.addFlags");
        return true;
      }
      catch (...) {}
      return false;
    }
    bool pandora::video::restoreScreenSaver() noexcept {
      try {
        pandora::hardware::AndroidJavaSession jenv;
        pandora::hardware::AndroidBindings& bindings = pandora::hardware::AndroidBindings::globalInstance();
        bindings.bindDisplayClasses(jenv);
        bindings.bindWindowClass(jenv);
        
        jobject activity = pandora::system::AndroidApp::instance().state().activity->clazz;
        jobject window = jenv.env().CallObjectMethod(activity, bindings._activity.getWindow); // window = activity.getWindow()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "activity.getWindow", window);
        
        jenv.env().CallVoidMethod(window, bindings._window.clearFlags, FLAG_KEEP_SCREEN_ON); // window.clearFlags(FLAG_KEEP_SCREEN_ON)
        _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "window.clearFlags");
        return true;
      }
      catch (...) {}
      return false;
    }
    
    void pandora::video::notifyScreenActivity(pandora::video::WindowHandle) noexcept {
      try {
        pandora::hardware::AndroidJavaSession jenv;
        pandora::hardware::AndroidBindings& bindings = pandora::hardware::AndroidBindings::globalInstance();
        bindings.bindDisplayClasses(jenv);
        bindings.bindWindowClass(jenv);
        
        jobject activity = pandora::system::AndroidApp::instance().state().activity->clazz;
        jobject window = jenv.env().CallObjectMethod(activity, bindings._activity.getWindow); // window = activity.getWindow()
        _P_THROW_ON_ACCESS_FAILURE(jenv, "activity.getWindow", window);
        
        jenv.env().CallVoidMethod(window, bindings._window.addFlags, FLAG_KEEP_SCREEN_ON); // window.addFlags(FLAG_KEEP_SCREEN_ON)
        _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "window.addFlags");
        jenv.env().CallVoidMethod(window, bindings._window.clearFlags, FLAG_KEEP_SCREEN_ON); // window.clearFlags(FLAG_KEEP_SCREEN_ON)
        _P_THROW_ON_FIELD_ACCESS_FAILURE(jenv, "window.clearFlags");
      }
      catch (...) {}
    }


# elif defined(_P_ENABLE_LINUX_WAYLAND)
    static struct zwp_idle_inhibitor_v1* idleInhibitor = nullptr;

    bool pandora::video::disableScreenSaver(pandora::video::WindowHandle surface) noexcept {
      try {
        if (idleInhibitor != nullptr)
          return true;
        pandora::hardware::LibrariesWayland& libs = pandora::hardware::LibrariesWayland::instance();
        if (libs.wp.idleInhibitManager == nullptr)
          return false;
        
        idleInhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(libs.wp.idleInhibitManager, (wl_surface*)surface);
        return (idleInhibitor != nullptr);
      }
      catch (...) {}
      return false;
    }
    bool pandora::video::restoreScreenSaver() noexcept {
      try {
        if (idleInhibitor != nullptr) {
          zwp_idle_inhibitor_v1_destroy(idleInhibitor);
          idleInhibitor = nullptr;
        }
        return true;
      }
      catch (...) {}
      return false;
    }
    
    void pandora::video::notifyScreenActivity(pandora::video::WindowHandle surface) noexcept {
      if (disableScreenSaver(surface))
        restoreScreenSaver();
    }


# elif (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
    bool pandora::video::disableScreenSaver(pandora::video::WindowHandle) noexcept {
      try {
        pandora::hardware::LibrariesX11& libs = pandora::hardware::LibrariesX11::instance();
        if (libs.xss.isAvailable) {
          libs.xss.ScreenSaverSuspend_(libs.displayServer, True);
          return true;
        }
      }
      catch (...) {}
      return false;
    }
    bool pandora::video::restoreScreenSaver() noexcept {
      try {
        pandora::hardware::LibrariesX11& libs = pandora::hardware::LibrariesX11::instance();
        if (libs.xss.isAvailable) {
          libs.xss.ScreenSaverSuspend_(libs.displayServer, False);
          return true;
        }
      }
      catch (...) {}
      return false;
    }
    
    void pandora::video::notifyScreenActivity(pandora::video::WindowHandle) noexcept {
      try {
        pandora::hardware::LibrariesX11& libs = pandora::hardware::LibrariesX11::instance();
        if (libs.xlib.ResetScreenSaver_)
          libs.xlib.ResetScreenSaver_(libs.displayServer);
      }
      catch (...) {}
    }
# endif

#endif
