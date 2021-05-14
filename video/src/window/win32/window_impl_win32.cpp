/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window manager + builder - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <thread/spin_lock.h>
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include <windowsx.h>
# include <shellapi.h>
# include <Dbt.h>
# include <system/force_inline.h>
# include "video/window_keycodes.h"
# include "video/_private/_window_impl_win32.h"

#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

// -- constants --

# define __P_WINDOW_ID  L"PTWo" // instance ID per handle, for event processor ("Pandora-Toolbox Window object")

# define __P_VK_EXTENDED_FLAG       0x01000000  // win32 extended key flag
# define __P_VK_KEY_PREV_DOWN_FLAG  0x40000000  // win32 previous key-down state flag
# define __P_VK_KEY_UP_FLAG         0x80000000  // win32 key-up state flag
# define __P_VK_KEY_SCANCODE_OFFSET 16

  using namespace pandora::video;
  using pandora::hardware::DisplayArea;
  using pandora::hardware::DisplayMode;
  using pandora::hardware::DisplayMonitor;
  
  
  // Get last error message
  std::string __WindowImpl::formatLastError(const char* prefix) {
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
  

// -- global window count -- ---------------------------------------------------

  static int32_t g_windowCount{ 0 }; // global number of window instances
  static pandora::thread::SpinLock g_windowCountLock;
  
  int32_t __WindowImpl::getWindowInstanceCount() noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_windowCountLock);
    return g_windowCount;
  }
  static inline void __incrementWindowCount() noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_windowCountLock);
    ++g_windowCount;
  }
  static inline void __decrementWindowCount() noexcept {
    std::lock_guard<pandora::thread::SpinLock> guard(g_windowCountLock);
    if (g_windowCount > 0)
      --g_windowCount;
  }


// -- window style storage (backup/restore) -- ---------------------------------

  // Store style properties from existing window (backup)
  void WindowStyle::backupStyle(HWND handle) noexcept {
    // backup resources
    _appIcon = (HICON)GetClassLongPtr(handle, GCLP_HICON);
    _captionIcon = (HICON)GetClassLongPtr(handle, GCLP_HICONSM);
    _cursor = (HCURSOR)GetClassLongPtr(handle, GCLP_HCURSOR);
    _backgroundColor = (HBRUSH)GetClassLongPtr(handle, GCLP_HBRBACKGROUND);
    _menu = (HMENU)GetMenu(handle);
    _eventProcessor = (WNDPROC)GetWindowLongPtr(handle, GWLP_WNDPROC);
    
    // backup style
    if (GetWindowRect(handle, &_windowArea) == FALSE) {
      _windowArea.right = _windowArea.left + __P_DEFAULT_WINDOW_WIDTH;
      _windowArea.bottom = _windowArea.top + __P_DEFAULT_WINDOW_HEIGHT;
    }
    _windowStyle = (DWORD)GetWindowLong(handle, GWL_STYLE);
    _windowStyleExt = (DWORD)GetWindowLong(handle, GWL_EXSTYLE);
  }

  // Apply stored style properties on existing window (restore)
  void WindowStyle::applyStyle(HWND handle) noexcept {
    // restore resources
    SetClassLongPtr(handle, GCLP_HICONSM, (LONG_PTR)_captionIcon);
    SetClassLongPtr(handle, GCLP_HICON, (LONG_PTR)_appIcon);
    SetClassLongPtr(handle, GCLP_HCURSOR, (LONG_PTR)_cursor);
    SetClassLongPtr(handle, GCLP_HBRBACKGROUND, (LONG_PTR)_backgroundColor);
    SetMenu(handle, _menu);
    
    // restore style
    SetWindowLong(handle, GWL_STYLE, _windowStyle);
    SetWindowLong(handle, GWL_EXSTYLE, _windowStyleExt);
    SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)_eventProcessor);
    if (_windowArea.left != -1 || _windowArea.right != -1) {
      SetWindowPos(handle, HWND_TOP, (int)_windowArea.left, (int)_windowArea.top,
                   (int)_windowArea.right - _windowArea.left, (int)_windowArea.bottom - _windowArea.top, SWP_SHOWWINDOW);
    }
  }


// -- window creation/destruction -- -------------------------------------------

  // Window construction
  __WindowImpl::__WindowImpl(Window& container, const wchar_t* caption, const wchar_t* windowClassName, 
                             const Window::Builder::Params& params, HWND parent) // throws
  : _windowClassName(windowClassName),
    _monitor((params.monitor != nullptr) ? params.monitor : std::make_shared<DisplayMonitor>()), // throws
    _originalStyle(nullptr),
    _mouseInputBuffer(nullptr),
    _mouseInputBufferSize(0),
    _moduleInstance((params.moduleInstance) ? (HINSTANCE)params.moduleInstance 
                                            : (HINSTANCE)pandora::system::WindowsApp::instance().handle()), // throws
    _handle(nullptr),
    _parent(parent),
    _menuHandle((params.menu != nullptr) ? (HMENU)params.menu->handle() : nullptr),
    _container(container)
  {
    // set window style flags
    toWindowStyleFlags(params.displayMode, params.behavior, params.resizeMode, 
                       this->_currentStyleFlag, this->_currentStyleExtFlag);
    this->_mode = params.displayMode;
    this->_behavior = params.behavior;
    this->_resizeMode = (params.displayMode != WindowType::fullscreen && params.displayMode != WindowType::borderless) 
                      ? params.resizeMode : ResizeMode::fixed;
    this->_refreshRate = params.refreshRate;
    if (this->_parent != nullptr && params.displayMode != WindowType::dialog)
      this->_parent = nullptr; // only dialogs can have a parent window
    
    // display area: compute window decorations + replace user values (centered/default/...)
    DisplayArea windowArea;
    computeWindowDecorations(this->_currentStyleFlag, this->_currentStyleExtFlag, hasMenu(), this->_decorationSizes);
    computeUserArea(params.displayMode, params.behavior, params.clientArea, this->_decorationSizes, this->_lastClientArea, windowArea);
    this->_clientAreaRatio = (double)this->_lastClientArea.width / (double)this->_lastClientArea.height;

    // create new window
    this->_handle = CreateWindowExW(this->_currentStyleExtFlag, windowClassName, caption, this->_currentStyleFlag, 
                                    (int)windowArea.x, (int)windowArea.y, (int)windowArea.width, (int)windowArea.height,
                                    this->_parent, this->_menuHandle, this->_moduleInstance, nullptr);
    if (this->_handle == nullptr)
      throw std::runtime_error(formatLastError("Window creation failure: "));
    __incrementWindowCount(); // only increment after handle creation
    
    // reference window instance as user data (for event processor)
    if (SetPropW(this->_handle, __P_WINDOW_ID, (HANDLE)this) == FALSE)
      throw std::runtime_error(formatLastError("Window: user data creation (for message handling) failure: "));
  }
  
  // ---

  // Existing window re-styling
  __WindowImpl::__WindowImpl(Window& container, HWND existingHandle, 
                             const Window::Builder::Params& params, bool callExistingEventProc) // throws
  : _monitor((params.monitor != nullptr) ? params.monitor : std::make_shared<DisplayMonitor>()),
    _originalStyle(new WindowStyle(existingHandle)), // throws
    _mouseInputBuffer(nullptr),
    _mouseInputBufferSize(0), 
    _moduleInstance((params.moduleInstance) ? (HINSTANCE)params.moduleInstance 
                                            : (HINSTANCE)pandora::system::WindowsApp::instance().handle()), // throws
    _handle(existingHandle),
    _parent((HWND)GetWindowLongPtr(existingHandle, GWLP_HWNDPARENT)),
    _menuHandle((params.menu != nullptr) ? (HMENU)params.menu->handle() : nullptr),
    _container(container)
  {
    if (existingHandle == nullptr)
      throw std::invalid_argument("Window: existingHandle must not be NULL");
    __incrementWindowCount();

    // reference instance as user data (for event processor)
    if (SetPropW(this->_handle, __P_WINDOW_ID, (HANDLE)this) == FALSE)
      throw std::runtime_error(formatLastError("Window: user data creation (for message handling) failure: "));
    if (callExistingEventProc && _originalStyle->_eventProcessor != nullptr)
      __P_ADD_FLAG(this->_statusFlags, __P_FLAG_USE_ORIG_EVENT_PROC);
    
    // replace window resources
    SetLastError(0);
    if (SetWindowLongPtr(existingHandle, GWLP_WNDPROC, (LONG_PTR)&__WindowImpl::windowEventProcessor) == 0 && GetLastError() != 0)
      throw std::runtime_error(formatLastError("Window: event processor binding failure: ")); // if returns 0, verify if error occurred
    if (params.captionIcon)
      SetClassLongPtr(existingHandle, GCLP_HICONSM, (LONG_PTR)params.captionIcon->handle());
    if (params.appIcon)
      SetClassLongPtr(existingHandle, GCLP_HICON, (LONG_PTR)params.appIcon->handle());
    if (params.cursor)
      SetClassLongPtr(existingHandle, GCLP_HCURSOR, (LONG_PTR)params.cursor->handle());
    if (params.backgroundColor)
      SetClassLongPtr(existingHandle, GCLP_HBRBACKGROUND, (LONG_PTR)params.backgroundColor->handle());
    SetMenu(existingHandle, this->_menuHandle);

    // change window mode/style/position
    if (!setDisplayMode(params.displayMode, params.behavior, params.resizeMode, params.clientArea, params.refreshRate))
      throw std::runtime_error(formatLastError("Window: position/size application failure: "));
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
  }
  
  // ---

  // Destroy window (or restore existing window style)
  void __WindowImpl::_destroy() noexcept {
    if (this->_handle) {
      // if (this->_statusFlags & __P_FLAG_CURSOR_HOVER) // restore cursor + unregister to raw input
        // __disableCursorMode(this->_statusFlags);

      show(SW_HIDE); // hide window + restore original display mode
      __decrementWindowCount(); // inform 'pollEvents()' whether it's the last window or not -> decrement before destroying
      
      // existing window: restore original style
      if (this->_originalStyle != nullptr) {
        _originalStyle->applyStyle(this->_handle);
        ShowWindow(this->_handle, SW_SHOW);
      }
      // new window: destroy
      else {
        DestroyWindow(this->_handle);
        if (!this->_windowClassName.empty())
          UnregisterClassW(this->_windowClassName.c_str(), this->_moduleInstance);
      }
      RemovePropW(this->_handle, __P_WINDOW_ID); // unreference window instance
      this->_handle = nullptr;
    }
  }
  // Instance destruction
  __WindowImpl::~__WindowImpl() noexcept {
    _destroy();
    if (_mouseInputBuffer != nullptr)
      free(_mouseInputBuffer);
  }
  
  // ---
  
  // Get current monitor scaling (based on DPI)
  float __WindowImpl::contentScale() const noexcept {
    float scaleX, scaleY;
    this->_monitor->getMonitorScaling(scaleX, scaleY, this->_handle);
    
    // fullscreen: adjust according to resolution (compared to desktop resolution)
    if (this->_mode == WindowType::fullscreen) {
      std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
      uint32_t windowHeight = this->_lastClientArea.height + (uint32_t)this->_decorationSizes.top 
                            + (uint32_t)this->_decorationSizes.bottom;
      
      if (windowHeight < this->_monitor->attributes().screenArea.height) {
        double actualScaleY = (double)scaleY * (double)windowHeight 
                            / (double)this->_monitor->attributes().screenArea.height;
        scaleY = (float)actualScaleY;
      }
    }
    return scaleY;
  }
  
  
