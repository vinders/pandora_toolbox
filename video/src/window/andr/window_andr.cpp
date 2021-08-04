/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
Description : Window manager + builder - JNI implementation (Android)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <stdexcept>
# include "video/window_keycodes.h"
# include "video/window.h"

  using namespace pandora::video;
  using pandora::hardware::DisplayArea;
  //using pandora::hardware::DisplayMonitor;


// -- Window Builder - new window -- -------------------------------------------

  // configure window class context + create new window
  std::unique_ptr<Window> Window::Builder::create(const window_char* contextName, const window_char* caption, 
                                                  WindowHandle parentWindow) { // throws
    if (contextName == nullptr || *contextName == 0)
      throw std::invalid_argument("Window.Builder: context name must not be NULL or empty");
  
    //...
    
    if (this->_params.cursor == nullptr)
      this->_params.cursor = WindowResource::buildCursor(SystemCursor::pointer);

    if (this->_params.backgroundColor == nullptr)
      this->_params.backgroundColor = WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0));
    
    // window creation
    return std::unique_ptr<Window>(new Window(contextName, caption, this->_params, parentWindow));
  }
  
  // ---
  
  // new window construction
  Window::Window(const window_char* contextName, const window_char* caption, 
                 const Window::Builder::Params& params, WindowHandle parentWindow) { // throws
    // resources
    this->_appIcon = params.appIcon;
    this->_captionIcon = params.captionIcon;
    this->_cursor = params.cursor;
    this->_backgroundColor = params.backgroundColor;
    this->_menu = params.menu;
    
    //this->_impl
    //...
    throw std::runtime_error("Window: not implemented");
  }


// -- Window Builder - existing window -- --------------------------------------

  // build instance from existing window
  std::unique_ptr<Window> Window::Builder::update(WindowHandle existingHandle, bool callExistingEventProc) { // throws
    if (this->_params.backgroundColor == nullptr)
      this->_params.backgroundColor = WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0));
    
    return std::unique_ptr<Window>(new Window(this->_params, existingHandle, callExistingEventProc));
  }
  
  // existing window re-styling
  Window::Window(const Window::Builder::Params& params, WindowHandle existingHandle, bool callExistingEventProc) { // throws
    // resources
    this->_appIcon = params.appIcon;
    this->_captionIcon = params.captionIcon;
    this->_cursor = params.cursor;
    this->_backgroundColor = params.backgroundColor;
    this->_menu = params.menu;
    
    //this->_impl
    //...
    throw std::runtime_error("Window: not implemented");
  }


// -- Window - destruction -- --------------------------------------------------

  // destroy window (or restore existing window style)
  Window::~Window() noexcept {
    if (this->_impl) {
      //delete _impl;
      this->_impl = nullptr;
    }
  }


// -- accessors -- -------------------------------------------------------------
  
  // Get native window handle
  WindowHandle Window::handle() const noexcept { /*...*/return (WindowHandle)0; }
  // Get window type (fullscreen/borderless/window/...)
  WindowType Window::displayMode() const noexcept { /*...*/return WindowType::window; }
  // Get window type (fullscreen/borderless/window/...)
  WindowBehavior Window::behavior() const noexcept { /*...*/return WindowBehavior::none; }
  // Get current resizing mode (resizable X/Y, homothety)
  ResizeMode Window::resizeMode() const noexcept { /*...*/return ResizeMode::fixed; }
  // Get native menubar handle
  MenuHandle Window::menuHandle() const noexcept { /*...*/return (MenuHandle)0; }
  // Verify if current window has a parent window
  bool Window::hasParent() const noexcept { /*...*/return false; }
  
  // get current window visibility state
  Window::VisibleState Window::visibleState() const noexcept { /*...*/return Window::VisibleState::none; }
  
  // get current window area (position / size)
  DisplayArea Window::getWindowArea() const noexcept { /*...*/return DisplayArea{0,0,0,0}; }
  // get current window client area (position / size)
  DisplayArea Window::getClientArea() const noexcept { /*...*/return DisplayArea{0,0,0,0}; }
  // get current window client area size
  PixelSize Window::getClientSize() const noexcept { /*...*/return PixelSize{0,0}; }
  // get current monitor scaling (based on DPI)
  float Window::contentScale() const noexcept { /*...*/return 1.0f; }
  
  // ---
  
  // Get display monitor on which the window is located
  const pandora::hardware::DisplayMonitor& Window::displayMonitor() const noexcept { throw std::runtime_error("not implemented"); }
  
  // Read current caption title
  pandora::memory::LightString Window::getCaption() const { return pandora::memory::LightString{}; }
  
  // Get current cursor mode (visibility/capture)
  Window::CursorMode Window::getCursorMode() const noexcept { return Window::CursorMode::visible; }
  
  // Get current mouse pointer position
  PixelPosition Window::getCursorPosition(Window::CursorPositionType mode) const noexcept {
    //...
    return { -1, -1 };
  }

  // ---
  
  // Get last error message (on change failure)
  pandora::memory::LightString Window::getLastError() noexcept {
    return pandora::memory::LightString("not implemented");
  }
  
  
