/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window manager + builder - Win32 implementation (Windows)
--------------------------------------------------------------------------------
Includes hpp implementations at the end of the file 
(grouped object improves compiler optimizations + reduces executable size)
*******************************************************************************/
#ifdef _WINDOWS

# define NOMINMAX
# define NOMCX
# define NOMETAFILE
# define NOOPENFILE
# define NOSCROLL
# define NOSOUND
# define NOKANJI
# define NOHELP
# define NOPROFILER
# define NOTAPE

# include <cstdint>
# include <cstring>
# include <thread/spin_lock.h>
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include <windowsx.h>
# include <shellapi.h>
# include <Dbt.h>

# include "video/window_events.h"
# include "video/window_keycodes.h"
# include "video/window_resource.h"
# include "video/_private/_window_impl_win32.h"
# include "video/window.h"

# include <system/force_inline.h>
# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
#   define __if_constexpr if constexpr
# else
#   define __if_constexpr if
# endif

  using namespace pandora::video;
  using pandora::hardware::DisplayArea;
  using pandora::hardware::DisplayMode;
  using pandora::hardware::DisplayMonitor;
  using pandora::system::AppInstanceHandle;
  using pandora::system::WindowsApp;


// -----------------------------------------------------------------------------
// PORTABLE INTERFACE
// -----------------------------------------------------------------------------

// -- Window Builder - new window -- -------------------------------------------

  // Configure window class context + create new window
  std::unique_ptr<Window> Window::Builder::create(const window_char* contextName, const window_char* caption, 
                                                  WindowHandle parentWindow) { // throws
    if (contextName == nullptr || *contextName == 0)
      throw std::invalid_argument("Window: NULL/empty contextName");
  
    WNDCLASSEXW windowClass;
    ZeroMemory(&windowClass, sizeof(WNDCLASSEXW));
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpszClassName = contextName;
    
    // style / events
    windowClass.lpfnWndProc = &__WindowImpl::windowEventProcessor;
    windowClass.style = __WindowImpl::toWindowClassStyleFlag(this->_params.behavior);
    
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
      this->_params.backgroundColor = WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0));
    windowClass.hbrBackground = (HBRUSH)this->_params.backgroundColor->handle();
    
    // window creation
    if (RegisterClassExW(&windowClass) == 0)
      throw std::runtime_error("Window: context reg. failure");
    try {
      return std::unique_ptr<Window>(new Window(contextName, caption, this->_params, parentWindow));
    }
    catch (...) {
      UnregisterClassW(contextName, windowClass.hInstance);
      throw;
    }
  }
  
  // ---
  
  // New window construction
  Window::Window(const window_char* contextName, const window_char* caption, 
                 const Window::Builder::Params& params, WindowHandle parentWindow) { // throws
    // resources
    this->_appIcon = params.appIcon;
    this->_captionIcon = params.captionIcon;
    this->_cursor = params.cursor;
    this->_backgroundColor = params.backgroundColor;
    this->_menu = params.menu;
    
    this->_impl = new __WindowImpl(*this, (const wchar_t*)caption, (const wchar_t*)contextName,
                                   params, (HWND)parentWindow); // throws
  }


// -- Window Builder - existing window -- --------------------------------------

  // Build instance from existing window
  std::unique_ptr<Window> Window::Builder::update(WindowHandle existingHandle, bool callExistingEventProc) { // throws
    if (this->_params.backgroundColor == nullptr)
      this->_params.backgroundColor = WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0));
    
    return std::unique_ptr<Window>(new Window(this->_params, existingHandle, callExistingEventProc));
  }
  
  // Existing window re-styling
  Window::Window(const Window::Builder::Params& params, WindowHandle existingHandle, bool callExistingEventProc) { // throws
    // resources
    this->_appIcon = params.appIcon;
    this->_captionIcon = params.captionIcon;
    this->_cursor = params.cursor;
    this->_backgroundColor = params.backgroundColor;
    this->_menu = params.menu;
    
    this->_impl = new __WindowImpl(*this, (HWND)existingHandle, params, callExistingEventProc); // throws
  }


// -- Window - destruction -- --------------------------------------------------

  // destroy window (or restore existing window style)
  Window::~Window() noexcept {
    if (this->_impl) {
      delete _impl;
      this->_impl = nullptr;
    }
  }


// -- accessors -- -------------------------------------------------------------
  
  // Get native window handle
  WindowHandle Window::handle() const noexcept { return (WindowHandle)this->_impl->handle(); }
  // Get window type (fullscreen/borderless/window/...)
  WindowType Window::displayMode() const noexcept { return this->_impl->displayMode(); }
  // Get window type (fullscreen/borderless/window/...)
  WindowBehavior Window::behavior() const noexcept { return this->_impl->behavior(); }
  // Get current resizing mode (resizable X/Y, homothety)
  ResizeMode Window::resizeMode() const noexcept { return this->_impl->resizeMode(); }
  // Get native menubar handle
  MenuHandle Window::menuHandle() const noexcept { return (MenuHandle)this->_impl->menuHandle(); }
  // Verify if current window has a parent window
  bool Window::hasParent() const noexcept { return this->_impl->hasParent(); }
  
  // Get current window visibility state
  Window::VisibleState Window::visibleState() const noexcept { return this->_impl->getVisibleState(); }
  
  // Get current window area (position / size)
  DisplayArea Window::getWindowArea() const noexcept { return this->_impl->lastWindowArea(); }
  // Get current window client area (position / size)
  DisplayArea Window::getClientArea() const noexcept { return this->_impl->lastClientArea(); }
  // Get current window client area size
  PixelSize Window::getClientSize() const noexcept { return this->_impl->lastClientSize(); }
  // Get current monitor scaling (based on DPI)
  float Window::contentScale() const noexcept { return this->_impl->contentScale(); }
  
  // ---
  
  // Get display monitor on which the window is located
  const pandora::hardware::DisplayMonitor& Window::displayMonitor() const noexcept { return this->_impl->displayMonitor(); }
  
  // Read current caption title
  std::basic_string<window_char> Window::getCaption() const {
    WCHAR buffer[128]{ 0 };
    int length = GetWindowTextW(this->_impl->handle(), buffer, sizeof(buffer)/sizeof(WCHAR));
    if (length > 0) // result != -1, and not empty
      return std::basic_string<window_char>(buffer, length); 
    return L""; 
  }
  
  // Get current cursor mode (visibility/capture)
  Window::CursorMode Window::getCursorMode() const noexcept { return this->_impl->getCursorMode(); }
  
  // Get current mouse pointer position
  PixelPosition Window::getCursorPosition(Window::CursorPositionType mode) const noexcept {
    POINT pos;
    if (GetCursorPos(&pos) != FALSE) {
      if (mode == Window::CursorPositionType::relative) { // relative to client area
        if (ScreenToClient(this->_impl->handle(), &pos) != FALSE)
          return PixelPosition{ pos.x, pos.y };
        else {
          PixelPosition clientPos = this->_impl->lastClientPosition();
          return PixelPosition{ pos.x - clientPos.x, pos.y - clientPos.y };
        }
      }
      else // absolute
        return PixelPosition{ pos.x, pos.y };
    }
    return { -1, -1 };
  }

  // ---
  
  // Get last error message (on change failure)
  std::string Window::getLastError() {
    return __WindowImpl::formatLastError(nullptr);
  }
  
  
