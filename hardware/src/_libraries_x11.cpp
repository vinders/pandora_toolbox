/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <stdexcept>
# include <climits>
# include <ctime>
# include <dlfcn.h>
# include "hardware/_private/_libraries_x11.h"

  pandora::hardware::LibrariesX11 pandora::hardware::LibrariesX11::_libs{};

  using namespace pandora::hardware;


  // -- library binding utilities --
  
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
  
  Atom LibrariesX11::_findAtom(Atom* atomArray, uint32_t arrayLength, const char* targetName) noexcept {
    const Atom atom = this->xlib.InternAtom_(this->displayServer, targetName, False);
    for (uint32_t i = 0; i < arrayLength; ++i) {
      if (atomArray[i] == atom)
        return atom;
    }
    return None;
  }
  
  
  // -- display monitor utilities --
  
  void LibrariesX11::readSystemDpi() noexcept {
    dpiX = dpiY = __P_HARDWARE_X11_BASE_DPI; // default value if not found in Xft.dpi
    if (this->displayServer == nullptr)
      return;
    
    // read Xft.dpi properties
    char* resManagerId = this->xlib.ResourceManagerString_(this->displayServer);
    if (resManagerId != nullptr) {
      XrmDatabase db = this->xlib.rmGetStringDatabase_(resManagerId);
      if (db) {
        char* type = nullptr;
        XrmValue value;
        if (this->xlib.rmGetResource_(db, "Xft.dpi", "Xft.Dpi", &type, &value) && type && strcmp(type, "String") == 0) {
          dpiX = dpiY = atof(value.addr);
          if (dpiX <= 0.0f || dpiY <= 0.0f)
            dpiX = dpiY = __P_HARDWARE_X11_BASE_DPI;
        }
        this->xlib.rmDestroyDatabase_(db);
      }
    }
  }
  
  
  // -- atom properties --
  
  uint32_t LibrariesX11::getWindowProperty(Window window, Atom property, Atom type, unsigned char** value) noexcept {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;
    if (this->xlib.GetWindowProperty_(this->displayServer, window, property, 0, LONG_MAX, False, type, 
                                      &actualType, &actualFormat, &itemCount, &bytesAfter, value) != Success) {
      if (type != AnyPropertyType && actualType != None) {
        type = actualType;
        this->xlib.GetWindowProperty_(this->displayServer, window, property, 0, LONG_MAX, False, type, 
                                      &actualType, &actualFormat, &itemCount, &bytesAfter, value);
      }
    }
    return static_cast<uint32_t>(itemCount);
  }
  
  uint32_t LibrariesX11::getWindowProperty(Window window, Atom property, Atom type, unsigned char** value, 
                                           uint32_t length, int32_t retryCount) noexcept {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;
    do {
      if (this->xlib.GetWindowProperty_(this->displayServer, window, property, 0, length, False, type, 
                                        &actualType, &actualFormat, &itemCount, &bytesAfter, value) != Success || itemCount != length || bytesAfter != 0) {
        if (type != AnyPropertyType && actualType != None)
          type = actualType;
        
        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 1000000;
        nanosleep(&sleepTime, nullptr);
      }
    } 
    while (*value == nullptr && --retryCount >= 0);
    return static_cast<uint32_t>(itemCount);
  }
  
  
  // -- init --

  void LibrariesX11::init() {
    if (this->_isInit)
      return;
    
#   if !defined(X_HAVE_UTF8_STRING)
      // 'C' locale breaks wide-char input (used when no UTF-8 support) -> apply environment's locale instead
      if (strcmp(setlocale(LC_CTYPE, nullptr), "C") == 0)
        setlocale(LC_CTYPE, "");
#   endif

    // load Xlib library
#if defined(__CYGWIN__)
      this->xlib.instance = _loadLibrary("libX11-6.so");
#else
      this->xlib.instance = _loadLibrary("libX11.so.6");
#endif
    if (this->xlib.instance == nullptr) {
      this->xlib.instance = _loadLibrary("libX11.so");
      if (this->xlib.instance == nullptr)
        throw std::runtime_error("LibrariesX11: libX11 could not be found");
    }
    
    _bindXlib();
    
    // enable Xlib concurrency support (must be first Xlib call)
    this->xlib.InitThreads_();
    // enable XRM functions (resource management, DPI...)
    this->xlib.rmInitialize_(); 
    readSystemDpi();

    // obtain Xorg server connection - required
    this->displayServer = this->xlib.OpenDisplay_(nullptr);
    if (this->displayServer == nullptr)
      throw std::runtime_error("LibrariesX11: connection failure to display");
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
      _bindXrandr();
    
      // register to screen change notifications
      if (this->randr.isAvailable && this->randr.SelectInput_)
        this->randr.SelectInput_(this->displayServer, this->rootWindow, RROutputChangeNotifyMask);
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
      _bindXinerama();
    }
    
    // load Atom properties
    _bindAtoms();
    
    // locale/input
    if (this->xlib.SupportsLocale_() && this->xlib.isUtf8) {
      this->xlib.SetLocaleModifiers_("");
    }

    this->_isInit = true;
  }

  // -- shutdown --

  void LibrariesX11::shutdown() noexcept {
    if (this->displayServer != nullptr) {
      this->xlib.CloseDisplay_(this->displayServer);
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
    if (this->xlib.instance != nullptr) {
      _freeLibrary(this->xlib.instance);
      this->xlib.instance = nullptr;
    }
    
    this->_isInit = false;
  }
  
  
  // -- library bindings --
  
  void LibrariesX11::_bindXlib() noexcept {
    this->xlib.AllocClassHint_ = _getSymbolAddress<__x11_XAllocClassHint>(this->xlib.instance, "XAllocClassHint");
    this->xlib.AllocColor_ = _getSymbolAddress<__x11_XAllocColor>(this->xlib.instance, "XAllocColor");
    this->xlib.AllocSizeHints_ = _getSymbolAddress<__x11_XAllocSizeHints>(this->xlib.instance, "XAllocSizeHints");
    this->xlib.AllocWMHints_ = _getSymbolAddress<__x11_XAllocWMHints>(this->xlib.instance, "XAllocWMHints");
    this->xlib.ChangeProperty_ = _getSymbolAddress<__x11_XChangeProperty>(this->xlib.instance, "XChangeProperty");
    this->xlib.ChangeWindowAttributes_ = _getSymbolAddress<__x11_XChangeWindowAttributes>(this->xlib.instance, "XChangeWindowAttributes");
    this->xlib.CheckIfEvent_ = _getSymbolAddress<__x11_XCheckIfEvent>(this->xlib.instance, "XCheckIfEvent");
    this->xlib.CheckTypedWindowEvent_ = _getSymbolAddress<__x11_XCheckTypedWindowEvent>(this->xlib.instance, "XCheckTypedWindowEvent");
    this->xlib.CloseDisplay_ = _getSymbolAddress<__x11_XCloseDisplay>(this->xlib.instance, "XCloseDisplay");
    this->xlib.CloseIM_ = _getSymbolAddress<__x11_XCloseIM>(this->xlib.instance, "XCloseIM");
    this->xlib.ConvertSelection_ = _getSymbolAddress<__x11_XConvertSelection>(this->xlib.instance, "XConvertSelection");
    this->xlib.CreateColormap_ = _getSymbolAddress<__x11_XCreateColormap>(this->xlib.instance, "XCreateColormap");
    this->xlib.CreateFontCursor_ = _getSymbolAddress<__x11_XCreateFontCursor>(this->xlib.instance, "XCreateFontCursor");
    this->xlib.CreateFontSet_ = _getSymbolAddress<__x11_XCreateFontSet>(this->xlib.instance, "XCreateFontSet");
    this->xlib.CreateGC_ = _getSymbolAddress<__x11_XCreateGC>(this->xlib.instance, "XCreateGC");
    this->xlib.CreateIC_ = _getSymbolAddress<__x11_XCreateIC>(this->xlib.instance, "XCreateIC");
    this->xlib.CreateRegion_ = _getSymbolAddress<__x11_XCreateRegion>(this->xlib.instance, "XCreateRegion");
    this->xlib.CreateSimpleWindow_ = _getSymbolAddress<__x11_XCreateSimpleWindow>(this->xlib.instance, "XCreateSimpleWindow");
    this->xlib.CreateWindow_ = _getSymbolAddress<__x11_XCreateWindow>(this->xlib.instance, "XCreateWindow");
    this->xlib.DefineCursor_ = _getSymbolAddress<__x11_XDefineCursor>(this->xlib.instance, "XDefineCursor");
    this->xlib.DeleteContext_ = _getSymbolAddress<__x11_XDeleteContext>(this->xlib.instance, "XDeleteContext");
    this->xlib.DeleteProperty_ = _getSymbolAddress<__x11_XDeleteProperty>(this->xlib.instance, "XDeleteProperty");
    this->xlib.DestroyIC_ = _getSymbolAddress<__x11_XDestroyIC>(this->xlib.instance, "XDestroyIC");
    this->xlib.DestroyRegion_ = _getSymbolAddress<__x11_XDestroyRegion>(this->xlib.instance, "XDestroyRegion");
    this->xlib.DestroyWindow_ = _getSymbolAddress<__x11_XDestroyWindow>(this->xlib.instance, "XDestroyWindow");
    this->xlib.DisplayKeycodes_ = _getSymbolAddress<__x11_XDisplayKeycodes>(this->xlib.instance, "XDisplayKeycodes");
    this->xlib.EventsQueued_ = _getSymbolAddress<__x11_XEventsQueued>(this->xlib.instance, "XEventsQueued");
    this->xlib.FillRectangle_ = _getSymbolAddress<__x11_XFillRectangle>(this->xlib.instance, "XFillRectangle");
    this->xlib.FilterEvent_ = _getSymbolAddress<__x11_XFilterEvent>(this->xlib.instance, "XFilterEvent");
    this->xlib.FindContext_ = _getSymbolAddress<__x11_XFindContext>(this->xlib.instance, "XFindContext");
    this->xlib.Flush_ = _getSymbolAddress<__x11_XFlush>(this->xlib.instance, "XFlush");
    this->xlib.Free_ = _getSymbolAddress<__x11_XFree>(this->xlib.instance, "XFree");
    this->xlib.FreeColormap_ = _getSymbolAddress<__x11_XFreeColormap>(this->xlib.instance, "XFreeColormap");
    this->xlib.FreeCursor_ = _getSymbolAddress<__x11_XFreeCursor>(this->xlib.instance, "XFreeCursor");
    this->xlib.FreeEventData_ = _getSymbolAddress<__x11_XFreeEventData>(this->xlib.instance, "XFreeEventData");
    this->xlib.FreeFontSet_ = _getSymbolAddress<__x11_XFreeFontSet>(this->xlib.instance, "XFreeFontSet");
    this->xlib.FreeGC_ = _getSymbolAddress<__x11_XFreeGC>(this->xlib.instance, "XFreeGC");
    this->xlib.FreeStringList_ = _getSymbolAddress<__x11_XFreeStringList>(this->xlib.instance, "XFreeStringList");
    this->xlib.GetErrorText_ = _getSymbolAddress<__x11_XGetErrorText>(this->xlib.instance, "XGetErrorText");
    this->xlib.GetEventData_ = _getSymbolAddress<__x11_XGetEventData>(this->xlib.instance, "XGetEventData");
    this->xlib.GetICValues_ = _getSymbolAddress<__x11_XGetICValues>(this->xlib.instance, "XGetICValues");
    this->xlib.GetIMValues_ = _getSymbolAddress<__x11_XGetIMValues>(this->xlib.instance, "XGetIMValues");
    this->xlib.GetInputFocus_ = _getSymbolAddress<__x11_XGetInputFocus>(this->xlib.instance, "XGetInputFocus");
    this->xlib.GetKeyboardMapping_ = _getSymbolAddress<__x11_XGetKeyboardMapping>(this->xlib.instance, "XGetKeyboardMapping");
    this->xlib.GetScreenSaver_ = _getSymbolAddress<__x11_XGetScreenSaver>(this->xlib.instance, "XGetScreenSaver");
    this->xlib.GetSelectionOwner_ = _getSymbolAddress<__x11_XGetSelectionOwner>(this->xlib.instance, "XGetSelectionOwner");
    this->xlib.GetVisualInfo_ = _getSymbolAddress<__x11_XGetVisualInfo>(this->xlib.instance, "XGetVisualInfo");
    this->xlib.GetWMNormalHints_ = _getSymbolAddress<__x11_XGetWMNormalHints>(this->xlib.instance, "XGetWMNormalHints");
    this->xlib.GetWindowAttributes_ = _getSymbolAddress<__x11_XGetWindowAttributes>(this->xlib.instance, "XGetWindowAttributes");
    this->xlib.GetWindowProperty_ = _getSymbolAddress<__x11_XGetWindowProperty>(this->xlib.instance, "XGetWindowProperty");
    this->xlib.GrabPointer_ = _getSymbolAddress<__x11_XGrabPointer>(this->xlib.instance, "XGrabPointer");
    this->xlib.IconifyWindow_ = _getSymbolAddress<__x11_XIconifyWindow>(this->xlib.instance, "XIconifyWindow");
    this->xlib.InitThreads_ = _getSymbolAddress<__x11_XInitThreads>(this->xlib.instance, "XInitThreads");
    this->xlib.InternAtom_ = _getSymbolAddress<__x11_XInternAtom>(this->xlib.instance, "XInternAtom");
    this->xlib.LookupString_ = _getSymbolAddress<__x11_XLookupString>(this->xlib.instance, "XLookupString");
    this->xlib.MapRaised_ = _getSymbolAddress<__x11_XMapRaised>(this->xlib.instance, "XMapRaised");
    this->xlib.MapWindow_ = _getSymbolAddress<__x11_XMapWindow>(this->xlib.instance, "XMapWindow");
    this->xlib.MoveResizeWindow_ = _getSymbolAddress<__x11_XMoveResizeWindow>(this->xlib.instance, "XMoveResizeWindow");
    this->xlib.MoveWindow_ = _getSymbolAddress<__x11_XMoveWindow>(this->xlib.instance, "XMoveWindow");
    this->xlib.NextEvent_ = _getSymbolAddress<__x11_XNextEvent>(this->xlib.instance, "XNextEvent");
    this->xlib.OpenDisplay_ = _getSymbolAddress<__x11_XOpenDisplay>(this->xlib.instance, "XOpenDisplay");
    this->xlib.OpenIM_ = _getSymbolAddress<__x11_XOpenIM>(this->xlib.instance, "XOpenIM");
    this->xlib.PeekEvent_ = _getSymbolAddress<__x11_XPeekEvent>(this->xlib.instance, "XPeekEvent");
    this->xlib.Pending_ = _getSymbolAddress<__x11_XPending>(this->xlib.instance, "XPending");
    this->xlib.QueryExtension_ = _getSymbolAddress<__x11_XQueryExtension>(this->xlib.instance, "XQueryExtension");
    this->xlib.QueryPointer_ = _getSymbolAddress<__x11_XQueryPointer>(this->xlib.instance, "XQueryPointer");
    this->xlib.RaiseWindow_ = _getSymbolAddress<__x11_XRaiseWindow>(this->xlib.instance, "XRaiseWindow");
    this->xlib.RegisterIMInstantiateCallback_ = _getSymbolAddress<__x11_XRegisterIMInstantiateCallback>(this->xlib.instance, "XRegisterIMInstantiateCallback");
    this->xlib.ResizeWindow_ = _getSymbolAddress<__x11_XResizeWindow>(this->xlib.instance, "XResizeWindow");
    this->xlib.ResourceManagerString_ = _getSymbolAddress<__x11_XResourceManagerString>(this->xlib.instance, "XResourceManagerString");
    this->xlib.SaveContext_ = _getSymbolAddress<__x11_XSaveContext>(this->xlib.instance, "XSaveContext");
    this->xlib.SelectInput_ = _getSymbolAddress<__x11_XSelectInput>(this->xlib.instance, "XSelectInput");
    this->xlib.SendEvent_ = _getSymbolAddress<__x11_XSendEvent>(this->xlib.instance, "XSendEvent");
    this->xlib.SetClassHint_ = _getSymbolAddress<__x11_XSetClassHint>(this->xlib.instance, "XSetClassHint");
    this->xlib.SetErrorHandler_ = _getSymbolAddress<__x11_XSetErrorHandler>(this->xlib.instance, "XSetErrorHandler");
    this->xlib.SetICFocus_ = _getSymbolAddress<__x11_XSetICFocus>(this->xlib.instance, "XSetICFocus");
    this->xlib.SetIMValues_ = _getSymbolAddress<__x11_XSetIMValues>(this->xlib.instance, "XSetIMValues");
    this->xlib.SetInputFocus_ = _getSymbolAddress<__x11_XSetInputFocus>(this->xlib.instance, "XSetInputFocus");
    this->xlib.SetLocaleModifiers_ = _getSymbolAddress<__x11_XSetLocaleModifiers>(this->xlib.instance, "XSetLocaleModifiers");
    this->xlib.SetScreenSaver_ = _getSymbolAddress<__x11_XSetScreenSaver>(this->xlib.instance, "XSetScreenSaver");
    this->xlib.SetSelectionOwner_ = _getSymbolAddress<__x11_XSetSelectionOwner>(this->xlib.instance, "XSetSelectionOwner");
    this->xlib.SetWMHints_ = _getSymbolAddress<__x11_XSetWMHints>(this->xlib.instance, "XSetWMHints");
    this->xlib.SetWMNormalHints_ = _getSymbolAddress<__x11_XSetWMNormalHints>(this->xlib.instance, "XSetWMNormalHints");
    this->xlib.SetWMProtocols_ = _getSymbolAddress<__x11_XSetWMProtocols>(this->xlib.instance, "XSetWMProtocols");
    this->xlib.SupportsLocale_ = _getSymbolAddress<__x11_XSupportsLocale>(this->xlib.instance, "XSupportsLocale");
    this->xlib.Sync_ = _getSymbolAddress<__x11_XSync>(this->xlib.instance, "XSync");
    this->xlib.TranslateCoordinates_ = _getSymbolAddress<__x11_XTranslateCoordinates>(this->xlib.instance, "XTranslateCoordinates");
    this->xlib.UndefineCursor_ = _getSymbolAddress<__x11_XUndefineCursor>(this->xlib.instance, "XUndefineCursor");
    this->xlib.UngrabPointer_ = _getSymbolAddress<__x11_XUngrabPointer>(this->xlib.instance, "XUngrabPointer");
    this->xlib.UnmapWindow_ = _getSymbolAddress<__x11_XUnmapWindow>(this->xlib.instance, "XUnmapWindow");
    this->xlib.UnsetICFocus_ = _getSymbolAddress<__x11_XUnsetICFocus>(this->xlib.instance, "XUnsetICFocus");
    this->xlib.VisualIDFromVisual_ = _getSymbolAddress<__x11_XVisualIDFromVisual>(this->xlib.instance, "XVisualIDFromVisual");
    this->xlib.WarpPointer_ = _getSymbolAddress<__x11_XWarpPointer>(this->xlib.instance, "XWarpPointer");
    this->xlib.kbFreeKeyboard_ = _getSymbolAddress<__x11_XkbFreeKeyboard>(this->xlib.instance, "XkbFreeKeyboard");
    this->xlib.kbFreeNames_ = _getSymbolAddress<__x11_XkbFreeNames>(this->xlib.instance, "XkbFreeNames");
    this->xlib.kbGetMap_ = _getSymbolAddress<__x11_XkbGetMap>(this->xlib.instance, "XkbGetMap");
    this->xlib.kbGetNames_ = _getSymbolAddress<__x11_XkbGetNames>(this->xlib.instance, "XkbGetNames");
    this->xlib.kbGetState_ = _getSymbolAddress<__x11_XkbGetState>(this->xlib.instance, "XkbGetState");
    this->xlib.kbKeycodeToKeysym_ = _getSymbolAddress<__x11_XkbKeycodeToKeysym>(this->xlib.instance, "XkbKeycodeToKeysym");
    this->xlib.kbQueryExtension_ = _getSymbolAddress<__x11_XkbQueryExtension>(this->xlib.instance, "XkbQueryExtension");
    this->xlib.kbSelectEventDetails_ = _getSymbolAddress<__x11_XkbSelectEventDetails>(this->xlib.instance, "XkbSelectEventDetails");
    this->xlib.kbSetDetectableAutoRepeat_ = _getSymbolAddress<__x11_XkbSetDetectableAutoRepeat>(this->xlib.instance, "XkbSetDetectableAutoRepeat");
    this->xlib.rmDestroyDatabase_ = _getSymbolAddress<__x11_XrmDestroyDatabase>(this->xlib.instance, "XrmDestroyDatabase");
    this->xlib.rmGetResource_ = _getSymbolAddress<__x11_XrmGetResource>(this->xlib.instance, "XrmGetResource");
    this->xlib.rmGetStringDatabase_ = _getSymbolAddress<__x11_XrmGetStringDatabase>(this->xlib.instance, "XrmGetStringDatabase");
    this->xlib.rmInitialize_ = _getSymbolAddress<__x11_XrmInitialize>(this->xlib.instance, "XrmInitialize");
    this->xlib.rmUniqueQuark_ = _getSymbolAddress<__x11_XrmUniqueQuark>(this->xlib.instance, "XrmUniqueQuark");
    this->xlib.SetForeground_ = _getSymbolAddress<__x11_XSetForeground>(this->xlib.instance, "XSetForeground");
    this->xlib.UnregisterIMInstantiateCallback_ = _getSymbolAddress<__x11_XUnregisterIMInstantiateCallback>(this->xlib.instance, "XUnregisterIMInstantiateCallback");
    this->xlib.utf8DrawString_ = _getSymbolAddress<__x11_Xutf8DrawString>(this->xlib.instance, "Xutf8DrawString");
    this->xlib.utf8LookupString_ = _getSymbolAddress<__x11_Xutf8LookupString>(this->xlib.instance, "Xutf8LookupString");
    this->xlib.utf8SetWMProperties_ = _getSymbolAddress<__x11_Xutf8SetWMProperties>(this->xlib.instance, "Xutf8SetWMProperties");
    this->xlib.utf8TextExtents_ = _getSymbolAddress<__x11_Xutf8TextExtents>(this->xlib.instance, "Xutf8TextExtents");

    // Xlib UTF-8 support
    this->xlib.isUtf8 = (this->xlib.utf8LookupString_ != nullptr && this->xlib.utf8SetWMProperties_ != nullptr);
  }
  
  void LibrariesX11::_bindXrandr() noexcept {
    this->randr.GetCrtcInfo_ = _getSymbolAddress<__x11_XRRGetCrtcInfo>(this->randr.instance, "XRRGetCrtcInfo");
    this->randr.FreeCrtcInfo_ = _getSymbolAddress<__x11_XRRFreeCrtcInfo>(this->randr.instance, "XRRFreeCrtcInfo");
    this->randr.GetOutputInfo_ = _getSymbolAddress<__x11_XRRGetOutputInfo>(this->randr.instance, "XRRGetOutputInfo");
    this->randr.GetOutputPrimary_ = _getSymbolAddress<__x11_XRRGetOutputPrimary>(this->randr.instance, "XRRGetOutputPrimary");
    this->randr.FreeOutputInfo_ = _getSymbolAddress<__x11_XRRFreeOutputInfo>(this->randr.instance, "XRRFreeOutputInfo");
    this->randr.GetProviderInfo_ = _getSymbolAddress<__x11_XRRGetProviderInfo>(this->randr.instance, "XRRGetProviderInfo");
    this->randr.FreeProviderInfo_ = _getSymbolAddress<__x11_XRRFreeProviderInfo>(this->randr.instance, "XRRFreeProviderInfo");
    this->randr.GetScreenResourcesCurrent_ = _getSymbolAddress<__x11_XRRGetScreenResourcesCurrent>(this->randr.instance, "XRRGetScreenResourcesCurrent");
    this->randr.FreeScreenResources_ = _getSymbolAddress<__x11_XRRFreeScreenResources>(this->randr.instance, "XRRFreeScreenResources");
    this->randr.GetProviderResources_ = _getSymbolAddress<__x11_XRRGetProviderResources>(this->randr.instance, "XRRGetProviderResources");
    this->randr.FreeProviderResources_ = _getSymbolAddress<__x11_XRRFreeProviderResources>(this->randr.instance, "XRRFreeProviderResources");
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
    }
  }
  
  void LibrariesX11::_bindXinerama() noexcept {
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
  
  void LibrariesX11::_bindAtoms() noexcept {
    // string format
    this->atoms.NULL_ATOM = this->xlib.InternAtom_(this->displayServer, "NULL", False);
    this->atoms.UTF8_STRING = this->xlib.InternAtom_(this->displayServer, "UTF8_STRING", False);
    this->atoms.ATOM_PAIR = this->xlib.InternAtom_(this->displayServer, "ATOM_PAIR", False);

    // clipboard / selection
    this->atoms.CLIPBOARD = this->xlib.InternAtom_(this->displayServer, "CLIPBOARD", False);
    this->atoms.INCR = this->xlib.InternAtom_(this->displayServer, "INCR", False);
    this->atoms.MULTIPLE = this->xlib.InternAtom_(this->displayServer, "MULTIPLE", False);
    this->atoms.PRIMARY = this->xlib.InternAtom_(this->displayServer, "PRIMARY", False);
    this->atoms.TARGETS = this->xlib.InternAtom_(this->displayServer, "TARGETS", False);
    this->atoms.CLIPBOARD_MANAGER = this->xlib.InternAtom_(this->displayServer, "CLIPBOARD_MANAGER", False);
    this->atoms.SAVE_TARGETS = this->xlib.InternAtom_(this->displayServer, "SAVE_TARGETS", False);

    // drag and drop
    this->atoms.TextUriList = this->xlib.InternAtom_(this->displayServer, "text/uri-list", False);
    this->atoms.XdndAware = this->xlib.InternAtom_(this->displayServer, "XdndAware", False);
    this->atoms.XdndActionCopy = this->xlib.InternAtom_(this->displayServer, "XdndActionCopy", False);
    this->atoms.XdndDrop = this->xlib.InternAtom_(this->displayServer, "XdndDrop", False);
    this->atoms.XdndEnter = this->xlib.InternAtom_(this->displayServer, "XdndEnter", False);
    this->atoms.XdndFinished = this->xlib.InternAtom_(this->displayServer, "XdndFinished", False);
    this->atoms.XdndPosition = this->xlib.InternAtom_(this->displayServer, "XdndPosition", False);
    this->atoms.XdndSelection = this->xlib.InternAtom_(this->displayServer, "XdndSelection", False);
    this->atoms.XdndStatus = this->xlib.InternAtom_(this->displayServer, "XdndStatus", False);
    this->atoms.XdndTypeList = this->xlib.InternAtom_(this->displayServer, "XdndTypeList", False);

    // ICCCM/EWMH/Motif window properties (do not require WM support)
    this->atoms.MOTIF_WM_HINTS = this->xlib.InternAtom_(this->displayServer, "_MOTIF_WM_HINTS", False);
    this->atoms.NET_SUPPORTED = this->xlib.InternAtom_(this->displayServer, "_NET_SUPPORTED", False);
    this->atoms.NET_SUPPORTING_WM_CHECK = this->xlib.InternAtom_(this->displayServer, "_NET_SUPPORTING_WM_CHECK", False);
    this->atoms.NET_WM_BYPASS_COMPOSITOR = this->xlib.InternAtom_(this->displayServer, "_NET_WM_BYPASS_COMPOSITOR", False);
    this->atoms.NET_WM_ICON = this->xlib.InternAtom_(this->displayServer, "_NET_WM_ICON", False);
    this->atoms.NET_WM_ICON_NAME = this->xlib.InternAtom_(this->displayServer, "_NET_WM_ICON_NAME", False);
    this->atoms.NET_WM_NAME = this->xlib.InternAtom_(this->displayServer, "_NET_WM_NAME", False);
    this->atoms.NET_WM_PING = this->xlib.InternAtom_(this->displayServer, "_NET_WM_PING", False);
    this->atoms.NET_WM_PID = this->xlib.InternAtom_(this->displayServer, "_NET_WM_PID", False);
    this->atoms.NET_WM_WINDOW_OPACITY = this->xlib.InternAtom_(this->displayServer, "_NET_WM_WINDOW_OPACITY", False);
    this->atoms.WM_DELETE_WINDOW = this->xlib.InternAtom_(this->displayServer, "WM_DELETE_WINDOW", False);
    this->atoms.WM_PROTOCOLS = this->xlib.InternAtom_(this->displayServer, "WM_PROTOCOLS", False);
    this->atoms.WM_STATE = this->xlib.InternAtom_(this->displayServer, "WM_STATE", False);

    // compositing manager selection (name with screen number)
    char compMgrName[32];
    snprintf(compMgrName, sizeof(compMgrName), "_NET_WM_CM_S%u", this->screenIndex);
    this->atoms.NET_WM_CM_SCREEN = this->xlib.InternAtom_(this->displayServer, compMgrName, False);
    
    // verify EWMH WM compliance
    bool isExmhCompliant = false;
    Window* rootWindowId = nullptr;
    if (getWindowProperty(this->rootWindow, this->atoms.NET_SUPPORTING_WM_CHECK, XA_WINDOW, (unsigned char**)&rootWindowId) > 0) {
      Window* frameWindowId = nullptr;
      if (getWindowProperty(*rootWindowId, this->atoms.NET_SUPPORTING_WM_CHECK, XA_WINDOW, (unsigned char**)&frameWindowId) > 0) {
        isExmhCompliant = (*rootWindowId == *frameWindowId);
        this->xlib.Free_(frameWindowId);
      }
      this->xlib.Free_(rootWindowId);
    }
    // WM feature support queries
    if (isExmhCompliant) {
      Atom* supportedAtoms = nullptr;
      uint32_t atomCount = getWindowProperty(this->rootWindow, this->atoms.NET_SUPPORTED, XA_ATOM, (unsigned char**)&supportedAtoms);
      if (atomCount > 0 && supportedAtoms != nullptr) {
        this->atoms.NET_ACTIVE_WINDOW = _findAtom(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");
        this->atoms.NET_CURRENT_DESKTOP = _findAtom(supportedAtoms, atomCount, "_NET_CURRENT_DESKTOP");
        this->atoms.NET_FRAME_EXTENTS = _findAtom(supportedAtoms, atomCount, "_NET_FRAME_EXTENTS");
        this->atoms.NET_REQUEST_FRAME_EXTENTS = _findAtom(supportedAtoms, atomCount, "_NET_REQUEST_FRAME_EXTENTS");
        this->atoms.NET_WM_FULLSCREEN_MONITORS = _findAtom(supportedAtoms, atomCount, "_NET_WM_FULLSCREEN_MONITORS");
        this->atoms.NET_WM_STATE = _findAtom(supportedAtoms, atomCount, "_NET_WM_STATE");
        this->atoms.NET_WM_STATE_ABOVE = _findAtom(supportedAtoms, atomCount, "_NET_WM_STATE_ABOVE");
        this->atoms.NET_WM_STATE_DEMANDS_ATTENTION = _findAtom(supportedAtoms, atomCount, "_NET_WM_STATE_DEMANDS_ATTENTION");
        this->atoms.NET_WM_STATE_FULLSCREEN = _findAtom(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");
        this->atoms.NET_WM_STATE_MAXIMIZED_VERT = _findAtom(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_VERT");
        this->atoms.NET_WM_STATE_MAXIMIZED_HORZ = _findAtom(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_HORZ");
        this->atoms.NET_WM_WINDOW_TYPE = _findAtom(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE");
        this->atoms.NET_WM_WINDOW_TYPE_DIALOG = _findAtom(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_DIALOG");
        this->atoms.NET_WM_WINDOW_TYPE_NORMAL = _findAtom(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_NORMAL");
        this->atoms.NET_WORKAREA = _findAtom(supportedAtoms, atomCount, "_NET_WORKAREA");
        
        this->xlib.Free_(supportedAtoms);
      }
    }
  }
#endif
