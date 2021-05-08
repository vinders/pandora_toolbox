/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <cassert>
# include <cstdio>
# include <atomic>
# include <mutex>
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include <windowsx.h>
# include <shellapi.h>
# include <Dbt.h>
# include <system/force_inline.h>
# include "video/window_keycodes.h"
# include "video/window.h"

// -- constants --

# define __P_WINDOW_ID  L"PTWo" // instance ID for event processor (Pandora-Toolbox Window object)

# define __P_DEFAULT_WINDOW_WIDTH      800  // default width (if system work area can't be read)
# define __P_DEFAULT_WINDOW_HEIGHT     600  // default height (if system work area can't be read)
# define __P_DEFAULT_MIN_WINDOW_WIDTH  220  // min resizable width
# define __P_DEFAULT_MIN_WINDOW_HEIGHT 100  // min resizable height

# define __P_VK_EXTENDED_FLAG       0x01000000  // win32 extended key flag
# define __P_VK_KEY_PREV_DOWN_FLAG  0x40000000  // win32 previous key-down state flag
# define __P_VK_KEY_UP_FLAG         0x80000000  // win32 key-up state flag
# define __P_VK_KEY_SCANCODE_OFFSET 16


  namespace pandora {
    namespace video {
      // win32-specific impl class
      class __WindowImpl final {
      public:
        static LRESULT CALLBACK windowEventProcessor(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
        
        static __forceinline bool processKeyboardEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam);
        static __forceinline bool processMouseEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam);
        
        static void readCurrentWindowSize(Window& window, uint32_t& outWidth, uint32_t& outHeight) noexcept;

        struct StyleBackup {
          // original window resources
          HICON appIcon = nullptr;
          HICON captionIcon = nullptr;
          HCURSOR cursor = nullptr;
          HBRUSH backgroundColor = nullptr;
          HMENU menu = nullptr;
          // original window style
          RECT windowArea{ 0,0,0,0 };
          DWORD windowStyle = 0;
          DWORD windowStyleExt = 0;
          WNDPROC eventProcessor = nullptr;
        };

        StyleBackup originalStyle;
        bool hasOriginalStyleBackup = false;
        
        double initialRatio = 4.0/3.0;
        DWORD windowStyle = 0;
        DWORD windowStyleExt = 0;
        uint32_t pixelsPerScrollUnit = 1;
        uint32_t minWidth = __P_DEFAULT_MIN_WINDOW_WIDTH;
        uint32_t minHeight = __P_DEFAULT_MIN_WINDOW_HEIGHT;
        
        int32_t lastCursorPosX = -1;
        int32_t lastCursorPosY = -1;
        RAWINPUT* rawInputBuffer = nullptr;
        size_t rawInputBufferSize = 0;
      };

      // win32 decoration size on each side of a window
      struct WindowDecorationSize {
        int32_t left;
        int32_t right;
        int32_t top;
        int32_t bottom;
      };
    }
  }
  static std::atomic_int32_t g_windowCount{ 0 }; // global number of window instances

// -- window state flags --

# define __P_FLAG_USE_ORIG_EVENT_PROC 0x0001  // use event processor from "original style" backup as default
# define __P_FLAG_FIRST_DISPLAY_DONE  0x0002  // window has been displayed at least once (-> api methods available)
# define __P_FLAG_MODE_CHANGED        0x0004  // fullscreen currently with another display mode than system desktop
# define __P_FLAG_POWER_SUSPENDED     0x0008  // host system is locked or suspended

# define __P_FLAGS_VISIBILITY_MASK    0x01F0  // -- flags aligned with VisibleState enum values --
# define __P_FLAGS_VISIBILITY_OFFSET  4
# define __P_FLAG_WINDOW_VISIBLE      0x0010  // window is currently on screen (not hidden, not minimized)
# define __P_FLAG_WINDOW_ACTIVE       0x0020  // window is currently active
# define __P_FLAG_WINDOW_MINIMIZED    0x0040  // window is minimized
# define __P_FLAG_WINDOW_MAXIMIZED    0x0080  // window is maximized
# define __P_FLAG_reserved__          0x0100

# define __P_FLAG_MODE_CHANGE_PENDING 0x0200  // window display change has just occurred
# define __P_FLAG_RESIZED_MOVED       0x0400  // window is currently being resized or moved
# define __P_FLAG_CURSOR_RAW_INPUT    0x0800  // use raw move events for mouse
# define __P_FLAG_CURSOR_HIDDEN     0x1000  // cursor should be hidden when the window is active
# define __P_FLAG_CURSOR_HOVER        0x2000  // cursor is currently in client area
# define __P_FLAG_CURSOR_MENULOOP     0x4000  // menu loop is currently active
# define __P_FLAG_CURSOR_EVENT_REG    0x8000  // registration to native cursor events for current window
  
  using namespace pandora::video;
  using pandora::hardware::DisplayArea;
  using pandora::hardware::DisplayMode;
  using pandora::hardware::DisplayMonitor;