// -- display changes -- -------------------------------------------------------
  
  // Change window visibility state
  bool Window::show(Window::VisibilityCommand state) noexcept {
    return this->_impl->show((this->_impl->displayMode() != WindowType::fullscreen) 
                            ? __WindowImpl::toWindowVisibilityFlag(state) 
                            : __WindowImpl::toFullscreenVisibilityFlag(state) );
  }
  // Change position
  bool Window::move(int32_t x, int32_t y) noexcept { return this->_impl->move(x, y); }
  // Change size
  bool Window::resize(uint32_t width, uint32_t height) noexcept { return this->_impl->resize(width, height); }
  // Change size + position
  bool Window::resize(const DisplayArea& clientArea) noexcept { return this->_impl->resize(clientArea); }
  
  // ---
  
  // Change window style and position/size/resolution
  bool Window::setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resizeMode,
                              const DisplayArea& clientArea, uint32_t rate) {
    return this->_impl->setDisplayMode(type, behavior, resizeMode, clientArea, rate);
  }
  // Define minimum size limits for the user (ignored if not resizable)
  bool Window::setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept {
    return this->_impl->setMinClientAreaSize(minWidth, minHeight);
  }

  // ---
  
  // Clear entire client area (with background color)
  bool Window::clearClientArea() noexcept {
    HWND handle = this->_impl->handle();
    RECT clientArea;
    this->_impl->lastRelativeClientRect(clientArea);

    HDC hdc = GetDC(handle);
    if (hdc) {
      FillRect(hdc, &clientArea, (HBRUSH)this->_backgroundColor->handle());
      ReleaseDC(handle, hdc); 
      return true;
    }
    return false;
  }
  // Clear rectangle relative to client area (with background color)
  bool Window::clear(const DisplayArea& area) noexcept {
    HWND handle = this->_impl->handle();
    RECT drawArea{};
    drawArea.left = (int)area.x;
    drawArea.top = (int)area.y;
    drawArea.right = (int)area.x + (int)area.width;
    drawArea.bottom = (int)area.y + (int)area.height;
    
    HDC hdc = GetDC(handle);
    if (hdc) {
      FillRect(hdc, &drawArea, (HBRUSH)this->_backgroundColor->handle());
      ReleaseDC(handle, hdc); 
      return true;
    }
    return false;
  }
  
  
// -- resource changes -- ------------------------------------------------------
  
  // Change mouse pointer X/Y position
  bool Window::setCursorPosition(int32_t x, int32_t y, Window::CursorPositionType mode) noexcept {
    return this->_impl->setCursorPosition(x, y, mode);
  }
  // Change mouse pointer visibility/limits/events
  void Window::setCursorMode(CursorMode cursorMode) noexcept {
    return this->_impl->setCursorMode(cursorMode);
  }
  
  // Change window title
  bool Window::setCaption(const window_char* caption) noexcept {
    return (SetWindowTextW(this->_impl->handle(), (caption != nullptr) ? caption : L"") != FALSE);
  }
  // Replace window menu (or remove if null)
  bool Window::setMenu(std::shared_ptr<WindowResource> menu) noexcept {
    if (this->_impl->setMenu((menu != nullptr) ? (HMENU)menu->handle() : nullptr)) {
      this->_menu = menu;
      return true;
    }
    return false;
  }
  
  // Change cursor resource
  bool Window::setCursor(std::shared_ptr<WindowResource> cursor) noexcept {
    if (cursor != nullptr && cursor->handle()) {
      SetLastError(0);
      if (SetClassLongPtr(this->_impl->handle(), GCLP_HCURSOR, (LONG_PTR)cursor->handle()) != 0 || GetLastError() == 0) {
        this->_cursor = cursor;
        return true;
      }
    }
    return false;
  }
  // Change background color resource
  bool Window::setBackgroundColorBrush(std::shared_ptr<WindowResource> colorBrush) noexcept {
    if (colorBrush != nullptr && colorBrush->handle()) {
      SetLastError(0);
      if (SetClassLongPtr(this->_impl->handle(), GCLP_HBRBACKGROUND, (LONG_PTR)colorBrush->handle()) != 0 || GetLastError() == 0) {
        this->_backgroundColor = colorBrush;
        return true;
      }
    }
    return false;
  }

  
