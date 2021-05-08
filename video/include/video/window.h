/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <thread/spin_lock.h>
#include <hardware/display_monitor.h>
#include <system/preprocessor_tools.h>
#include "video/window_handle.h"
#include "video/window_input.h"
#include "video/window_resource.h"

namespace pandora {
  namespace video {
    class __WindowImpl;
#   ifdef _WINDOWS
      typedef wchar_t window_char;
#   else
      typedef char window_char;
#   endif
    
    /// @brief Type of window (display mode + decoration)
    enum class WindowType : uint32_t {
      fullscreen = 0u, ///< Fullscreen display (warning: no x/y position, no resize mode)
      borderless = 1u, ///< Borderless window (splash screen or "fullscreen window", no resize mode)
      bordered = 2u, ///< Popup dialog with no caption
      dialog = 3u, ///< Popup dialog with caption + close button
      window = 4u  ///< Normal window with caption + buttons
    };
    /// @brief Window behavior settings (bit-mask flags)
    enum class WindowBehavior : uint32_t {
      none          = 0u,     ///< no widgets
      scrollH       = 0x001u, ///< horizontal scroll bar
      scrollV       = 0x002u, ///< vertical scroll bar
      dropShadow    = 0x010u, ///< Additional drop shadow to emphasize Z-order (useful for popup dialogs)
      topMost       = 0x100u, ///< window displayed above all non-topmost windows
      aboveTaskbar  = 0x200u, ///< window displayed above taskbar / status bar (useful for borderless windows)
      globalContext = 0x400u  ///< Use global context for display/paint/rendering (faster, but shared between all windows with this option).
                              ///  Only recommended for single window apps, for main display window only, or for duplicate/mirrored windows
    };
    /// @brief Window resize mode (bit-mask flags)
    enum class ResizeMode : uint32_t {
      fixed       = 0u,   ///< fixed width, fixed height (does not allow maximization)
      resizableX  = 0x1u, ///< resizable width 
      resizableY  = 0x2u, ///< resizable height
      resizable   = 0x3u, ///< resizable width/height (== resizableX|resizableY)
      homothety   = 0x4u  ///< homothety (ratio kept when resized)
    };
    /// @brief Pixel position (screen coordinates or window coordinates)
    struct PixelPosition {
      int32_t x;
      int32_t y;
    };
    /// @brief Pixel size (screen size or window size)
    struct PixelSize {
      uint32_t width;
      uint32_t height;
    };

    // ---
    
    /// @class Window
    /// @brief Operating-system window management
    /// @remarks - new windows are hidden -> call show(true) to display them
    ///          - mouse pointer is visible by default -> call showCursor(false) to hide it
    ///          - pollEvents or pollCurrentWindowEvents must be called regularly
    /// @warning It is recommended to enable DPI awareness (manifest or via DisplayMonitor) before creating window or monitor instance.
    class Window final {
    public:
      /// @brief Show/hide/raw cursor mode
      enum class CursorMode : uint32_t {
        visible   = 0u, ///< Show cursor + receive normal move events (position)
        hidden    = 1u, ///< Hide cursor + receive normal move events (position)
        hiddenRaw = 2u  ///< Hide cursor + receive raw move events (delta) -- should only be used for one window
      };
      /// @brief Type of cursor position to get/set
      enum class CursorPositionType : uint32_t {
        relative = 0u, ///< Relative to window client coordinates
        absolute = 1u  ///< Absolute position on screen
      };
      /// @brief Window visibility state command param
      enum class VisibilityCommand : uint32_t {
        hide         = 0u, ///< Hide window
        show         = 1u, ///< Show window (active)
        showInactive = 2u, ///< Show window (inactive)
        restore      = 3u, ///< Show window with original size/position (active, not minimized or maximized anymore)
        minimize     = 4u, ///< Minimize window (inactive)
        maximize     = 5u  ///< Maximize window (active)
      };
      /// @brief Window visibility state (bit-mask flags)
      /// @warning internal dev: values are aligned with impl-specific defines -> keep them aligned
      enum class VisibleState : uint32_t {
        none         = 0u,   ///< Hide window
        visible      = 0x1u, ///< Visible window (on screen)
        active       = 0x2u, ///< Active window (focus)
        minimized    = 0x4u, ///< Minimized
        maximized    = 0x8u  ///< Maximization button ON (might be minimized at the same time!)
      };

