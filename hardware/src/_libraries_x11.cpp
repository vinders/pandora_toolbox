/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdlib>
# include <cstring>
# include <dlfcn.h>
# include "hardware/_private/_libraries_x11.h"

  pandora::hardware::LibrariesX11 pandora::hardware::LibrariesX11::_libs{};

  using namespace pandora::hardware;


  // -- utilities --
  
  static inline LibHandle _loadLibrary(const char fileName[]) noexcept {
    return dlopen(fileName, RTLD_LAZY | RTLD_LOCAL);
  }
  static inline void _freeLibrary(LibHandle lib) noexcept {
    dlclose(lib);
  }
  template <typename _Signature>
  static inline _Signature _getSymbolAddress(LibHandle lib, const char name[]) noexcept {
    return (_Signature)dlsym(lib, name);
  }
  
  void LibrariesX11::readSystemDpi() noexcept {
    dpiX = dpiY = __P_HARDWARE_X11_BASE_DPI; // default value if not found in Xft.dpi
    if (this->displayServer == nullptr)
      return;
    
    // read Xft.dpi properties
    char* resManagerId = XResourceManagerString(this->displayServer);
    if (resManagerId != nullptr) {
      XrmDatabase db = XrmGetStringDatabase(resManagerId);
      if (db) {
        char* type = nullptr;
        XrmValue value;
        if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && type && strcmp(type, "String") == 0) {
          dpiX = dpiY = atof(value.addr);
          if (dpiX <= 0.0f || dpiY <= 0.0f)
            dpiX = dpiY = __P_HARDWARE_X11_BASE_DPI;
        }
        XrmDestroyDatabase(db);
      }
    }
  }
  
  // -- init --

  bool LibrariesX11::init() noexcept {
    if (this->_isInit)
      return true;
    
#   if !defined(X_HAVE_UTF8_STRING)
      // 'C' locale breaks wide-char input (used when no UTF-8 support) -> apply environment's locale instead
      if (strcmp(setlocale(LC_CTYPE, nullptr), "C") == 0)
        setlocale(LC_CTYPE, "");
#   endif
    XInitThreads();  // enable Xlib concurrency support (must be first Xlib call)
    XrmInitialize(); // enable XRM functions (resource management, DPI...)
    readSystemDpi();

    // obtain Xorg server connection - required
    this->displayServer = XOpenDisplay(nullptr);
    if (this->displayServer == nullptr)
      return false;
    this->screenIndex = DefaultScreen(this->displayServer);
    this->rootWindow = RootWindow(this->displayServer, this->screenIndex);

    // load Xrandr library
#   if defined(__CYGWIN__)
      this->randr.instance = _loadLibrary("libXrandr-2.so");
#   else
      this->randr.instance = _loadLibrary("libXrandr.so.2");
#   endif
    if (this->randr.instance == nullptr)
      this->randr.instance = _loadLibrary("libXrandr.so");

    if (this->randr.instance != nullptr) {
      this->randr.GetCrtcInfo_ = _getSymbolAddress<__x11_XRRGetCrtcInfo>(this->randr.instance, "XRRGetCrtcInfo");
      this->randr.FreeCrtcInfo_ = _getSymbolAddress<__x11_XRRFreeCrtcInfo>(this->randr.instance, "XRRFreeCrtcInfo");
      this->randr.GetOutputInfo_ = _getSymbolAddress<__x11_XRRGetOutputInfo>(this->randr.instance, "XRRGetOutputInfo");
      this->randr.GetOutputPrimary_ = _getSymbolAddress<__x11_XRRGetOutputPrimary>(this->randr.instance, "XRRGetOutputPrimary");
      this->randr.FreeOutputInfo_ = _getSymbolAddress<__x11_XRRFreeOutputInfo>(this->randr.instance, "XRRFreeOutputInfo");
      this->randr.GetScreenResourcesCurrent_ = _getSymbolAddress<__x11_XRRGetScreenResourcesCurrent>(this->randr.instance, "XRRGetScreenResourcesCurrent");
      this->randr.FreeScreenResources_ = _getSymbolAddress<__x11_XRRFreeScreenResources>(this->randr.instance, "XRRFreeScreenResources");
      this->randr.QueryExtension_ = _getSymbolAddress<__x11_XRRQueryExtension>(this->randr.instance, "XRRQueryExtension");
      this->randr.QueryVersion_ = _getSymbolAddress<__x11_XRRQueryVersion>(this->randr.instance, "XRRQueryVersion");
      this->randr.SelectInput_ = _getSymbolAddress<__x11_XRRSelectInput>(this->randr.instance, "XRRSelectInput");
      this->randr.SetCrtcConfig_ = _getSymbolAddress<__x11_XRRSetCrtcConfig>(this->randr.instance, "XRRSetCrtcConfig");
      this->randr.UpdateConfiguration_ = _getSymbolAddress<__x11_XRRUpdateConfiguration>(this->randr.instance, "XRRUpdateConfiguration");
      
      // verify version (if less than 1.3 -> disable)
      if (this->randr.QueryExtension_ 
      && this->randr.QueryExtension_(this->displayServer, &(this->randr.eventBase), &(this->randr.errorBase)) ) {
        if (this->randr.QueryVersion_
        && this->randr.QueryVersion_(this->displayServer, &(this->randr.versionMajor), &(this->randr.versionMinor)) ) {
          this->randr.isAvailable = (this->randr.versionMajor > 1 || this->randr.versionMinor >= 3);
        }
      }
      // verify monitor support (if no monitor or misconfigured randr -> disable)
      if (this->randr.isAvailable) {
        if (this->randr.GetScreenResourcesCurrent_ && this->randr.FreeScreenResources_) {
          XRRScreenResources* screenResources = this->randr.GetScreenResourcesCurrent_(this->displayServer, this->rootWindow);
          this->randr.isAvailable = (screenResources != nullptr && screenResources->ncrtc > 0);
          if (screenResources != nullptr)
            this->randr.FreeScreenResources_(screenResources);
        }
        
        // register to screen change notifications
        if (this->randr.isAvailable && this->randr.SelectInput_)
          this->randr.SelectInput_(this->displayServer, this->rootWindow, RROutputChangeNotifyMask);
      }
    }

    // load xinerama library
#   if defined(__CYGWIN__)
      this->xinerama.instance = _loadLibrary("libXinerama-1.so");
#   else
      this->xinerama.instance = _loadLibrary("libXinerama.so.1");
#   endif
    if (this->xinerama.instance == nullptr)
      this->xinerama.instance = _loadLibrary("libXinerama.so");

    if (this->xinerama.instance != nullptr) {
      this->xinerama.IsActive_ = _getSymbolAddress<__x11_XineramaIsActive>(this->xinerama.instance, "XineramaIsActive");
      this->xinerama.QueryExtension_ = _getSymbolAddress<__x11_XineramaQueryExtension>(this->xinerama.instance, "XineramaQueryExtension");
      this->xinerama.QueryVersion_ = _getSymbolAddress<__x11_XineramaQueryVersion>(this->xinerama.instance, "XineramaQueryVersion");
      this->xinerama.QueryScreens_ = _getSymbolAddress<__x11_XineramaQueryScreens>(this->xinerama.instance, "XineramaQueryScreens");
      
      if (this->xinerama.QueryExtension_ 
      && this->xinerama.QueryExtension_(this->displayServer, &(this->xinerama.eventBase), &(this->xinerama.errorBase)) ) {
        if (this->xinerama.QueryVersion_
        && this->xinerama.QueryVersion_(this->displayServer, &(this->xinerama.versionMajor), &(this->xinerama.versionMinor)) ) {
          this->xinerama.isAvailable = (this->xinerama.IsActive_ && this->xinerama.IsActive_(this->displayServer));
        }
      }
    }

    this->_isInit = true;
    return true;
  }

  // -- shutdown --

  void LibrariesX11::shutdown() noexcept {
    if (this->displayServer != nullptr) {
      XCloseDisplay(this->displayServer);
      this->displayServer = nullptr;
    }
    if (this->randr.instance != nullptr) {
      this->randr.isAvailable = false;
      _freeLibrary(this->randr.instance);
      this->randr.instance = nullptr;
    }
    if (this->xinerama.instance != nullptr) {
      this->xinerama.isAvailable = false;
      _freeLibrary(this->xinerama.instance);
      this->xinerama.instance = nullptr;
    }
    
    this->_isInit = false;
  }
#endif