// -- change display mode -- ---------------------------------------------------

  // Adjust window visibility/size (first display)
  bool __WindowImpl::firstShow(int visibilityFlag) noexcept {
    bool isVisibilityCommandSet = false;
    
    if (this->_mode == WindowType::fullscreen) { // fullscreen mode
      isVisibilityCommandSet = (visibilityFlag != SW_HIDE && visibilityFlag != SW_SHOWMINNOACTIVE);
      enterFullscreenMode(lastWindowArea(), this->_refreshRate);
    }
    else { // window mode
      isVisibilityCommandSet = (visibilityFlag == SW_SHOW || visibilityFlag == SW_SHOWDEFAULT
                             || (visibilityFlag == SW_SHOWMAXIMIZED && (this->_currentStyleFlag & WS_MAXIMIZE)) );
      if (ShowWindow(this->_handle, isVisibilityCommandSet ? visibilityFlag : SW_SHOW) == FALSE)
        return false; // display failure
    }
    
    // process display events
    if (this->_container.pollCurrentWindowEvents() == false)
      return false; // WM_QUIT received -> stop here
      
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
    adjustVisibleClientSize(); // adjust size, based on actual window
    
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
    if (isVisibilityCommandSet) {
      SetForegroundWindow(this->_handle);
      SetFocus(this->_handle);
      return true;
    }
    else
      return show(visibilityFlag);
  }
  
  // ---

  // Change window visibility
  bool __WindowImpl::show(int visibilityFlag) noexcept {
    // displayed for the first time
    if ((this->_statusFlags & __P_FLAG_FIRST_DISPLAY_DONE) == 0)
      return firstShow(visibilityFlag);

    if (visibilityFlag == SW_HIDE || visibilityFlag == SW_SHOWMINNOACTIVE) {
      if (ShowWindow(this->_handle, visibilityFlag) == FALSE)
        return false;
    }
    else {
      if (ShowWindow(this->_handle, (this->_mode == WindowType::fullscreen) ? SW_SHOWMAXIMIZED : visibilityFlag) == FALSE)
        return false;
      SetForegroundWindow(this->_handle);
      SetFocus(this->_handle);
    }
    return true;
  }
  
  // ---

  // Change window style and position/size/resolution
  bool __WindowImpl::setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resizeMode,
                                    const DisplayArea& userArea, uint32_t rate) {
    if (this->_parent != nullptr && type != WindowType::dialog) { // child window must be a dialog
      SetLastError(ERROR_DS_ROOT_MUST_BE_NC);
      return false;
    }
    if ((this->_statusFlags & __P_FLAG_FULLSCREEN_ON) && type != WindowType::fullscreen)
      exitFullscreenMode();
    
    // set window style flags
    DWORD prevStyle = this->_currentStyleFlag; // must be set before calling 'enterFullscreenMode'
    DWORD prevStyleExt = this->_currentStyleExtFlag;
    if (type == WindowType::fullscreen || type == WindowType::borderless)
      resizeMode = ResizeMode::fixed;
    toWindowStyleFlags(type, behavior, resizeMode, this->_currentStyleFlag, this->_currentStyleExtFlag);
    
    // display area: compute window decorations + replace user values (centered/default/...)
    DisplayArea windowArea, clientArea;
    WindowDecorationSize decorationSizes;
    computeWindowDecorations(this->_currentStyleFlag, this->_currentStyleExtFlag, hasMenu(), decorationSizes);
    computeUserArea(type, behavior, userArea, decorationSizes, clientArea, windowArea);
    
    // apply display mode
    if (type == WindowType::fullscreen) {
      enterFullscreenMode(windowArea, rate); // change resolution
    }
    else { // window mode
      SetWindowLongPtr(this->_handle, GWL_STYLE, this->_currentStyleFlag | toWindowClassStyleFlag(behavior) | WS_VISIBLE);
      SetWindowLongPtr(this->_handle, GWL_EXSTYLE, this->_currentStyleExtFlag);
        
      if (SetWindowPos(this->_handle, HWND_TOP, (int)windowArea.x,(int)windowArea.y,
                        (int)windowArea.width,(int)windowArea.height, SWP_SHOWWINDOW | SWP_FRAMECHANGED) == FALSE) {
        SetWindowLongPtr(this->_handle, GWL_STYLE, prevStyle | WS_VISIBLE); // revert on failure
        SetWindowLongPtr(this->_handle, GWL_EXSTYLE, prevStyleExt);
        this->_currentStyleFlag = prevStyle;
        this->_currentStyleExtFlag = prevStyleExt;
        if (this->_mode == WindowType::fullscreen)
          enterFullscreenMode(windowArea, rate);
        return false;
      }
    }
    
    {// lock scope -> copy params after success
      std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
      this->_mode = type;
      this->_behavior = behavior;
      this->_resizeMode = resizeMode;
      this->_refreshRate = rate;
      this->_decorationSizes = decorationSizes;
      this->_lastClientArea = clientArea;
      this->_clientAreaRatio = (double)this->_lastClientArea.width / (double)this->_lastClientArea.height;
    }
    // process display events
    if (this->_container.pollCurrentWindowEvents() == false)
      return false; // WM_QUIT received -> stop here
    
    // adjust size, based on actual window
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
    adjustVisibleClientSize();
    if (isScrollable())
      adjustScrollbarPageSize(clientArea);
    
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
    SetForegroundWindow(this->_handle);
    SetFocus(this->_handle);
    return true;
  }
  
  // ---
  
  // Replace window menu (or remove if null)
  bool __WindowImpl::setMenu(HMENU menuHandle) noexcept {
    if (SetMenu(this->_handle, menuHandle) != FALSE || menuHandle == nullptr) {
      bool hasSizeChanged = ((menuHandle && !this->_menuHandle) || (this->_menuHandle && !menuHandle));
      this->_menuHandle = menuHandle;
      
      if (hasSizeChanged) {
        std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
        if (this->_statusFlags & __P_FLAG_WINDOW_VISIBLE)
          adjustVisibleClientSize();
        else
          __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
      }
      return true;
    }
    return false;
  }