      ~Window() noexcept;
      Window(const Window&) = delete;
      Window(Window&&) = delete; // no move: registered pointers -> address must not change
      Window& operator=(const Window&) = delete;
      Window& operator=(Window&&) = delete; // no move
      
      
      // -- accessors --
      
      inline WindowHandle handle() const noexcept { return this->_handle; }       ///< Get native window handle
      inline WindowType displayMode() const noexcept { return this->_mode; }      ///< Get window type (fullscreen/borderless/window/...)
      inline ResizeMode resizeMode() const noexcept { return this->_resizeMode; } ///< Get current resizing mode (resizable X/Y, homothety)
      inline bool isOrphan() const noexcept { return (this->_parent == (WindowHandle)0); }///< Verify if window has a parent (false) or not (true)

      VisibleState visibleState() const noexcept; ///< Get current window visibility state
      pandora::hardware::DisplayArea getClientArea() const noexcept; ///< Get copy of current position/size of the client area
      PixelSize getClientSize() const noexcept; ///< Get current size of the client area
      float contentScale() const noexcept;      ///< Read scale factor to use for content (based on DPI)
      
      PixelPosition cursorPosition(CursorPositionType mode) noexcept;///< Get current mouse pointer position (-1 on error)
      int32_t getScrollPositionV() const noexcept; ///< Read vertical scroll box position (-1 on error)
      int32_t getScrollPositionH() const noexcept; ///< Read horizontal scroll box position (-1 on error)
      
      /// @brief Get last error message (if a setter returns false)
      /// @returns Last error (if available) or empty string
      static std::string getLastError();
      
      
      // -- display changes --
      
      bool show(VisibilityCommand state = VisibilityCommand::show) noexcept; ///< Show/hide/maximize/minimize/restore window
      bool move(int32_t x, int32_t y) noexcept; ///< Move window to position (size not changed)
      bool resize(uint32_t width, uint32_t height) noexcept; ///< Change window size (or resolution in fullscreen)
      bool resize(uint32_t width, uint32_t height, uint32_t rate) noexcept; ///< Change window size (or resolution/rate in fullscreen)
      bool resize(const pandora::hardware::DisplayArea& clientArea) noexcept; ///< Change window size and position
      
      /// @brief Change window type and behavior + position and size/resolution
      bool setDisplayMode(WindowType type, WindowBehavior components, const pandora::hardware::DisplayArea& clientArea, ResizeMode resizeMode, uint32_t rate = 0);
      bool setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept; ///< Define minimum size limits for the user (ignored if not resizable)
      
      bool clearClientArea() noexcept; ///< Clear entire client area (with background color)
      bool clear(const pandora::hardware::DisplayArea& area) noexcept; ///< Clear rectangle relative to client area (with background color)
      
      
      // -- resource changes --
      
      bool setCursor(std::shared_ptr<WindowResource> cursor) noexcept; ///< Change mouse pointer image (won't change cursor visibility -> see 'setMouseHandler')
      bool setCursorPosition(int32_t x, int32_t y, CursorPositionType mode) noexcept; ///< Change mouse pointer position (won't change cursor visibility -> see 'setMouseHandler')
      
      bool setCaption(const window_char* caption) noexcept; ///< Change title in window caption
      bool setMenu(MenuHandle menu) noexcept; ///< Add/replace native menu bar (use NULL to remove current menu)
      bool setBackgroundColorBrush(std::shared_ptr<WindowResource> colorBrush) noexcept;///< Change background color brush (will not affect display -> call clearClientArea())
      