// -- API helpers -- -----------------------------------------------------------

  // get last error message
  static std::string __formatLastError(const char* prefix) {
    DWORD errorNb = ::GetLastError();
    if (errorNb != 0) {
      LPSTR buffer = nullptr;
      size_t length = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   nullptr, errorNb, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, nullptr);
      if (buffer != nullptr) {
        try {
          std::string message(buffer, length);
          LocalFree(buffer);
          return (prefix) ? std::string(prefix) + message : message;
        }
        catch (const std::bad_alloc&) { LocalFree(buffer); } // no leaks
      }
    }
    return (prefix) ? std::string(prefix) + "unknown error" : "";
  }

  // convert portable visibility to native window flag
  static inline int __toNativeWindowVisibility(Window::VisibilityCommand state, uint32_t currentFlags, uint32_t& outFlags) noexcept {
    switch(state) {
      case Window::VisibilityCommand::hide:
        outFlags = (currentFlags & ~((uint32_t)(__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_WINDOW_MINIMIZED)) );
        return SW_HIDE;
      case Window::VisibilityCommand::show:
        outFlags = ( (currentFlags | (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE)) & ~((uint32_t)__P_FLAG_WINDOW_MINIMIZED) );
        return SW_SHOW;
      case Window::VisibilityCommand::showInactive:
        outFlags = ( (currentFlags | __P_FLAG_WINDOW_VISIBLE) & ~((uint32_t)(__P_FLAG_WINDOW_MINIMIZED | __P_FLAG_WINDOW_ACTIVE)) );
        return SW_SHOWNA;
      case Window::VisibilityCommand::restore:
        outFlags = ( (currentFlags | (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE)) & ~((uint32_t)(__P_FLAG_WINDOW_MINIMIZED | __P_FLAG_WINDOW_MAXIMIZED)) );
        return SW_SHOWDEFAULT;
      case Window::VisibilityCommand::minimize:
        outFlags = ( (currentFlags | __P_FLAG_WINDOW_MINIMIZED) & ~((uint32_t)(__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE)) );
        return SW_SHOWMINNOACTIVE;
      case Window::VisibilityCommand::maximize:
        outFlags = ( (currentFlags | (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_WINDOW_MAXIMIZED)) & ~((uint32_t)__P_FLAG_WINDOW_MINIMIZED) );
        return SW_SHOWMAXIMIZED;
      default: 
        outFlags = ( (currentFlags | __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE) & ~((uint32_t)(__P_FLAG_WINDOW_MINIMIZED | __P_FLAG_WINDOW_MAXIMIZED)) );
        return SW_SHOWDEFAULT;
    }
  }
  // convert portable visibility to native fullscreen flag
  static inline int __toNativeFullscreenVisibility(Window::VisibilityCommand state, uint32_t currentFlags, uint32_t& outFlags) noexcept {
    switch(state) {
      case Window::VisibilityCommand::hide:
        outFlags = (currentFlags & ~((uint32_t)(__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_WINDOW_MINIMIZED)) );
        return SW_HIDE;
      case Window::VisibilityCommand::minimize:
        outFlags = ( (currentFlags | __P_FLAG_WINDOW_MINIMIZED) & ~((uint32_t)(__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE)) );
        return SW_SHOWMINNOACTIVE;
      default:
        outFlags = ( (currentFlags | (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_WINDOW_MAXIMIZED)) & ~((uint32_t)__P_FLAG_WINDOW_MINIMIZED) );
        return SW_SHOWMAXIMIZED;
    }
  }

  // convert portable flags to native window style
  static void __toNativeWindowStyle(WindowType type, WindowBehavior behavior, ResizeMode resizeMode, 
                                    DWORD& outWindowStyle, DWORD& outWindowStyleExt) noexcept {
    outWindowStyle = (WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    outWindowStyleExt = 0;
    
    // fullscreen style
    if (type == WindowType::fullscreen) {
      outWindowStyle |= WS_POPUP;
      outWindowStyleExt |= WS_EX_TOPMOST;
    }
    // window/dialog style
    else {
      switch (type) {
        case WindowType::borderless: 
          outWindowStyle |= WS_POPUP;
          break;
        case WindowType::bordered: 
          outWindowStyle |= (WS_POPUP | WS_DLGFRAME | WS_BORDER); 
          if ((resizeMode & ResizeMode::resizable) == true) // resizableX and/or resizableY
            outWindowStyle |= WS_SIZEBOX;
          break;
        case WindowType::dialog: 
          outWindowStyle |= (WS_POPUP | WS_CAPTION | WS_SYSMENU); 
          if ((resizeMode & ResizeMode::resizable) == true) // resizableX and/or resizableY
            outWindowStyle |= WS_SIZEBOX;
          break;
        case WindowType::window:
        default: 
          outWindowStyle |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
          if ((resizeMode & ResizeMode::resizable) == true) // resizableX and/or resizableY
            outWindowStyle |= (WS_MAXIMIZEBOX | WS_SIZEBOX);
          break;
      }
      if ((behavior & WindowBehavior::topMost) == true)
        outWindowStyleExt |= WS_EX_TOPMOST;
    }
    
    // common behaviors
    if ((behavior & WindowBehavior::scrollH) == true)
      outWindowStyle |= WS_HSCROLL;
    if ((behavior & WindowBehavior::scrollV) == true)
      outWindowStyle |= WS_VSCROLL;
    if ((behavior & WindowBehavior::aboveTaskbar) == true)
      outWindowStyleExt |= WS_EX_APPWINDOW;
  }

  // convert portable behavior to native window-class style
  static UINT __toNativeClassStyle(WindowBehavior behavior) noexcept {
    UINT classStyle = (CS_HREDRAW | CS_VREDRAW);
    if ((behavior & WindowBehavior::dropShadow) == true)
      classStyle |= CS_DROPSHADOW;
    if ((behavior & WindowBehavior::globalContext) == true)
      classStyle |= CS_OWNDC;
    
    return classStyle;
  }


// -- client area helpers -- ---------------------------------------------------

  // calculate expected size of window decorations (based on window style)
  static inline void __toNativeWindowDecoration(const DisplayMonitor& monitor, WindowType type, 
                                                DWORD windowStyle, DWORD windowStyleExt, bool hasMenu, 
                                                WindowDecorationSize& outSizes) noexcept {
    if (type == WindowType::fullscreen || type == WindowType::borderless) { // no decorations
      outSizes.left = outSizes.right = outSizes.top = outSizes.bottom = 0;
    }
    else { // window/dialog
      DisplayArea dummyClientArea { monitor.attributes().workArea.x + 16, monitor.attributes().workArea.y + 64, 
                                    monitor.attributes().workArea.width - 48, monitor.attributes().workArea.height - 80 };
      auto dummyWindowArea = monitor.convertClientAreaToWindowArea(dummyClientArea, (WindowHandle)0, hasMenu, 
                                                                   (uint32_t)windowStyle, (uint32_t)windowStyleExt);
      outSizes.left = dummyClientArea.x - dummyWindowArea.x;
      outSizes.top = dummyClientArea.y - dummyWindowArea.y;
      outSizes.right = (int32_t)dummyWindowArea.width - (int32_t)dummyClientArea.width - outSizes.left;
      outSizes.left = (int32_t)dummyWindowArea.height - (int32_t)dummyClientArea.height - outSizes.top;
    }
  }
  // convert user-defined client area coord to native coord
  static inline int32_t __toNativeClientCoord(int32_t userCoord, int32_t originCoord, int32_t maxCoord, uint32_t expectedClientAreaSize) noexcept {
    switch (userCoord) {
      case Window::Builder::defaultPosition(): return originCoord;
      case Window::Builder::centeredPosition(): return (originCoord + maxCoord - (int32_t)expectedClientAreaSize)/2;
      default: return userCoord;
    }
  }

  // read up-to-date window client area size/position (the window must have been displayed at least once)
  static bool __readCurrentClientArea(WindowHandle handle, DisplayArea& outClientArea) noexcept {
    RECT clientArea;
    if (GetClientRect((HWND)handle, &clientArea) != FALSE && clientArea.right > clientArea.left) { // get size
      outClientArea.width = (uint32_t)(clientArea.right - clientArea.left);
      outClientArea.height = (uint32_t)(clientArea.bottom - clientArea.top);
      
      POINT pos{ 0, 0 };
      if (ClientToScreen((HWND)handle, &pos) != FALSE) { // get position
        outClientArea.x = (uint32_t)pos.x;
        outClientArea.y = (uint32_t)pos.y;
        return true;
      }
    }
    return false;
  }
  // read window size based on current client area
  void __WindowImpl::readCurrentWindowSize(Window& window, uint32_t& outWidth, uint32_t& outHeight) noexcept {
    if (window._mode == WindowType::fullscreen) { // screen size
      std::lock_guard<pandora::thread::SpinLock> guard(window._clientAreaLock);
      outWidth = window._clientArea.width;
      outHeight = window._clientArea.height;
    }
    else { // window/dialog
      RECT windowPos;
      if (GetWindowRect((HWND)window._handle, &windowPos) != FALSE) {
        outWidth = windowPos.right - windowPos.left;
        outHeight = windowPos.bottom - windowPos.top;
      }
      else
        outWidth = 0;
      
      if (outWidth <= 0 || outHeight <= 0) {
        WindowDecorationSize decorationSizes;
        __toNativeWindowDecoration(*(window._monitor), window._mode, window._impl->windowStyle, window._impl->windowStyleExt, 
                                   (window._menuHandle != nullptr), decorationSizes);
                                   
        std::lock_guard<pandora::thread::SpinLock> guard(window._clientAreaLock);
        outWidth = window._clientArea.width + decorationSizes.left + decorationSizes.right;
        outHeight = window._clientArea.height + decorationSizes.top + decorationSizes.bottom;
      }
    }
  }
  
  // ---
  
  // convert user-defined client area to native absolute client + window areas
  static void __toNativeAbsoluteArea(WindowType displayMode, bool hasMenu, const DisplayArea& userArea,
                                     const DisplayMonitor& monitor, DWORD windowStyle, DWORD windowStyleExt,
                                     DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    // fullscreen -> defaults to screen size + ignore position
    if (displayMode == WindowType::fullscreen) {
      outWindowArea.x = outClientArea.x = monitor.attributes().screenArea.x;
      outWindowArea.y = outClientArea.y = monitor.attributes().screenArea.y;
      outWindowArea.width = outClientArea.width = (userArea.width != Window::Builder::defaultSize())
                                                ? userArea.width 
                                                : monitor.attributes().screenArea.width;
      outWindowArea.height = outClientArea.height = (userArea.height != Window::Builder::defaultSize())
                                                  ? userArea.height 
                                                  : monitor.attributes().screenArea.height;
    }
    // window/dialog -> defaults to work area
    else {
      WindowDecorationSize decorationSizes;
      __toNativeWindowDecoration(monitor, displayMode, windowStyle, windowStyleExt, hasMenu, decorationSizes);
      
      outClientArea.width = (userArea.width != Window::Builder::defaultSize()) 
                          ? userArea.width 
                          : monitor.attributes().workArea.width - (uint32_t)decorationSizes.left - (uint32_t)decorationSizes.right;
      outClientArea.height = (userArea.height != Window::Builder::defaultSize()) 
                           ? userArea.height 
                           : monitor.attributes().workArea.height - (uint32_t)decorationSizes.top - (uint32_t)decorationSizes.bottom;
      outWindowArea.width = outClientArea.width + (uint32_t)decorationSizes.left + (uint32_t)decorationSizes.right;
      outWindowArea.height = outClientArea.height + (uint32_t)decorationSizes.top + (uint32_t)decorationSizes.bottom;
      
      int32_t maxCoordX = monitor.attributes().workArea.x + (int32_t)monitor.attributes().workArea.width - decorationSizes.right;
      int32_t maxCoordY = monitor.attributes().workArea.y + (int32_t)monitor.attributes().workArea.height - decorationSizes.bottom;
      outClientArea.x = __toNativeClientCoord(userArea.x, monitor.attributes().workArea.x, maxCoordX, outClientArea.width);
      outClientArea.y = __toNativeClientCoord(userArea.y, monitor.attributes().workArea.y, maxCoordY, outClientArea.height);
      outWindowArea.x = outClientArea.x - decorationSizes.left;
      outWindowArea.y = outClientArea.y - decorationSizes.top;
    }
  }
  
  // convert user-defined child client area to native client area (absolute) + window area (relative to parent window)
  static void __toNativeChildArea(WindowHandle parentWindow, WindowType displayMode, bool hasMenu, const DisplayArea& userArea,
                                  const DisplayMonitor& monitor, DWORD windowStyle, DWORD windowStyleExt,
                                  DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    // get parent window size (window area)
    assert(parentWindow != nullptr);
    RECT parentArea;
    if (GetWindowRect((HWND)parentWindow, &parentArea) == 0) {
      // on failure, use work area
      parentArea.left = monitor.attributes().workArea.x;
      parentArea.right = monitor.attributes().workArea.x + (int32_t)monitor.attributes().workArea.width;
      parentArea.top = monitor.attributes().workArea.y;
      parentArea.bottom = monitor.attributes().workArea.y + (int32_t)monitor.attributes().workArea.height;
    }

    // fullscreen -> defaults to screen size + ignore position
    if (displayMode == WindowType::fullscreen) {
      outWindowArea.x = outClientArea.x = monitor.attributes().screenArea.x - (int32_t)parentArea.left;
      outWindowArea.y = outClientArea.y = monitor.attributes().screenArea.y - (int32_t)parentArea.top;
      outWindowArea.width = outClientArea.width = (userArea.width != Window::Builder::defaultSize())
                                                ? userArea.width 
                                                : monitor.attributes().screenArea.width;
      outWindowArea.height = outClientArea.height = (userArea.height != Window::Builder::defaultSize())
                                                  ? userArea.height 
                                                  : monitor.attributes().screenArea.height;
    }
    // window/dialog -> defaults to work area
    else {
      WindowDecorationSize decorationSizes;
      __toNativeWindowDecoration(monitor, displayMode, windowStyle, windowStyleExt, hasMenu, decorationSizes);
      
      outClientArea.width = (userArea.width != Window::Builder::defaultSize()) 
                          ? userArea.width 
                          : monitor.attributes().workArea.width - (uint32_t)decorationSizes.left - (uint32_t)decorationSizes.right;
      outClientArea.height = (userArea.height != Window::Builder::defaultSize()) 
                           ? userArea.height 
                           : monitor.attributes().workArea.height - (uint32_t)decorationSizes.top - (uint32_t)decorationSizes.bottom;
      outWindowArea.width = outClientArea.width + (uint32_t)decorationSizes.left + (uint32_t)decorationSizes.right;
      outWindowArea.height = outClientArea.height + (uint32_t)decorationSizes.top + (uint32_t)decorationSizes.bottom;
      
      outClientArea.x = __toNativeClientCoord(userArea.x, (int32_t)parentArea.left + decorationSizes.left, 
                                              (int32_t)parentArea.right - decorationSizes.right, outClientArea.width);
      outClientArea.y = __toNativeClientCoord(userArea.y, (int32_t)parentArea.top + decorationSizes.top, 
                                              (int32_t)parentArea.bottom - decorationSizes.bottom, outClientArea.height);
      outWindowArea.x = outClientArea.x - decorationSizes.left;
      outWindowArea.y = outClientArea.y - decorationSizes.top;
    }
    // to absolute client area
    outClientArea.x = outClientArea.x + (int32_t)parentArea.left;
    outClientArea.y = outClientArea.y + (int32_t)parentArea.top;
  }
  
  // ---
  
  // set scrollbar page size, after window resize
  void __adjustScrollbarPageSize(HWND handle, __WindowImpl& impl, const DisplayArea& clientArea) {
    SCROLLINFO scrollInfo;
    ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    
    uint32_t pixelsPerUnit = impl.pixelsPerScrollUnit;
    if (impl.windowStyle & WS_VSCROLL) {
      scrollInfo.fMask = SIF_PAGE;
      scrollInfo.nPage = clientArea.height / pixelsPerUnit;
      SetScrollInfo((HWND)handle, SB_VERT, &scrollInfo, TRUE);
    }
    if (impl.windowStyle & WS_HSCROLL) {
      scrollInfo.fMask = SIF_PAGE;
      scrollInfo.nPage = clientArea.width / pixelsPerUnit;
      SetScrollInfo((HWND)handle, SB_HORZ, &scrollInfo, TRUE);
    }
  }


// -- display helpers -- ----------------------------------------------------

  // initialize first display + verify client area
  bool Window::_validateFirstDisplay() noexcept {
    // fullscreen -> no adjustments required
    if (this->_mode == WindowType::fullscreen) {
      if (ShowWindow((HWND)this->_handle, SW_SHOWMAXIMIZED) == FALSE)
        return false;
      pollCurrentWindowEvents(); // 'show' event processing
      return true;
    }
    
    // window / dialog
    if (ShowWindow((HWND)this->_handle, SW_SHOW) == FALSE)
      return false;
    pollCurrentWindowEvents(); // 'show' event processing
    
    // verify client area -> if size is incorrect, fix it
    DisplayArea realClientArea;
    if (__readCurrentClientArea(this->_handle, realClientArea)) {
      this->_clientArea.x = realClientArea.x;
      this->_clientArea.y = realClientArea.y;

      RECT windowPos;
      if (GetWindowRect((HWND)this->_handle, &windowPos) != FALSE) {
        int diffX = (int)this->_clientArea.width - (int)realClientArea.width;
        int diffY = (int)this->_clientArea.height - (int)realClientArea.height;
        SetWindowPos((HWND)this->_handle, HWND_TOP, 0, 0, diffX + windowPos.right - windowPos.left, diffY + windowPos.bottom - windowPos.top, 
                      (SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW));
      }
      else if ((this->_resizeMode & ResizeMode::homothety) == true) {// if can't fix -> at least update value (unless homothety)
        std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
        this->_clientArea.width = realClientArea.width;
        this->_clientArea.height = realClientArea.height;
      }
    }
    return true;
  }
  
  // ---

  // go to fullscreen mode
  static bool __setFullscreenResolution(DisplayMonitor& monitor, WindowHandle handle, 
                                        DisplayArea clientArea, uint32_t rate, uint32_t& outFlags) {
    DisplayMode orig = monitor.getDisplayMode();
    SetWindowRgn((HWND)handle, nullptr, false); // disable window region without redraw
    
    // same resolution -> no change
    if (clientArea.width == orig.width && clientArea.height == orig.height && (rate == 0 || rate == orig.refreshRate)) {
      SetWindowPos((HWND)handle, nullptr, (int)monitor.attributes().screenArea.x,(int)monitor.attributes().screenArea.y, 0,0, SWP_NOSIZE | SWP_NOZORDER);
      InvalidateRect((HWND)handle, nullptr, true);
      return true;
    }
    
    // change resolution
    DisplayMode videoMode{ 0 };
    videoMode.width = clientArea.width;
    videoMode.height = clientArea.height;
    videoMode.bitDepth = 32;
    videoMode.refreshRate = rate;
    if (!monitor.setDisplayMode(videoMode))
      return false;
    
    // set position to origin
    outFlags |= __P_FLAG_MODE_CHANGE_PENDING;
    SetWindowPos((HWND)handle, nullptr, (int)monitor.attributes().screenArea.x,(int)monitor.attributes().screenArea.y, 0,0, SWP_NOSIZE | SWP_NOZORDER);
    InvalidateRect((HWND)handle, nullptr, true);
    outFlags |= __P_FLAG_MODE_CHANGED;
    return true;
  }

  // revert to desktop mode
  static void __resetDesktopResolution(DisplayMonitor& monitor, WindowHandle handle, uint32_t& inOutFlags) {
    // if currently in different resolution, reset to desktop resolution
    if (inOutFlags & __P_FLAG_MODE_CHANGED) {
      monitor.setDefaultDisplayMode();
      inOutFlags = (inOutFlags & ~((uint32_t)__P_FLAG_MODE_CHANGED) );
      inOutFlags |= __P_FLAG_MODE_CHANGE_PENDING;
      
      InvalidateRect((HWND)handle, nullptr, true);
    }
  }


// -- event management -- ------------------------------------------------------

  static bool _isAppAlive = true;

  // Process/forward pending events for all existing windows (user input, size changes, shutdown...).
  bool Window::pollEvents() noexcept {
    MSG msg;
    if (_isAppAlive) {
      while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT && g_windowCount <= 0) {
          _isAppAlive = false;
          break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      }
    }
    return _isAppAlive;
  }
  // Process/forward pending events for current window only (user input, size changes, shutdown...).
  bool Window::pollCurrentWindowEvents() noexcept {
    bool _isAlive = (this->_handle != nullptr);
    MSG msg;
    if (_isAlive) {
      while (PeekMessageW(&msg, (HWND)this->_handle, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
          _isAlive = false;
          break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      }
    }
    return _isAlive;
  }
  
  // ---
  
  // cursor in client area: hide cursor / enable raw mouse events
  static void __enableCursorMode(WindowHandle handle, __WindowImpl& impl, uint32_t& inOutFlag) noexcept {
    if (inOutFlag & __P_FLAG_CURSOR_RAW_INPUT) {
      const RAWINPUTDEVICE device = { 0x01, 0x02, 0, (HWND)handle };
      if (!RegisterRawInputDevices(&device, 1, sizeof(device))) {
        fprintf(stderr, "Window: failed to enable raw cursor input mode");
        return;
      }
      
      POINT cursorPos;
      if (GetCursorPos(&cursorPos) && ScreenToClient((HWND)handle, &cursorPos)) {
        impl.lastCursorPosX = cursorPos.x;
        impl.lastCursorPosY = cursorPos.y;
      }
    }
    if (inOutFlag & __P_FLAG_CURSOR_HIDDEN)
      ShowCursor(false);
  }
  // cursor out of range: show cursor / disable raw mouse events
  static void __disableCursorMode(uint32_t& inOutFlag) noexcept {
    if (inOutFlag & __P_FLAG_CURSOR_RAW_INPUT) {
      const RAWINPUTDEVICE device = { 0x01, 0x02, RIDEV_REMOVE, nullptr };
      RegisterRawInputDevices(&device, 1, sizeof(device));
    }
    if (inOutFlag & __P_FLAG_CURSOR_HIDDEN)
      ShowCursor(true);
  }
  
  // set/replace mouse event handler (NULL to unregister)
  void Window::setMouseHandler(MouseEventHandler handler, CursorMode cursor) noexcept {
    this->_onMouseEvent = handler;
    if (this->_nativeFlag & __P_FLAG_CURSOR_HOVER)
      __disableCursorMode(this->_nativeFlag);
    
    if (handler) {
      switch (cursor) {
        case CursorMode::hidden:
          this->_nativeFlag = ((this->_nativeFlag | __P_FLAG_CURSOR_HIDDEN) & ~((uint32_t)__P_FLAG_CURSOR_RAW_INPUT)); break;
        case CursorMode::hiddenRaw:
          this->_nativeFlag |= (__P_FLAG_CURSOR_HIDDEN | __P_FLAG_CURSOR_RAW_INPUT); break;
        case CursorMode::visible:
        default:
          this->_nativeFlag = (this->_nativeFlag & ~((uint32_t)(__P_FLAG_CURSOR_HIDDEN | __P_FLAG_CURSOR_RAW_INPUT))); break;
      }
      if (this->_nativeFlag & __P_FLAG_CURSOR_HOVER)
        __enableCursorMode(this->_handle, *(this->_impl), this->_nativeFlag);
    }
    else
      this->_nativeFlag = (this->_nativeFlag & ~((uint32_t)(__P_FLAG_CURSOR_HIDDEN | __P_FLAG_CURSOR_RAW_INPUT)));
  }
  
  // ---
  
  // process all keyboard events
  __forceinline bool __WindowImpl::processKeyboardEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam) {
    KeyboardEvent eventType = KeyboardEvent::none;
    uint32_t keyCode = 0;
    uint32_t value = 0;
    
    switch (message) {
      // key pressed/released
      case WM_KEYDOWN:    
      case WM_KEYUP:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP: {
        eventType = (message <= WM_KEYUP) ? KeyboardEvent::keyChange : KeyboardEvent::altKeyChange;
        if ((lParam & __P_VK_KEY_UP_FLAG) == 0) // down
          value = (lParam & __P_VK_KEY_PREV_DOWN_FLAG) ? (uint32_t)KeyTransition::same : (uint32_t)KeyTransition::down;
        else // up
          value = (uint32_t)KeyTransition::up;
        
        switch ((int)wParam) {
          case VK_F10: { // F10 key -> might be activation of menu
            if (message == WM_SYSKEYDOWN && window->_menuHandle)
              eventType = KeyboardEvent::activateMenu;
            keyCode = (uint32_t)wParam;
            break;
          }
          case VK_SNAPSHOT: { // VK_SNAPSHOT doesn't send WM_KEYDOWN events -> create them
            keyCode = (uint32_t)wParam;
            if (value == (uint32_t)KeyTransition::up) 
              window->_onKeyboardEvent(eventType, keyCode, (uint32_t)KeyTransition::down);
            break;
          }
          // Alt/Enter/Shift/Ctrl -> identify left/right
          case VK_MENU:    keyCode = (lParam & __P_VK_EXTENDED_FLAG) ? VK_RCONTROL : VK_LCONTROL; break;
          case VK_RETURN:  keyCode = (lParam & __P_VK_EXTENDED_FLAG) ? VK_RETURN : _P_VK_ENTER_PAD; break;
          case VK_SHIFT:   keyCode = (uint32_t)MapVirtualKey( (((UINT)lParam >> __P_VK_KEY_SCANCODE_OFFSET) & 0xFFu), MAPVK_VSC_TO_VK_EX); break;
          case VK_CONTROL: {
            if (lParam & __P_VK_EXTENDED_FLAG)
              keyCode = VK_RCONTROL;
            else {
              keyCode = VK_LCONTROL; 
              
              MSG next; // alt-Gr == Ctrl + R-alt -> hide Ctrl if part of alt-Gr
              const DWORD time = GetMessageTime();
              if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE)) {
                if ((next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN) && next.wParam == VK_MENU && (next.lParam & __P_VK_EXTENDED_FLAG) && next.time == time)
                  eventType = KeyboardEvent::none; // do not expose Ctrl part
              }
            }
            break;
          }
          case VK_PROCESSKEY: eventType = KeyboardEvent::none; break; // IME key filtering notification -> do not expose
          default: keyCode = (uint32_t)wParam; break; // normal key
        }
        break;
      }
      // text character input
      case WM_CHAR:
      case WM_SYSCHAR: {
        if ((lParam & __P_VK_KEY_UP_FLAG) == 0) { // down only
          eventType = KeyboardEvent::charInput;
          keyCode = (uint32_t)wParam;
          value = (uint32_t)(lParam & 0xFF);
        }
        break;
      }
    }
    return (eventType != KeyboardEvent::none && window->_onKeyboardEvent(eventType, keyCode, value));
  }
  
  // ---
  
  // process general mouse events (note: not WM_MOUSELEAVE/WM_MOUSEHOVER/WM_INPUT -> located outside of [WM_MOUSEFIRST;WM_MOUSELAST])
  __forceinline bool __WindowImpl::processMouseEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam) {
    MouseEvent eventType = MouseEvent::none;
    uint32_t value = 0;
    uint8_t activeKeys = 0;
    int32_t x = (int32_t)GET_X_LPARAM(lParam);
    int32_t y = (int32_t)GET_Y_LPARAM(lParam);
    
    switch (message) {
      case WM_MOUSEMOVE: { // TME_LEAVE is unregistered when the cursor leaves the window -> register when it comes back
        if ((window->_nativeFlag & __P_FLAG_CURSOR_EVENT_REG) == 0) {
          TRACKMOUSEEVENT tme;
          ZeroMemory(&tme, sizeof(tme));
          tme.cbSize = sizeof(tme);
          tme.dwFlags = TME_LEAVE | TME_HOVER;
          tme.dwHoverTime = HOVER_DEFAULT;
          tme.hwndTrack = (HWND)window->_handle;
          TrackMouseEvent(&tme);
          
          window->_nativeFlag |= __P_FLAG_CURSOR_EVENT_REG;
        }
        if (window->_nativeFlag & __P_FLAG_CURSOR_RAW_INPUT)
          return false; // do not expose "normal" move events in raw mode
        
        eventType = MouseEvent::mouseMove; activeKeys = (uint8_t)wParam;
        break;
      }
      // button "value" aligned with MouseButton enum values
      case WM_LBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = 0; activeKeys = (uint8_t)wParam; break;
      case WM_LBUTTONUP:     eventType = MouseEvent::buttonUp;     value = 0; activeKeys = (uint8_t)wParam; break;
      case WM_LBUTTONDBLCLK: eventType = MouseEvent::buttonDouble; value = 0; activeKeys = (uint8_t)wParam; break;
      case WM_MBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = 1; activeKeys = (uint8_t)wParam; break;
      case WM_MBUTTONUP:     eventType = MouseEvent::buttonUp;     value = 1; activeKeys = (uint8_t)wParam; break;
      case WM_MBUTTONDBLCLK: eventType = MouseEvent::buttonDouble; value = 1; activeKeys = (uint8_t)wParam; break;
      case WM_RBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = 2; activeKeys = (uint8_t)wParam; break;
      case WM_RBUTTONUP:     eventType = MouseEvent::buttonUp;     value = 2; activeKeys = (uint8_t)wParam; break;
      case WM_RBUTTONDBLCLK: eventType = MouseEvent::buttonDouble; value = 2; activeKeys = (uint8_t)wParam; break;
      case WM_XBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_XBUTTONUP:     eventType = MouseEvent::buttonUp;     value = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_XBUTTONDBLCLK: eventType = MouseEvent::buttonDouble; value = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_MOUSEWHEEL:    eventType = MouseEvent::mouseWheelV; value = (int32_t)GET_WHEEL_DELTA_WPARAM(wParam);
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_MOUSEHWHEEL:   eventType = MouseEvent::mouseWheelH; value = (int32_t)GET_WHEEL_DELTA_WPARAM(wParam);
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
    }
    return (eventType != MouseEvent::none && window->_onMouseEvent(eventType, x, y, value, activeKeys));
  }
  
  // ---

  // main window event processor
  LRESULT CALLBACK __WindowImpl::windowEventProcessor(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    Window* window = reinterpret_cast<Window*>(GetPropW(handle, __P_WINDOW_ID));
    if (window == nullptr)
      return DefWindowProcW(handle, message, wParam, lParam); // system window processor

    // > mouse events (note: missing WM_MOUSELEAVE and WM_INPUT: see "else" section) --
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
      if (window->_onMouseEvent && __WindowImpl::processMouseEvent(window, message, wParam, lParam))
        return 0; // handler said to ignore system processing
    }
    // > keyboard events --
    else if (message <= WM_KEYLAST && message >= WM_KEYFIRST) {
      if (window->_onKeyboardEvent && __WindowImpl::processKeyboardEvent(window, message, wParam, lParam))
        return 0; // handler said to ignore system processing
    }
    else {
      switch (message) {
        // > custom mouse events
        case WM_MOUSEHOVER: { // mouse enters client area
          if ((window->_nativeFlag & __P_FLAG_CURSOR_HOVER) == 0) {
            window->_nativeFlag |= __P_FLAG_CURSOR_HOVER;
            if ((window->_nativeFlag & __P_FLAG_CURSOR_MENULOOP) == 0)
              __enableCursorMode(window->_handle, *(window->_impl), window->_nativeFlag);
          }
          break;
        }
        case WM_MOUSELEAVE: { // mouse leaves client area (-> unregisters TME_LEAVE)
          if ((window->_nativeFlag & (__P_FLAG_CURSOR_HOVER|__P_FLAG_CURSOR_MENULOOP)) == __P_FLAG_CURSOR_HOVER)
            __disableCursorMode(window->_nativeFlag);
        
          window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_CURSOR_EVENT_REG | __P_FLAG_CURSOR_HOVER) );
          if (window->_onMouseEvent && window->_onMouseEvent(MouseEvent::mouseLeave, 0, 0, 0, 0))
            return 0;
          break;
        }
        case WM_ENTERMENULOOP: { // menu loop entered
          if (window->_nativeFlag & __P_FLAG_CURSOR_HOVER)
            __disableCursorMode(window->_nativeFlag);

          window->_nativeFlag |= __P_FLAG_CURSOR_MENULOOP;
          break;
        }
        case WM_EXITMENULOOP: { // menu loop left
          if (window->_nativeFlag & __P_FLAG_CURSOR_HOVER)
            __enableCursorMode(window->_handle, *(window->_impl), window->_nativeFlag);
          
          window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_CURSOR_MENULOOP));
          break;
        }
        // raw mouse moves
        case WM_INPUT: {
          if (window->_onMouseEvent && (window->_nativeFlag & __P_FLAG_CURSOR_RAW_INPUT)) {
            if (window->_impl->lastCursorPosX == -1) {
              POINT pos;
              if (GetCursorPos(&pos) && ScreenToClient((HWND)window->_handle, &pos)) {
                window->_impl->lastCursorPosX = pos.x;
                window->_impl->lastCursorPosY = pos.y;
              }
            }
            
            UINT rawSize = 0;
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &rawSize, sizeof(RAWINPUTHEADER)) == 0) {
              //adjust buffer size
              if (rawSize > window->_impl->rawInputBufferSize) {
                if (window->_impl->rawInputBuffer)
                  free(window->_impl->rawInputBuffer);
                
                window->_impl->rawInputBuffer = (RAWINPUT*)calloc(1, rawSize);
                if (window->_impl->rawInputBuffer == nullptr) {
                  window->_impl->rawInputBufferSize = 0;
                  fprintf(stderr, "Window: raw mouse input data allocation failure");
                  break;
                }
                window->_impl->rawInputBufferSize = rawSize;
              }
              
              if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window->_impl->rawInputBuffer, &rawSize, sizeof(RAWINPUTHEADER)) != (UINT)-1) {
                RAWINPUT& rawData = *(window->_impl->rawInputBuffer);
                if (rawData.header.dwType != RIM_TYPEMOUSE)
                  break; // not a mouse event
                
                int32_t deltaX, deltaY;
                if (rawData.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
                  deltaX = rawData.data.mouse.lLastX - window->_impl->lastCursorPosX;
                  deltaY = rawData.data.mouse.lLastY - window->_impl->lastCursorPosY;
                  window->_impl->lastCursorPosX = rawData.data.mouse.lLastX;
                  window->_impl->lastCursorPosY = rawData.data.mouse.lLastY;
                }
                else { // relative
                  deltaX = rawData.data.mouse.lLastX;
                  deltaY = rawData.data.mouse.lLastY;
                  window->_impl->lastCursorPosX += deltaX;
                  window->_impl->lastCursorPosY += deltaY;
                }
                
                window->_onMouseEvent(MouseEvent::rawMotion, deltaX, deltaY, 0, 0);
              }
              else
                fprintf(stderr, "Window: failed to obtain raw input data");
            }
            else
              fprintf(stderr, "Window: failed to obtain raw input data size");
            break;
          }
          break;
        }

        // > window size/move/scroll events --
        
        // manual user move/resize beginning
        case WM_ENTERSIZEMOVE: {
          if ((window->_resizeMode & ResizeMode::resizable) == true)
            window->_nativeFlag |= __P_FLAG_RESIZED_MOVED;
          else
            return 0;
          break;
        }
        // manuel user move/resize complete
        case WM_EXITSIZEMOVE: {
          if ((window->_resizeMode & ResizeMode::resizable) == true) {
            DisplayArea newClientArea;
            __readCurrentClientArea(window->_handle, newClientArea);
            bool hasChanged = false;
            { 
              std::lock_guard<pandora::thread::SpinLock> guard(window->_clientAreaLock);
              hasChanged = (window->_clientArea.x != newClientArea.x || window->_clientArea.y != newClientArea.y 
                         || window->_clientArea.width != newClientArea.width || window->_clientArea.height != newClientArea.height);
              window->_clientArea = newClientArea; 
            }
            
            window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_RESIZED_MOVED));
            if (hasChanged) {
              if (window->_impl->windowStyle & (WS_VSCROLL | WS_HSCROLL))
                __adjustScrollbarPageSize(handle, *(window->_impl), newClientArea);
              
              if (window->_onWindowEvent)
                window->_onWindowEvent(WindowEvent::sizePositionChanged, 0, newClientArea.x, newClientArea.y, 
                                       newClientArea.width, (uint64_ptr)newClientArea.height);
            }
          }
          break;
        }
        // window is moving
        case WM_MOVE: {
          if (window->_onWindowEvent) {
            uint32_t width = 0, height = 0;
            { std::lock_guard<pandora::thread::SpinLock> guard(window->_clientAreaLock);
              width = window->_clientArea.width;
              height = window->_clientArea.height;
            } // unlock before calling handler
            
            if (window->_nativeFlag & __P_FLAG_RESIZED_MOVED) // user move tracking
              window->_onWindowEvent(WindowEvent::sizePositionTrack, 0, static_cast<int32_t>((int16_t)LOWORD(lParam)), 
                                     static_cast<int32_t>((int16_t)HIWORD(lParam)), width, (uint64_ptr)height);
            else // system move done
              window->_onWindowEvent(WindowEvent::sizePositionChanged, 0, static_cast<int32_t>((int16_t)LOWORD(lParam)), 
                                     static_cast<int32_t>((int16_t)HIWORD(lParam)), width, (uint64_ptr)height);
          }
          break;
        }
        // window is resized
        case WM_SIZING: {
          if ((window->_resizeMode & ResizeMode::resizable) == true) {
            RECT* area = (RECT*)lParam;
            if (area != nullptr) {
              uint32_t width = static_cast<uint32_t>(area->right - area->left);
              uint32_t height = static_cast<uint32_t>(area->bottom - area->top);

              // homothety: force size constraints
              BOOL isUpdated = FALSE;
              if ((window->_resizeMode & ResizeMode::homothety) == true) {
                isUpdated = TRUE;
                switch ((int)wParam) {
                  case WMSZ_TOPLEFT:
                  case WMSZ_TOPRIGHT: area->top = area->bottom - static_cast<int>((double)width / window->_impl->initialRatio + 0.50001); break;
                  case WMSZ_LEFT:
                  case WMSZ_RIGHT:
                  case WMSZ_BOTTOMLEFT:
                  case WMSZ_BOTTOMRIGHT: area->bottom = area->top + static_cast<int>((double)width / window->_impl->initialRatio + 0.50001); break;
                  case WMSZ_TOP:
                  case WMSZ_BOTTOM: area->right = area->left + static_cast<int>(window->_impl->initialRatio * (double)height + 0.50001); break;
                }
              }

              // all modes: handler size tracking
              if (window->_nativeFlag & __P_FLAG_RESIZED_MOVED)
                window->_onWindowEvent(WindowEvent::sizePositionTrack, 0, (int32_t)area->left, (int32_t)area->top, 
                                       width, (uint64_ptr)height);
              if (isUpdated)
                return TRUE;
            }
          }
          break;
        }
        // resized with default action (restore/maximize/minimize)
        case WM_SIZE: {
          bool hasSizeChanged = false;
          DisplayArea newClientArea{ 0 };
          switch (wParam) {
            case SIZE_MINIMIZED: {
              window->_nativeFlag = ((window->_nativeFlag | __P_FLAG_WINDOW_MINIMIZED) 
                                  & ~((uint32_t)(__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_RESIZED_MOVED)));

              SendMessage(handle, WM_MOUSELEAVE, 0, 0);
              if (window->_onWindowEvent)
                window->_onWindowEvent(WindowEvent::stateChanged, (uint32_t)WindowVisibleActive::hidden, 0,0,0,0);
              break;
            }
            case SIZE_MAXIMIZED: {
              hasSizeChanged = true;
              bool wasMinimized = (window->_nativeFlag & __P_FLAG_WINDOW_MINIMIZED);
              window->_nativeFlag = ((window->_nativeFlag | __P_FLAG_WINDOW_MAXIMIZED | __P_FLAG_WINDOW_VISIBLE) 
                                  & ~((uint32_t)(__P_FLAG_WINDOW_MINIMIZED | __P_FLAG_RESIZED_MOVED)));
              __readCurrentClientArea(window->_handle, newClientArea);
              if (window->_onWindowEvent && wasMinimized) {
                if (wasMinimized)
                  window->_onWindowEvent(WindowEvent::stateChanged, (window->_nativeFlag & __P_FLAG_WINDOW_ACTIVE) 
                                                                   ? (uint32_t)WindowVisibleActive::active 
                                                                   : (uint32_t)WindowVisibleActive::inactive, 0,0,0,0);
              }
              break;
            }
            case SIZE_RESTORED: {
              hasSizeChanged = true;
              bool wasMinimized = (window->_nativeFlag & __P_FLAG_WINDOW_MINIMIZED);
              window->_nativeFlag = ((window->_nativeFlag | __P_FLAG_WINDOW_VISIBLE) & ~((uint32_t)(__P_FLAG_WINDOW_MINIMIZED | __P_FLAG_WINDOW_MAXIMIZED | __P_FLAG_RESIZED_MOVED)));
              __readCurrentClientArea(window->_handle, newClientArea);
              if (window->_onWindowEvent && wasMinimized) {
                if (wasMinimized)
                  window->_onWindowEvent(WindowEvent::stateChanged, (window->_nativeFlag & __P_FLAG_WINDOW_ACTIVE) 
                                                                   ? (uint32_t)WindowVisibleActive::active 
                                                                   : (uint32_t)WindowVisibleActive::inactive, 0,0,0,0);
              }
              break;
            }
          }

          if (hasSizeChanged) {
            if (window->_impl->windowStyle & (WS_VSCROLL | WS_HSCROLL))
              __adjustScrollbarPageSize(handle, *(window->_impl), newClientArea);
            
            { std::lock_guard<pandora::thread::SpinLock> guard(window->_clientAreaLock);
              window->_clientArea = newClientArea; } // unlock before calling handler
            
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::sizePositionChanged, 0, newClientArea.x, newClientArea.y, (uint32_t)LOWORD(lParam), (uint64_ptr)HIWORD(lParam));
          }
          break;
        }
        // size limits
        case WM_GETMINMAXINFO: {
          if (window->_mode != WindowType::fullscreen) {
            MINMAXINFO* limits = (MINMAXINFO*)lParam;
            if (limits != nullptr) {
              uint32_t currentWidth = 0, currentHeight = 0;
              __WindowImpl::readCurrentWindowSize(*window, currentWidth, currentHeight);
              
              {// set minimum size for window (add window decoration to min client size)
                std::lock_guard<pandora::thread::SpinLock> guard(window->_clientAreaLock);
                limits->ptMinTrackSize.x = (int)window->_impl->minWidth + (int)currentWidth - (int)window->_clientArea.width;
                limits->ptMinTrackSize.y = (int)window->_impl->minHeight + (int)currentHeight - (int)window->_clientArea.height;
              }
              
              // homothety: if not both dimensions resizable, none can be resized
              if ((window->_resizeMode & ResizeMode::homothety) == true) {
                if ((window->_resizeMode & ResizeMode::resizable) != ResizeMode::resizable) {
                  limits->ptMaxSize.x = limits->ptMinTrackSize.x = limits->ptMaxTrackSize.x = (int)currentWidth;
                  limits->ptMaxSize.y = limits->ptMinTrackSize.y = limits->ptMaxTrackSize.y = (int)currentHeight;
                }
              }
              else { // normal resize limits
                if ((window->_resizeMode & ResizeMode::resizableX) == 0)
                  limits->ptMaxSize.x = limits->ptMinTrackSize.x = limits->ptMaxTrackSize.x = (int)currentWidth;
                if ((window->_resizeMode & ResizeMode::resizableY) == 0)
                  limits->ptMaxSize.y = limits->ptMinTrackSize.y = limits->ptMaxTrackSize.y = (int)currentHeight;
              }
              return 0;
            }
          }
          break;
        }

        // scrollbar slider change
        case WM_VSCROLL:
        case WM_HSCROLL: { 
          if (window->_onWindowEvent) {
            if (LOWORD(wParam) == SB_THUMBTRACK) {
              uint32_t pos = (uint32_t)HIWORD(wParam);
              if (window->_onWindowEvent((message == WM_VSCROLL) ? WindowEvent::scrollPositionVTrack 
                                                                 : WindowEvent::scrollPositionHTrack, 0, pos, pos, 0, 0))
                return 0;
            }
            else if (LOWORD(wParam) == SB_THUMBPOSITION) {
              uint32_t pos = (uint32_t)HIWORD(wParam);
              if (window->_onWindowEvent((message == WM_VSCROLL) ? WindowEvent::scrollPositionVChanged 
                                                                 : WindowEvent::scrollPositionHChanged, 0, pos, pos, 0, 0))
                return 0;
            }
          }
          break;
        }
        
        // > general window events --

        // show/hide window
        case WM_SHOWWINDOW: {
          if (wParam == TRUE) { // visible
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::stateChanged, (window->_nativeFlag & __P_FLAG_WINDOW_ACTIVE) 
                                                               ? (uint32_t)WindowVisibleActive::active 
                                                               : (uint32_t)WindowVisibleActive::inactive, 0,0,0,0);
            window->_nativeFlag |= __P_FLAG_WINDOW_VISIBLE;
          }
          else {
            window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_WINDOW_VISIBLE));
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::stateChanged, (uint32_t)WindowVisibleActive::hidden, 0,0,0,0);
          }
          break;
        }
        // current/other window selected, or ALT+TAB to/from other window
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP: {
          if (wParam) { // active
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::stateChanged, (uint32_t)WindowVisibleActive::active, 0,0,0,0);
            window->_nativeFlag |= __P_FLAG_WINDOW_ACTIVE;
            
            std::lock_guard<pandora::thread::SpinLock> guard(window->_clientAreaLock);
            __readCurrentClientArea(window->_handle, window->_clientArea);
          }
          else { // inactive
            window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_WINDOW_ACTIVE));
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::stateChanged, (window->_nativeFlag & __P_FLAG_WINDOW_VISIBLE) 
                                                               ? (uint32_t)WindowVisibleActive::inactive 
                                                               : (uint32_t)WindowVisibleActive::hidden, 0,0,0,0);
            
            if (window->_mode == WindowType::fullscreen && (window->_nativeFlag & __P_FLAG_MODE_CHANGED)) // if fullscreen, minimize
              SendMessage(handle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
          }
          break;
        }
        // window decoration painting -> prevent if no decoration
        case WM_NCACTIVATE:
        case WM_NCPAINT: {
          if (window->_mode == WindowType::fullscreen || window->_mode == WindowType::borderless)
            return TRUE;
          break;
        }
        
        // menu shortcut
        case WM_MENUCHAR: {
          if (window->_menuHandle == nullptr) // if no menu handler -> invalid mnemonic or accelerator key -> avoid beep
            return MAKELRESULT(0, MNC_CLOSE);
          break;
        }
        // menu selection
        case WM_MENUSELECT: {
          if (window->_menuHandle == nullptr)
            window->_onWindowEvent(WindowEvent::menuSelected, (HIWORD(wParam) == MF_SYSMENU) ? 1 : 0, 0,0, (uint32_t)LOWORD(wParam), (uint64_ptr)lParam);
          break;
        }

        // window close attempt -> notify handler
        case WM_CLOSE: {
          if (!window->_onWindowEvent || !window->_onWindowEvent(WindowEvent::windowClosed, 0, 0,0,0,0))
            window->_destroyWindow();
          return 0;
        }
        case WM_DESTROY: { // close success -> confirmation message
          PostQuitMessage(0);
          return 0;
        }
        
        // drag and drop
        case WM_DROPFILES: {
          if (window->_onWindowEvent) {
            uint32_t count = (uint32_t)DragQueryFileW((HDROP)wParam, 0xFFFFFFFF, nullptr, 0);
            wchar_t** paths = (wchar_t**)calloc(count, sizeof(wchar_t*));
            if (paths == nullptr)
              return 0;
            
            POINT dropPos;
            DragQueryPoint((HDROP)wParam, &dropPos);
            
            for (uint32_t i = 0; i < count; ++i) {
              UINT length = DragQueryFileW((HDROP)wParam, i, nullptr, 0);
              paths[i] = (wchar_t*)calloc((size_t)length + 1, sizeof(wchar_t));
              if (paths[i] == nullptr)
                return 0;

              DragQueryFileW((HDROP)wParam, i, paths[i], length + 1);
            }
            
            // notify handler
            window->_onWindowEvent(WindowEvent::dropFiles, 0, (uint32_t)dropPos.x, (uint32_t)dropPos.y, count, (uint64_ptr)paths);
            DragFinish((HDROP)wParam);
            
            for (uint32_t i = 0; i < count; ++i)
              free(paths[i]);
            free(paths);
            return 0;
          }
          break;
        }
        
        // > hardware events --
        
        // display resolution change
        case WM_DISPLAYCHANGE: { 
          if (window->_nativeFlag & __P_FLAG_MODE_CHANGE_PENDING) { // change made by the window -> just reset flag
            window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_MODE_CHANGE_PENDING));
          }
          else { // external change -> verify if not just another fullscreen window
            RECT fgArea;
            MONITORINFO monitorInfo = { 0 };
            monitorInfo.cbSize = sizeof(MONITORINFO);
            HWND fgWindow = GetForegroundWindow();
            if (!fgWindow || !GetMonitorInfo(MonitorFromWindow((HWND)window->_handle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo)
            || !GetWindowRect(fgWindow, &fgArea) 
            || fgArea.left != monitorInfo.rcMonitor.left || fgArea.top != monitorInfo.rcMonitor.top
            || fgArea.right != monitorInfo.rcMonitor.right || fgArea.bottom != monitorInfo.rcMonitor.bottom) {
              // not a fullscreen window -> try to update monitor info
              try { 
                auto updatedMonitor = std::make_shared<DisplayMonitor>(window->_monitor->attributes().id, false);
                window->_monitor = updatedMonitor;
              }
              catch (...) {}
            }
          }
          break;
        }
        // display DPI change
        case WM_DPICHANGED: {
          RECT* suggestedArea = (RECT*)lParam;
          if (suggestedArea != nullptr) {
            // find appropriate window monitor
            bool isSameMonitor = true;
            int32_t centerX = suggestedArea->left + (suggestedArea->right - suggestedArea->left)/2;
            int32_t centerY = suggestedArea->top + (suggestedArea->bottom - suggestedArea->top)/2;
            
            if (centerX < window->_monitor->attributes().screenArea.x || centerX >= window->_monitor->attributes().screenArea.x + (int32_t)window->_monitor->attributes().screenArea.width
            ||  centerY < window->_monitor->attributes().screenArea.y || centerY >= window->_monitor->attributes().screenArea.y + (int32_t)window->_monitor->attributes().screenArea.height) {
              try {
                auto monitors = DisplayMonitor::listAvailableMonitors();
                for (auto& it : monitors) {
                  if (centerX >= it.attributes().screenArea.x && centerX < it.attributes().screenArea.x + (int32_t)it.attributes().screenArea.width
                  &&  centerY >= it.attributes().screenArea.y && centerY < it.attributes().screenArea.y + (int32_t)it.attributes().screenArea.height) {
                    window->_monitor = std::make_shared<DisplayMonitor>(std::move(it));
                    isSameMonitor = false;
                    break;
                  }
                }
              }
              catch (...) { break; }
            }
            
            WindowDecorationSize decorationSizes;
            __toNativeWindowDecoration(*(window->_monitor), window->_mode, window->_impl->windowStyle, window->_impl->windowStyleExt, 
                                       (window->_menuHandle != nullptr), decorationSizes);
            
            // try to keep same client area on new monitor
            uint32_t clientWidth, clientHeight;
            { std::lock_guard<pandora::thread::SpinLock> guard(window->_clientAreaLock);
              clientWidth = window->_clientArea.width;
              clientHeight = window->_clientArea.height;}
            SetWindowPos((HWND)window->_handle, nullptr, suggestedArea->left, suggestedArea->top, 
                         (int)clientWidth + decorationSizes.left + decorationSizes.right, 
                         (int)clientHeight + decorationSizes.top + decorationSizes.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
            
            // notify handler, to let parent app decide if client size should be adjusted
            if (window->_onWindowEvent) {
              uint32_t adjustedClientWidth = static_cast<uint32_t>(suggestedArea->right - suggestedArea->left - decorationSizes.left - decorationSizes.right);
              uint32_t adjustedClientHeight = static_cast<uint32_t>(suggestedArea->bottom - suggestedArea->top - decorationSizes.top - decorationSizes.bottom);
              
              window->_onWindowEvent(WindowEvent::dpiChanged, isSameMonitor ? 0 : 1, adjustedClientWidth, adjustedClientHeight,
                                     static_cast<uint32_t>(window->contentScale()*100.0f), (uint64_ptr)LOWORD(wParam));
            }
            return 0;
          }
          break;
        }
        // external device connected/removed (gamepad, joystick...)
        case WM_DEVICECHANGE: { 
          if (window->_onWindowEvent && (void*)lParam != nullptr) {
            if (wParam == DBT_DEVICEARRIVAL) {
              if (((DEV_BROADCAST_HDR*)lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                window->_onWindowEvent(WindowEvent::deviceInterfaceChange, 1, 0,0,0,0); // never block this message
            }
            else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
              if (((DEV_BROADCAST_HDR*)lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                window->_onWindowEvent(WindowEvent::deviceInterfaceChange, 0, 0,0,0,0); // never block this message
            }
          }
          break;
        }
        // system operation
        case WM_SYSCOMMAND: {
          switch (wParam & 0xFFF0) {
            case SC_SCREENSAVE:
            case SC_MONITORPOWER: // screen saver / blanking
              if (window->_mode == WindowType::fullscreen) // disallow screen saver in fullscreen mode
                return 0;
              break;
            case SC_KEYMENU: // menu access with ALT
              if (window->_menuHandle == nullptr) // no menu -> ignore
                return 0;
          }
          break;
        }
        // system suspend/resume
        case WM_POWERBROADCAST: {
          if (wParam == PBT_APMQUERYSUSPEND) {
            window->_nativeFlag |= __P_FLAG_POWER_SUSPENDED;
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::suspendResume, 1, 0,0,0,0);
          }
          else if (wParam == PBT_APMRESUMEAUTOMATIC) {
            window->_nativeFlag = (window->_nativeFlag & ~((uint32_t)__P_FLAG_POWER_SUSPENDED));
            if (window->_onWindowEvent)
              window->_onWindowEvent(WindowEvent::suspendResume, 0, 0,0,0,0);
          }
          break;
        }
        // input language change
        case WM_INPUTLANGCHANGE: {
          if (window->_onWindowEvent)
            window->_onWindowEvent(WindowEvent::inputLangChanged, (uint32_t)wParam, ((uint32_t)lParam & 0xFF), 
                                   (((uint32_t)lParam & 0xFF00) >> 8), 0, (uint64_ptr)lParam);
          break; 
        }
      }
    }
    
    // if original window processor existed, use it
    if (window->_nativeFlag & __P_FLAG_USE_ORIG_EVENT_PROC)
      return window->_impl->originalStyle.eventProcessor(handle, message, wParam, lParam);
    return DefWindowProcW(handle, message, wParam, lParam); // system window processor
  }