// -- style flag conversions -- ------------------------------------------------

  // Convert portable flags to native window style flags
  void __WindowImpl::toWindowStyleFlags(WindowType type, WindowBehavior behavior, ResizeMode resizeMode, 
                                        DWORD& outWindowStyle, DWORD& outWindowStyleExt) noexcept {
    outWindowStyle = (WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    outWindowStyleExt = 0;
    
    // fullscreen style
    if (type == WindowType::fullscreen) {
      outWindowStyle |= (WS_POPUP | WS_MAXIMIZE | WS_SYSMENU);
      outWindowStyleExt |= WS_EX_TOPMOST;
    }
    // window/dialog style
    else {
      switch (type) {
        case WindowType::borderless: 
          outWindowStyle |= WS_POPUP | WS_SYSMENU;
          break;
        case WindowType::bordered: 
          outWindowStyle |= (WS_POPUP | WS_BORDER); 
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

  // Convert portable behavior to native window-class style flag
  UINT __WindowImpl::toWindowClassStyleFlag(WindowBehavior behavior) noexcept {
    UINT classStyle = (CS_HREDRAW | CS_VREDRAW);
    if ((behavior & WindowBehavior::dropShadow) == true)
      classStyle |= CS_DROPSHADOW;
    if ((behavior & WindowBehavior::globalContext) == true)
      classStyle |= CS_OWNDC;
    
    return classStyle;
  }
  
  // Convert portable visibility to native window flag
  int __WindowImpl::toWindowVisibilityFlag(Window::VisibilityCommand state) noexcept {
    switch(state) {
      case Window::VisibilityCommand::hide: return SW_HIDE;
      case Window::VisibilityCommand::show: return SW_SHOW;
      case Window::VisibilityCommand::showInactive: return SW_SHOWNA;
      case Window::VisibilityCommand::restore:  return SW_SHOWDEFAULT;
      case Window::VisibilityCommand::minimize: return SW_SHOWMINNOACTIVE;
      case Window::VisibilityCommand::maximize: return SW_SHOWMAXIMIZED;
      default: return SW_SHOWDEFAULT;
    }
  }
  // Convert portable visibility to native fullscreen flag
  int __WindowImpl::toFullscreenVisibilityFlag(Window::VisibilityCommand state) noexcept {
    switch(state) {
      case Window::VisibilityCommand::hide: return SW_HIDE;
      case Window::VisibilityCommand::minimize: return SW_SHOWMINNOACTIVE;
      default: return SW_SHOWMAXIMIZED;
    }
  }


// -- window metrics -- --------------------------------------------------------

  // Calculate expected size of future window decorations (based on window style)
  void __WindowImpl::computeWindowDecorations(DWORD windowStyle, DWORD windowStyleExt, 
                                              bool hasMenu, WindowDecorationSize& outSizes) noexcept {
    DisplayArea dummyClientArea { this->_monitor->attributes().workArea.x + 32,
                                  this->_monitor->attributes().workArea.y + 96,
                                  this->_monitor->attributes().workArea.width - 64,
                                  this->_monitor->attributes().workArea.height - 128 };
    auto dummyWindowArea = this->_monitor->convertClientAreaToWindowArea(dummyClientArea, (WindowHandle)nullptr, 
                                                       hasMenu, (uint32_t)windowStyle, (uint32_t)windowStyleExt);
    outSizes.left = dummyClientArea.x - dummyWindowArea.x;
    outSizes.top = dummyClientArea.y - dummyWindowArea.y;
    outSizes.right = (int32_t)dummyWindowArea.width - (int32_t)dummyClientArea.width - outSizes.left;
    outSizes.bottom = (int32_t)dummyWindowArea.height - (int32_t)dummyClientArea.height - outSizes.top;
  }
  
  // Read actual size of visible window decorations
  void __WindowImpl::readVisibleWindowDecorations(const DisplayArea& clientArea, WindowDecorationSize& outSizes) noexcept {
    RECT windowPos;
    if (GetWindowRect(this->_handle, &windowPos) != FALSE && windowPos.right > windowPos.left) {
      outSizes.left = clientArea.x - (int32_t)windowPos.left;
      outSizes.top = clientArea.y - (int32_t)windowPos.top;
      outSizes.right = (int32_t)windowPos.right - (int32_t)clientArea.width - clientArea.x;
      outSizes.bottom = (int32_t)windowPos.bottom - (int32_t)clientArea.height - clientArea.y;
    }
    else if (this->_parent != nullptr) {
      computeWindowDecorations(this->_currentStyleFlag, this->_currentStyleExtFlag, hasMenu(), outSizes);
    }
    else {
      DisplayArea windowArea = this->_monitor->convertClientAreaToWindowArea(clientArea, (WindowHandle)this->_handle, 
                                                           hasMenu(), (uint32_t)this->_currentStyleFlag, 
                                                           (uint32_t)this->_currentStyleExtFlag);
      outSizes.left = clientArea.x - windowArea.x;
      outSizes.top = clientArea.y - windowArea.y;
      outSizes.right = (int32_t)windowArea.width - (int32_t)clientArea.width - outSizes.left;
      outSizes.bottom = (int32_t)windowArea.height - (int32_t)clientArea.height - outSizes.top;
    }
  }
  
  // ---
  
  // Read size of visible window
  bool __WindowImpl::readVisibleWindowArea(DisplayArea& outWindowArea) noexcept {
    RECT windowPos;
    if (GetWindowRect(this->_handle, &windowPos) != FALSE && windowPos.right > windowPos.left) {
      outWindowArea.x = (int32_t)windowPos.left;
      outWindowArea.y = (int32_t)windowPos.top;
      outWindowArea.width = (uint32_t)(windowPos.right - windowPos.left);
      outWindowArea.height = (uint32_t)(windowPos.bottom - windowPos.top);
      return true;
    }
    return false;
  }
  // Read size of visible client-area
  bool __WindowImpl::readVisibleClientArea(DisplayArea& outClientArea) noexcept {
    RECT clientArea;
    if (GetClientRect(this->_handle, &clientArea) != FALSE && clientArea.right > clientArea.left) { // size
      outClientArea.width = static_cast<uint32_t>(clientArea.right - clientArea.left);
      outClientArea.height = static_cast<uint32_t>(clientArea.bottom - clientArea.top);
      
      POINT pos{ 0, 0 };
      if (ClientToScreen(this->_handle, &pos) != FALSE) { // position
        outClientArea.x = (int32_t)pos.x;
        outClientArea.y = (int32_t)pos.y;
        return true;
      }
    }
    return false;
  }
  
  // Convert user-defined coord to client-area coord
  static inline int32_t __toClientAreaCoord(int32_t userCoord, int32_t minCoord, int32_t maxCoord, uint32_t clientSize) noexcept {
    switch (userCoord) {
      case Window::Builder::defaultPosition(): return minCoord;
      case Window::Builder::centeredPosition(): return (minCoord + maxCoord - (int32_t)clientSize)/2;
      default: return userCoord;
    }
  }
  
  // ---
  
  // Change mouse pointer X/Y position
  bool __WindowImpl::setCursorPosition(int32_t x, int32_t y, Window::CursorPositionType mode) noexcept {
    if (mode == Window::CursorPositionType::relative) {
      POINT pos{ (int)x, (int)y };
      if (ClientToScreen(this->_handle, &pos) != FALSE)
        return (SetCursorPos(pos.x, pos.y) != FALSE);

      std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
      pos.x = this->_lastClientArea.x + x; // on failure, use client area to convert value manually
      pos.x = this->_lastClientArea.y + y;
      return (SetCursorPos(pos.x, pos.y) != FALSE);
    }
    else // absolute
      return (SetCursorPos((int)x, (int)y) != FALSE);
  }
  
  // ---
  
  // Change vertical/horizontal scrollbar ranges
  bool __WindowImpl::setScrollbarRange(uint32_t posH, uint32_t posV, uint32_t horizontalMax, 
                                       uint32_t verticalMax, uint32_t pixelsPerUnit) noexcept {
    if (posV > verticalMax || posH > horizontalMax || pixelsPerUnit == 0)
      return false;
    
    BOOL repaint = (this->_statusFlags & __P_FLAG_WINDOW_VISIBLE) ? TRUE : FALSE;
    this->_scrollUnit = pixelsPerUnit;
    
    SCROLLINFO scrollInfo;
    ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    
    if (isScrollableH()) {
      this->_lastScrollPosition.x = (int32_t)posH;
      this->_maxScrollPosition.width = horizontalMax;
      
      scrollInfo.fMask = (SIF_POS | SIF_RANGE | SIF_PAGE);
      scrollInfo.nPos = (int)posH;
      scrollInfo.nMin = 0;
      scrollInfo.nMax = (int)horizontalMax;
      scrollInfo.nPage = this->_lastClientArea.width / pixelsPerUnit;
      SetScrollInfo(this->_handle, SB_HORZ, &scrollInfo, repaint);
    }
    if (isScrollableV()) {
      this->_lastScrollPosition.y = (int32_t)posV;
      this->_maxScrollPosition.height = verticalMax;
      
      scrollInfo.fMask = (SIF_POS | SIF_RANGE | SIF_PAGE);
      scrollInfo.nPos = (int)posV;
      scrollInfo.nMin = 0;
      scrollInfo.nMax = (int)verticalMax;
      scrollInfo.nPage = this->_lastClientArea.height / pixelsPerUnit;
      SetScrollInfo(this->_handle, SB_VERT, &scrollInfo, repaint);
    }
    return true;
  }
  
  // Set scrollbar page size, based on client-area size
  void __WindowImpl::adjustScrollbarPageSize(const DisplayArea& clientArea) noexcept {
    SCROLLINFO scrollInfo;
    ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    
    if (isScrollableH()) {
      scrollInfo.fMask = SIF_PAGE;
      scrollInfo.nPage = clientArea.width / this->_scrollUnit;
      SetScrollInfo(this->_handle, SB_HORZ, &scrollInfo, TRUE);
    }
    if (isScrollableV()) {
      scrollInfo.fMask = SIF_PAGE;
      scrollInfo.nPage = clientArea.height / this->_scrollUnit;
      SetScrollInfo(this->_handle, SB_VERT, &scrollInfo, TRUE);
    }
  }
  
  // Set lider position in horizontal scrollbar
  bool __WindowImpl::setScrollPositionH(uint32_t pos) noexcept {
    if (isScrollableH() && pos <= this->_maxScrollPosition.width) {
      this->_lastScrollPosition.x = (int32_t)pos;
      
      SCROLLINFO scrollInfo;
      ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
      scrollInfo.cbSize = sizeof(SCROLLINFO);
      scrollInfo.fMask = SIF_POS;
      scrollInfo.nPos = (int)pos;
      SetScrollInfo(this->_handle, SB_HORZ, &scrollInfo, (this->_statusFlags & __P_FLAG_WINDOW_VISIBLE) ? TRUE : FALSE);
      return true;
    }
    return false;
  }
  // Set slider position in vertical scrollbar
  bool __WindowImpl::setScrollPositionV(uint32_t pos) noexcept {
    if (isScrollableV() && pos <= this->_maxScrollPosition.height) {
      this->_lastScrollPosition.y = (int32_t)pos;
      
      SCROLLINFO scrollInfo;
      ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));
      scrollInfo.cbSize = sizeof(SCROLLINFO);
      scrollInfo.fMask = SIF_POS;
      scrollInfo.nPos = (int)pos;
      SetScrollInfo(this->_handle, SB_VERT, &scrollInfo, (this->_statusFlags & __P_FLAG_WINDOW_VISIBLE) ? TRUE : FALSE);
      return true;
    }
    return false;
  }
  

// -- user area computation -- -------------------------------------------------

  // Verify if a borderless window has "fullscreen" behavior
  static inline bool __isBorderlessFull(const DisplayMonitor& monitor, WindowType mode, 
                                        WindowBehavior behavior, uint32_t width, uint32_t height) noexcept {
    return (mode == WindowType::borderless && (behavior & WindowBehavior::aboveTaskbar) == true
        && (width == monitor.attributes().screenArea.width 
          || width == Window::Builder::defaultSize()
          || height == monitor.attributes().screenArea.height 
          || height == Window::Builder::defaultSize())
        );
  }
  // Get parent area (on failure, use work area)
  static inline void __getParentArea(HWND parent, const DisplayMonitor& monitor, DisplayArea& outArea) noexcept {
    RECT parentArea;
    if (GetWindowRect(parent, &parentArea) != FALSE) {
      outArea.x = (int32_t)parentArea.left;
      outArea.y = (int32_t)parentArea.top;
      outArea.width = static_cast<uint32_t>(parentArea.right - parentArea.left);
      outArea.height = static_cast<uint32_t>(parentArea.bottom - parentArea.top);
    }
    else
      outArea = monitor.attributes().workArea;
  }
  
  // Computer client-size and window-size of window (based on user-defined values)
  template <bool _IsFullscreen>
  static inline void __computeUserSize(uint32_t userWidth, uint32_t userHeight, const DisplayArea& parentArea,
                                       const WindowDecorationSize& decorationSizes,
                                       DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    __if_constexpr (_IsFullscreen) {
      outWindowArea.width = (userWidth != Window::Builder::defaultSize()) ? userWidth : parentArea.width;
      outWindowArea.height = (userHeight != Window::Builder::defaultSize()) ? userHeight : parentArea.height;
      outClientArea.width = (uint32_t)((int32_t)outWindowArea.width - decorationSizes.left - decorationSizes.right);
      outClientArea.height = (uint32_t)((int32_t)outWindowArea.height - decorationSizes.top - decorationSizes.bottom);
    }
    else {
      outClientArea.width = (userWidth != Window::Builder::defaultSize()) 
                          ? userWidth 
                          : (uint32_t)((int32_t)parentArea.width - decorationSizes.left - decorationSizes.right);
      outClientArea.height = (userHeight != Window::Builder::defaultSize()) 
                           ? userHeight
                           : (uint32_t)((int32_t)parentArea.height - decorationSizes.top - decorationSizes.bottom);
      outWindowArea.width = (uint32_t)((int32_t)outClientArea.width + decorationSizes.left + decorationSizes.right);
      outWindowArea.height = (uint32_t)((int32_t)outClientArea.height + decorationSizes.top + decorationSizes.bottom);
    }
  }
  
  // Computer client-position (absolute) and window-position (absolute/relative) of window (based on user-defined values)
  template <bool _IsFullscreen, bool _IsRelative>
  static inline void __computeUserPosition(int32_t userX, int32_t userY, uint32_t clientWidth, uint32_t clientHeight, 
                                           const DisplayArea& parentArea, const WindowDecorationSize& decorationSizes,
                                           DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    __if_constexpr (_IsFullscreen) {
      outWindowArea.x = parentArea.x;
      outWindowArea.y = parentArea.y;
      outClientArea.x = outWindowArea.x + decorationSizes.left;
      outClientArea.y = outWindowArea.y + decorationSizes.top;
    }
    else __if_constexpr (_IsRelative) {
      outClientArea.x = __toClientAreaCoord(userX, decorationSizes.left, (int32_t)parentArea.width - decorationSizes.right, clientWidth);
      outClientArea.y = __toClientAreaCoord(userY, decorationSizes.top, (int32_t)parentArea.height - decorationSizes.bottom, clientHeight);
      outWindowArea.x = outClientArea.x - decorationSizes.left;
      outWindowArea.y = outClientArea.y - decorationSizes.top;
      outClientArea.x += parentArea.x; // to absolute client position
      outClientArea.y += parentArea.y;
    }
    else {
      int32_t maxCoordX = parentArea.x + (int32_t)parentArea.width - decorationSizes.right;
      int32_t maxCoordY = parentArea.y + (int32_t)parentArea.height - decorationSizes.bottom;
      outClientArea.x = __toClientAreaCoord(userX, parentArea.x + decorationSizes.left, maxCoordX, clientWidth);
      outClientArea.y = __toClientAreaCoord(userY, parentArea.y + decorationSizes.top, maxCoordY, clientHeight);
      outWindowArea.x = outClientArea.x - decorationSizes.left;
      outWindowArea.y = outClientArea.y - decorationSizes.top;
    }
  }
  
  // ---

  // Compute client-position (absolute) and window-position (absolute if root/relative if child), based on user-defined values
  void __WindowImpl::computeUserPosition(int32_t x, int32_t y, const WindowDecorationSize& decorationSizes,
                                         DisplayArea& inOutClientArea, DisplayArea& outWindowPos) noexcept {
    // child dialog -> relative window-area
    if (this->_parent != nullptr) {
      DisplayArea parentArea;
      __getParentArea(this->_parent, *(this->_monitor), parentArea);
      __computeUserPosition<false,true>(x, y, inOutClientArea.width, inOutClientArea.height,
                                        parentArea, decorationSizes, inOutClientArea, outWindowPos);
    }
    // root window -> absolute window-area
    else {
      bool isBorderlessFull = __isBorderlessFull(*(this->_monitor), this->_mode, this->_behavior, 
                                                 inOutClientArea.width + decorationSizes.left + decorationSizes.right, 
                                                 inOutClientArea.height + decorationSizes.top + decorationSizes.bottom);
      if (this->_mode == WindowType::fullscreen || isBorderlessFull) { // full screen -> decorations must fit inside -> reduce client area
        __computeUserPosition<true,false>(x, y, inOutClientArea.width, inOutClientArea.height,
                                          this->_monitor->attributes().screenArea, decorationSizes, 
                                          inOutClientArea, outWindowPos);
      }
      else { // window/dialog -> decorations outside of client area
        __computeUserPosition<false,false>(x, y, inOutClientArea.width, inOutClientArea.height,
                                           this->_monitor->attributes().workArea, decorationSizes, 
                                           inOutClientArea, outWindowPos);
      }
    }
  }
  // Compute client-size and window-size, based on user-defined values
  void __WindowImpl::computeUserSize(uint32_t width, uint32_t height, const WindowDecorationSize& decorationSizes, 
                                     DisplayArea& outClientSize, DisplayArea& outWindowSize) noexcept {
    // child dialog -> relative window-area
    if (this->_parent != nullptr) {
      DisplayArea parentArea;
      __getParentArea(this->_parent, *(this->_monitor), parentArea);
      __computeUserSize<false>(width, height, parentArea, decorationSizes, outClientSize, outWindowSize);
    }
    // root window -> absolute window-area
    else {
      bool isBorderlessFull = __isBorderlessFull(*(this->_monitor), this->_mode, this->_behavior, width, height);
      if (this->_mode == WindowType::fullscreen || isBorderlessFull) { // full screen -> decorations must fit inside -> reduce client area
        __computeUserSize<true>(width, height, this->_monitor->attributes().screenArea, 
                                decorationSizes, outClientSize, outWindowSize);
      }
      else { // window/dialog -> decorations outside of client area
        __computeUserSize<false>(width, height, this->_monitor->attributes().workArea, 
                                 decorationSizes, outClientSize, outWindowSize);
      }
    }
  }
  // Compute client-area (absolute) and window-area (absolute if root/relative if child), based on user-defined values
  void __WindowImpl::computeUserArea(WindowType mode, WindowBehavior behavior, const DisplayArea& userArea, 
                                     const WindowDecorationSize& decorationSizes,
                                     DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    // child dialog -> relative window-area
    if (this->_parent != nullptr) {
      DisplayArea parentArea;
      __getParentArea(this->_parent, *(this->_monitor), parentArea);
      
      __computeUserSize<false>(userArea.width, userArea.height, parentArea, decorationSizes, outClientArea, outWindowArea);
      __computeUserPosition<false,true>(userArea.x, userArea.y, outClientArea.width, outClientArea.height,
                                        parentArea, decorationSizes, outClientArea, outWindowArea);
    }
    // root window -> absolute window-area
    else {
      bool isBorderlessFull = __isBorderlessFull(*(this->_monitor), mode, behavior, userArea.width, userArea.height);
      if (mode == WindowType::fullscreen || isBorderlessFull) { // full screen -> decorations must fit inside -> reduce client area
        __computeUserSize<true>(userArea.width, userArea.height, this->_monitor->attributes().screenArea, 
                                decorationSizes, outClientArea, outWindowArea);
        __computeUserPosition<true,false>(userArea.x, userArea.y, outClientArea.width, outClientArea.height,
                                          this->_monitor->attributes().screenArea, decorationSizes, 
                                          outClientArea, outWindowArea);
      }
      else { // window/dialog -> decorations outside of client area
        __computeUserSize<false>(userArea.width, userArea.height, this->_monitor->attributes().workArea, 
                                 decorationSizes, outClientArea, outWindowArea);
        __computeUserPosition<false,false>(userArea.x, userArea.y, outClientArea.width, outClientArea.height,
                                           this->_monitor->attributes().workArea, decorationSizes, 
                                           outClientArea, outWindowArea);
      }
    }
  }
  
  // ---
  
  // Adjust client size after first display (to match requested size)
  void __WindowImpl::adjustVisibleClientSize() noexcept {
    // verify client area -> if size is incorrect, fix it
    DisplayArea realClientArea, windowArea;
    if (readVisibleClientArea(realClientArea)) {
      if (readVisibleWindowArea(windowArea)) {
        this->_decorationSizes.left = realClientArea.x - windowArea.x;
        this->_decorationSizes.top = realClientArea.y - windowArea.y;
        this->_decorationSizes.right = (int32_t)windowArea.width - (int32_t)realClientArea.width - this->_decorationSizes.left;
        this->_decorationSizes.bottom = (int32_t)windowArea.height - (int32_t)realClientArea.height - this->_decorationSizes.top;
      }
      else {
        this->_decorationSizes.left += realClientArea.x - this->_lastClientArea.x;
        this->_decorationSizes.top += realClientArea.y - this->_lastClientArea.y;
        this->_decorationSizes.right += this->_lastClientArea.x + (int32_t)this->_lastClientArea.width 
                                      - realClientArea.x - (int32_t)realClientArea.width;
        this->_decorationSizes.bottom += this->_lastClientArea.y + (int32_t)this->_lastClientArea.height 
                                      - realClientArea.y - (int32_t)realClientArea.height;
      }
      
      // full screen -> no adjustments (just refresh stored values)
      if (this->_mode == WindowType::fullscreen || this->_mode == WindowType::borderless) {
        this->_lastClientArea = realClientArea;
      }
      else { // window/dialog -> resize
        this->_lastClientArea.x = realClientArea.x;
        this->_lastClientArea.y = realClientArea.y;
        if (SetWindowPos(this->_handle, HWND_TOP, 0, 0, 
                         (int)this->_lastClientArea.width + (int)this->_decorationSizes.left + (int)this->_decorationSizes.right, 
                         (int)this->_lastClientArea.height + (int)this->_decorationSizes.top + (int)this->_decorationSizes.bottom, 
                         (SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW)) == FALSE) {
          this->_lastClientArea.width = realClientArea.width; // on failure, refresh stored values
          this->_lastClientArea.height = realClientArea.height;
        }
      }
    }
  }


// -- display resolution -- ----------------------------------------------------

  // Go to fullscreen mode
  void __WindowImpl::enterFullscreenMode(DisplayArea windowArea, uint32_t rate) noexcept {
    DisplayMode currentMode = this->_monitor->getDisplayMode();
    if (windowArea.width != currentMode.width || windowArea.height != currentMode.height 
    || (rate != pandora::hardware::undefinedRefreshRate() && rate != currentMode.refreshRate) ) {
      DisplayMode videoMode{};
      videoMode.width = windowArea.width;
      videoMode.height = windowArea.height;
      videoMode.bitDepth = 32;
      videoMode.refreshRate = rate;
      
      this->_monitor->setDisplayMode(videoMode);
      __P_ADD_FLAG(this->_statusFlags, __P_FLAG_MODE_CHANGE_PENDING | __P_FLAG_FULLSCREEN_RES);
    }
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FULLSCREEN_ON);
    
    SetWindowLongPtr(this->_handle, GWL_STYLE, this->_currentStyleFlag | WS_VISIBLE);
    SetWindowLongPtr(this->_handle, GWL_EXSTYLE, this->_currentStyleExtFlag);
    SetWindowPos(this->_handle, HWND_TOP, (int)windowArea.x,(int)windowArea.y, (int)windowArea.width, (int)windowArea.height, SWP_FRAMECHANGED);
  }
  
  // Exit fullscreen mode
  void __WindowImpl::exitFullscreenMode() noexcept {
    if (this->_statusFlags & __P_FLAG_FULLSCREEN_RES) {
      this->_monitor->setDefaultDisplayMode();
      __P_ADD_REMOVE_FLAGS(this->_statusFlags, __P_FLAG_MODE_CHANGE_PENDING, __P_FLAG_FULLSCREEN_RES);
    }
    __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_FULLSCREEN_ON);
  }


