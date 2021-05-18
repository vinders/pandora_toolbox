/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstddef>
# include <cstdint>
# include <unistd.h>
# include <unordered_map>
# include <X11/Xlib.h>
# include <X11/Xatom.h>
# include <X11/Xutil.h>
# include <X11/Xresource.h>
# include <X11/XKBlib.h>
# include <X11/extensions/Xrandr.h> // display mode
# include <X11/extensions/Xinerama.h> // legacy monitor indices
# include <X11/extensions/scrnsaver.h>

# define __P_HARDWARE_X11_BASE_DPI 96.0f

  namespace pandora {
    namespace hardware {
      using LibHandle = void*;
      
      // xlib functions
      typedef XClassHint* (* __x11_XAllocClassHint)(void);
      typedef Status      (* __x11_XAllocColor)(Display*,Colormap,XColor*);
      typedef XSizeHints* (* __x11_XAllocSizeHints)(void);
      typedef XWMHints*   (* __x11_XAllocWMHints)(void);
      typedef int         (* __x11_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
      typedef int         (* __x11_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*);
      typedef Bool        (* __x11_XCheckIfEvent)(Display*,XEvent*,Bool(*)(Display*,XEvent*,XPointer),XPointer);
      typedef Bool        (* __x11_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*);
      typedef int         (* __x11_XCloseDisplay)(Display*);
      typedef Status      (* __x11_XCloseIM)(XIM);
      typedef int         (* __x11_XConvertSelection)(Display*,Atom,Atom,Atom,Window,Time);
      typedef Colormap    (* __x11_XCreateColormap)(Display*,Window,Visual*,int);
      typedef Cursor      (* __x11_XCreateFontCursor)(Display*,unsigned int);
      typedef XFontSet    (* __x11_XCreateFontSet)(Display*,char*,char***,int*,char**);
      typedef GC          (* __x11_XCreateGC)(Display*,Drawable,unsigned long,XGCValues*);
      typedef XIC         (* __x11_XCreateIC)(XIM,...);
      typedef Region      (* __x11_XCreateRegion)(void);
      typedef Window      (* __x11_XCreateSimpleWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,unsigned long,unsigned long);
      typedef Window      (* __x11_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
      typedef int         (* __x11_XDefineCursor)(Display*,Window,Cursor);
      typedef int         (* __x11_XDeleteContext)(Display*,XID,XContext);
      typedef int         (* __x11_XDeleteProperty)(Display*,Window,Atom);
      typedef void        (* __x11_XDestroyIC)(XIC);
      typedef int         (* __x11_XDestroyRegion)(Region);
      typedef int         (* __x11_XDestroyWindow)(Display*,Window);
      typedef int         (* __x11_XDisplayKeycodes)(Display*,int*,int*);
      typedef int         (* __x11_XEventsQueued)(Display*,int);
      typedef int         (* __x11_XFillRectangle)(Display*,Drawable,GC,int,int,unsigned int,unsigned int);
      typedef Bool        (* __x11_XFilterEvent)(XEvent*,Window);
      typedef int         (* __x11_XFindContext)(Display*,XID,XContext,XPointer*);
      typedef int         (* __x11_XFlush)(Display*);
      typedef int         (* __x11_XFree)(void*);
      typedef int         (* __x11_XFreeColormap)(Display*,Colormap);
      typedef int         (* __x11_XFreeCursor)(Display*,Cursor);
      typedef void        (* __x11_XFreeEventData)(Display*,XGenericEventCookie*);
      typedef void        (* __x11_XFreeFontSet)(Display*,XFontSet);
      typedef int         (* __x11_XFreeGC)(Display*,GC);
      typedef void        (* __x11_XFreeStringList)(char**);
      typedef int         (* __x11_XGetErrorText)(Display*,int,char*,int);
      typedef Bool        (* __x11_XGetEventData)(Display*,XGenericEventCookie*);
      typedef char*       (* __x11_XGetICValues)(XIC,...);
      typedef char*       (* __x11_XGetIMValues)(XIM,...);
      typedef int         (* __x11_XGetInputFocus)(Display*,Window*,int*);
      typedef KeySym*     (* __x11_XGetKeyboardMapping)(Display*,KeyCode,int,int*);
      typedef int         (* __x11_XGetScreenSaver)(Display*,int*,int*,int*,int*);
      typedef Window      (* __x11_XGetSelectionOwner)(Display*,Atom);
      typedef XVisualInfo* (* __x11_XGetVisualInfo)(Display*,long,XVisualInfo*,int*);
      typedef Status      (* __x11_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*);
      typedef Status      (* __x11_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
      typedef int         (* __x11_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**);
      typedef int         (* __x11_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
      typedef Status      (* __x11_XIconifyWindow)(Display*,Window,int);
      typedef Status      (* __x11_XInitThreads)(void);
      typedef Atom        (* __x11_XInternAtom)(Display*,const char*,Bool);
      typedef int         (* __x11_XLookupString)(XKeyEvent*,char*,int,KeySym*,XComposeStatus*);
      typedef int         (* __x11_XMapRaised)(Display*,Window);
      typedef int         (* __x11_XMapWindow)(Display*,Window);
      typedef int         (* __x11_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
      typedef int         (* __x11_XMoveWindow)(Display*,Window,int,int);
      typedef int         (* __x11_XNextEvent)(Display*,XEvent*);
      typedef Display*    (* __x11_XOpenDisplay)(const char*);
      typedef XIM         (* __x11_XOpenIM)(Display*,XrmDatabase*,char*,char*);
      typedef int         (* __x11_XPeekEvent)(Display*,XEvent*);
      typedef int         (* __x11_XPending)(Display*);
      typedef Bool        (* __x11_XQueryExtension)(Display*,const char*,int*,int*,int*);
      typedef Bool        (* __x11_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
      typedef int         (* __x11_XRaiseWindow)(Display*,Window);
      typedef Bool        (* __x11_XRegisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
      typedef int         (* __x11_XResetScreenSaver)(Display*);
      typedef int         (* __x11_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
      typedef char*       (* __x11_XResourceManagerString)(Display*);
      typedef int         (* __x11_XSaveContext)(Display*,XID,XContext,const char*);
      typedef int         (* __x11_XSelectInput)(Display*,Window,long);
      typedef Status      (* __x11_XSendEvent)(Display*,Window,Bool,long,XEvent*);
      typedef int         (* __x11_XSetClassHint)(Display*,Window,XClassHint*);
      typedef XErrorHandler (* __x11_XSetErrorHandler)(XErrorHandler);
      typedef void        (* __x11_XSetICFocus)(XIC);
      typedef char*       (* __x11_XSetIMValues)(XIM,...);
      typedef int         (* __x11_XSetInputFocus)(Display*,Window,int,Time);
      typedef char*       (* __x11_XSetLocaleModifiers)(const char*);
      typedef int         (* __x11_XSetScreenSaver)(Display*,int,int,int,int);
      typedef int         (* __x11_XSetSelectionOwner)(Display*,Atom,Window,Time);
      typedef int         (* __x11_XSetWMHints)(Display*,Window,XWMHints*);
      typedef void        (* __x11_XSetWMNormalHints)(Display*,Window,XSizeHints*);
      typedef Status      (* __x11_XSetWMProtocols)(Display*,Window,Atom*,int);
      typedef Bool        (* __x11_XSupportsLocale)(void);
      typedef int         (* __x11_XSync)(Display*,Bool);
      typedef Bool        (* __x11_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
      typedef int         (* __x11_XUndefineCursor)(Display*,Window);
      typedef int         (* __x11_XUngrabPointer)(Display*,Time);
      typedef int         (* __x11_XUnmapWindow)(Display*,Window);
      typedef void        (* __x11_XUnsetICFocus)(XIC);
      typedef VisualID    (* __x11_XVisualIDFromVisual)(Visual*);
      typedef int         (* __x11_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
      typedef void        (* __x11_XkbFreeKeyboard)(XkbDescPtr,unsigned int,Bool);
      typedef void        (* __x11_XkbFreeNames)(XkbDescPtr,unsigned int,Bool);
      typedef XkbDescPtr  (* __x11_XkbGetMap)(Display*,unsigned int,unsigned int);
      typedef Status      (* __x11_XkbGetNames)(Display*,unsigned int,XkbDescPtr);
      typedef Status      (* __x11_XkbGetState)(Display*,unsigned int,XkbStatePtr);
      typedef KeySym      (* __x11_XkbKeycodeToKeysym)(Display*,KeyCode,int,int);
      typedef Bool        (* __x11_XkbQueryExtension)(Display*,int*,int*,int*,int*,int*);
      typedef Bool        (* __x11_XkbSelectEventDetails)(Display*,unsigned int,unsigned int,unsigned long,unsigned long);
      typedef Bool        (* __x11_XkbSetDetectableAutoRepeat)(Display*,Bool,Bool*);
      typedef void        (* __x11_XrmDestroyDatabase)(XrmDatabase);
      typedef Bool        (* __x11_XrmGetResource)(XrmDatabase,const char*,const char*,char**,XrmValue*);
      typedef XrmDatabase (* __x11_XrmGetStringDatabase)(const char*);
      typedef void        (* __x11_XrmInitialize)(void);
      typedef XrmQuark    (* __x11_XrmUniqueQuark)(void);
      typedef int         (* __x11_XSetForeground)(Display*,GC,unsigned long); 
      typedef Bool        (* __x11_XUnregisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
      typedef void        (* __x11_Xutf8DrawString)(Display*,Drawable,XFontSet,GC,int,int,char*,int);
      typedef int         (* __x11_Xutf8LookupString)(XIC,XKeyPressedEvent*,char*,int,KeySym*,Status*);
      typedef void        (* __x11_Xutf8SetWMProperties)(Display*,Window,const char*,const char*,char**,int,XSizeHints*,XWMHints*,XClassHint*);
      typedef int         (* __x11_Xutf8TextExtents)(XFontSet,char*,int,XRectangle*,XRectangle*);
      
      // randr functions
      typedef XRRCrtcInfo*   (* __x11_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);
      typedef void           (* __x11_XRRFreeCrtcInfo)(XRRCrtcInfo*);
      typedef XRROutputInfo* (* __x11_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
      typedef RROutput       (* __x11_XRRGetOutputPrimary)(Display*,Window);
      typedef void           (* __x11_XRRFreeOutputInfo)(XRROutputInfo*);
      typedef XRRProviderInfo* (* __x11_XRRGetProviderInfo)(Display*,XRRScreenResources*,RRProvider);
      typedef void           (* __x11_XRRFreeProviderInfo)(XRRProviderInfo*);
      typedef XRRScreenResources* (* __x11_XRRGetScreenResourcesCurrent)(Display*,Window);
      typedef void           (* __x11_XRRFreeScreenResources)(XRRScreenResources*);
      typedef XRRProviderResources* (* __x11_XRRGetProviderResources)(Display*,Window);
      typedef void           (* __x11_XRRFreeProviderResources)(XRRProviderResources*);
      typedef Bool           (* __x11_XRRQueryExtension)(Display*,int*,int*);
      typedef Status         (* __x11_XRRQueryVersion)(Display*,int*,int*);
      typedef void           (* __x11_XRRSelectInput)(Display*,Window,int);
      typedef Status         (* __x11_XRRSetCrtcConfig)(Display*,XRRScreenResources*,RRCrtc,Time,int,int,RRMode,Rotation,RROutput*,int);
      typedef int            (* __x11_XRRUpdateConfiguration)(XEvent*);

      // xinerama functions
      typedef Bool   (* __x11_XineramaIsActive)(Display*);
      typedef Bool   (* __x11_XineramaQueryExtension)(Display*,int*,int*);
      typedef Status (* __x11_XineramaQueryVersion)(Display*,int*,int*);
      typedef XineramaScreenInfo* (* __x11_XineramaQueryScreens)(Display*,int*);

      // xf86vm functions
      typedef Bool (* __x11_XF86VidModeQueryExtension)(Display*,int*,int*);
      
      // xss functions
      typedef void        (* __x11_XScreenSaverSuspend)(Display*,Bool);
      
      
      // library loader - X11
      struct LibrariesX11 final {
        LibrariesX11() = default;
        ~LibrariesX11() noexcept { shutdown(); }
        
        Display* displayServer = nullptr;
        int      screenIndex   = 0;
        Window   rootWindow;
        float dpiX = __P_HARDWARE_X11_BASE_DPI;
        float dpiY = __P_HARDWARE_X11_BASE_DPI;
        std::unordered_map<RROutput,RRMode> originalModes;
        
        struct {
          LibHandle instance = nullptr;
          bool      isUtf8   = true;
          __x11_XAllocClassHint      AllocClassHint_ = nullptr;
          __x11_XAllocColor          AllocColor_ = nullptr;
          __x11_XAllocSizeHints      AllocSizeHints_ = nullptr;
          __x11_XAllocWMHints        AllocWMHints_ = nullptr;
          __x11_XChangeProperty      ChangeProperty_ = nullptr;
          __x11_XChangeWindowAttributes ChangeWindowAttributes_ = nullptr;
          __x11_XCheckIfEvent        CheckIfEvent_ = nullptr;
          __x11_XCheckTypedWindowEvent CheckTypedWindowEvent_ = nullptr;
          __x11_XCloseDisplay        CloseDisplay_ = nullptr;
          __x11_XCloseIM             CloseIM_ = nullptr;
          __x11_XConvertSelection    ConvertSelection_ = nullptr;
          __x11_XCreateColormap      CreateColormap_ = nullptr;
          __x11_XCreateFontCursor    CreateFontCursor_ = nullptr;
          __x11_XCreateFontSet       CreateFontSet_ = nullptr;
          __x11_XCreateGC            CreateGC_ = nullptr;
          __x11_XCreateIC            CreateIC_ = nullptr;
          __x11_XCreateRegion        CreateRegion_ = nullptr;
          __x11_XCreateSimpleWindow  CreateSimpleWindow_ = nullptr;
          __x11_XCreateWindow        CreateWindow_ = nullptr;
          __x11_XDefineCursor        DefineCursor_ = nullptr;
          __x11_XDeleteContext       DeleteContext_ = nullptr;
          __x11_XDeleteProperty      DeleteProperty_ = nullptr;
          __x11_XDestroyIC           DestroyIC_ = nullptr;
          __x11_XDestroyRegion       DestroyRegion_ = nullptr;
          __x11_XDestroyWindow       DestroyWindow_ = nullptr;
          __x11_XDisplayKeycodes     DisplayKeycodes_ = nullptr;
          __x11_XEventsQueued        EventsQueued_ = nullptr;
          __x11_XFillRectangle       FillRectangle_ = nullptr;
          __x11_XFilterEvent         FilterEvent_ = nullptr;
          __x11_XFindContext         FindContext_ = nullptr;
          __x11_XFlush               Flush_ = nullptr;
          __x11_XFree                Free_ = nullptr;
          __x11_XFreeColormap        FreeColormap_ = nullptr;
          __x11_XFreeCursor          FreeCursor_ = nullptr;
          __x11_XFreeEventData       FreeEventData_ = nullptr;
          __x11_XFreeFontSet         FreeFontSet_ = nullptr;
          __x11_XFreeGC              FreeGC_ = nullptr;
          __x11_XFreeStringList      FreeStringList_ = nullptr;
          __x11_XGetErrorText        GetErrorText_ = nullptr;
          __x11_XGetEventData        GetEventData_ = nullptr;
          __x11_XGetICValues         GetICValues_ = nullptr;
          __x11_XGetIMValues         GetIMValues_ = nullptr;
          __x11_XGetInputFocus       GetInputFocus_ = nullptr;
          __x11_XGetKeyboardMapping  GetKeyboardMapping_ = nullptr;
          __x11_XGetScreenSaver      GetScreenSaver_ = nullptr;
          __x11_XGetSelectionOwner   GetSelectionOwner_ = nullptr;
          __x11_XGetVisualInfo       GetVisualInfo_ = nullptr;
          __x11_XGetWMNormalHints    GetWMNormalHints_ = nullptr;
          __x11_XGetWindowAttributes GetWindowAttributes_ = nullptr;
          __x11_XGetWindowProperty   GetWindowProperty_ = nullptr;
          __x11_XGrabPointer         GrabPointer_ = nullptr;
          __x11_XIconifyWindow       IconifyWindow_ = nullptr;
          __x11_XInitThreads         InitThreads_ = nullptr;
          __x11_XInternAtom          InternAtom_ = nullptr;
          __x11_XLookupString        LookupString_ = nullptr;
          __x11_XMapRaised           MapRaised_ = nullptr;
          __x11_XMapWindow           MapWindow_ = nullptr;
          __x11_XMoveResizeWindow    MoveResizeWindow_ = nullptr;
          __x11_XMoveWindow          MoveWindow_ = nullptr;
          __x11_XNextEvent           NextEvent_ = nullptr;
          __x11_XOpenDisplay         OpenDisplay_ = nullptr;
          __x11_XOpenIM              OpenIM_ = nullptr;
          __x11_XPeekEvent           PeekEvent_ = nullptr;
          __x11_XPending             Pending_ = nullptr;
          __x11_XQueryExtension      QueryExtension_ = nullptr;
          __x11_XQueryPointer        QueryPointer_ = nullptr;
          __x11_XRaiseWindow         RaiseWindow_ = nullptr;
          __x11_XRegisterIMInstantiateCallback RegisterIMInstantiateCallback_ = nullptr;
          __x11_XResetScreenSaver    ResetScreenSaver_ = nullptr;
          __x11_XResizeWindow        ResizeWindow_ = nullptr;
          __x11_XResourceManagerString ResourceManagerString_ = nullptr;
          __x11_XSaveContext         SaveContext_ = nullptr;
          __x11_XSelectInput         SelectInput_ = nullptr;
          __x11_XSendEvent           SendEvent_ = nullptr;
          __x11_XSetClassHint        SetClassHint_ = nullptr;
          __x11_XSetErrorHandler     SetErrorHandler_ = nullptr;
          __x11_XSetICFocus          SetICFocus_ = nullptr;
          __x11_XSetIMValues         SetIMValues_ = nullptr;
          __x11_XSetInputFocus       SetInputFocus_ = nullptr;
          __x11_XSetLocaleModifiers  SetLocaleModifiers_ = nullptr;
          __x11_XSetScreenSaver      SetScreenSaver_ = nullptr;
          __x11_XSetSelectionOwner   SetSelectionOwner_ = nullptr;
          __x11_XSetWMHints          SetWMHints_ = nullptr;
          __x11_XSetWMNormalHints    SetWMNormalHints_ = nullptr;
          __x11_XSetWMProtocols      SetWMProtocols_ = nullptr;
          __x11_XSupportsLocale      SupportsLocale_ = nullptr;
          __x11_XSync                Sync_ = nullptr;
          __x11_XTranslateCoordinates TranslateCoordinates_ = nullptr;
          __x11_XUndefineCursor      UndefineCursor_ = nullptr;
          __x11_XUngrabPointer       UngrabPointer_ = nullptr;
          __x11_XUnmapWindow         UnmapWindow_ = nullptr;
          __x11_XUnsetICFocus        UnsetICFocus_ = nullptr;
          __x11_XVisualIDFromVisual  VisualIDFromVisual_ = nullptr;
          __x11_XWarpPointer         WarpPointer_ = nullptr;
          __x11_XkbFreeKeyboard      kbFreeKeyboard_ = nullptr;
          __x11_XkbFreeNames         kbFreeNames_ = nullptr;
          __x11_XkbGetMap            kbGetMap_ = nullptr;
          __x11_XkbGetNames          kbGetNames_ = nullptr;
          __x11_XkbGetState          kbGetState_ = nullptr;
          __x11_XkbKeycodeToKeysym   kbKeycodeToKeysym_ = nullptr;
          __x11_XkbQueryExtension    kbQueryExtension_ = nullptr;
          __x11_XkbSelectEventDetails kbSelectEventDetails_ = nullptr;
          __x11_XkbSetDetectableAutoRepeat kbSetDetectableAutoRepeat_ = nullptr;
          __x11_XrmDestroyDatabase   rmDestroyDatabase_ = nullptr;
          __x11_XrmGetResource       rmGetResource_ = nullptr;
          __x11_XrmGetStringDatabase rmGetStringDatabase_ = nullptr;
          __x11_XrmInitialize        rmInitialize_ = nullptr;
          __x11_XrmUniqueQuark       rmUniqueQuark_ = nullptr;
          __x11_XSetForeground       SetForeground_ = nullptr;
          __x11_XUnregisterIMInstantiateCallback UnregisterIMInstantiateCallback_ = nullptr;
          __x11_Xutf8DrawString      utf8DrawString_ = nullptr;
          __x11_Xutf8LookupString    utf8LookupString_ = nullptr;
          __x11_Xutf8SetWMProperties utf8SetWMProperties_ = nullptr;
          __x11_Xutf8TextExtents     utf8TextExtents_ = nullptr;
        } xlib;
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
          __x11_XRRGetProviderInfo     GetProviderInfo_ = nullptr;
          __x11_XRRFreeProviderInfo    FreeProviderInfo_ = nullptr;
          __x11_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent_ = nullptr;
          __x11_XRRFreeScreenResources FreeScreenResources_ = nullptr;
          __x11_XRRGetProviderResources GetProviderResources_ = nullptr;
          __x11_XRRFreeProviderResources FreeProviderResources_ = nullptr;
          __x11_XRRQueryExtension      QueryExtension_ = nullptr;
          __x11_XRRQueryVersion        QueryVersion_ = nullptr;
          __x11_XRRSelectInput         SelectInput_ = nullptr;
          __x11_XRRSetCrtcConfig       SetCrtcConfig_ = nullptr;
          __x11_XRRUpdateConfiguration UpdateConfiguration_ = nullptr;
        } randr;
        struct {
          bool      isAvailable  = false;
          LibHandle instance     = nullptr;
          int       versionMajor = 0;
          int       versionMinor = 0;
          int       eventBase    = 0;
          int       errorBase    = 0;
          __x11_XineramaIsActive       IsActive_ = nullptr;
          __x11_XineramaQueryExtension QueryExtension_ = nullptr;
          __x11_XineramaQueryVersion   QueryVersion_ = nullptr;
          __x11_XineramaQueryScreens   QueryScreens_ = nullptr;
        } xinerama;
        struct {
          bool isAvailable   = false;
          LibHandle instance = nullptr;
          __x11_XScreenSaverSuspend  ScreenSaverSuspend_ = nullptr;
        } xss;
        
        struct {
          // window manager
          Atom MOTIF_WM_HINTS;
          Atom NET_ACTIVE_WINDOW;
          Atom NET_CURRENT_DESKTOP;
          Atom NET_FRAME_EXTENTS;
          Atom NET_REQUEST_FRAME_EXTENTS;
          Atom NET_SUPPORTED;
          Atom NET_SUPPORTING_WM_CHECK;
          Atom NET_WORKAREA;
          Atom NET_WM_BYPASS_COMPOSITOR;
          Atom NET_WM_CM_SCREEN;
          Atom NET_WM_FULLSCREEN_MONITORS;
          Atom NET_WM_ICON_NAME;
          Atom NET_WM_ICON;
          Atom NET_WM_NAME;
          Atom NET_WM_PID;
          Atom NET_WM_PING;
          Atom NET_WM_STATE;
          Atom NET_WM_STATE_ABOVE;
          Atom NET_WM_STATE_DEMANDS_ATTENTION;
          Atom NET_WM_STATE_FULLSCREEN;
          Atom NET_WM_STATE_MAXIMIZED_HORZ;
          Atom NET_WM_STATE_MAXIMIZED_VERT;
          Atom NET_WM_WINDOW_OPACITY;
          Atom NET_WM_WINDOW_TYPE;
          Atom NET_WM_WINDOW_TYPE_DIALOG;
          Atom NET_WM_WINDOW_TYPE_NORMAL;
          Atom WM_PROTOCOLS;
          Atom WM_STATE;
          Atom WM_DELETE_WINDOW;
          // clipboard / selection
          Atom ATOM_PAIR;
          Atom CLIPBOARD;
          Atom CLIPBOARD_MANAGER;
          Atom COMPOUND_STRING;
          Atom INCR;
          Atom MULTIPLE;
          Atom NULL_ATOM;
          Atom PRIMARY;
          Atom SAVE_TARGETS;
          Atom TARGETS;
          Atom UTF8_STRING;
          // drag and drop
          Atom TextUriList;
          Atom XdndActionCopy;
          Atom XdndAware;
          Atom XdndDrop;
          Atom XdndEnter;
          Atom XdndFinished;
          Atom XdndPosition;
          Atom XdndSelection;
          Atom XdndStatus;
          Atom XdndTypeList;
        } atoms;


        // initialize available libraries
        void init();
        // close libraries
        void shutdown() noexcept;
        // enable DPI awareness
        void readSystemDpi() noexcept;
        // get simple atom value
        uint32_t getWindowProperty(Window window, Atom property, Atom type, unsigned char** value) noexcept;
        // get window manager atom value (with retries)
        uint32_t getWindowProperty(Window window, Atom property, Atom type, unsigned char** value, 
                                   uint32_t length, int32_t retryCount = 10) noexcept;
        
        // get global instance
        static inline LibrariesX11& instance() {
          if (!_libs._isInit)
            _libs.init();
          return _libs;
        }

      private:
        Atom _findAtom(Atom* atomArray, uint32_t arrayLength, const char* targetName) noexcept;
        
        void _bindXlib() noexcept;
        void _bindXrandr() noexcept;
        void _bindXinerama() noexcept;
        void _bindAtoms() noexcept;
      
      private:
        bool _isInit = false;
        static LibrariesX11 _libs;
      };
    }
  }
#endif