// -- Window -- ----------------------------------------------------------------

  // -- accessors --
  
  // get current window visibility state
  Window::VisibleState Window::visibleState() const noexcept {
    uint32_t visibilityFlags = (this->_nativeFlag & __P_FLAGS_VISIBILITY_MASK) >> __P_FLAGS_VISIBILITY_OFFSET;
    return static_cast<Window::VisibleState>(visibilityFlags);
  }
  
  // get current window client area (position / size)
  DisplayArea Window::getClientArea() const noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
    return this->_clientArea;
  }
  // get current window client area size
  PixelSize Window::getClientSize() const noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
    return PixelSize{ this->_clientArea.width, this->_clientArea.height };
  }
  
  // get current monitor scaling (based on DPI)
  float Window::contentScale() const noexcept {
    assert(this->_monitor != nullptr);
    float scaleX, scaleY;
    this->_monitor->getMonitorScaling(scaleX, scaleY, this->_handle);
    return scaleY;
  }
  
  // ---
  
  // get current mouse pointer position
  PixelPosition Window::cursorPosition(Window::CursorPositionType mode) noexcept {
    POINT pos;
    if (GetCursorPos(&pos) != FALSE) {
      if (mode == Window::CursorPositionType::relative) { // relative to client area
        if (ScreenToClient((HWND)this->_handle, &pos) != FALSE)
          return PixelPosition{ pos.x, pos.y };

        std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock); // fallback
        return PixelPosition{ pos.x - this->_clientArea.x, pos.y - this->_clientArea.y };
      }
      else // absolute
        return PixelPosition{ pos.x, pos.y };
    }
    return { -1, -1 };
  }
  
  // read vertical scroll box position
  int32_t Window::getScrollPositionV() const noexcept {
    SCROLLINFO scrollInfo;
    ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    scrollInfo.fMask = SIF_POS;
    
    if ((this->_impl->windowStyle & WS_VSCROLL) && GetScrollInfo((HWND)this->_handle, SB_VERT, &scrollInfo) != FALSE)
      return scrollInfo.nPos;
    return -1;
  }
  // read horizontal scroll box position
  int32_t Window::getScrollPositionH() const noexcept {
    SCROLLINFO scrollInfo;
    ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    scrollInfo.fMask = SIF_POS;
    
    if ((this->_impl->windowStyle & WS_HSCROLL) && GetScrollInfo((HWND)this->_handle, SB_HORZ, &scrollInfo) != FALSE)
      return scrollInfo.nPos;
    return -1;
  }
  
  
  // -- display changes - visibility --
  
  // change window visibility state
  bool Window::show(Window::VisibilityCommand state) noexcept {
    // displayed for the first time
    if ((this->_nativeFlag & __P_FLAG_FIRST_DISPLAY_DONE) == 0) {
      if (!_validateFirstDisplay()) {
        return false;
      }
      this->_nativeFlag |= __P_FLAG_FIRST_DISPLAY_DONE;
    }
    
    bool isSuccess = false;
    uint32_t newFlags = 0;
    uint32_t prevFlags = this->_nativeFlag;
    
    // fullscreen -> change resolution
    if (this->_mode == WindowType::fullscreen) {
      int showWindowValue = __toNativeFullscreenVisibility(state, prevFlags, newFlags);
      
      // was hidden/minimized
      if ((prevFlags & __P_FLAG_WINDOW_VISIBLE) == 0) {
        // set fullscreen if (state != minimized/hidden)
        if (newFlags & __P_FLAG_WINDOW_VISIBLE) {
          ShowWindow((HWND)this->_handle, showWindowValue); // no need to check (re-done in next call)
          isSuccess = __setFullscreenResolution(*(this->_monitor), this->_handle, this->_clientArea, 
                                                this->_refreshRate, newFlags);
          if (!isSuccess) // on failure, restore previous state
            ShowWindow((HWND)this->_handle, (prevFlags & __P_FLAG_WINDOW_MINIMIZED) ? SW_SHOWMINNOACTIVE : SW_HIDE);
        }
        else // still not visible -> update minimized/hidden
          isSuccess = (ShowWindow((HWND)this->_handle, showWindowValue) != FALSE);
      }
      
      // was visible
      else {
        // restore desktop resolution if (state == minimized/hidden)
        if ((newFlags & __P_FLAG_WINDOW_VISIBLE) == 0) {
          __resetDesktopResolution(*(this->_monitor), this->_handle, newFlags);
          isSuccess = (ShowWindow((HWND)this->_handle, showWindowValue) != FALSE);
          
          if (!isSuccess) { // on failure, return fullscreen
            this->_nativeFlag = (prevFlags & ~((uint32_t)__P_FLAG_MODE_CHANGED));
            __setFullscreenResolution(*(this->_monitor), this->_handle, this->_clientArea, 
                                      this->_refreshRate, this->_nativeFlag);
          }
        }
        else // show/showInactive/maximize/restore are the same in fullscreen mode
          isSuccess = true; // already visible
      }
    }
    // window/dialog -> update visibility
    else {
      isSuccess = (ShowWindow((HWND)this->_handle, __toNativeWindowVisibility(state, prevFlags, newFlags)) != FALSE);
    }
    
    // update internal status + activate window (if requested)
    if (isSuccess) {
      this->_nativeFlag = newFlags;
      if (newFlags & __P_FLAG_WINDOW_ACTIVE) {
        SetForegroundWindow((HWND)this->_handle);
        SetFocus((HWND)this->_handle);
      }
    }
    return isSuccess;
  }
  
  
  // -- display changes - size/position/type --
  
  // change position
  bool Window::move(int32_t x, int32_t y) noexcept {
    if (this->_mode == WindowType::fullscreen)
      return (x == 0 && y == 0);
    
    auto clientArea = getClientArea();
    clientArea.x = x;
    clientArea.y = y;
    auto windowArea = this->_monitor->convertClientAreaToWindowArea(clientArea, this->_handle, (this->_menuHandle != nullptr), 
                                                                    _impl->windowStyle, _impl->windowStyleExt);
    if (SetWindowPos((HWND)this->_handle, nullptr, (int)windowArea.x, (int)windowArea.y, 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW)) != FALSE) {
      { std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
      this->_clientArea.x = x;
      this->_clientArea.y = y; }
      InvalidateRect((HWND)this->_handle, nullptr, true); // repaint
      return true;
    }
    return false;
  }
  
  // change size
  bool Window::resize(uint32_t width, uint32_t height) noexcept {
    auto clientArea = getClientArea();
    clientArea.width = width;
    clientArea.height = height;
    
    if (this->_mode == WindowType::fullscreen) {
      if (!__setFullscreenResolution(*(this->_monitor), this->_handle, clientArea, this->_refreshRate, this->_nativeFlag))
        return false;
    }
    else { // window/dialog
      auto windowArea = this->_monitor->convertClientAreaToWindowArea(clientArea, this->_handle, (this->_menuHandle != nullptr), 
                                                                      _impl->windowStyle, _impl->windowStyleExt);
      if (SetWindowPos((HWND)this->_handle, nullptr, 0, 0, (int)windowArea.width, (int)windowArea.height, (SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW)) == FALSE)
        return false;
    }
    
    { std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
    this->_clientArea.width = width;
    this->_clientArea.height = height; }
    _impl->initialRatio = (double)width / (double)height;
    
    InvalidateRect((HWND)this->_handle, nullptr, true); // repaint
    return true;
  }
  
  // change size + rate
  bool Window::resize(uint32_t width, uint32_t height, uint32_t rate) noexcept {
    uint32_t prevRate = this->_refreshRate;
    this->_refreshRate = rate;
    
    if (!resize(width, height)) {
      this->_refreshRate = prevRate;
      return false;
    }
    return true;
  }
  
  // change size + position
  bool Window::resize(const DisplayArea& clientArea) noexcept {
    if (this->_mode == WindowType::fullscreen) {
      if (!__setFullscreenResolution(*(this->_monitor), this->_handle, clientArea, this->_refreshRate, this->_nativeFlag))
        return false;
    }
    else { // window/dialog
      auto windowArea = this->_monitor->convertClientAreaToWindowArea(clientArea, this->_handle, (this->_menuHandle != nullptr), 
                                                                      _impl->windowStyle, _impl->windowStyleExt);
      if (SetWindowPos((HWND)this->_handle, nullptr, 0, 0, (int)windowArea.width, (int)windowArea.height, (SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW)) == FALSE)
        return false;
    }
    
    { std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
    this->_clientArea = clientArea; }
    _impl->initialRatio = (double)clientArea.width / (double)clientArea.height;
    
    InvalidateRect((HWND)this->_handle, nullptr, true); // repaint
    return true;
  }
  
  // ---
  
  // change window style and position/size/resolution
  bool Window::setDisplayMode(WindowType type, WindowBehavior behavior, const DisplayArea& clientArea, 
                              ResizeMode resizeMode, uint32_t rate) {
    // set window style
    DWORD windowStyle, windowStyleExt;
    __toNativeWindowStyle(type, behavior, resizeMode, windowStyle, windowStyleExt);
    SetWindowLong((HWND)this->_handle, GWL_STYLE, (windowStyle | __toNativeClassStyle(behavior)));
    SetWindowLong((HWND)this->_handle, GWL_EXSTYLE, windowStyleExt);
    
    // set window area size/position
    DisplayArea windowArea;
    DisplayArea newClientArea;
    if (this->_parent == nullptr)
      __toNativeAbsoluteArea(type, (this->_menuHandle != nullptr), clientArea, *(this->_monitor), 
                             windowStyle, windowStyleExt, newClientArea, windowArea);
    else
      __toNativeChildArea(this->_parent, type, (this->_menuHandle != nullptr), clientArea, *(this->_monitor), 
                          windowStyle, windowStyleExt, newClientArea, windowArea);
    
    if (type == WindowType::fullscreen) {
      ShowWindow((HWND)this->_handle, SW_SHOWMAXIMIZED); // also shown by SetWindowPos in next function -> no need to verify here
      if (!__setFullscreenResolution(*(this->_monitor), this->_handle, newClientArea, rate, this->_nativeFlag))
        return false;
    }
    else if (SetWindowPos((HWND)this->_handle, HWND_TOP, (int)windowArea.x,(int)windowArea.y,
                          (int)windowArea.width,(int)windowArea.height, SWP_SHOWWINDOW) == FALSE) {
      return false;
    }
    
    this->_mode = type;
    this->_behavior = behavior;
    this->_resizeMode = resizeMode;
    this->_refreshRate = rate;
    _impl->windowStyle = windowStyle;
    _impl->windowStyleExt = windowStyleExt;
    this->_nativeFlag |= (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE);
    if (type == WindowType::fullscreen)
      this->_nativeFlag |= __P_FLAG_WINDOW_MAXIMIZED;
    
    {std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock);
    this->_clientArea = newClientArea;}
    _impl->initialRatio = (double)newClientArea.width / (double)newClientArea.height;
    
    SetForegroundWindow((HWND)this->_handle); // activate + set input
    SetFocus((HWND)this->_handle);
    
    InvalidateRect((HWND)this->_handle, nullptr, true); // repaint
    return true;
  }
  
  // define minimum size limits for the user (ignored if not resizable)
  bool Window::setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept {
    if ((this->_resizeMode & ResizeMode::homothety) == true) { // homothety: force ratio for min size
      this->_impl->minWidth = static_cast<uint32_t>(_impl->initialRatio * (double)minHeight + 0.50001);
      if (this->_impl->minWidth >= minWidth)
        this->_impl->minHeight = minHeight;
      else {
        this->_impl->minWidth = minWidth;
        this->_impl->minHeight = static_cast<uint32_t>((double)minWidth / _impl->initialRatio + 0.50001);
      }
    }
    else {
      this->_impl->minWidth = minWidth;
      this->_impl->minHeight = minHeight;
    }
    return true;
  }
  
  
  // -- display changes - content --
  
  // clear entire client area (with background color)
  bool Window::clearClientArea() noexcept {
    RECT clientArea;
    if (this->_backgroundColor != nullptr && GetClientRect((HWND)this->_handle, &clientArea) != FALSE) {
      HDC hdc = GetDC((HWND)this->_handle);
      if (hdc) {
        FillRect(hdc, &clientArea, (HBRUSH)(this->_backgroundColor->handle()));
        ReleaseDC((HWND)this->_handle, hdc); 
        return true;
      }
    }
    return false;
  }
  // clear rectangle relative to client area (with background color)
  bool Window::clear(const DisplayArea& area) noexcept {
    RECT clientArea;
    if (this->_backgroundColor != nullptr && GetClientRect((HWND)this->_handle, &clientArea) != FALSE) {
      HDC hdc = GetDC((HWND)this->_handle);
      if (hdc) {
        clientArea.left += (int)area.x;
        clientArea.top += (int)area.y;
        clientArea.right = clientArea.left + (int)area.width;
        clientArea.bottom = clientArea.top + (int)area.height;
        FillRect(hdc, &clientArea, (HBRUSH)(this->_backgroundColor->handle()));
        ReleaseDC((HWND)this->_handle, hdc); 
        return true;
      }
    }
    return false;
  }
  
  // ---
  
  // get last error message (on change failure)
  std::string Window::getLastError() {
    return __formatLastError(nullptr);
  }
  
  
  // -- resource changes --
  
  // change mouse pointer X/Y position
  bool Window::setCursorPosition(int32_t x, int32_t y, Window::CursorPositionType mode) noexcept {
    if (mode == Window::CursorPositionType::relative) {
      POINT pos{ (int)x, (int)y };
      if (ClientToScreen((HWND)this->_handle, &pos) != FALSE) {
        return (SetCursorPos(pos.x, pos.y) != FALSE);
      }
      { std::lock_guard<pandora::thread::SpinLock> guard(_clientAreaLock); // fallback
      pos.x = this->_clientArea.x + x;
      pos.y = this->_clientArea.y + y; }
      return (SetCursorPos(pos.x, pos.y) != FALSE);
    }
    else // absolute
      return (SetCursorPos((int)x, (int)y) != FALSE);
  }
  
  // change window title
  bool Window::setCaption(const window_char* caption) noexcept {
    return (SetWindowTextW((HWND)this->_handle, (caption != nullptr) ? caption : L"") != FALSE);
  }
  // replace window menu (or remove if null)
  bool Window::setMenu(MenuHandle menu) noexcept {
    if (SetMenu((HWND)this->_handle, (HMENU)menu) != FALSE || menu == nullptr) {
      this->_menuHandle = menu;
      return true;
    }
    return false;
  }
  
  // change cursor resource
  bool Window::setCursor(std::shared_ptr<WindowResource> cursor) noexcept {
    if (cursor != nullptr && cursor->handle()) {
      SetLastError(0);
      if (SetClassLongPtr((HWND)this->_handle, GCLP_HCURSOR, (LONG_PTR)cursor->handle()) != 0 || GetLastError() == 0) {
        this->_cursor = cursor;
        return true;
      }
    }
    return false;
  }
  // change background color resource
  bool Window::setBackgroundColorBrush(std::shared_ptr<WindowResource> colorBrush) noexcept {
    if (colorBrush != nullptr && colorBrush->handle()) {
      SetLastError(0);
      if (SetClassLongPtr((HWND)this->_handle, GCLP_HBRBACKGROUND, (LONG_PTR)colorBrush->handle()) != 0 || GetLastError() == 0) {
        this->_backgroundColor = colorBrush;
        return true;
      }
    }
    return false;
  }
  
  // change vertical/horizontal scrollbar ranges
  bool Window::setScrollbarRange(uint16_t posV, uint16_t posH, uint16_t verticalMax, uint16_t horizontalMax, uint32_t pixelsPerUnit) noexcept {
    if (posV > verticalMax || posH > horizontalMax || pixelsPerUnit == 0)
      return false;
    
    SCROLLINFO scrollInfo;
    ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    
    BOOL repaint = (this->_nativeFlag & __P_FLAG_WINDOW_VISIBLE) ? TRUE : FALSE;
    this->_impl->pixelsPerScrollUnit = pixelsPerUnit;
    
    if (this->_impl->windowStyle & WS_VSCROLL) {
      scrollInfo.fMask = (SIF_POS | SIF_RANGE | SIF_PAGE);
      scrollInfo.nPos = posV;
      scrollInfo.nMin = 0;
      scrollInfo.nMax = verticalMax;
      scrollInfo.nPage = this->_clientArea.height / pixelsPerUnit;
      SetScrollInfo((HWND)this->_handle, SB_VERT, &scrollInfo, repaint);
    }
    if (this->_impl->windowStyle & WS_HSCROLL) {
      scrollInfo.fMask = (SIF_POS | SIF_RANGE | SIF_PAGE);
      scrollInfo.nPos = posH;
      scrollInfo.nMin = 0;
      scrollInfo.nMax = horizontalMax;
      scrollInfo.nPage = this->_clientArea.width / pixelsPerUnit;
      SetScrollInfo((HWND)this->_handle, SB_HORZ, &scrollInfo, repaint);
    }
    return true;
  }
  // change position of slider in vertical scrollbar
  bool Window::setScrollPositionV(uint16_t pos) noexcept {
    if (this->_impl->windowStyle & WS_VSCROLL) {
      SCROLLINFO scrollInfo;
      ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
      scrollInfo.cbSize = sizeof(SCROLLINFO);
      scrollInfo.fMask = SIF_POS;
      scrollInfo.nPos = pos;
      SetScrollInfo((HWND)this->_handle, SB_VERT, &scrollInfo, (this->_nativeFlag & __P_FLAG_WINDOW_VISIBLE) ? TRUE : FALSE);
      return true;
    }
    return false;
  }
  // change position of slider in horizontal scrollbar
  bool Window::setScrollPositionH(uint16_t pos) noexcept {
    if (this->_impl->windowStyle & WS_HSCROLL) {
      SCROLLINFO scrollInfo;
      ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
      scrollInfo.cbSize = sizeof(SCROLLINFO);
      scrollInfo.fMask = SIF_POS;
      scrollInfo.nPos = pos;
      SetScrollInfo((HWND)this->_handle, SB_HORZ, &scrollInfo, (this->_nativeFlag & __P_FLAG_WINDOW_VISIBLE) ? TRUE : FALSE);
      return true;
    }
    return false;
  }
  
  