      /// @brief Change vertical/horizontal scrollbar ranges (only values for existing scrollbars will be used)
      /// @remarks Scroll units don't need to be pixels, but the same unit should be used by the event handler for updating display.
      bool setScrollbarRange(uint16_t posV, uint16_t posH, uint16_t verticalMax, uint16_t horizontalMax, uint32_t pixelsPerUnit = 1) noexcept;
      bool setScrollPositionV(uint16_t pos) noexcept; ///< Change position of slider in vertical scrollbar
      bool setScrollPositionH(uint16_t pos) noexcept; ///< Change position of slider in horizontal scrollbar
      
      
      // -- input events --
      
      inline void setWindowHandler(WindowEventHandler handler) noexcept { _onWindowEvent = handler; }      ///< Set/replace window event handler (NULL to unregister)
      inline void setKeyboardHandler(KeyboardEventHandler handler) noexcept { _onKeyboardEvent = handler; }///< Set/replace keyboard event handler (NULL to unregister)
      void setMouseHandler(MouseEventHandler handler, CursorMode cursor = CursorMode::visible) noexcept;   ///< Set/replace mouse event handler (NULL to unregister)
      inline void setTouchHandler(TouchEventHandler handler) noexcept { _onTouchEvent = handler; }         ///< Set/replace touch event handler (NULL to unregister)
      
      /// @brief Process/forward pending events for all existing windows (user input, size changes, shutdown...).
      ///        - Should be called regularly, to dispatch window events.
      ///        - Must be called synchronously in the main loop.
      /// @remarks - Either call 'pollEvents' in main loop, once for all existing windows.
      ///          - Either call 'pollCurrentWindowEvents' regularly for each window instance.
      ///            Also useful to update to only one a specific modal window (and blocking other windows while it exists).
      /// @returns Global application status: alive (true) or quitting (false)
      static bool pollEvents() noexcept;
      
      /// @brief Process/forward pending events for current window only (user input, size changes, shutdown...).
      ///        - Should be called regularly, to dispatch window events.
      ///        - Must be called in the thread that created the window.
      /// @remarks - Either call 'pollEvents' in main loop, once for all existing windows.
      ///          - Either call 'pollCurrentWindowEvents' regularly for each window instance.
      ///            Also useful to update to only one a specific modal window (and blocking other windows while it exists).
      /// @returns Current window status: alive (true) or destroyed (false)
      bool pollCurrentWindowEvents() noexcept;
      

      // -- window builder -- --------------------------------------------------

      /// @class Window.Builder
      /// @brief Window object builder, to configure and create a Window instance.
      /// @remarks The same builder can be used to build multiple Window instances.
      class Builder final {
      public:
        Builder() = default;
        ~Builder() = default;
        
        Builder(const Builder&) = delete;
        Builder(Builder&&) = default;
        Builder& operator=(const Builder&) = delete;
        Builder& operator=(Builder&&) = default;
        
        // -- display params --
        
        static inline constexpr int32_t defaultPosition() noexcept { return 0x7FFFFFFF; }  ///< default system X/Y coords
        static inline constexpr int32_t centeredPosition() noexcept { return 0x7FFFFFFE; } ///< center window
        static inline constexpr uint32_t defaultSize() noexcept { return 0; } ///< default system window size (or desktop size in fullscreen)
        
        /// @brief Set display monitor on which the window should appear (if omitted, will use default monitor)
        inline Builder& setParentMonitor(std::shared_ptr<pandora::hardware::DisplayMonitor> monitor) noexcept;
        /// @brief Set window type (dialog/window/fullscreen/...) + behaviors (scrollable/resizable/top-most/...)
        inline Builder& setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resize) noexcept;
        /// @brief Set window refresh rate in milliHertz (only used in fullscreen mode, if value != 0)
        inline Builder& setRefreshRate(uint32_t rateMilliHz) noexcept;
        
        /// @brief Set X/Y coordinates of the client area of the window (ignored if fullscreen)
        /// @remarks - Values can also be defaultPosition(), centeredPosition().
        ///          - If no parent window: relative to screen defined by parent monitor (or default monitor, if not specified).
        ///          - If parent window provided: relative to parent window position.
        inline Builder& setPosition(int32_t x, int32_t y) noexcept;
        /// @brief Set width/height of the client area of the window (or resolution if fullscreen)
        /// @remarks Values can also be defaultSize().
        inline Builder& setSize(uint32_t width, uint32_t height) noexcept;
        