// -- event management -- ------------------------------------------------------

  // Set/replace window/hardware event handler (NULL to unregister)
  void Window::setWindowHandler(WindowEventHandler handler) noexcept { this->_impl->setWindowHandler(handler); }
  // Set/replace size/position event handler (NULL to unregister)
  void Window::setPositionHandler(PositionEventHandler handler) noexcept { this->_impl->setPositionHandler(handler); }
  // Set/replace keyboard event handler (NULL to unregister)
  void Window::setKeyboardHandler(KeyboardEventHandler handler) noexcept { this->_impl->setKeyboardHandler(handler); }
  // Set/replace mouse event handler (NULL to unregister)
  void Window::setMouseHandler(MouseEventHandler handler, Window::CursorMode cursorMode) noexcept { this->_impl->setMouseHandler(handler, cursorMode); }
  
  // ---

  // Process/forward pending events for all existing windows (user input, size changes, shutdown...).
  bool Window::pollEvents() noexcept {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        break;
      
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    return (__WindowImpl::getWindowInstanceCount() > 0);
  }
  // Process/forward pending events for current window only (user input, size changes, shutdown...).
  bool Window::pollCurrentWindowEvents() noexcept {
    HWND handle = this->_impl->handle();
    bool _isAlive = (handle != nullptr);

    MSG msg;
    if (_isAlive) {
      while (PeekMessageW(&msg, handle, 0, 0, PM_REMOVE)) {
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
  
  // Send 'windowClose' event to any window instance
  void Window::sendCloseEvent(WindowHandle target) noexcept {
    if (target != nullptr)
      SendMessage((HWND)target, WM_CLOSE, 0, 0);
  }
  

// _____________________________________________________________________________


// -----------------------------------------------------------------------------
// PRIVATE IMPLEMENTATION
// -----------------------------------------------------------------------------

// -- constants --

# define __P_WINDOW_ID  L"PTWo" // instance ID per handle, for event processor ("Pandora-Toolbox Window object")

# define __P_VK_EXTENDED_FLAG       0x01000000  // win32 extended key flag
# define __P_VK_KEY_PREV_DOWN_FLAG  0x40000000  // win32 previous key-down state flag
# define __P_VK_KEY_UP_FLAG         0x80000000  // win32 key-up state flag
# define __P_VK_KEY_SCANCODE_OFFSET 16

  namespace pandora {
    namespace video {
      class __WindowImplEventProc final {
      public:
        static __forceinline int refreshClientArea(__WindowImpl& window, DisplayArea& outArea) noexcept;
        static void refreshOnMaximizeRestore(__WindowImpl& window, bool isMaximized, LPARAM lParam, DisplayArea& outArea) noexcept;
        static __forceinline void resizeWithHomothety(__WindowImpl& window, int movedBorder, RECT& inOutArea) noexcept;
        
        static __forceinline void setCursorOnHover(uint32_t& statusFlag) noexcept;
        static __forceinline void unsetCursorOnLeave(uint32_t& statusFlag) noexcept;
        static __forceinline void setCursorOnMenuExit(__WindowImpl& window) noexcept;
        static __forceinline void unsetCursorOnMenuEnter(uint32_t& statusFlag) noexcept;
        static void activateCursor(__WindowImpl& window) noexcept;
        static void unactivateCursor(__WindowImpl& window) noexcept;
        static void centerCursor(__WindowImpl& window) noexcept;
        static __forceinline void clipCursor(__WindowImpl& window) noexcept;
        
        static bool findWindowMonitor(__WindowImpl& window, const RECT& suggestedArea) noexcept;
        static __forceinline void adjustWindowSizeOnDpiChange(__WindowImpl& window, const RECT& suggestedArea, 
                                                              WindowDecorationSize& outDecorationSizes) noexcept;
        
        static void dragDropFiles(__WindowImpl& window, HDROP dropHandle) noexcept;

        static __forceinline bool processKeyboardEvent(__WindowImpl& window, UINT message, WPARAM wParam, LPARAM lParam);
        static __forceinline bool processMouseEvent(__WindowImpl& window, UINT message, WPARAM wParam, LPARAM lParam);
        static __forceinline void processRawInputEvent(__WindowImpl& window, LPARAM lParam);
      };
    }
  }
  
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
    _oldProp = GetPropW(handle, __P_WINDOW_ID); // if parent is a pandora::video::Window instance, backup prop
    _appIcon = (HICON)GetClassLongPtr(handle, GCLP_HICON);
    _captionIcon = (HICON)GetClassLongPtr(handle, GCLP_HICONSM);
    _cursor = (HCURSOR)GetClassLongPtr(handle, GCLP_HCURSOR);
    _backgroundColor = (HBRUSH)GetClassLongPtr(handle, GCLP_HBRBACKGROUND);
    _menu = (HMENU)GetMenu(handle);
    _eventProcessor = (WNDPROC)GetWindowLongPtr(handle, GWLP_WNDPROC);
    
    WCHAR buffer[128]{ 0 };
    if (GetWindowTextW(handle, buffer, sizeof(buffer)/sizeof(WCHAR)) > 0)
      _caption = buffer;
    
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
    if (!_caption.empty())
      SetWindowTextW(handle, _caption.c_str());
    
    // restore style
    SetWindowLong(handle, GWL_STYLE, _windowStyle | WS_VISIBLE);
    SetWindowLong(handle, GWL_EXSTYLE, _windowStyleExt);
    SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)_eventProcessor);

    SetWindowPos(handle, HWND_TOP, (int)_windowArea.left, (int)_windowArea.top,
                 (int)_windowArea.right - _windowArea.left, (int)_windowArea.bottom - _windowArea.top, 
                 SWP_SHOWWINDOW | SWP_NOZORDER | SWP_FRAMECHANGED);
    if (_oldProp) // if parent is a pandora::video::Window instance, restore prop (after restoring size, in case it's fixed-size)
      SetPropW(handle, __P_WINDOW_ID, _oldProp);
    if (_menu)
      DrawMenuBar(handle);
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
      throw std::runtime_error("Window: creation failed");
    __incrementWindowCount(); // only increment after handle creation
    
    // reference window instance as user data (for event processor)
    if (SetPropW(this->_handle, __P_WINDOW_ID, (HANDLE)this) == FALSE)
      throw std::runtime_error("Window: message handling config failed");
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
      throw std::invalid_argument("Window: NULL existingHandle");
    __incrementWindowCount();

    // reference instance as user data (for event processor)
    if (SetPropW(this->_handle, __P_WINDOW_ID, (HANDLE)this) == FALSE)
      throw std::runtime_error("Window: message handling config failed");
    if (callExistingEventProc && _originalStyle->_eventProcessor != nullptr)
      __P_ADD_FLAG(this->_statusFlags, __P_FLAG_USE_ORIG_EVENT_PROC);
    
    // replace window resources
    SetLastError(0);
    if (SetWindowLongPtr(existingHandle, GWLP_WNDPROC, (LONG_PTR)&__WindowImpl::windowEventProcessor) == 0 && GetLastError() != 0)
      throw std::runtime_error("Window: event binding failed"); // if returns 0, verify if error occurred
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
      throw std::runtime_error("Window: position/size change failed");
    if (this->_menuHandle)
      DrawMenuBar(this->_handle);
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
  }
  
  // ---

  // Destroy window (or restore existing window style)
  void __WindowImpl::_destroy() noexcept {
    if (this->_handle) {
      __WindowImplEventProc::unsetCursorOnLeave(this->_statusFlags);
      __WindowImplEventProc::unactivateCursor(*this);
      if (this->_mode == WindowType::fullscreen || this->_statusFlags & __P_FLAG_FULLSCREEN_ON)
        show(SW_SHOWMINNOACTIVE); // minimize fullscreen window + restore original display mode
      
      __decrementWindowCount(); // inform 'pollEvents()' whether it's the last window or not -> decrement before destroying

      // existing window: restore original style
      if (this->_originalStyle != nullptr) {
        RemovePropW(this->_handle, __P_WINDOW_ID); // unreference window instance BEFORE restoring it (avoid constraints from current instance)
        show(SW_RESTORE);
        _originalStyle->applyStyle(this->_handle);
        this->_handle = nullptr;
      }
      // new window: destroy
      else {
        HWND handle = this->_handle;
        this->_handle = nullptr; // reset before destroying -> avoid recursive destruction by WM_DESTROY
        DestroyWindow(handle);
        RemovePropW(handle, __P_WINDOW_ID); // unreference window instance AFTER destroying it -> let WM_DESTROY send WM_QUIT
      }
    }
    if (!this->_windowClassName.empty())
      UnregisterClassW(this->_windowClassName.c_str(), this->_moduleInstance);
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
    PixelSize expectedClientSize = lastClientSize();
    
    if (this->_mode == WindowType::fullscreen) { // fullscreen mode
      isVisibilityCommandSet = (visibilityFlag != SW_HIDE && visibilityFlag != SW_SHOWMINNOACTIVE);
      enterFullscreenMode(lastWindowArea(), this->_refreshRate);
    }
    else { // window mode
      SetLastError(0);
      isVisibilityCommandSet = (visibilityFlag == SW_SHOW || visibilityFlag == SW_SHOWDEFAULT || visibilityFlag == SW_RESTORE
                             || (visibilityFlag == SW_SHOWMAXIMIZED && (this->_currentStyleFlag & WS_MAXIMIZE)) );
      if (ShowWindow(this->_handle, isVisibilityCommandSet ? visibilityFlag : SW_SHOW) == FALSE && GetLastError() != 0)
        return false; // display failure
    }
    if (this->_menuHandle)
      DrawMenuBar(this->_handle);
    
    adjustVisibleClientSize(expectedClientSize); // adjust size, based on actual window
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

    SetLastError(0);
    if (visibilityFlag == SW_HIDE || visibilityFlag == SW_SHOWMINNOACTIVE) {
      if (ShowWindow(this->_handle, visibilityFlag) == FALSE && GetLastError() != 0)
        return false;
    }
    else {
      if (ShowWindow(this->_handle, (this->_mode == WindowType::fullscreen) ? SW_SHOWMAXIMIZED : visibilityFlag) == FALSE && GetLastError() != 0)
        return false;
      if (visibilityFlag != SW_SHOWNA || this->_mode == WindowType::fullscreen) {
        SetForegroundWindow(this->_handle);
        SetFocus(this->_handle);
      }
    }
    return true;
  }
  
  // ---

  // Change window style and position/size/resolution
  bool __WindowImpl::setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resizeMode,
                                    const DisplayArea& userArea, uint32_t rate) {
    if ((this->_statusFlags & __P_FLAG_FULLSCREEN_ON) && type != WindowType::fullscreen)
      exitFullscreenMode();
    __WindowImplEventProc::unactivateCursor(*this); // release capture before changing window mode
    
    DWORD prevStyle = this->_currentStyleFlag; // must be set before calling 'enterFullscreenMode'
    DWORD prevStyleExt = this->_currentStyleExtFlag;
    auto prevMode = this->_mode;
    auto prevBehavior = this->_behavior;
    auto prevResize = this->_resizeMode;
    auto prevRate = this->_refreshRate;

    // set window style
    if (type == WindowType::fullscreen || type == WindowType::borderless)
      resizeMode = ResizeMode::fixed;
    toWindowStyleFlags(type, behavior, resizeMode, this->_currentStyleFlag, this->_currentStyleExtFlag);
    this->_mode = type;
    this->_behavior = behavior;
    this->_resizeMode = resizeMode;
    this->_refreshRate = rate;
    
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
        this->_mode = prevMode;
        this->_behavior = prevBehavior;
        this->_resizeMode = prevResize;
        this->_refreshRate = prevRate;
        if (this->_mode == WindowType::fullscreen) // restore previous resolution on failure
          enterFullscreenMode(windowArea, rate);
        if (this->_statusFlags & __P_FLAG_WINDOW_ACTIVE)
          __WindowImplEventProc::activateCursor(*this);
        return false;
      }
    }
    
    this->_decorationSizes = decorationSizes;
    this->_lastClientArea = clientArea;
    this->_clientAreaRatio = (double)clientArea.width / (double)clientArea.height;

    // adjust size, based on actual window
    adjustVisibleClientSize(PixelSize{ clientArea.width, clientArea.height });

    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
    SetForegroundWindow(this->_handle);
    SetFocus(this->_handle);
    __WindowImplEventProc::activateCursor(*this); // restore capture
    return true;
  }
  
  // ---
  
  // Replace window menu (or remove if null)
  bool __WindowImpl::setMenu(HMENU menuHandle) noexcept {
    if (this->_statusFlags & __P_FLAG_WINDOW_VISIBLE) {
      auto expectedClientSize = lastClientSize();
      if (SetMenu(this->_handle, menuHandle) != FALSE || menuHandle == nullptr) {
        bool hasSizeChanged = ((menuHandle && !this->_menuHandle) || (this->_menuHandle && !menuHandle));
        this->_menuHandle = menuHandle;
        DrawMenuBar(this->_handle);

        if (hasSizeChanged)
          adjustVisibleClientSize(expectedClientSize);
        return true;
      }
    }
    else {
      if (SetMenu(this->_handle, menuHandle) != FALSE || menuHandle == nullptr) {
        bool hasSizeChanged = ((menuHandle && !this->_menuHandle) || (this->_menuHandle && !menuHandle));
        this->_menuHandle = menuHandle;
        if (hasSizeChanged)
          __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_FIRST_DISPLAY_DONE);
        return true;
      }
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
    if ((behavior & WindowBehavior::aboveTaskbar) == true)
      outWindowStyleExt |= WS_EX_APPWINDOW;
  }

  // Convert portable behavior to native window-class style flag
  UINT __WindowImpl::toWindowClassStyleFlag(WindowBehavior behavior) noexcept {
    UINT classStyle = (CS_HREDRAW | CS_VREDRAW);
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
      case Window::VisibilityCommand::restore:  return SW_RESTORE;
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
  
  // ---
  
  // Change mouse pointer X/Y position
  bool __WindowImpl::setCursorPosition(int32_t x, int32_t y, Window::CursorPositionType mode) noexcept {
    if (mode == Window::CursorPositionType::relative) {
      POINT pos{ (int)x, (int)y };
      if (ClientToScreen(this->_handle, &pos) != FALSE)
        return (SetCursorPos(pos.x, pos.y) != FALSE);

      pos.x = this->_lastClientArea.x + x; // on failure, use client area to convert value manually
      pos.x = this->_lastClientArea.y + y;
      return (SetCursorPos(pos.x, pos.y) != FALSE);
    }
    else // absolute
      return (SetCursorPos((int)x, (int)y) != FALSE);
  }
  // Change cursor visibility/limits/events
  void __WindowImpl::setCursorMode(Window::CursorMode cursorMode) noexcept {
    __WindowImplEventProc::unactivateCursor(*this);
    __WindowImplEventProc::unsetCursorOnLeave(this->_statusFlags);
    
    this->_cursorMode = cursorMode;
    switch (cursorMode) {
      case Window::CursorMode::clipped:
        __P_ADD_REMOVE_FLAGS(this->_statusFlags, __P_FLAG_CURSOR_CLIP, __P_FLAG_CURSOR_HIDE | __P_FLAG_CURSOR_RAW); break;
        break;
      case Window::CursorMode::hidden:
        __P_ADD_REMOVE_FLAGS(this->_statusFlags, __P_FLAG_CURSOR_HIDE, __P_FLAG_CURSOR_CLIP | __P_FLAG_CURSOR_RAW); break;
      case Window::CursorMode::hiddenRaw:
        __P_ADD_FLAG(this->_statusFlags, __P_FLAG_CURSOR_HIDE | __P_FLAG_CURSOR_CLIP | __P_FLAG_CURSOR_RAW); break;
      case Window::CursorMode::visible:
      default:
        __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_CURSOR_HIDE | __P_FLAG_CURSOR_CLIP | __P_FLAG_CURSOR_RAW); break;
    }
    if (this->_statusFlags & __P_FLAG_CURSOR_IS_HOVER)
      __WindowImplEventProc::setCursorOnHover(this->_statusFlags);
    if (this->_statusFlags & __P_FLAG_WINDOW_ACTIVE) {
      __WindowImplEventProc::activateCursor(*this);
      SetForegroundWindow(this->_handle);
      SetFocus(this->_handle);
    }
  }
  