// -- display size/position --
  
  // Change position
  bool __WindowImpl::move(int32_t x, int32_t y) noexcept {
    if (this->_mode == WindowType::fullscreen)
      return false;
    
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
    DisplayArea windowPos, clientArea = this->_lastClientArea;
    computeUserPosition(x, y, this->_decorationSizes, clientArea, windowPos);

    if (SetWindowPos(this->_handle, HWND_TOP, (int)windowPos.x,(int)windowPos.y, 0, 0, 
                     (SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW)) != FALSE) {
      this->_lastClientArea.x = clientArea.x;
      this->_lastClientArea.y = clientArea.y;
      InvalidateRect(this->_handle, nullptr, true); // repaint
      return true;
    }
    return false;
  }
  
  // Change size
  bool __WindowImpl::resize(uint32_t width, uint32_t height) noexcept {
    if (this->_mode == WindowType::fullscreen)
      return false;
    
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
    DisplayArea clientSize, windowSize;
    computeUserSize(width, height, this->_decorationSizes, clientSize, windowSize);
    
    if (SetWindowPos(this->_handle, HWND_TOP, 0, 0, (int)windowSize.width,(int)windowSize.height, 
                     (SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW)) != FALSE) {
      this->_lastClientArea.width = clientSize.width;
      this->_lastClientArea.height = clientSize.height;
      this->_clientAreaRatio = (double)this->_lastClientArea.width / (double)this->_lastClientArea.height;
      if (isScrollable())
        adjustScrollbarPageSize(this->_lastClientArea);
    
      InvalidateRect(this->_handle, nullptr, true); // repaint
      return true;
    }
    return false;
  }
  
  // Change size + position
  bool __WindowImpl::resize(const DisplayArea& userArea) noexcept {
    if (this->_mode == WindowType::fullscreen)
      return false;
    
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
    DisplayArea clientArea, windowArea;
    computeUserArea(this->_mode, this->_behavior, userArea, this->_decorationSizes, clientArea, windowArea);

    if (SetWindowPos(this->_handle, HWND_TOP, (int)windowArea.x,(int)windowArea.y, 
                     (int)windowArea.width,(int)windowArea.height, (SWP_NOZORDER | SWP_SHOWWINDOW)) != FALSE) {
      this->_lastClientArea = clientArea;
      this->_clientAreaRatio = (double)this->_lastClientArea.width / (double)this->_lastClientArea.height;
      if (isScrollable())
        adjustScrollbarPageSize(this->_lastClientArea);
      
      InvalidateRect(this->_handle, nullptr, true); // repaint
      return true;
    }
    return false;
  }
  
  // ---
  
  // Define minimum size limits for the user
  bool __WindowImpl::setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept {
    if ((this->_resizeMode & ResizeMode::homothety) == true) { // homothety: force ratio for min size
      this->_minClientSize.width = static_cast<uint32_t>(this->_clientAreaRatio * (double)minHeight + 0.50001);
      if (this->_minClientSize.width >= minWidth)
        this->_minClientSize.height = minHeight;
      else {
        this->_minClientSize.width = minWidth;
        this->_minClientSize.height = static_cast<uint32_t>((double)minWidth / this->_clientAreaRatio + 0.50001);
      }
    }
    else {
      this->_minClientSize.width = minWidth;
      this->_minClientSize.height = minHeight;
    }
    return true;
  }