        // -- resources --
        
        /// @brief Set window caption/taskbar icons (if 'captionIcon' is omitted, 'appIcon' will be used for both)
        inline Builder& setIcon(std::shared_ptr<WindowResource> appIcon, std::shared_ptr<WindowResource> captionIcon = nullptr) noexcept;
        /// @brief Set custom mouse pointer (to use default pointer: do not set, or set to NULL)
        inline Builder& setCursor(std::shared_ptr<WindowResource> cursor) noexcept;
        /// @brief Set background color brush for window client area
        inline Builder& setBackgroundColor(std::shared_ptr<WindowResource> colorBrush) noexcept;
        /// @brief Set native menu bar (if not set or NULL, no menu bar will appear).
        /// @warning The native menu must not be destroyed while the Window instance uses it.
        inline Builder& setMenu(MenuHandle handle) noexcept;
#       ifdef _WINDOWS
          /// @brief Set handle of current module instance (library or executable linked to this class)
          inline Builder& setModuleInstance(void* handle) noexcept { this->_params.moduleInstance = handle; return *this; }
#       endif
        
        // -- builder --
        
        /// @brief Create a new Window with current builder params.
        /// @param[in] contextName   Unique identifier for window context (not empty, no special characters, no spaces, max 256 characters).
        /// @param[in] caption       Title text in window caption
        /// @param[in] parentWindow  Handle of parent window (optional, only for sub-windows and dialogs)
        /// @warning - On Windows, * <system/windows_app.h> must have been initialized with the module handle (of current executable/library module).
        ///                        * or 'setModuleInstance' must be called.
        ///          - It is recommended to enable DPI awareness (manifest or via DisplayMonitor) before creating window or monitor instance.
        /// @throws invalid_argument/runtime_error/logic_error/bad_alloc on window creation failure
        /// @returns Valid window instance (hidden)
        std::unique_ptr<Window> create(const window_char* contextName, const window_char* caption, WindowHandle parentWindow = nullptr); // throws

        /// @brief Build Window instance with current params from an existing window handle.
        /// @remarks Useful to turn a splash/GUI window into the app window, then restore original window on exit.
        /// @param[in] existingHandle     Handle of existing window to update (required)
        /// @param[in] callOrigEventProc  Use event processor of original window as default fallback
        /// @warning It is recommended to enable DPI awareness before creating window or monitor instance.
        /// @returns Valid window instance (visible)
        /// @throws runtime_error/logic_error/bad_alloc on window update failure
        std::unique_ptr<Window> update(WindowHandle existingHandle, bool callOrigEventProc = false); // throws

        // ---

        struct Params {
          // resources
          std::shared_ptr<pandora::hardware::DisplayMonitor> monitor = nullptr;
          std::shared_ptr<WindowResource> appIcon = nullptr;
          std::shared_ptr<WindowResource> captionIcon = nullptr;
          std::shared_ptr<WindowResource> cursor = nullptr;
          std::shared_ptr<WindowResource> backgroundColor = nullptr;
          MenuHandle menu = nullptr;
#         ifdef _WINDOWS
            void* moduleInstance = nullptr;
#         endif
          // display params
          WindowType displayMode = WindowType::window;
          WindowBehavior behavior = WindowBehavior::none;
          ResizeMode resizeMode = ResizeMode::fixed;
          pandora::hardware::DisplayArea clientArea { defaultPosition(),defaultPosition(), defaultSize(),defaultSize() };
          uint32_t refreshRate = 0;
        };
      private:
        Params _params;
      };

    private:
      Window(const window_char* contextName, const window_char* caption, const Builder::Params& params, WindowHandle parentWindow); // throws
      Window(const Builder::Params& params, WindowHandle existingHandle, bool callOrigEventProc); // throws

      bool _validateFirstDisplay() noexcept;
      void _destroyWindow() noexcept;
      