// -- user area computation -- -------------------------------------------------

  // Verify if a borderless window has "fullscreen" behavior
  static inline bool __isBorderlessFull(const DisplayMonitor& monitor, const WindowDecorationSize& decorationSizes,
                                        WindowType mode, WindowBehavior behavior, uint32_t width, uint32_t height) noexcept {
    return (mode == WindowType::borderless && (behavior & WindowBehavior::aboveTaskbar) == true
         && (width  == Window::Builder::defaultSize()
          || height == Window::Builder::defaultSize()
          || width + decorationSizes.left + decorationSizes.right  == monitor.attributes().screenArea.width 
          || height + decorationSizes.top + decorationSizes.bottom == monitor.attributes().screenArea.height)
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
  template <bool _IsFullscreen>
  static inline void __computeUserPosition(int32_t userX, int32_t userY, uint32_t clientWidth, uint32_t clientHeight, 
                                           const DisplayArea& parentArea, const DisplayArea& workArea, 
                                           const WindowDecorationSize& decorationSizes,
                                           DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    __if_constexpr (_IsFullscreen) {
      outWindowArea.x = workArea.x;
      outWindowArea.y = workArea.y;
      outClientArea.x = outWindowArea.x + decorationSizes.left;
      outClientArea.y = outWindowArea.y + decorationSizes.top;
    }
    else { // window/dialog
      switch (userX) {
        case Window::Builder::defaultPosition(): // left of work area
          outClientArea.x = workArea.x + decorationSizes.left;
          break;
        case Window::Builder::centeredPosition(): // centered in work area
          outClientArea.x = workArea.x + ((int32_t)workArea.width - (int32_t)clientWidth - decorationSizes.left - decorationSizes.right)/2;
          if (outClientArea.x - decorationSizes.left < workArea.x) // clamp
            outClientArea.x += workArea.x - outClientArea.x + decorationSizes.left;
          break;
        default: outClientArea.x = parentArea.x + userX; break;
      }
      switch (userY) {
        case Window::Builder::defaultPosition(): // top of work area
          outClientArea.y = workArea.y + decorationSizes.top;
          break;
        case Window::Builder::centeredPosition(): // centered in work area
          outClientArea.y = workArea.y + ((int32_t)workArea.height - (int32_t)clientHeight - decorationSizes.top - decorationSizes.bottom)/2;
          if (outClientArea.y - decorationSizes.top < workArea.y) // clamp
            outClientArea.y += workArea.y - outClientArea.y + decorationSizes.top;
          break;
        default: outClientArea.y = parentArea.y + userY; break;
      }
      
      outWindowArea.x = outClientArea.x - decorationSizes.left;
      outWindowArea.y = outClientArea.y - decorationSizes.top;
    }
  }
  
  // ---

  // Compute client-position (absolute) and window-position (absolute if root/relative if child), based on user-defined values
  void __WindowImpl::computeUserPosition(int32_t x, int32_t y, const WindowDecorationSize& decorationSizes,
                                         DisplayArea& inOutClientArea, DisplayArea& outWindowPos) noexcept {
    // fullscreen -> decorations must fit inside -> reduce client area
    if (this->_mode == WindowType::fullscreen 
    || __isBorderlessFull(*(this->_monitor), decorationSizes, this->_mode, this->_behavior, inOutClientArea.width, inOutClientArea.height)) {
      
      __computeUserPosition<true>(x, y, inOutClientArea.width, inOutClientArea.height,
                                  this->_monitor->attributes().screenArea, this->_monitor->attributes().screenArea, 
                                  decorationSizes, inOutClientArea, outWindowPos);
    }
    // window/dialog -> decorations outside of client area
    else {
      DisplayArea parentArea;
      if (this->_parent != nullptr)
        __getParentArea(this->_parent, *(this->_monitor), parentArea);

      __computeUserPosition<false>(x, y, inOutClientArea.width, inOutClientArea.height,
                                   (this->_parent != nullptr) ? parentArea : this->_monitor->attributes().workArea,
                                   this->_monitor->attributes().workArea, decorationSizes, 
                                   inOutClientArea, outWindowPos);
    }
  }
  // Compute client-size and window-size, based on user-defined values
  void __WindowImpl::computeUserSize(uint32_t width, uint32_t height, const WindowDecorationSize& decorationSizes, 
                                     DisplayArea& outClientSize, DisplayArea& outWindowSize) noexcept {
    // fullscreen -> decorations must fit inside -> reduce client area
    if (this->_mode == WindowType::fullscreen 
    || __isBorderlessFull(*(this->_monitor), decorationSizes, this->_mode, this->_behavior, width, height) ) {
      
      __computeUserSize<true>(width, height, this->_monitor->attributes().screenArea, 
                              decorationSizes, outClientSize, outWindowSize);
    }
    // window/dialog -> decorations outside of client area
    else {
      DisplayArea parentArea;
      if (this->_parent != nullptr)
        __getParentArea(this->_parent, *(this->_monitor), parentArea);

      __computeUserSize<false>(width, height, (this->_parent != nullptr) ? parentArea : this->_monitor->attributes().workArea,
                               decorationSizes, outClientSize, outWindowSize);
    }
  }
  // Compute client-area (absolute) and window-area (absolute if root/relative if child), based on user-defined values
  void __WindowImpl::computeUserArea(WindowType mode, WindowBehavior behavior, const DisplayArea& userArea, 
                                     const WindowDecorationSize& decorationSizes,
                                     DisplayArea& outClientArea, DisplayArea& outWindowArea) noexcept {
    // fullscreen -> decorations must fit inside -> reduce client area
    if (this->_mode == WindowType::fullscreen 
    || __isBorderlessFull(*(this->_monitor), decorationSizes, mode, behavior, userArea.width, userArea.height) ) {
      __computeUserSize<true>(userArea.width, userArea.height, this->_monitor->attributes().screenArea, 
                              decorationSizes, outClientArea, outWindowArea);
      __computeUserPosition<true>(userArea.x, userArea.y, outClientArea.width, outClientArea.height,
                                  this->_monitor->attributes().screenArea, this->_monitor->attributes().screenArea,
                                  decorationSizes, outClientArea, outWindowArea);
    }
    // window/dialog -> decorations outside of client area
    else {
      DisplayArea parentArea;
      if (this->_parent != nullptr)
        __getParentArea(this->_parent, *(this->_monitor), parentArea);
      
      __computeUserSize<false>(userArea.width, userArea.height, 
                               (this->_parent != nullptr) ? parentArea : this->_monitor->attributes().workArea,
                               decorationSizes, outClientArea, outWindowArea);
      __computeUserPosition<false>(userArea.x, userArea.y, outClientArea.width, outClientArea.height,
                                   (this->_parent != nullptr) ? parentArea : this->_monitor->attributes().workArea,
                                   this->_monitor->attributes().workArea, decorationSizes, 
                                   outClientArea, outWindowArea);
    }
  }
  
  // ---
  
  // Adjust client size after first display (to match requested size)
  void __WindowImpl::adjustVisibleClientSize(const PixelSize& expected) noexcept {
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
                         (int)expected.width + (int)this->_decorationSizes.left + (int)this->_decorationSizes.right, 
                         (int)expected.height + (int)this->_decorationSizes.top + (int)this->_decorationSizes.bottom, 
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
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_FULLSCREEN_ON);

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
    
    SetWindowLongPtr(this->_handle, GWL_STYLE, this->_currentStyleFlag | WS_VISIBLE);
    SetWindowLongPtr(this->_handle, GWL_EXSTYLE, this->_currentStyleExtFlag);
    __P_ADD_FLAG(this->_statusFlags, __P_FLAG_SIZE_HANDLER_CHANGE);
    SetWindowPos(this->_handle, HWND_TOP, (int)windowArea.x,(int)windowArea.y, (int)windowArea.width, (int)windowArea.height, SWP_FRAMECHANGED);
    __P_REMOVE_FLAG(this->_statusFlags, __P_FLAG_SIZE_HANDLER_CHANGE);
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
    
    DisplayArea clientSize, windowSize;
    computeUserSize(width, height, this->_decorationSizes, clientSize, windowSize);
    
    double prevRatio = this->_clientAreaRatio;
    this->_clientAreaRatio = (double)clientSize.width / (double)clientSize.height; // update ratio before, for homothety checks
    if (SetWindowPos(this->_handle, HWND_TOP, 0, 0, (int)windowSize.width,(int)windowSize.height, 
                     (SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW)) != FALSE) {
      this->_lastClientArea.width = clientSize.width;
      this->_lastClientArea.height = clientSize.height;
    
      InvalidateRect(this->_handle, nullptr, true); // repaint
      return true;
    }
    else
      this->_clientAreaRatio = prevRatio;
    return false;
  }
  
  // Change size + position
  bool __WindowImpl::resize(const DisplayArea& userArea) noexcept {
    if (this->_mode == WindowType::fullscreen)
      return false;
    
    DisplayArea clientArea, windowArea;
    computeUserArea(this->_mode, this->_behavior, userArea, this->_decorationSizes, clientArea, windowArea);

    double prevRatio = this->_clientAreaRatio;
    this->_clientAreaRatio = (double)clientArea.width / (double)clientArea.height; // update ratio before, for homothety checks
    if (SetWindowPos(this->_handle, HWND_TOP, (int)windowArea.x,(int)windowArea.y, 
                     (int)windowArea.width,(int)windowArea.height, (SWP_NOZORDER | SWP_SHOWWINDOW)) != FALSE) {
      this->_lastClientArea = clientArea;
      
      InvalidateRect(this->_handle, nullptr, true); // repaint
      return true;
    }
    else
      this->_clientAreaRatio = prevRatio;
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

  // Refresh client-area + returns: 0(error) / 1(no changes) / 2 (changed)
  __forceinline int __WindowImplEventProc::refreshClientArea(__WindowImpl& window, DisplayArea& outArea) noexcept {
    if (window.readVisibleClientArea(outArea)) {
      if (window._lastClientArea.x != outArea.x || window._lastClientArea.width != outArea.width
        || window._lastClientArea.y != outArea.y || window._lastClientArea.height != outArea.height) {

        window._lastClientArea = outArea;
        return 2;
      }
      return 1;
    }
    return 0;
  }

  // On maximize/restore event, refresh client-area + decorations + force homothety
  void __WindowImplEventProc::refreshOnMaximizeRestore(__WindowImpl& window, bool isMaximized, LPARAM lParam, DisplayArea& outArea) noexcept {
    if (__WindowImplEventProc::refreshClientArea(window, outArea) > 0) {
      window.readVisibleWindowDecorations(outArea, window._decorationSizes);
    }
    else {
      window._lastClientArea.width = outArea.width = (uint32_t)LOWORD(lParam);
      window._lastClientArea.height = outArea.height = (uint32_t)HIWORD(lParam);
      window.computeWindowDecorations(window._currentStyleFlag, window._currentStyleExtFlag,
                                      window.hasMenu(), window._decorationSizes);
    }

    // force homothety when maximized
    if (isMaximized && (window._resizeMode & ResizeMode::homothety) == true) {
      uint32_t adjustedClientHeight = static_cast<uint32_t>((double)outArea.width / window._clientAreaRatio + 0.50001);
      if (adjustedClientHeight != outArea.height) {
        if (adjustedClientHeight > outArea.height)
          window._lastClientArea.width = outArea.width = static_cast<uint32_t>((double)outArea.height * window._clientAreaRatio + 0.50001);
        else
          window._lastClientArea.height = outArea.height = adjustedClientHeight;

        __P_ADD_FLAG(window._statusFlags, __P_FLAG_SIZE_HANDLER_CHANGE);
        SetWindowPos(window._handle, HWND_TOP, 0, 0, (int)outArea.width + (int)window._decorationSizes.left + (int)window._decorationSizes.right,
                     (int)outArea.height + (int)window._decorationSizes.top + (int)window._decorationSizes.bottom, SWP_NOMOVE | SWP_NOZORDER);
        __P_REMOVE_FLAG(window._statusFlags, __P_FLAG_SIZE_HANDLER_CHANGE);
      }
    }
  }

  // force homothety + clamp (screen edges) when window is resized
  __forceinline void __WindowImplEventProc::resizeWithHomothety(__WindowImpl& window, int movedBorder, RECT& inOutArea) noexcept {
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
  
  // Cursor in client area: hide cursor
  __forceinline void __WindowImplEventProc::setCursorOnHover(uint32_t& statusFlags) noexcept {
    if ((statusFlags & (__P_FLAG_CURSOR_IN_MENULOOP|__P_FLAG_CURSOR_HIDE|__P_FLAG_CURSOR_RAW)) == __P_FLAG_CURSOR_HIDE)
      ShowCursor(false);
  }
  // Cursor out of range: show cursor
  __forceinline void __WindowImplEventProc::unsetCursorOnLeave(uint32_t& statusFlags) noexcept {
    if ((statusFlags & (__P_FLAG_CURSOR_IS_HOVER|__P_FLAG_CURSOR_IN_MENULOOP|__P_FLAG_CURSOR_HIDE|__P_FLAG_CURSOR_RAW)) == (__P_FLAG_CURSOR_IS_HOVER|__P_FLAG_CURSOR_HIDE))
      ShowCursor(true);
  }

  // Cursor in client-area after menu exit: hide cursor
  __forceinline void __WindowImplEventProc::setCursorOnMenuExit(__WindowImpl& window) noexcept {
    if ((window._statusFlags & (__P_FLAG_CURSOR_IS_HOVER|__P_FLAG_CURSOR_HIDE|__P_FLAG_CURSOR_RAW)) == (__P_FLAG_CURSOR_IS_HOVER|__P_FLAG_CURSOR_HIDE)) {
      ShowCursor(false);
    }
    else if (window._statusFlags & __P_FLAG_CURSOR_IS_CAPTURED) {
      if (window._statusFlags & __P_FLAG_CURSOR_RAW) {
        const RAWINPUTDEVICE device = { 0x01, 0x02, 0, window._handle };
        if (!RegisterRawInputDevices(&device, 1, sizeof(device)))
          return; // failure -> do not hide cursor -> let user know that mouse capture doesn't work
        
        ShowCursor(false); // hide cursor
        __WindowImplEventProc::centerCursor(window);
      }
    }
  }
  // Cursor on menu enter: show cursor
  __forceinline void __WindowImplEventProc::unsetCursorOnMenuEnter(uint32_t& statusFlags) noexcept {
    if ((statusFlags & (__P_FLAG_CURSOR_IS_HOVER|__P_FLAG_CURSOR_HIDE|__P_FLAG_CURSOR_RAW)) == (__P_FLAG_CURSOR_IS_HOVER|__P_FLAG_CURSOR_HIDE)) {
      ShowCursor(true);
    }
    else if (statusFlags & __P_FLAG_CURSOR_IS_CAPTURED) {
      if (statusFlags & __P_FLAG_CURSOR_RAW) {
        const RAWINPUTDEVICE device = { 0x01, 0x02, RIDEV_REMOVE, nullptr };
        RegisterRawInputDevices(&device, 1, sizeof(device));
      }
      ShowCursor(true); // show cursor
    }
  }
  
  // Cursor active: clip cursor to client-area
  void __WindowImplEventProc::activateCursor(__WindowImpl& window) noexcept {
    if ((window._statusFlags & (__P_FLAG_CURSOR_CLIP|__P_FLAG_CURSOR_IS_CAPTURED)) == __P_FLAG_CURSOR_CLIP) {
      if (window._statusFlags & __P_FLAG_CURSOR_RAW) {
        const RAWINPUTDEVICE device = { 0x01, 0x02, 0, window._handle };
        if (!RegisterRawInputDevices(&device, 1, sizeof(device)))
          return; // failure -> do not hide cursor -> let user know that mouse capture doesn't work
        
        ShowCursor(false); // hide cursor
        __WindowImplEventProc::centerCursor(window);
      }
      else
        __WindowImplEventProc::clipCursor(window);
      
      RECT clientBounds;
      window.lastAbsoluteClientRect(clientBounds);
      ClipCursor(&clientBounds);
      __P_ADD_FLAG(window._statusFlags, __P_FLAG_CURSOR_IS_CAPTURED);
    }
  }
  // Cursor unactivated: remove cursor constraints
  void __WindowImplEventProc::unactivateCursor(__WindowImpl& window) noexcept {
    ClipCursor(nullptr);
    if ((window._statusFlags & (__P_FLAG_CURSOR_HIDE|__P_FLAG_CURSOR_IS_CAPTURED)) == (__P_FLAG_CURSOR_HIDE|__P_FLAG_CURSOR_IS_CAPTURED)) {
      if (window._statusFlags & __P_FLAG_CURSOR_RAW) {
        const RAWINPUTDEVICE device = { 0x01, 0x02, RIDEV_REMOVE, nullptr };
        RegisterRawInputDevices(&device, 1, sizeof(device));
      }
      ShowCursor(true); // show cursor
    }
    __P_REMOVE_FLAG(window._statusFlags, __P_FLAG_CURSOR_IS_CAPTURED);
  }
  
  // recenter cursor position (for raw event mode)
  void __WindowImplEventProc::centerCursor(__WindowImpl& window) noexcept {
    if (window._statusFlags & __P_FLAG_CURSOR_IS_HOVER) {
      window._lastCursorPosition.x = window._lastCursorPosition.y = __P_RAW_ABSOLUTE_MAX_COORD/2;
      
      auto centerPos = window.lastAbsoluteClientCenter();
      if (!SetCursorPos((int)centerPos.x, (int)centerPos.y)) { // recenter cursor
        POINT cursorPos;
        if (GetCursorPos(&cursorPos)) {
          auto& clientArea = window.lastClientArea();
          window._lastCursorPosition.x = static_cast<int32_t>( ((double)cursorPos.x - (double)clientArea.x) * (double)__P_RAW_ABSOLUTE_MAX_COORD / (double)clientArea.width + 0.5);
          window._lastCursorPosition.y = static_cast<int32_t>( ((double)cursorPos.y - (double)clientArea.y) * (double)__P_RAW_ABSOLUTE_MAX_COORD / (double)clientArea.height + 0.5);
        }
      }
    }
  }
  // force cursor to remain in client-area bounds
  __forceinline void __WindowImplEventProc::clipCursor(__WindowImpl& window) noexcept {
    POINT cursorPos;
    if (GetCursorPos(&cursorPos)) {
      if (cursorPos.x < (int)window._lastClientArea.x)
        cursorPos.x = (int)window._lastClientArea.x + 1;
      else if (cursorPos.x >= (int)window._lastClientArea.x + (int)window._lastClientArea.width)
        cursorPos.x = (int)window._lastClientArea.x + (int)window._lastClientArea.width - 2;
      
      if (cursorPos.y < (int)window._lastClientArea.y)
        cursorPos.y = (int)window._lastClientArea.y + 1;
      else if (cursorPos.y >= (int)window._lastClientArea.y + (int)window._lastClientArea.height)
        cursorPos.y = (int)window._lastClientArea.y + (int)window._lastClientArea.height - 2;

      SendMessage(window._handle, WM_MOUSEMOVE, 0, MAKELPARAM(cursorPos.x, cursorPos.y));
      SetCursorPos(cursorPos.x, cursorPos.y);
    }
  }

  // enable tracking of TME_HOVER and/or TME_LEAVE events
  static inline void __trackMouseHoverLeave(HWND handle, DWORD eventTypes, bool isClipped) noexcept {
    TRACKMOUSEEVENT tme;
    ZeroMemory(&tme, sizeof(tme));
    tme.cbSize = sizeof(tme);
    tme.dwFlags = eventTypes;
    tme.dwHoverTime = isClipped ? 0 : 16u;
    tme.hwndTrack = handle;
    TrackMouseEvent(&tme);
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
      __P_ADD_FLAG(window._statusFlags, __P_FLAG_SIZE_HANDLER_CHANGE);
      if (SetWindowPos(window._handle, HWND_TOP, suggestedArea.left, suggestedArea.top,
                       (int)window._lastClientArea.width + outDecorationSizes.left + outDecorationSizes.right,
                        (int)window._lastClientArea.height + outDecorationSizes.top + outDecorationSizes.bottom, SWP_NOZORDER | SWP_NOACTIVATE) == FALSE) {
        window._lastClientArea = newClientArea; // on failure, update client-area
      }
      __P_REMOVE_FLAG(window._statusFlags, __P_FLAG_SIZE_HANDLER_CHANGE);
    }
  }
  
  // ---
  
  // Drop files (drag and drop operation)
  void __WindowImplEventProc::dragDropFiles(__WindowImpl& window, HDROP dropHandle) noexcept {
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
  void __WindowImpl::setMouseHandler(MouseEventHandler handler, Window::CursorMode cursorMode) noexcept { 
    this->_onMouseEvent = handler; 
    if (this->_cursorMode != cursorMode)
      setCursorMode(cursorMode);
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
      case WM_MOUSEMOVE: { 
        // TME_LEAVE is unregistered when the cursor leaves the window -> register when it comes back
        if ((window._statusFlags & __P_FLAG_CURSOR_IS_TRACKING) == 0) {
          __trackMouseHoverLeave(window._handle, TME_LEAVE | TME_HOVER, (window._statusFlags & __P_FLAG_CURSOR_CLIP));
          __P_ADD_FLAG(window._statusFlags, __P_FLAG_CURSOR_IS_TRACKING);
        }
        // stop external capture after clipping confirmed
        if (window._statusFlags & __P_FLAG_CAPTURE_HANDLER_ON) {
          ReleaseCapture();
          __P_REMOVE_FLAG(window._statusFlags, __P_FLAG_CAPTURE_HANDLER_ON);
        }
        // do not expose move events in raw mode
        if (window._statusFlags & __P_FLAG_CURSOR_RAW)
          return false; 
        
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
  
  // process raw mouse motion events (WM_INPUT)
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

      int32_t deltaX = 0, deltaY = 0;
      if (rawData.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
        deltaX = rawData.data.mouse.lLastX - window._lastCursorPosition.x;
        deltaY = rawData.data.mouse.lLastY - window._lastCursorPosition.y;

        window._lastCursorPosition = window.lastAbsoluteClientCenter();
        if (!SetCursorPos((int)window._lastCursorPosition.x, (int)window._lastCursorPosition.y)) { // recenter cursor
          window._lastCursorPosition.x = rawData.data.mouse.lLastX;
          window._lastCursorPosition.y = rawData.data.mouse.lLastY;
        }
      }
      else { // relative
        deltaX = rawData.data.mouse.lLastX;
        deltaY = rawData.data.mouse.lLastY;
        __WindowImplEventProc::centerCursor(window);
      }
      if (deltaX || deltaY)
        window._onMouseEvent(&window._container, MouseEvent::rawMotion, deltaX, deltaY, 0, 0); // handler
    }
  }
  
  // ---

  // main window event processor
  LRESULT CALLBACK __WindowImpl::windowEventProcessor(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    bool isCommandEvent = false;
    __WindowImpl* window = reinterpret_cast<__WindowImpl*>(GetPropW(handle, __P_WINDOW_ID));
    if (window == nullptr)
      return DefWindowProcW(handle, message, wParam, lParam); // system event processor

    // > mouse events (note: missing WM_MOUSELEAVE and WM_INPUT: see "else" section) --
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
      if (window->_onMouseEvent && __WindowImplEventProc::processMouseEvent(*window, message, wParam, lParam))
        return 0; // handler said to ignore system processing
      isCommandEvent = true;
    }
    // > keyboard events --
    else if (message <= WM_KEYLAST && message >= WM_KEYFIRST) {
      if (window->_onKeyboardEvent && __WindowImplEventProc::processKeyboardEvent(*window, message, wParam, lParam))
        return 0; // handler said to ignore system processing
      isCommandEvent = true;
    }
    else {
      switch (message) {
        // > custom mouse move events
        case WM_MOUSEHOVER: { // mouse enters client area
          if ((window->_statusFlags & __P_FLAG_CURSOR_IS_HOVER) == 0) {
            __P_ADD_FLAG(window->_statusFlags, __P_FLAG_CURSOR_IS_HOVER);
            __WindowImplEventProc::setCursorOnHover(window->_statusFlags);
          }
          break;
        }
        case WM_MOUSELEAVE: { // mouse leaves client area (-> unregisters TME_LEAVE -> re-registered in WM_MOUSEMOVE in 'processMouseEvent')
          // mouse clipping -> prevent from leaving (only useful when ClipCursor(RECT) fails)
          if ((window->_statusFlags & (__P_FLAG_CURSOR_IS_CAPTURED|__P_FLAG_CURSOR_CLIP|__P_FLAG_CURSOR_IN_MENULOOP)) == (__P_FLAG_CURSOR_IS_CAPTURED|__P_FLAG_CURSOR_CLIP)) {
            if (window->_statusFlags & __P_FLAG_CURSOR_RAW) // raw events -> recenter
              __WindowImplEventProc::centerCursor(*window);
            else
              __WindowImplEventProc::clipCursor(*window);
            SetCapture(window->_handle);
            __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_CAPTURE_HANDLER_ON, __P_FLAG_CURSOR_IS_TRACKING);
            return 0;
          }
          __WindowImplEventProc::unsetCursorOnLeave(window->_statusFlags);
          __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_CURSOR_IS_TRACKING | __P_FLAG_CURSOR_IS_HOVER);

          if (window->_onMouseEvent && window->_onMouseEvent(&window->_container, MouseEvent::mouseLeave, 0, 0, 0, 0))
            return 0;
          break;
        }
        case WM_ENTERMENULOOP: { // menu loop entered
          __WindowImplEventProc::unsetCursorOnMenuEnter(window->_statusFlags);
          __P_ADD_FLAG(window->_statusFlags, __P_FLAG_CURSOR_IN_MENULOOP);
          break;
        }
        case WM_EXITMENULOOP: { // menu loop left
          __WindowImplEventProc::setCursorOnMenuExit(*window);
          __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_CURSOR_IN_MENULOOP);
          break;
        }
        case WM_INPUT: { // raw mouse moves
          if ((window->_statusFlags & __P_FLAG_CURSOR_RAW) && window->_onMouseEvent)
            __WindowImplEventProc::processRawInputEvent(*window, lParam);
          isCommandEvent = true;
          break;
        }

        // > window size/move events --
        
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

          // verify if monitor has changed
          if (window->_mode != WindowType::fullscreen && window->_mode != WindowType::borderless 
          && (window->_statusFlags & (__P_FLAG_FULLSCREEN_ON|__P_FLAG_WINDOW_VISIBLE)) == __P_FLAG_WINDOW_VISIBLE) {
            RECT clientBounds;
            window->lastAbsoluteClientRect(clientBounds);
            if (__WindowImplEventProc::findWindowMonitor(*window, clientBounds)) {
              window->_onWindowEvent(&window->_container, WindowEvent::monitorChanged, static_cast<uint32_t>(window->contentScale()*100.0f),
                window->_lastClientArea.width, window->_lastClientArea.height, (void*)window->_monitor->handle());
            }
          }
          break;
        }
        case WM_MOVE: { // window currently moving / moved by command
          if (window->_onPositionEvent) {
            window->_onPositionEvent(&window->_container, (window->_statusFlags & __P_FLAG_RESIZED_MOVED)
                                     ? PositionEvent::sizePositionTrack : PositionEvent::sizePositionChanged, // user move tracking / system move
                                     GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), window->_lastClientArea.width, window->_lastClientArea.height);
          }
          // adapt constraints if mouse captured
          if (window->_statusFlags & __P_FLAG_CURSOR_IS_CAPTURED) {
            RECT clientBounds;
            window->lastAbsoluteClientRect(clientBounds);
            ClipCursor(&clientBounds);
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
              __WindowImplEventProc::unactivateCursor(*window);
              __WindowImplEventProc::unsetCursorOnLeave(window->_statusFlags);
              if (window->_statusFlags & __P_FLAG_FULLSCREEN_ON)
                window->exitFullscreenMode();
              __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_WINDOW_MINIMIZED, 
                                   __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE | __P_FLAG_RESIZED_MOVED | __P_FLAG_CURSOR_IS_TRACKING | __P_FLAG_CURSOR_IS_HOVER);
              
              if (window->_onWindowEvent)
                window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (uint32_t)WindowActivity::hidden, 0,0,nullptr);
              break;
            }
            case SIZE_RESTORED:
            case SIZE_MAXIMIZED: {
              bool wasActive = (window->_statusFlags & (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE)) == (__P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE);
              if (wParam == SIZE_RESTORED)
                __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE, 
                                     __P_FLAG_WINDOW_MINIMIZED | __P_FLAG_WINDOW_MAXIMIZED | __P_FLAG_RESIZED_MOVED);
              else
                __P_ADD_REMOVE_FLAGS(window->_statusFlags, __P_FLAG_WINDOW_MAXIMIZED | __P_FLAG_WINDOW_VISIBLE | __P_FLAG_WINDOW_ACTIVE, 
                                     __P_FLAG_WINDOW_MINIMIZED | __P_FLAG_RESIZED_MOVED);
              if (window->_statusFlags & __P_FLAG_SIZE_HANDLER_CHANGE)
                break;

              if (!wasActive) {
                if (window->_onWindowEvent)
                  window->_onWindowEvent(&window->_container, WindowEvent::stateChanged, (uint32_t)WindowActivity::active, 0,0,nullptr);
              }

              if (window->_mode != WindowType::fullscreen && (window->_statusFlags & __P_FLAG_FULLSCREEN_ON) == 0) {
                DisplayArea clientArea;
                __WindowImplEventProc::refreshOnMaximizeRestore(*window, (wParam == SIZE_MAXIMIZED), lParam, clientArea);
                if (window->_onPositionEvent)
                  window->_onPositionEvent(&window->_container, PositionEvent::sizePositionChanged, 
                    clientArea.x, clientArea.y, clientArea.width, clientArea.height);
              }
              else {
                window->enterFullscreenMode(window->lastWindowArea(), window->_refreshRate);
                SetForegroundWindow(window->_handle);
                SetFocus(window->_handle);
              }
              
              // adapt constraints if mouse captured
              if (window->_statusFlags & __P_FLAG_CURSOR_IS_CAPTURED) {
                RECT clientBounds;
                window->lastAbsoluteClientRect(clientBounds);
                ClipCursor(&clientBounds);
              }
              break;
            }
          }
          break;
        }
        // size limits
        case WM_GETMINMAXINFO: {
          if (window->_mode != WindowType::fullscreen && (window->_statusFlags & __P_FLAG_FULLSCREEN_ON) == 0 && (MINMAXINFO*)lParam != nullptr) {
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
                if ((window->_behavior & WindowBehavior::aboveTaskbar) == 0) {
                  limits->ptMaxPosition.x = (int)window->_monitor->attributes().workArea.x;
                  limits->ptMaxPosition.y = (int)window->_monitor->attributes().workArea.y;
                }
                if ((window->_resizeMode & ResizeMode::resizableX) == 0) {
                  limits->ptMaxSize.x = limits->ptMinTrackSize.x = limits->ptMaxTrackSize.x = (int)windowArea.width;
                  limits->ptMaxSize.y = (int)window->_monitor->attributes().workArea.height;
                }
                if ((window->_resizeMode & ResizeMode::resizableY) == 0) {
                  limits->ptMaxSize.x = (int)window->_monitor->attributes().workArea.width;
                  limits->ptMaxSize.y = limits->ptMinTrackSize.y = limits->ptMaxTrackSize.y = (int)windowArea.height;
                }
              }
            }
            return 0;
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
              if ((window->_statusFlags & (__P_FLAG_WINDOW_MINIMIZED|__P_FLAG_CURSOR_CLIP)) == (__P_FLAG_WINDOW_MINIMIZED|__P_FLAG_CURSOR_CLIP))
                ShowWindow(window->_handle, SW_RESTORE);
              __WindowImplEventProc::activateCursor(*window);
            }
          }
          else if (window->_statusFlags & __P_FLAG_WINDOW_ACTIVE) { // inactive
            __P_REMOVE_FLAG(window->_statusFlags, __P_FLAG_WINDOW_ACTIVE);
            __WindowImplEventProc::unactivateCursor(*window);
            
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
          if (window->_onWindowEvent == nullptr || !window->_onWindowEvent(&window->_container, WindowEvent::windowClosed, 0,0,0,nullptr))
            window->_destroy();
          return 0;
        }
        case WM_DESTROY: { // close success -> confirmation message
          if (window->_handle) { // WM_CLOSE not received: destroyed by a parent window
            __decrementWindowCount();  // inform pollEvents
            window->_handle = nullptr; // inform pollCurrentWindowEvents
          }
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
        case WM_COMMAND: {
          if (window->_onWindowEvent && window->hasMenu() && HIWORD(wParam) == 0)
            window->_onWindowEvent(&window->_container, WindowEvent::menuCommand, 0, (int32_t)wParam,0, nullptr);
          isCommandEvent = true;
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
          // external change -> verify if not just another fullscreen window
          else if (window->_mode != WindowType::fullscreen && (window->_statusFlags &__P_FLAG_FULLSCREEN_ON) == 0) {
            RECT fgArea;
            MONITORINFO monitorInfo{};
            ZeroMemory(&monitorInfo, sizeof(monitorInfo));
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
              window->_onWindowEvent(&window->_container, WindowEvent::monitorChanged, static_cast<uint32_t>(window->contentScale()*100.0f),
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
          isCommandEvent = true;
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
              else if (window->_onWindowEvent)
                window->_onWindowEvent(&window->_container, WindowEvent::menuCommand, 1, 0,(int32_t)lParam, nullptr);
              break;
            default: break;
          }
          isCommandEvent = true;
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
          isCommandEvent = true;
          break;
        }
        // input language change
        case WM_INPUTLANGCHANGE: {
          if (window->_onWindowEvent)
            window->_onWindowEvent(&window->_container, WindowEvent::inputLangChanged, (uint32_t)wParam, 
                                   ((int32_t)lParam & 0xFF), (((int32_t)lParam & 0xFF00) >> 8), (void*)lParam);
          isCommandEvent = true;
          break; 
        }
        default: isCommandEvent = true; break;
      }
    }
    
    if ((window->_statusFlags & __P_FLAG_USE_ORIG_EVENT_PROC) && isCommandEvent) // call original event proc if requested
      return window->_originalStyle->_eventProcessor(handle, message, wParam, lParam);
    return DefWindowProcW(handle, message, wParam, lParam); // system window processor
  }

# undef __if_constexpr


// -----------------------------------------------------------------------------
// Include hpp implementations
// -----------------------------------------------------------------------------
# include "./window_events_win32.hpp"
# include "./window_resource_win32.hpp"

#endif
