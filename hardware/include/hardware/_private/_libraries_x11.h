/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstddef>
# include <cstdint>
# include <unistd.h>
# include <X11/Xlib.h>
# include <X11/Xresource.h>
# include <X11/extensions/Xrandr.h> // display mode
# include <X11/extensions/Xinerama.h> // legacy monitor indices

# define __P_X11_BASE_DPI 96.0f

  namespace pandora {
    namespace hardware {
      using LibHandle = void*;
      
      // randr functions
      typedef XRRCrtcInfo*   (* __x11_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);
      typedef void           (* __x11_XRRFreeCrtcInfo)(XRRCrtcInfo*);
      typedef XRROutputInfo* (* __x11_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
      typedef RROutput       (* __x11_XRRGetOutputPrimary)(Display*,Window);
      typedef void           (* __x11_XRRFreeOutputInfo)(XRROutputInfo*);
      typedef XRRScreenResources* (* __x11_XRRGetScreenResourcesCurrent)(Display*,Window);
      typedef void           (* __x11_XRRFreeScreenResources)(XRRScreenResources*);
      typedef Bool           (* __x11_XRRQueryExtension)(Display*,int*,int*);
      typedef Status         (* __x11_XRRQueryVersion)(Display*,int*,int*);
      typedef void           (* __x11_XRRSelectInput)(Display*,Window,int);
      typedef Status         (* __x11_XRRSetCrtcConfig)(Display*,XRRScreenResources*,RRCrtc,Time,int,int,RRMode,Rotation,RROutput*,int);
      typedef int            (* __x11_XRRUpdateConfiguration)(XEvent*);

      // xinerama functions
      typedef Bool (* __x11_XineramaIsActive)(Display*);
      typedef Bool (* __x11_XineramaQueryExtension)(Display*,int*,int*);
      typedef XineramaScreenInfo* (* __x11_XineramaQueryScreens)(Display*,int*);

      // xf86vm functions
      typedef Bool (* __x11_XF86VidModeQueryExtension)(Display*,int*,int*);
      
      
      // library loader - X11
      struct LibrariesX11 final {
        LibrariesX11() = default;
        ~LibrariesX11() noexcept { shutdown(); }
        
        Display* displayServer = nullptr;
        int      screenIndex   = 0;
        Window   rootWindow;
        float dpiX = __P_HARDWARE_X11_BASE_DPI;
        float dpiY = __P_HARDWARE_X11_BASE_DPI;
        
        struct {
          bool      isAvailable  = false;
          LibHandle instance     = nullptr;
          int       versionMajor = 0;
          int       versionMinor = 0;
          int       eventBase    = 0;
          int       errorBase    = 0;
          __x11_XRRGetCrtcInfo         GetCrtcInfo_ = nullptr;
          __x11_XRRFreeCrtcInfo        FreeCrtcInfo_ = nullptr;
          __x11_XRRGetOutputInfo       GetOutputInfo_ = nullptr;
          __x11_XRRGetOutputPrimary    GetOutputPrimary_ = nullptr;
          __x11_XRRFreeOutputInfo      FreeOutputInfo_ = nullptr;
          __x11_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent_ = nullptr;
          __x11_XRRFreeScreenResources FreeScreenResources_ = nullptr;
          __x11_XRRQueryExtension      QueryExtension_ = nullptr;
          __x11_XRRQueryVersion        QueryVersion_ = nullptr;
          __x11_XRRSelectInput         SelectInput_ = nullptr;
          __x11_XRRSetCrtcConfig       SetCrtcConfig_ = nullptr;
          __x11_XRRUpdateConfiguration UpdateConfiguration_ = nullptr;
        } randr;
        struct {
          bool      isAvailable  = false;
          LibHandle instance     = nullptr
          int       versionMajor = 0;
          int       versionMinor = 0;
          int       eventBase    = 0;
          int       errorBase    = 0;
          __x11_XineramaIsActive       IsActive_ = nullptr;
          __x11_XineramaQueryExtension QueryExtension_ = nullptr;
          __x11_XineramaQueryVersion   QueryVersion_ = nullptr;
          __x11_XineramaQueryScreens   QueryScreens_ = nullptr;
        } xinerama;


        // initialize available libraries
        bool init() noexcept;
        // close libraries
        void shutdown() noexcept;
        // enable DPI awareness
        void readSystemDpi() noexcept
        
        // get global instance
        static inline LibrariesX11& instance() noexcept {
          if (!_libs._isInit)
            _libs.init();
          return _libs;
        }

      private:
        bool _isInit = false;
        static LibrariesX11 _libs;
      };
    }
  }
#endif