      friend class pandora::video::Window::Builder;
      friend class pandora::video::__WindowImpl;
      
    private:
      // resources
      std::basic_string<window_char> _contextName;
      std::shared_ptr<pandora::hardware::DisplayMonitor> _monitor = nullptr;
      std::shared_ptr<WindowResource> _appIcon = nullptr;
      std::shared_ptr<WindowResource> _captionIcon = nullptr;
      std::shared_ptr<WindowResource> _cursor = nullptr;
      std::shared_ptr<WindowResource> _backgroundColor = nullptr;
      MenuHandle _menuHandle = nullptr;
      void* _moduleInstance = nullptr;

      // display params
      WindowHandle _handle = (WindowHandle)0;
      WindowType _mode = WindowType::window;
      ResizeMode _resizeMode = ResizeMode::fixed;
      WindowBehavior _behavior = WindowBehavior::none;
      uint32_t _refreshRate = 0; // mHz
      mutable pandora::thread::SpinLock _clientAreaLock;
      pandora::hardware::DisplayArea _clientArea { 0,0,0,0 };
      
      // input events
      WindowEventHandler _onWindowEvent = nullptr;
      KeyboardEventHandler _onKeyboardEvent = nullptr;
      MouseEventHandler _onMouseEvent = nullptr;
      TouchEventHandler _onTouchEvent = nullptr;
      
      // window manager
      __WindowImpl* _impl = nullptr; // implementation-specific
      WindowHandle _parent = (WindowHandle)0;
      uint32_t _nativeFlag = 0;
    };
  }
}

_P_FLAGS_OPERATORS(pandora::video::WindowBehavior, uint32_t)
_P_FLAGS_OPERATORS(pandora::video::ResizeMode, uint32_t)
_P_FLAGS_OPERATORS(pandora::video::Window::VisibleState, uint32_t)

// ---

// -- window builder - display params --

inline pandora::video::Window::Builder& 
pandora::video::Window::Builder::setParentMonitor(std::shared_ptr<pandora::hardware::DisplayMonitor> monitor) noexcept { 
  this->_params.monitor = monitor; 
  return *this;
}
inline pandora::video::Window::Builder& 
pandora::video::Window::Builder::setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resize) noexcept { 
  this->_params.displayMode = type; 
  this->_params.behavior = behavior; 
  this->_params.resizeMode = resize; 
  return *this;
}
inline pandora::video::Window::Builder& pandora::video::Window::Builder::setRefreshRate(uint32_t rateMilliHz) noexcept {
  this->_params.refreshRate = rateMilliHz;
  return *this;
}
inline pandora::video::Window::Builder& pandora::video::Window::Builder::setPosition(int32_t x, int32_t y) noexcept { 
  this->_params.clientArea.x = x; 
  this->_params.clientArea.y = y; 
  return *this; 
}
inline pandora::video::Window::Builder& pandora::video::Window::Builder::setSize(uint32_t width, uint32_t height) noexcept { 
  this->_params.clientArea.width = width; 
  this->_params.clientArea.height = height; 
  return *this; 
}

// -- window builder - resources --

pandora::video::Window::Builder& 
pandora::video::Window::Builder::setIcon(std::shared_ptr<WindowResource> appIcon, std::shared_ptr<WindowResource> captionIcon) noexcept { 
  _params.appIcon = appIcon; 
  _params.captionIcon = captionIcon; 
  return *this;
}
pandora::video::Window::Builder& pandora::video::Window::Builder::setCursor(std::shared_ptr<WindowResource> cursor) noexcept { 
  _params.cursor = cursor; 
  return *this;
}
pandora::video::Window::Builder& pandora::video::Window::Builder::setBackgroundColor(std::shared_ptr<WindowResource> colorBrush) noexcept { 
  _params.backgroundColor = colorBrush; 
  return *this;
}
pandora::video::Window::Builder& pandora::video::Window::Builder::setMenu(MenuHandle handle) noexcept { 
  _params.menu = handle; 
  return *this;
}