// -- display changes -- -------------------------------------------------------
  
  // Change window visibility state
  bool Window::show(Window::VisibilityCommand state) noexcept { /*...*/return false; }
  // Change position
  bool Window::move(int32_t x, int32_t y) noexcept { /*...*/return false; }
  // Change size
  bool Window::resize(uint32_t width, uint32_t height) noexcept { /*...*/return false; }
  // Change size + position
  bool Window::resize(const DisplayArea& clientArea) noexcept { /*...*/return false; }
  
  // ---
  
  // Change window style and position/size/resolution
  bool Window::setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resizeMode,
                              const DisplayArea& clientArea, uint32_t rate) {
    //...
    return false;
  }
  // Define minimum size limits for the user (ignored if not resizable)
  bool Window::setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept {
    //...
    return false;
  }

  // ---
  
  // Clear entire client area (with background color)
  bool Window::clearClientArea() noexcept {
    //...
    return false;
  }
  // Clear rectangle relative to client area (with background color)
  bool Window::clear(const DisplayArea& area) noexcept {
    //...
    return false;
  }
  
  
// -- resource changes -- ------------------------------------------------------
  
  // Change mouse pointer X/Y position
  bool Window::setCursorPosition(int32_t x, int32_t y, Window::CursorPositionType mode) noexcept {
    //...
    return false;
  }
  // Change mouse pointer visibility/limits/events
  void Window::setCursorMode(CursorMode cursorMode) noexcept {
    //...
  }
  
  // Change window title
  bool Window::setCaption(const window_char* caption) noexcept {
    //...
    return false;
  }
  // Replace window menu (or remove if null)
  bool Window::setMenu(std::shared_ptr<WindowResource> menu) noexcept {
    //...
    return false;
  }
  
  // Change cursor resource
  bool Window::setCursor(std::shared_ptr<WindowResource> cursor) noexcept {
    //...
    return false;
  }
  // Change background color resource
  bool Window::setBackgroundColorBrush(std::shared_ptr<WindowResource> colorBrush) noexcept {
    //...
    return false;
  }

  
// -- event management -- ------------------------------------------------------

  // Set/replace window/hardware event handler (NULL to unregister)
  void Window::setWindowHandler(WindowEventHandler handler) noexcept { /*...*/ }
  // Set/replace size/position event handler (NULL to unregister)
  void Window::setPositionHandler(PositionEventHandler handler) noexcept { /*...*/ }
  // Set/replace keyboard event handler (NULL to unregister)
  void Window::setKeyboardHandler(KeyboardEventHandler handler) noexcept { /*...*/ }
  // Set/replace mouse event handler (NULL to unregister)
  void Window::setMouseHandler(MouseEventHandler handler, Window::CursorMode cursor) noexcept { /*...*/ }
  
  // ---

  // Process/forward pending events for all existing windows (user input, size changes, shutdown...).
  bool Window::pollEvents() noexcept {
    //...
    return false;
  }
  // Process/forward pending events for current window only (user input, size changes, shutdown...).
  bool Window::pollCurrentWindowEvents() noexcept {
    //...
    return false;
  }
  
  // Send 'windowClose' event to any window instance
  void Window::sendCloseEvent(WindowHandle target) noexcept {
    //...
  }

#endif