// -- Window Builder - new window -- -------------------------------------------

  // configure window class context + create new window
  std::unique_ptr<Window> Window::Builder::create(const window_char* contextName, const window_char* caption, 
                                                  WindowHandle parentWindow) { // throws
    if (contextName == nullptr || *contextName == 0)
      throw std::invalid_argument("Window.Builder: context name must not be NULL or empty");
  
    WNDCLASSEXW windowClass;
    ZeroMemory(&windowClass, sizeof(WNDCLASSEXW));
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpszClassName = contextName;
    
    // style / events
    windowClass.lpfnWndProc = &__WindowImpl::windowEventProcessor;
    windowClass.style = __toNativeClassStyle(this->_params.behavior);
    
    // resources
    windowClass.hInstance = (this->_params.moduleInstance) 
                          ? (HINSTANCE)this->_params.moduleInstance 
                          : (HINSTANCE)pandora::system::WindowsApp::instance().handle(); // throws
    
    if (this->_params.appIcon != nullptr)
      windowClass.hIcon = (HICON)this->_params.appIcon->handle();
    if (this->_params.captionIcon != nullptr)
      windowClass.hIconSm = (HICON)this->_params.captionIcon->handle(); 
    
    if (this->_params.cursor == nullptr)
      this->_params.cursor = WindowResource::buildCursor(SystemCursor::pointer);
    windowClass.hCursor = (HCURSOR)this->_params.cursor->handle();

    if (this->_params.backgroundColor == nullptr)
      this->_params.backgroundColor = WindowResource::buildColorBrush(WindowResource::systemWindowColor());
    windowClass.hbrBackground = (HBRUSH)this->_params.backgroundColor->handle();
    
    // window creation
    if (RegisterClassExW(&windowClass) == 0)
      throw std::runtime_error("Window.Builder: context class registration failure");
    try {
      return std::unique_ptr<Window>(new Window(contextName, caption, this->_params, parentWindow));
    }
    catch (...) {
      UnregisterClassW(contextName, windowClass.hInstance);
      throw;
    }
  }
  
  // ---
  
  // new window construction
  Window::Window(const window_char* contextName, const window_char* caption, const Window::Builder::Params& params, WindowHandle parentWindow) { // throws
    this->_contextName = contextName;
    this->_moduleInstance = (params.moduleInstance) 
                          ? (void*)params.moduleInstance 
                          : (void*)pandora::system::WindowsApp::instance().handle(); // throws
    // resources
    this->_impl = new __WindowImpl(); // throws
    this->_impl->hasOriginalStyleBackup = false;
    this->_appIcon = params.appIcon;
    this->_captionIcon = params.captionIcon;
    this->_cursor = params.cursor;
    this->_backgroundColor = params.backgroundColor;
    this->_menuHandle = params.menu;
    this->_parent = parentWindow;

    if (params.monitor != nullptr)
      this->_monitor = params.monitor;
    else
      this->_monitor = std::make_shared<DisplayMonitor>(); // use default monitor if not specified

    // window style
    DWORD windowStyle, windowStyleExt;
    __toNativeWindowStyle(params.displayMode, params.behavior, params.resizeMode, windowStyle, windowStyleExt);
    this->_mode = params.displayMode;
    this->_behavior = params.behavior;
    this->_resizeMode = params.resizeMode;
    this->_refreshRate = params.refreshRate;
    _impl->windowStyle = windowStyle;
    _impl->windowStyleExt = windowStyleExt;
    
    // convert area size/position
    DisplayArea windowArea;
    if (parentWindow == nullptr)
      __toNativeAbsoluteArea(this->_mode, (this->_menuHandle != nullptr), params.clientArea, *(this->_monitor), 
                             windowStyle, windowStyleExt, this->_clientArea, windowArea);
    else
      __toNativeChildArea(parentWindow, this->_mode, (this->_menuHandle != nullptr), params.clientArea, *(this->_monitor), 
                          windowStyle, windowStyleExt, this->_clientArea, windowArea);
    _impl->initialRatio = (double)this->_clientArea.width / (double)this->_clientArea.height;

    // create new window
    this->_handle = CreateWindowExW(windowStyleExt, contextName, caption, windowStyle, 
                                    (int)windowArea.x,(int)windowArea.y,(int)windowArea.width,(int)windowArea.height, 
                                    (HWND)parentWindow, (HMENU)params.menu, (HINSTANCE)this->_moduleInstance, nullptr);
    if (this->_handle == nullptr)
      throw std::runtime_error(__formatLastError("Window creation failure: "));
    ++(g_windowCount); // increase instance counter (for exit handler) -> decreased in destructor if a handle exists
    
    // reference instance as user data for event handler
    if (SetPropW((HWND)this->_handle, __P_WINDOW_ID, (HANDLE)this) == FALSE)
      throw std::runtime_error(__formatLastError("Window: user data (for message handling) failure: "));
    _isAppAlive = true;
  }


