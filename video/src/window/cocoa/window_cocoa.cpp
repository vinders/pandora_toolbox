/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Window manager + builder - Cocoa implementation (Mac OS)
*******************************************************************************/
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <TargetConditionals.h>
#endif
#if !defined(_WINDOWS) && defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
# include <stdexcept>
# include "video/window_keycodes.h"
# include "video/window.h"

  using namespace pandora::video;
  using pandora::hardware::DisplayArea;
  //using pandora::hardware::DisplayMonitor;


// -- accessors -- -------------------------------------------------------------

  // Get current window visibility state
  Window::VisibleState Window::visibleState() const noexcept { return Window::VisibleState::none; }
  // Get copy of current position/size of the client area
  DisplayArea Window::getClientArea() const noexcept { return DisplayArea{ 0 }; }
  // Get current size of the client area
  PixelSize Window::getClientSize() const noexcept { return PixelSize{ 0 }; }
  // Read scale factor to use for content (based on DPI)
  float Window::contentScale() const noexcept { return 1.0f; }

  // Get current mouse pointer position (-1 on error)
  PixelPosition Window::cursorPosition(CursorPositionType mode) noexcept { return PixelPosition{ 0 }; }
  // Read vertical scroll box position (-1 on error)
  int32_t Window::getScrollPositionV() const noexcept { return 0; }
  // Read horizontal scroll box position (-1 on error)
  int32_t Window::getScrollPositionH() const noexcept { return 0; }
  
  // Get last error message (if a setter returns false)
  std::string Window::getLastError() { return ""; }


// -- display changes -- -------------------------------------------------------
  
  // Show/hide/maximize/minimize/restore window
  bool Window::show(Window::VisibilityCommand state) noexcept { return false; }
  // Move window to position (size not changed)
  bool Window::move(int32_t x, int32_t y) noexcept { return false; }
  // Change window size (or resolution in fullscreen)
  bool Window::resize(uint32_t width, uint32_t height) noexcept { return false; }
  // Change window size (or resolution/rate in fullscreen)
  bool Window::resize(uint32_t width, uint32_t height, uint32_t rate) noexcept { return false; }
  // Change window size and position
  bool Window::resize(const DisplayArea& clientArea) noexcept { return false; }
  
  // Change window type and behavior + position and size/resolution
  bool Window::setDisplayMode(WindowType type, WindowBehavior components, ResizeMode resizeMode,
                              const DisplayArea& clientArea, uint32_t rate) { return false; }
  // Define minimum size limits for the user (ignored if not resizable)
  bool Window::setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept { return false; }
  
  // Clear entire client area (with background color)
  bool Window::clearClientArea() noexcept { return false; }
  // Clear rectangle relative to client area (with background color)
  bool Window::clear(const DisplayArea& area) noexcept { return false; }


// -- resource changes -- ------------------------------------------------------

  // Change mouse pointer image (won't change cursor visibility -> see 'setMouseHandler')
  bool Window::setCursor(std::shared_ptr<WindowResource> cursor) noexcept { return false; }
  // Change mouse pointer position (won't change cursor visibility -> see 'setMouseHandler')
  bool Window::setCursorPosition(int32_t x, int32_t y, CursorPositionType mode) noexcept { return false; }
  
  // Change title in window caption
  bool Window::setCaption(const window_char* caption) noexcept { return false; }
  // Add/replace native menu bar (use NULL to remove current menu)
  bool Window::setMenu(std::shared_ptr<WindowResource> menu) noexcept { return false; }
  // Change background color brush (will not affect display -> call clearClientArea())
  bool Window::setBackgroundColorBrush(std::shared_ptr<WindowResource> colorBrush) noexcept { return false; }
  
  // Change vertical/horizontal scrollbar ranges (only values for existing scrollbars will be used)
  bool Window::setScrollbarRange(uint16_t posH, uint16_t posV, uint16_t horizontalMax, 
                                 uint16_t verticalMax, uint32_t pixelsPerUnit) noexcept { return false; }
  // Change position of slider in vertical scrollbar
  bool Window::setScrollPositionV(uint16_t pos) noexcept { return false; }
  // Change position of slider in horizontal scrollbar
  bool Window::setScrollPositionH(uint16_t pos) noexcept { return false; }


// -- input events -- ----------------------------------------------------------

  // Set/replace mouse event handler (NULL to unregister)
  void Window::setMouseHandler(MouseEventHandler handler, Window::CursorMode cursor) noexcept {}

  // Process/forward pending events for all existing windows (user input, size changes, shutdown...).
  bool Window::pollEvents() noexcept { return false; }
  // Process/forward pending events for current window only (user input, size changes, shutdown...).
  bool Window::pollCurrentWindowEvents() noexcept { return false; }
    
  // Send 'windowClose' event to any window instance    
  void Window::sendCloseEvent(WindowHandle target) noexcept {}


// -- window builder -- --------------------------------------------------------

  // configure window class context + create new window
  std::unique_ptr<Window> Window::Builder::create(const window_char* contextName, const window_char* caption, WindowHandle parentWindow) { // throws
    return std::unique_ptr<Window>(new Window(contextName, caption, this->_params, parentWindow));
  }
  
  // new window construction
  Window::Window(const window_char* contextName, const window_char* caption, const Builder::Params& params, WindowHandle parentWindow) { // throws
    throw std::runtime_error("not implemented");
  }
  
  // ---

  // build instance from existing window
  std::unique_ptr<Window> Window::Builder::update(WindowHandle existingHandle, bool callOrigEventProc) { // throws
    return std::unique_ptr<Window>(new Window(this->_params, existingHandle, callOrigEventProc));
  }

  // exiting window re-styling
  Window::Window(const Builder::Params& params, WindowHandle existingHandle, bool callOrigEventProc) { // throws
    throw std::runtime_error("not implemented");
  }
  
  // ---
  
  Window::~Window() noexcept {}
  
  // ---

  bool Window::_validateFirstDisplay() noexcept {
    return false;
  }
  void Window::_destroyWindow() noexcept {
    
  }

#endif
