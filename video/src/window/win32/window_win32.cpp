/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window manager + builder - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <system/api/windows_api.h>
# include "video/window_keycodes.h"
# include "video/_private/_window_impl_win32.h"
# include "video/window.h"

  using namespace pandora::video;
  using pandora::hardware::DisplayArea;
  using pandora::hardware::DisplayMode;
  using pandora::hardware::DisplayMonitor;


// -- Window Builder - new window -- -------------------------------------------

  // Configure window class context + create new window
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
  
  // Read horizontal/vertical scroll box position
  PixelPosition Window::getScrollPosition() const noexcept { return this->_impl->lastScrollPosition(); }
  // Read vertical scroll box position
  int32_t Window::getScrollPositionV() const noexcept { return this->_impl->lastScrollPositionV(); }
  // Read horizontal scroll box position
  int32_t Window::getScrollPositionH() const noexcept { return this->_impl->lastScrollPositionH(); }
  
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
  
  // Change vertical/horizontal scrollbar ranges
  bool Window::setScrollbarRange(uint16_t posH, uint16_t posV, uint16_t horizontalMax, uint16_t verticalMax, uint32_t pixelsPerUnit) noexcept {
    return this->_impl->setScrollbarRange((uint32_t)posH, (uint32_t)posV, (uint32_t)horizontalMax, (uint32_t)verticalMax, pixelsPerUnit);
  }
  // Change position of slider in both scrollbars
  bool Window::setScrollPosition(const PixelPosition& pos) noexcept {
    return (this->_impl->setScrollPositionV((uint32_t)pos.y)
         || this->_impl->setScrollPositionH((uint32_t)pos.x));
  }
  // Change position of slider in vertical scrollbar
  bool Window::setScrollPositionV(uint16_t pos) noexcept {
    return this->_impl->setScrollPositionV((uint32_t)pos);
  }
  // Change position of slider in horizontal scrollbar
  bool Window::setScrollPositionH(uint16_t pos) noexcept {
    return this->_impl->setScrollPositionH((uint32_t)pos);
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

#endif