// -- Window Builder - existing window -- --------------------------------------

  static inline void __backupExistingWindowState(WindowHandle existingHandle, __WindowImpl& impl) noexcept {
    impl.hasOriginalStyleBackup = true;
    __WindowImpl::StyleBackup& orig = impl.originalStyle;

    // backup resources
    orig.appIcon = (HICON)GetClassLongPtr((HWND)existingHandle, GCLP_HICON);
    orig.captionIcon = (HICON)GetClassLongPtr((HWND)existingHandle, GCLP_HICONSM);
    orig.cursor = (HCURSOR)GetClassLongPtr((HWND)existingHandle, GCLP_HCURSOR);
    orig.backgroundColor = (HBRUSH)GetClassLongPtr((HWND)existingHandle, GCLP_HBRBACKGROUND);
    orig.menu = (HMENU)GetMenu((HWND)existingHandle);
    // backup style
    GetWindowRect((HWND)existingHandle, &(orig.windowArea));
    orig.windowStyle = GetWindowLong((HWND)existingHandle, GWL_STYLE);
    orig.windowStyleExt = GetWindowLong((HWND)existingHandle, GWL_EXSTYLE);
    orig.eventProcessor = (WNDPROC)GetWindowLongPtr((HWND)existingHandle, GWLP_WNDPROC);
  }

  // ---

  // build instance from existing window
  std::unique_ptr<Window> Window::Builder::update(WindowHandle existingHandle, bool callOrigEventProc) { // throws
    return std::unique_ptr<Window>(new Window(this->_params, existingHandle, callOrigEventProc));
  }
  
  // existing window re-styling
  Window::Window(const Window::Builder::Params& params, WindowHandle existingHandle, bool callOrigEventProc) { // throws
    if (existingHandle == (WindowHandle)0)
      throw std::invalid_argument("Window: existingHandle not be NULL");
    this->_moduleInstance = (params.moduleInstance) 
                          ? (void*)params.moduleInstance 
                          : (void*)pandora::system::WindowsApp::instance().handle(); // throws
    
    this->_impl = new __WindowImpl(); // throws
    __backupExistingWindowState(existingHandle, *_impl);
    if (callOrigEventProc && _impl->originalStyle.eventProcessor != nullptr)
      this->_nativeFlag |= __P_FLAG_USE_ORIG_EVENT_PROC;
    
    // set handle + monitor
    if (params.monitor != nullptr)
      this->_monitor = params.monitor;
    else
      this->_monitor = std::make_shared<DisplayMonitor>(); // use default monitor if not specified
    this->_handle = existingHandle;
    this->_parent = (WindowHandle)GetWindowLongPtr((HWND)existingHandle, GWLP_HWNDPARENT);
    ++(g_windowCount);
    
    // set event processor + reference instance as user data for event handler
    if (SetPropW((HWND)this->_handle, __P_WINDOW_ID, (HANDLE)this) == FALSE)
      throw std::runtime_error(__formatLastError("Window: user data (for message handling) failure: "));
    SetLastError(0);
    if (SetWindowLongPtr((HWND)existingHandle, GWLP_WNDPROC, (LONG_PTR)&__WindowImpl::windowEventProcessor) == 0 && GetLastError() != 0)
      throw std::runtime_error(__formatLastError("Window: event processor assignment failure: ")); // if returns 0, verify if error occurred
    
    // redefine window resources
    if (params.captionIcon) {
      this->_captionIcon = params.captionIcon;
      SetClassLongPtr((HWND)existingHandle, GCLP_HICONSM, (LONG_PTR)params.captionIcon->handle());
    }
    if (params.appIcon) {
      this->_appIcon = params.appIcon;
      SetClassLongPtr((HWND)existingHandle, GCLP_HICON, (LONG_PTR)params.appIcon->handle());
    }
    if (params.cursor) {
      this->_cursor = params.cursor;
      SetClassLongPtr((HWND)existingHandle, GCLP_HCURSOR, (LONG_PTR)params.cursor->handle());
    }
    if (params.backgroundColor) {
      this->_backgroundColor = params.backgroundColor;
      SetClassLongPtr((HWND)existingHandle, GCLP_HBRBACKGROUND, (LONG_PTR)params.backgroundColor->handle());
    }
    this->_menuHandle = params.menu;
    SetMenu((HWND)existingHandle, (HMENU)params.menu);
    
    // redefine window style + size/position
    if (!setDisplayMode(params.displayMode, params.behavior, params.clientArea, params.resizeMode, params.refreshRate))
      throw std::runtime_error(__formatLastError("Window: position/size application failure: "));
    
    this->_nativeFlag |= __P_FLAG_FIRST_DISPLAY_DONE;
    _isAppAlive = true;
  }