// _____________________________________________________________________________

// -- window event helpers -- --------------------------------------------------

  namespace pandora {
    namespace video {
      class __WindowImplEventProc final {
      public:
        static __forceinline int refreshClientArea(__WindowImpl& window, DisplayArea& outArea) noexcept;
        static void refreshOnMaximizeRestore(__WindowImpl& window, LPARAM lParam, DisplayArea& outArea) noexcept;
        static __forceinline void resizeWithHomothety(__WindowImpl& window, int movedBorder, RECT& inOutArea) noexcept;
        
        static __forceinline void readRelativeCursorPosition(HWND handle, const DisplayArea& clientArea, PixelPosition& outPos) noexcept;
        static __forceinline void refreshScrollPosition(__WindowImpl& window, UINT message, WPARAM wParam, 
                                                        uint32_t& outX, uint32_t& outY) noexcept;
        static void enableCursorMode(__WindowImpl& window) noexcept;
        static void disableCursorMode(uint32_t statusFlag) noexcept;
        
        static bool findWindowMonitor(__WindowImpl& window, const RECT& suggestedArea) noexcept;
        static __forceinline void adjustWindowSizeOnDpiChange(__WindowImpl& window, const RECT& suggestedArea, 
                                                              WindowDecorationSize& outDecorationSizes) noexcept;
        
        static __forceinline void dragDropFiles(__WindowImpl& window, HDROP dropHandle) noexcept;

        static __forceinline bool processKeyboardEvent(__WindowImpl& window, UINT message, WPARAM wParam, LPARAM lParam);
        static __forceinline bool processMouseEvent(__WindowImpl& window, UINT message, WPARAM wParam, LPARAM lParam);
        static __forceinline void processRawInputEvent(__WindowImpl& window, LPARAM lParam);
      };
    }
  }

  // ---

  // Refresh client-area + returns: 0(error) / 1(no changes) / 2 (changed)
  __forceinline int __WindowImplEventProc::refreshClientArea(__WindowImpl& window, DisplayArea& outArea) noexcept {
    if (window.readVisibleClientArea(outArea)) {
      std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
      if (window._lastClientArea.x != outArea.x || window._lastClientArea.width != outArea.width
        || window._lastClientArea.y != outArea.y || window._lastClientArea.height != outArea.height) {

        window._lastClientArea = outArea;
        if (window.isScrollable())
          window.adjustScrollbarPageSize(outArea);
        return 2;
      }
      return 1;
    }
    return 0;
  }

  // On maximize/restore event, refresh client-area + decorations + force homothety
  void __WindowImplEventProc::refreshOnMaximizeRestore(__WindowImpl& window, LPARAM lParam, DisplayArea& outArea) noexcept {
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
    if (__WindowImplEventProc::refreshClientArea(window, outArea) > 0) {
      window.readVisibleWindowDecorations(outArea, window._decorationSizes);
    }
    else {
      window._lastClientArea.width = outArea.width = (uint32_t)LOWORD(lParam);
      window._lastClientArea.height = outArea.height = (uint32_t)HIWORD(lParam);
      window.computeWindowDecorations(window._currentStyleFlag, window._currentStyleExtFlag,
                                      window.hasMenu(), window._decorationSizes);
      if (window.isScrollable())
        window.adjustScrollbarPageSize(outArea);
    }

    // force homothety when maximized/restored
    if ((window._resizeMode & ResizeMode::homothety) == true) {
      uint32_t adjustedClientHeight = static_cast<uint32_t>((double)outArea.width / window._clientAreaRatio + 0.50001);
      if (adjustedClientHeight > outArea.height) {
        uint32_t adjustedClientWidth =  static_cast<uint32_t>((double)outArea.height * window._clientAreaRatio + 0.50001);

        window._lastClientArea.width = outArea.width = adjustedClientWidth;
        SetWindowPos(window._handle, HWND_TOP, 0, 0, (int)outArea.width + (int)window._decorationSizes.left + (int)window._decorationSizes.right,
                     (int)outArea.height + (int)window._decorationSizes.top + (int)window._decorationSizes.bottom, SWP_NOMOVE | SWP_NOZORDER);
      }
      else if (adjustedClientHeight < outArea.height) {
        window._lastClientArea.height = outArea.height = (uint32_t)adjustedClientHeight;
        SetWindowPos(window._handle, HWND_TOP, 0, 0, (int)outArea.width + (int)window._decorationSizes.left + (int)window._decorationSizes.right, 
                     (int)outArea.height + (int)window._decorationSizes.top + (int)window._decorationSizes.bottom, SWP_NOMOVE | SWP_NOZORDER);
      }
    }
  }

  // force homothety + clamp (screen edges) when window is resized
  __forceinline void __WindowImplEventProc::resizeWithHomothety(__WindowImpl& window, int movedBorder, RECT& inOutArea) noexcept {
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
    const DisplayArea& screenArea = window._monitor->attributes().screenArea;

    switch (movedBorder) {
      case WMSZ_TOPLEFT:
      case WMSZ_TOPRIGHT: {
        uint32_t clientWidth = (uint32_t)((int32_t)inOutArea.right - (int32_t)inOutArea.left 
                             - window._decorationSizes.right - window._decorationSizes.left);
        inOutArea.top = inOutArea.bottom - static_cast<int>((double)clientWidth / window._clientAreaRatio + 0.50001) 
                      - (int)window._decorationSizes.bottom - (int)window._decorationSizes.top;
        // clamp
        if ((int32_t)inOutArea.top < screenArea.y) {
          int32_t offsetY = screenArea.y - (int32_t)inOutArea.top;
          int32_t offsetX = static_cast<int32_t>(window._clientAreaRatio * (double)offsetY + 0.50001);

          inOutArea.top = screenArea.y;
          if (movedBorder == WMSZ_TOPLEFT)
            inOutArea.left += (int)offsetX;
          else
            inOutArea.right -= (int)offsetX;
        }
        break;
      }
      case WMSZ_LEFT:
      case WMSZ_RIGHT:
      case WMSZ_BOTTOMLEFT:
      case WMSZ_BOTTOMRIGHT: {
        uint32_t clientWidth = (uint32_t)((int32_t)inOutArea.right - (int32_t)inOutArea.left 
                             - window._decorationSizes.right - window._decorationSizes.left);
        inOutArea.bottom = inOutArea.top + static_cast<int>((double)clientWidth / window._clientAreaRatio + 0.50001) 
                         + (int)window._decorationSizes.bottom + (int)window._decorationSizes.top;
        // clamp
        int32_t bottomBorder = (window._decorationSizes.bottom > 2) ? window._decorationSizes.bottom - 2 : window._decorationSizes.bottom;
        if ((int32_t)inOutArea.bottom - bottomBorder > screenArea.y + (int32_t)screenArea.height) {
          int32_t offsetY = (int32_t)inOutArea.bottom - bottomBorder - screenArea.y - (int32_t)screenArea.height;
          int32_t offsetX = static_cast<int32_t>(window._clientAreaRatio * (double)offsetY + 0.50001);

          inOutArea.bottom -= offsetY;
          if (movedBorder == WMSZ_LEFT || movedBorder == WMSZ_BOTTOMLEFT)
            inOutArea.left += (int)offsetX;
          else
            inOutArea.right -= (int)offsetX;
        }
        break;
      }
      case WMSZ_TOP:
      case WMSZ_BOTTOM: {
        uint32_t clientHeight = (uint32_t)((int32_t)inOutArea.bottom - (int32_t)inOutArea.top 
                              - window._decorationSizes.bottom - window._decorationSizes.top);
        inOutArea.right = inOutArea.left + static_cast<int>(window._clientAreaRatio * (double)clientHeight + 0.50001) 
                        + (int)window._decorationSizes.right + (int)window._decorationSizes.left;
        // clamp
        int32_t rightBorder = (window._decorationSizes.right > 2) ? window._decorationSizes.right - 2 : window._decorationSizes.right;
        if ((int32_t)inOutArea.right - rightBorder > screenArea.x + (int32_t)screenArea.width) {
          int32_t offsetX = (int32_t)inOutArea.right - rightBorder - screenArea.x - (int32_t)screenArea.width;
          int32_t offsetY = static_cast<int32_t>((double)offsetX / window._clientAreaRatio + 0.50001);

          inOutArea.right -= offsetX;
          if (movedBorder == WMSZ_TOP)
            inOutArea.top += (int)offsetY;
          else
            inOutArea.bottom -= (int)offsetY;
        }
        break;
      }
      default: break;
    }
  }
  
  // ---

  // Get current cursor position, relative to client-area (-1 on failure)
  __forceinline void __WindowImplEventProc::readRelativeCursorPosition(HWND handle, const DisplayArea& clientArea, PixelPosition& outPos) noexcept {
    POINT cursorPos;
    if (GetCursorPos(&cursorPos)) {
      if (ScreenToClient(handle, &cursorPos)) {
        outPos.x = cursorPos.x;
        outPos.y = cursorPos.y;
      }
      else {
        outPos.x = cursorPos.x - clientArea.x;
        outPos.y = cursorPos.y - clientArea.y;
      }
    }
    else
      outPos.x = outPos.y = __P_UNKNOWN_CURSOR_POS;
  }

  // refresh scroll position during scroll event
  __forceinline void __WindowImplEventProc::refreshScrollPosition(__WindowImpl& window, UINT message, WPARAM wParam, 
                                                                  uint32_t& outX, uint32_t& outY) noexcept {
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
    if (message == WM_HSCROLL)
      window._lastScrollPosition.x = (int32_t)HIWORD(wParam);
    else
      window._lastScrollPosition.y = (int32_t)HIWORD(wParam);

    outX = window._lastScrollPosition.x;
    outY = window._lastScrollPosition.y;
  }

  // Cursor in client area: hide cursor / enable raw mouse events
  void __WindowImplEventProc::enableCursorMode(__WindowImpl& window) noexcept {
    if (window._statusFlags & __P_FLAG_CURSOR_HIDE) {
      if (window._statusFlags & __P_FLAG_CURSOR_RAW_INPUT) {
        const RAWINPUTDEVICE device = { 0x01, 0x02, 0, window._handle };
        if (!RegisterRawInputDevices(&device, 1, sizeof(device)))
          return; // failure -> do not hide cursor -> let user know that mouse capture doesn't work
        
        std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
        readRelativeCursorPosition(window._handle, window._lastClientArea, window._lastCursorPosition);
      }
      ShowCursor(false); // hide cursor
    }
  }
  // Cursor out of range: show cursor / disable raw mouse events
  void __WindowImplEventProc::disableCursorMode(uint32_t statusFlag) noexcept {
    if (statusFlag & __P_FLAG_CURSOR_HIDE) {
      if (statusFlag & __P_FLAG_CURSOR_RAW_INPUT) {
        const RAWINPUTDEVICE device = { 0x01, 0x02, RIDEV_REMOVE, nullptr };
        RegisterRawInputDevices(&device, 1, sizeof(device));
      }
      ShowCursor(true);
    }
  }
  
  // ---
  
  // Find new monitor for a window (returns: true if monitor has changed)
  bool __WindowImplEventProc::findWindowMonitor(__WindowImpl& window, const RECT& suggestedArea) noexcept {
    bool hasChanged = false;
    int32_t centerX = suggestedArea.left + ((suggestedArea.right - suggestedArea.left)>>1);
    int32_t centerY = suggestedArea.top + ((suggestedArea.bottom - suggestedArea.top)>>1);
    
    const DisplayArea& screenArea = window._monitor->attributes().screenArea;
    if (centerX < screenArea.x || centerX >= screenArea.x + (int32_t)screenArea.width
    ||  centerY < screenArea.y || centerY >= screenArea.y + (int32_t)screenArea.height) {
      try {
        auto monitors = DisplayMonitor::listAvailableMonitors();
        for (auto& it : monitors) {
          const DisplayArea& itArea = it.attributes().screenArea;
          if (centerX >= itArea.x && centerX < itArea.x + (int32_t)itArea.width
          &&  centerY >= itArea.y && centerY < itArea.y + (int32_t)itArea.height) {
            window._monitor = std::make_shared<DisplayMonitor>(std::move(it));
            hasChanged = true;
            break;
          }
        }
      }
      catch (...) {}
    }
    return hasChanged;
  }
  
  // Try to keep same client-area after DPI change (adapt to new decorations)
  __forceinline void __WindowImplEventProc::adjustWindowSizeOnDpiChange(__WindowImpl& window, const RECT& suggestedArea, 
                                                                        WindowDecorationSize& outDecorationSizes) noexcept {
    std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
    
    DisplayArea newClientArea;
    if (window.readVisibleClientArea(newClientArea)) {
      window.readVisibleWindowDecorations(newClientArea, outDecorationSizes);
    }
    else {
      window.computeWindowDecorations(window._currentStyleFlag, window._currentStyleExtFlag, window.hasMenu(), outDecorationSizes);
      newClientArea.x = (int32_t)suggestedArea.left + outDecorationSizes.left;
      newClientArea.y = (int32_t)suggestedArea.top + outDecorationSizes.top;
      newClientArea.width += window._decorationSizes.left + window._decorationSizes.right - outDecorationSizes.left - outDecorationSizes.right;
      newClientArea.height += window._decorationSizes.top + window._decorationSizes.bottom - outDecorationSizes.top - outDecorationSizes.bottom;
    }
    
    // new decoration sizes -> update client-area
    if (outDecorationSizes.top != window._decorationSizes.top || outDecorationSizes.right != window._decorationSizes.right) {
      window._decorationSizes = outDecorationSizes;
      if (SetWindowPos(window._handle, HWND_TOP, suggestedArea.left, suggestedArea.top,
                       (int)window._lastClientArea.width + outDecorationSizes.left + outDecorationSizes.right,
                        (int)window._lastClientArea.height + outDecorationSizes.top + outDecorationSizes.bottom, SWP_NOZORDER | SWP_NOACTIVATE) == FALSE) {
        window._lastClientArea = newClientArea; // on failure, update client-area
      }
    }
  }
  
  // ---
  
  // Drop files (drag and drop operation)
  __forceinline void __WindowImplEventProc::dragDropFiles(__WindowImpl& window, HDROP dropHandle) noexcept {
    // allocate paths
    uint32_t count = (uint32_t)DragQueryFileW(dropHandle, 0xFFFFFFFF, nullptr, 0);
    wchar_t** paths = (wchar_t**)calloc(count, sizeof(wchar_t*));
    if (paths == nullptr)
      return;
    
    // get path of dropped files
    POINT dropPos;
    DragQueryPoint(dropHandle, &dropPos);
    
    for (uint32_t i = 0; i < count; ++i) {
      UINT length = DragQueryFileW(dropHandle, i, nullptr, 0);
      paths[i] = (wchar_t*)calloc((size_t)length + 1, sizeof(wchar_t));
      if (paths[i] == nullptr)
        return;
      DragQueryFileW(dropHandle, i, paths[i], length + 1);
    }
    
    // notify handler
    try {
      window._onWindowEvent(&window._container, WindowEvent::dropFiles, count, (int32_t)dropPos.x, (int32_t)dropPos.y, (void*)paths);
    }
    catch (...) {} // avoid leaks
    DragFinish(dropHandle);
    
    for (uint32_t i = 0; i < count; ++i) // release paths
      free(paths[i]);
    free(paths);
  }
  

  // -- event handlers -- ------------------------------------------------------
  
  // Set/replace window/hardware event handler (NULL to unregister)
  void __WindowImpl::setWindowHandler(WindowEventHandler handler) noexcept { this->_onWindowEvent = handler; }
  // Set/replace size/position event handler (NULL to unregister)
  void __WindowImpl::setPositionHandler(PositionEventHandler handler) noexcept { this->_onPositionEvent = handler; }
  // Set/replace keyboard event handler (NULL to unregister)
  void __WindowImpl::setKeyboardHandler(KeyboardEventHandler handler) noexcept { this->_onKeyboardEvent = handler; }
  
  // Set/replace mouse event handler (NULL to unregister)
  void __WindowImpl::setMouseHandler(MouseEventHandler handler, Window::CursorMode cursor) noexcept {
    this->_onMouseEvent = handler;
    if ((this->_statusFlags & (__P_FLAG_CURSOR_HOVER|__P_FLAG_CURSOR_HIDE)) == (__P_FLAG_CURSOR_HOVER|__P_FLAG_CURSOR_HIDE))
      __WindowImplEventProc::disableCursorMode(this->_statusFlags);
    
    if (handler) {
      switch (cursor) {
        case Window::CursorMode::hidden:
          __P_ADD_REMOVE_FLAGS(this->_statusFlags, __P_FLAG_CURSOR_HIDE, __P_FLAG_CURSOR_RAW_INPUT); break;
        case Window::CursorMode::hiddenRaw:
          __P_ADD_FLAG(this->_statusFlags, __P_FLAG_CURSOR_HIDE | __P_FLAG_CURSOR_RAW_INPUT); break;
        case Window::CursorMode::visible:
        default:
          __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_CURSOR_HIDE | __P_FLAG_CURSOR_RAW_INPUT); break;
      }
      if (this->_statusFlags & __P_FLAG_CURSOR_HOVER) {
        std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
        __WindowImplEventProc::enableCursorMode(*this);
      }
    }
    else
      __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_CURSOR_HIDE | __P_FLAG_CURSOR_RAW_INPUT);
  }


// -- event processing -- ------------------------------------------------------
  
  // process keyboard events ([WM_KEYFIRST;WM_KEYLAST])
  __forceinline bool __WindowImplEventProc::processKeyboardEvent(__WindowImpl& window, UINT message, WPARAM wParam, LPARAM lParam) {
    KeyboardEvent eventType = KeyboardEvent::none;
    uint32_t keyCode = 0;
    uint32_t value = 0;
    
    switch (message) {
      // key pressed/released
      case WM_KEYDOWN:    
      case WM_KEYUP:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP: {
        if ((lParam & __P_VK_KEY_UP_FLAG) == 0) { // down
          eventType = (message == WM_KEYDOWN) ? KeyboardEvent::keyDown : KeyboardEvent::altKeyDown;
          value = (lParam & __P_VK_KEY_PREV_DOWN_FLAG) ? (uint32_t)KeyTransition::same : (uint32_t)KeyTransition::down;
        }
        else { // up
          eventType = (message == WM_KEYUP) ? KeyboardEvent::keyUp : KeyboardEvent::altKeyUp;
          value = (uint32_t)KeyTransition::up;
        }
        
        switch ((int)wParam) {
          case VK_F10: { // F10 key -> might be activation of menu
            if (message == WM_SYSKEYDOWN && window.hasMenu())
              eventType = KeyboardEvent::activateMenu;
            keyCode = (uint32_t)wParam;
            break;
          }
          case VK_SNAPSHOT: { // VK_SNAPSHOT doesn't send WM_KEYDOWN events -> create them
            keyCode = (uint32_t)wParam;
            if (value == (uint32_t)KeyTransition::up) 
              window._onKeyboardEvent(&window._container, eventType, keyCode, (uint32_t)KeyTransition::down);
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
    return (eventType != KeyboardEvent::none && window._onKeyboardEvent(&window._container, eventType, keyCode, value));
  }
  
  // process general mouse events (note: not WM_MOUSELEAVE/WM_MOUSEHOVER/WM_INPUT -> located outside of [WM_MOUSEFIRST;WM_MOUSELAST])
  __forceinline bool __WindowImplEventProc::processMouseEvent(__WindowImpl& window, UINT message, WPARAM wParam, LPARAM lParam) {
    MouseEvent eventType = MouseEvent::none;
    uint32_t value = 0;
    uint8_t activeKeys = 0;
    int32_t x = (int32_t)GET_X_LPARAM(lParam);
    int32_t y = (int32_t)GET_Y_LPARAM(lParam);
    
    switch (message) {
      case WM_MOUSEMOVE: { // TME_LEAVE is unregistered when the cursor leaves the window -> register when it comes back
        if ((window._statusFlags & __P_FLAG_CURSOR_EVENT_REG) == 0) {
          TRACKMOUSEEVENT tme;
          ZeroMemory(&tme, sizeof(tme));
          tme.cbSize = sizeof(tme);
          tme.dwFlags = TME_LEAVE | TME_HOVER;
          tme.dwHoverTime = HOVER_DEFAULT;
          tme.hwndTrack = window._handle;
          TrackMouseEvent(&tme);

          __P_ADD_FLAG(window._statusFlags, __P_FLAG_CURSOR_EVENT_REG);
        }
        if (window._statusFlags & __P_FLAG_CURSOR_RAW_INPUT)
          return false; // do not expose "normal" move events in raw mode
        
        eventType = MouseEvent::mouseMove; activeKeys = (uint8_t)wParam;
        break;
      }
      // button "value" aligned with MouseButton enum values
      case WM_LBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = 0; activeKeys = (uint8_t)wParam; break;
      case WM_LBUTTONUP:     eventType = MouseEvent::buttonUp;     value = 0; activeKeys = (uint8_t)wParam; break;
      case WM_MBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = 1; activeKeys = (uint8_t)wParam; break;
      case WM_MBUTTONUP:     eventType = MouseEvent::buttonUp;     value = 1; activeKeys = (uint8_t)wParam; break;
      case WM_RBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = 2; activeKeys = (uint8_t)wParam; break;
      case WM_RBUTTONUP:     eventType = MouseEvent::buttonUp;     value = 2; activeKeys = (uint8_t)wParam; break;
      case WM_XBUTTONDOWN:   eventType = MouseEvent::buttonDown;   value = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_XBUTTONUP:     eventType = MouseEvent::buttonUp;     value = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_MOUSEWHEEL:    eventType = MouseEvent::mouseWheelV; value = (int32_t)GET_WHEEL_DELTA_WPARAM(wParam);
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
      case WM_MOUSEHWHEEL:   eventType = MouseEvent::mouseWheelH; value = (int32_t)GET_WHEEL_DELTA_WPARAM(wParam);
                             activeKeys = (uint8_t)GET_KEYSTATE_WPARAM(wParam); break;
    }
    return (eventType != MouseEvent::none && window._onMouseEvent(&window._container, eventType, x, y, value, activeKeys));
  }
  
  // process raw mouse motion events
  __forceinline void __WindowImplEventProc::processRawInputEvent(__WindowImpl& window, LPARAM lParam) {
    UINT rawSize = 0;
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &rawSize, sizeof(RAWINPUTHEADER)) != 0) // get data length
      return;

    //adjust buffer size
    if (rawSize > window._mouseInputBufferSize) {
      if (window._mouseInputBuffer)
        free(window._mouseInputBuffer);

      window._mouseInputBuffer = (RAWINPUT*)calloc(1, rawSize);
      if (window._mouseInputBuffer == nullptr) {
        window._mouseInputBufferSize = 0;
        return;
      }
      window._mouseInputBufferSize = rawSize;
    }

    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window._mouseInputBuffer, &rawSize, sizeof(RAWINPUTHEADER)) != (UINT)-1) {
      RAWINPUT& rawData = *(window._mouseInputBuffer);
      if (rawData.header.dwType != RIM_TYPEMOUSE) // not a mouse event
        return;

      int32_t deltaX, deltaY;
      if (rawData.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
        std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
        if (window._lastCursorPosition.x != __P_UNKNOWN_CURSOR_POS) {
          deltaX = rawData.data.mouse.lLastX - window._lastCursorPosition.x;
          deltaY = rawData.data.mouse.lLastY - window._lastCursorPosition.y;
        }
        else
          deltaX = deltaY = 0;

        window._lastCursorPosition.x = rawData.data.mouse.lLastX;
        window._lastCursorPosition.y = rawData.data.mouse.lLastY;
      }
      else { // relative
        std::lock_guard<pandora::thread::RecursiveSpinLock> guard(window._sizePositionLock);
        deltaX = rawData.data.mouse.lLastX;
        deltaY = rawData.data.mouse.lLastY;
        if (window._lastCursorPosition.x != __P_UNKNOWN_CURSOR_POS) {
          window._lastCursorPosition.x += deltaX;
          window._lastCursorPosition.y += deltaY;
        }
        else
          readRelativeCursorPosition(window._handle, window._lastClientArea, window._lastCursorPosition);
      }

      window._onMouseEvent(&window._container, MouseEvent::rawMotion, deltaX, deltaY, 0, 0); // handler
    }
  }
  
  // ---

  // main window event processor
  LRESULT CALLBACK __WindowImpl::windowEventProcessor(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    __WindowImpl* window = reinterpret_cast<__WindowImpl*>(GetPropW(handle, __P_WINDOW_ID));
    if (window == nullptr)
      return DefWindowProcW(handle, message, wParam, lParam); // system event processor

    // > mouse events (note: missing WM_MOUSELEAVE and WM_INPUT: see "else" section) --
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
      if (window->_onMouseEvent && __WindowImplEventProc::processMouseEvent(*window, message, wParam, lParam))
        return 0; // handler said to ignore system processing
    }
    // > keyboard events --
    else if (message <= WM_KEYLAST && message >= WM_KEYFIRST) {
      if (window->_onKeyboardEvent && __WindowImplEventProc::processKeyboardEvent(*window, message, wParam, lParam))
        return 0; // handler said to ignore system processing
    }
    else {
      switch (message) {
        // > custom mouse move events
        case WM_MOUSEHOVER: { // mouse enters client area
          if ((window->_statusFlags & __P_FLAG_CURSOR_HOVER) == 0) {
            __P_ADD_FLAG(window->_statusFlags, __P_FLAG_CURSOR_HOVER);
            if ((window->_statusFlags & __P_FLAG_CURSOR_MENULOOP) == 0)
              __WindowImplEventProc::enableCursorMode(*window);
          }
          break;
        }
        case WM_MOUSELEAVE: { // mouse leaves client area (-> unregisters TME_LEAVE -> re-registered in WM_MOUSEMOVE in 'processMouseEvent')
          if ((window->_statusFlags & (__P_FLAG_CURSOR_HOVER|__P_FLAG_CURSOR_MENULOOP)) == __P_FLAG_CURSOR_HOVER)
            __WindowImplEventProc::disableCursorMode(window->_statusFlags);
          __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_CURSOR_EVENT_REG | __P_FLAG_CURSOR_HOVER);

          if (window->_onMouseEvent && window->_onMouseEvent(&window->_container, MouseEvent::mouseLeave, 0, 0, 0, 0))
            return 0;
          break;
        }
        case WM_ENTERMENULOOP: { // menu loop entered
          if (window->_statusFlags & __P_FLAG_CURSOR_HOVER)
            __WindowImplEventProc::disableCursorMode(window->_statusFlags);
          __P_ADD_FLAG(window->_statusFlags, __P_FLAG_CURSOR_MENULOOP);
          break;
        }
        case WM_EXITMENULOOP: { // menu loop left
          if (window->_statusFlags & __P_FLAG_CURSOR_HOVER)
            __WindowImplEventProc::enableCursorMode(*window);
          __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_CURSOR_MENULOOP);
          break;
        }
        case WM_INPUT: { // raw mouse moves
          if ((window->_statusFlags & __P_FLAG_CURSOR_RAW_INPUT) && window->_onMouseEvent)
            __WindowImplEventProc::processRawInputEvent(*window, lParam);
          break;
        }

        // > window size/move/scroll events --
        
        case WM_ENTERSIZEMOVE: { // manual user move/resize beginning
          __P_ADD_FLAG(window->_statusFlags, __P_FLAG_RESIZED_MOVED);
          break;
        }
        case WM_EXITSIZEMOVE: { // manual user move/resize complete
          __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_RESIZED_MOVED);
          DisplayArea newClientArea;
          if (__WindowImplEventProc::refreshClientArea(*window, newClientArea) == 2 && window->_onPositionEvent)
            window->_onPositionEvent(&window->_container, PositionEvent::sizePositionChanged, 
                                     newClientArea.x, newClientArea.y, newClientArea.width, newClientArea.height);
          break;
        }
        case WM_MOVE: { // window currently moving / moved by command
          if (window->_onPositionEvent) {
            PixelSize clientSize = window->lastClientSize();
            int16_t x = LOWORD(lParam); // force 16bit ints to keep value sign
            int16_t y = HIWORD(lParam);
            window->_onPositionEvent(&window->_container, (window->_statusFlags & __P_FLAG_RESIZED_MOVED)
                                     ? PositionEvent::sizePositionTrack : PositionEvent::sizePositionChanged, // user move tracking / system move
                                     (int32_t)x, (int32_t)y, clientSize.width, clientSize.height);
          }
          break;
        }
        case WM_SIZING: { // window currently resized
          if ((window->_resizeMode & ResizeMode::resizable) == true) {
            if ((RECT*)lParam != nullptr) {
              bool isModified = ((window->_resizeMode & ResizeMode::homothety) == true);
              RECT* area = (RECT*)lParam;
              if (isModified)
                __WindowImplEventProc::resizeWithHomothety(*window, (int)wParam, *area);

              if (window->_onPositionEvent && (window->_statusFlags & __P_FLAG_RESIZED_MOVED))
                window->_onPositionEvent(&window->_container, PositionEvent::sizePositionTrack, // size tracking handler
                                         (int32_t)area->left, (int32_t)area->top, static_cast<uint32_t>(area->right - area->left), 
                                         static_cast<uint32_t>(area->bottom - area->top));
              if (isModified)
                return TRUE;
            }
          }
          break;
        }
        // resized with default action (restore/maximize/minimize)
        case WM_SIZE: {
          switch (wParam) {
            case SIZE_MINIMIZED: {
              if (window->_statusFlags & __P_FLAG_CURSOR_HOVER)
                __WindowImplEventProc::disableCursorMode(window->_statusFlags);
              if (window->_statusFlags & __P_FLAG_FULLSCREEN_ON)
                window->exitFullscreenMode();
              __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_WINDOW_MINIMIZED, 
                                   __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_RESIZED_MOVED | __P_FLAG_CURSOR_EVENT_REG | __P_FLAG_CURSOR_HOVER);
              
              if (window->_onWindowEvent)
                window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (uint32_t)WindowActivity::hidden, 0,0,nullptr);
              break;
            }
            case SIZE_RESTORED:
            case SIZE_MAXIMIZED: {
              bool wasActive = (window->_statusFlags & __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE) == (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE);
              if (wParam == SIZE_RESTORED)
                __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE, 
                                     __P_FLAG_WINDOW_MINIMIZED | __P_FLAG_WINDOW_MAXIMIZED | __P_FLAG_RESIZED_MOVED);
              else
                __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_WINDOW_MAXIMIZED | __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE, 
                                     __P_FLAG_WINDOW_MINIMIZED | __P_FLAG_RESIZED_MOVED);
              if (!wasActive) {
                if (window->_onWindowEvent)
                  window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (uint32_t)WindowActivity::active, 0,0,nullptr);
              }

              if (window->_mode != WindowType::fullscreen) {
                DisplayArea clientArea;
                __WindowImplEventProc::refreshOnMaximizeRestore(*window, lParam, clientArea);
                if (window->_onPositionEvent)
                  window->_onPositionEvent(&window->_container, PositionEvent::sizePositionChanged, 
                    clientArea.x, clientArea.y, clientArea.width, clientArea.height);
              }
              else {
                window->enterFullscreenMode(window->lastWindowArea(), window->_refreshRate);
                SetForegroundWindow(window->_handle);
                SetFocus(window->_handle);
              }
              break;
            }
          }
          break;
        }
        // size limits
        case WM_GETMINMAXINFO: {
          if (window->_mode != WindowType::fullscreen && (MINMAXINFO*)lParam != nullptr) {
            DisplayArea clientArea, windowArea;
            if (!window->readVisibleClientArea(clientArea))
              clientArea = window->lastClientArea();
            if (!window->readVisibleWindowArea(windowArea))
              windowArea = window->lastWindowArea();

            // min size
            MINMAXINFO* limits = (MINMAXINFO*)lParam;
            limits->ptMinTrackSize.x = (int)window->_minClientSize.width + (int)windowArea.width - (int)clientArea.width;
            limits->ptMinTrackSize.y = (int)window->_minClientSize.height + (int)windowArea.height - (int)clientArea.height;
            
            // resize limits
            if ((window->_resizeMode & ResizeMode::resizable) != ResizeMode::resizable) { // x and/or y not resizable
              // homothety: if not both dimensions resizable, none can be resized
              if ((window->_resizeMode & ResizeMode::homothety) == true) {
                limits->ptMaxSize.x = limits->ptMinTrackSize.x = limits->ptMaxTrackSize.x = (int)windowArea.width;
                limits->ptMaxSize.y = limits->ptMinTrackSize.y = limits->ptMaxTrackSize.y = (int)windowArea.height;
              }
              else {
                if ((window->_resizeMode & ResizeMode::resizableX) == 0)
                  limits->ptMaxSize.x = limits->ptMinTrackSize.x = limits->ptMaxTrackSize.x = (int)windowArea.width;
                if ((window->_resizeMode & ResizeMode::resizableY) == 0)
                  limits->ptMaxSize.y = limits->ptMinTrackSize.y = limits->ptMaxTrackSize.y = (int)windowArea.height;
              }
            }
            return 0;
          }
          break;
        }

        // scrollbar slider change
        case WM_VSCROLL:
        case WM_HSCROLL: {
          if (window->_onPositionEvent) {
            uint32_t type = (uint32_t)LOWORD(wParam);
            if (type == SB_THUMBTRACK) {
              uint32_t posX, posY;
              __WindowImplEventProc::refreshScrollPosition(*window, message, wParam, posX, posY);
              if (window->_onPositionEvent(&window->_container, PositionEvent::scrollPositionTrack, posX, posY,
                                           window->_maxScrollPosition.width, window->_maxScrollPosition.height))
                return 0;
            }
            else if (type == SB_THUMBPOSITION) {
              uint32_t posX, posY;
              __WindowImplEventProc::refreshScrollPosition(*window, message, wParam, posX, posY);
              if (window->_onPositionEvent(&window->_container, PositionEvent::scrollPositionChanged, posX, posY,
                                           window->_maxScrollPosition.width, window->_maxScrollPosition.height))
                return 0;
            }
          }
          break;
        }
        
        // > window visibility events --

        // show/hide window
        case WM_SHOWWINDOW: {
          if (wParam == TRUE) { // visible
            __P_ADD_FLAG(window->_statusFlags, __P_FLAG_WINDOW_VISIBLE);
            if (window->_onWindowEvent)
              window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (window->_statusFlags & __P_FLAG_WINDOW_ACTIVE) 
                                                                        ? (uint32_t)WindowActivity::active 
                                                                        : (uint32_t)WindowActivity::inactive, 0,0,nullptr);
            if (window->_mode == WindowType::fullscreen)
              window->enterFullscreenMode(window->lastWindowArea(), window->_refreshRate);
          }
          else {
            if (window->_statusFlags & __P_FLAG_FULLSCREEN_ON)
              window->exitFullscreenMode();
            __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_WINDOW_VISIBLE);
            if (window->_onWindowEvent)
              window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (uint32_t)WindowActivity::hidden, 0,0,nullptr);
          }
          break;
        }
        // current/other window selected /  win key / ALT+TAB to/from other window
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP: {
          if (wParam) { // active
            if ((window->_statusFlags & __P_FLAG_WINDOW_ACTIVE) == 0) {
              if (window->_onWindowEvent)
                window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (uint32_t)WindowActivity::active, 0, 0, nullptr);
              __P_ADD_FLAG(window->_statusFlags, __P_FLAG_WINDOW_ACTIVE | __P_FLAG_WINDOW_VISIBLE);

              if (window->_mode != WindowType::fullscreen) {
                DisplayArea clientArea;
                __WindowImplEventProc::refreshClientArea(*window, clientArea);
              }
            }
          }
          else if (window->_statusFlags & __P_FLAG_WINDOW_ACTIVE) { // inactive
            __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_WINDOW_ACTIVE);
            if (window->_mode == WindowType::fullscreen)
              SendMessage(window->_handle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            if (window->_onWindowEvent)
              window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (window->_statusFlags & __P_FLAG_WINDOW_VISIBLE) 
                ? (uint32_t)WindowActivity::inactive 
                : (uint32_t)WindowActivity::hidden, 0,0,nullptr);
          }
          break;
        }
        // window decoration painting -> prevent if fullscreen
        case WM_NCACTIVATE:
        case WM_NCPAINT: {
          if (window->_statusFlags & __P_FLAG_FULLSCREEN_ON)
            return TRUE;
          break;
        }
        
        // > window close/menu/drop events --
        
        // window close attempt -> notify handler
        case WM_CLOSE: {
          if (!window->_onWindowEvent || !window->_onWindowEvent(&window->_container, WindowEvent::windowClosed, 0,0,0,nullptr))
            window->_destroy();
          return 0;
        }
        case WM_DESTROY: { // close success -> confirmation message
          PostQuitMessage(0);
          return 0;
        }
        // menu shortcut
        case WM_MENUCHAR: {
          if (!window->hasMenu()) // if no menu handler: invalid mnemonic or accelerator key -> avoid beep
            return MAKELRESULT(0, MNC_CLOSE);
          break;
        }
        // menu selection
        case WM_MENUSELECT: {
          if (window->_onWindowEvent && window->hasMenu()) {
            if (HIWORD(wParam) == MF_SYSMENU) // submenu selected
              window->_onWindowEvent(&window->_container, WindowEvent::menuSelected, 1, (int32_t)LOWORD(wParam), -1, (void*)lParam);
            else // menu command item
              window->_onWindowEvent(&window->_container, WindowEvent::menuSelected, 0, -1, (int32_t)LOWORD(wParam), nullptr);
          }
          break;
        }
        // drag and drop
        case WM_DROPFILES: {
          if (window->_onWindowEvent) {
            __WindowImplEventProc::dragDropFiles(*window, (HDROP)wParam);
            return 0;
          }
          break;
        }
        
        // > hardware events --
        
        // display resolution change
        case WM_DISPLAYCHANGE: { 
          if (window->_statusFlags & __P_FLAG_MODE_CHANGE_PENDING) { // change made by the window -> just reset flag
            __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_MODE_CHANGE_PENDING);
          }
          else if (window->_mode != WindowType::fullscreen) { // external change -> verify if not just another fullscreen window
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
          if (window->_mode != WindowType::fullscreen && (RECT*)lParam != nullptr) {
            RECT* suggestedArea = (RECT*)lParam;
            bool isDifferentMonitor = __WindowImplEventProc::findWindowMonitor(*window, *suggestedArea); // find appropriate window monitor
            
            WindowDecorationSize decorationSizes;
            __WindowImplEventProc::adjustWindowSizeOnDpiChange(*window, *suggestedArea, decorationSizes); // keep same client-area on new monitor

            // notify handler, to let parent app decide if client size should be adjusted
            if (window->_onWindowEvent) {
              int32_t adjustedClientWidth = static_cast<int32_t>(suggestedArea->right - suggestedArea->left - decorationSizes.left - decorationSizes.right);
              int32_t adjustedClientHeight = static_cast<int32_t>(suggestedArea->bottom - suggestedArea->top - decorationSizes.top - decorationSizes.bottom);
              window->_onWindowEvent(&window->_container, WindowEvent::dpiChanged, static_cast<uint32_t>(window->contentScale()*100.0f),
                                     adjustedClientWidth, adjustedClientHeight, isDifferentMonitor ? (void*)window->_monitor->handle() : nullptr);
            }
            return 0;
          }
          break;
        }
        // external device connected/removed (gamepad, joystick...)
        case WM_DEVICECHANGE: { 
          if (window->_onWindowEvent && (DEV_BROADCAST_HDR*)lParam != nullptr) {
            if (wParam == DBT_DEVICEARRIVAL) {
              if (((DEV_BROADCAST_HDR*)lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                window->_onWindowEvent(&window->_container, WindowEvent::deviceInterfaceChange, 1, 0,0,nullptr);
            }
            else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
              if (((DEV_BROADCAST_HDR*)lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                window->_onWindowEvent(&window->_container, WindowEvent::deviceInterfaceChange, 0, 0,0,nullptr);
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
              if (!window->hasMenu()) // no menu -> ignore
                return 0;
              break;
            default: break;
          }
          break;
        }
        // system suspend/resume
        case WM_POWERBROADCAST: {
          if (wParam == PBT_APMQUERYSUSPEND) {
            __P_ADD_FLAG(window->_statusFlags, __P_FLAG_POWER_SUSPENDED);
            if (window->_onWindowEvent)
              window->_onWindowEvent(&window->_container, WindowEvent::suspendResume, 1, 0,0,nullptr);
          }
          else if (wParam == PBT_APMRESUMEAUTOMATIC) {
            __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_POWER_SUSPENDED);
            if (window->_onWindowEvent)
              window->_onWindowEvent(&window->_container, WindowEvent::suspendResume, 0, 0,0,nullptr);
          }
          break;
        }
        // input language change
        case WM_INPUTLANGCHANGE: {
          if (window->_onWindowEvent)
            window->_onWindowEvent(&window->_container, WindowEvent::inputLangChanged, (uint32_t)wParam, 
                                   ((int32_t)lParam & 0xFF), (((int32_t)lParam & 0xFF00) >> 8), (void*)lParam);
          break; 
        }
        default: break;
      }
    }
    
    // if original window processor existed, use it
    if (window->_statusFlags & __P_FLAG_USE_ORIG_EVENT_PROC)
      return window->_originalStyle->_eventProcessor(handle, message, wParam, lParam);
    return DefWindowProcW(handle, message, wParam, lParam); // system window processor
  }



# undef __if_constexpr
#endif