// -- Window - destruction -- --------------------------------------------------

  static void __restoreOriginalStyle(WindowHandle handle, __WindowImpl::StyleBackup& orig) noexcept {
    // restore resources
    SetClassLongPtr((HWND)handle, GCLP_HICONSM, (LONG_PTR)orig.captionIcon);
    SetClassLongPtr((HWND)handle, GCLP_HICON, (LONG_PTR)orig.appIcon);
    SetClassLongPtr((HWND)handle, GCLP_HCURSOR, (LONG_PTR)orig.cursor);
    SetClassLongPtr((HWND)handle, GCLP_HBRBACKGROUND, (LONG_PTR)orig.backgroundColor);
    SetMenu((HWND)handle, orig.menu);
    
    // restore style
    SetWindowLong((HWND)handle, GWL_STYLE, orig.windowStyle);
    SetWindowLong((HWND)handle, GWL_EXSTYLE, orig.windowStyleExt);
    
    RemovePropW((HWND)handle, __P_WINDOW_ID);
    SetWindowLongPtr((HWND)handle, GWLP_WNDPROC, (LONG_PTR)orig.eventProcessor);
    SetWindowPos((HWND)handle, HWND_TOP, (int)orig.windowArea.left, (int)orig.windowArea.top,
                 (int)orig.windowArea.right - orig.windowArea.left, (int)orig.windowArea.bottom - orig.windowArea.top, SWP_SHOWWINDOW);
  }
  
  // ---

  // destroy / restore window from handle
  void Window::_destroyWindow() noexcept {
    if (this->_handle) {
      if (this->_nativeFlag & __P_FLAG_CURSOR_HOVER) // restore cursor + unregister to raw input
        __disableCursorMode(this->_nativeFlag);

      show(Window::VisibilityCommand::hide); // restore original display mode if fullscreen + hide window
      
      --(g_windowCount); // decrement before destroying -> let handler know if it was the last window
      
      // existing window: restore original style
      if (_impl && _impl->hasOriginalStyleBackup) {
        __restoreOriginalStyle(this->_handle, _impl->originalStyle);
        ShowWindow((HWND)this->_handle, SW_SHOW);
      }
      
      // new window: destroy
      else {
        DestroyWindow((HWND)this->_handle);
        if (!this->_contextName.empty())
          UnregisterClassW(this->_contextName.c_str(), (HINSTANCE)this->_moduleInstance);
      }
      this->_handle = nullptr;
    }
  }
  
  // destroy window (or restore existing window style)
  Window::~Window() noexcept {
    _destroyWindow();
    if (this->_impl) {
      if (this->_impl->rawInputBuffer)
        free(this->_impl->rawInputBuffer);
      delete _impl;
    }
  }

#endif
