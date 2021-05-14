/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
# include <cstdint>
# include <cstddef>
# include <memory>
# include <mutex>
# include <thread/recursive_spin_lock.h>
# include <system/api/windows_api.h>
# include "video/window.h"

// -- constants --

# define __P_DEFAULT_WINDOW_WIDTH      800  // default width (if system work area can't be read)
# define __P_DEFAULT_WINDOW_HEIGHT     600  // default height (if system work area can't be read)
# define __P_DEFAULT_MIN_WINDOW_WIDTH  220  // min resizable width
# define __P_DEFAULT_MIN_WINDOW_HEIGHT 100  // min resizable height
# define __P_UNKNOWN_CURSOR_POS 0x7FFFFFFF

// -- window state flags --

# define __P_FLAGS_TYPE                              uint32_t
# define __P_ADD_FLAG(status,added)                  status |= (added)
# define __P_REMOVE_FLAG(status,removed)             status &= ~((__P_FLAGS_TYPE)(removed))
# define __P_ADD_REMOVE_FLAGS(status,added,removed)  status = ((status | (added)) & ~((__P_FLAGS_TYPE)(removed)))

# define __P_FLAG_USE_ORIG_EVENT_PROC 0x0001  // use event processor from "original style" backup as default
# define __P_FLAG_FIRST_DISPLAY_DONE  0x0002  // window has been displayed at least once (-> api methods available)
# define __P_FLAG_FULLSCREEN_ON       0x0004  // currently in fullscreen display
# define __P_FLAG_FULLSCREEN_RES      0x0008  // fullscreen currently with another display mode than system desktop

# define __P_FLAGS_VISIBILITY_MASK    0x00F0  // -- flags aligned with VisibleState enum values --
# define __P_FLAGS_VISIBILITY_OFFSET  4
# define __P_FLAG_WINDOW_VISIBLE      0x0010  // window is currently on screen (not hidden, not minimized)
# define __P_FLAG_WINDOW_ACTIVE       0x0020  // window is currently active
# define __P_FLAG_WINDOW_MINIMIZED    0x0040  // window is minimized
# define __P_FLAG_WINDOW_MAXIMIZED    0x0080  // window is maximized

# define __P_FLAG_POWER_SUSPENDED     0x0100  // host system is locked or suspended
# define __P_FLAG_MODE_CHANGE_PENDING 0x0200  // window display resolution change has just occurred
# define __P_FLAG_RESIZED_MOVED       0x0400  // window is currently being resized or moved
# define __P_FLAG_CURSOR_RAW_INPUT    0x0800  // use raw move events for mouse
# define __P_FLAG_CURSOR_HIDE         0x1000  // cursor should be hidden when the window is active
# define __P_FLAG_CURSOR_HOVER        0x2000  // cursor is currently in client area
# define __P_FLAG_CURSOR_MENULOOP     0x4000  // menu loop is currently active
# define __P_FLAG_CURSOR_EVENT_REG    0x8000  // registration to native cursor events for current window
# define __P_FLAG_SIZE_HANDLER_CHANGE 0x10000 // a size change has been triggered by the size handler (and should not be re-processed)
# define __P_FLAG_SCROLLRANGE_HANDLER_CHANGE 0x20000 // a scroll range change has been triggered by the size handler (and should not be re-processed)

namespace pandora {
  namespace video {
    class __WindowImplEventProc;
    
    // window decoration sizes (on each side of a window)
    struct WindowDecorationSize {
      int32_t left;
      int32_t right;
      int32_t top;
      int32_t bottom;
    };
    
    // window style storage (backup/restore)
    class WindowStyle final {
    public:
      WindowStyle() = default;
      WindowStyle(HWND handle) noexcept { backupStyle(handle); }
      ~WindowStyle() noexcept = default;
      
      void backupStyle(HWND handle) noexcept;// store style properties from existing window (backup)
      void applyStyle(HWND handle) noexcept; // apply stored style properties on existing window (restore)
      
    private:
      RECT _windowArea{ -1,-1,-1,-1 };

      HICON _appIcon = nullptr;
      HICON _captionIcon = nullptr;
      HCURSOR _cursor = nullptr;
      HBRUSH _backgroundColor = nullptr;
      HMENU _menu = nullptr;

    public:
      WNDPROC _eventProcessor = nullptr;
      DWORD _windowStyle = 0;
      DWORD _windowStyleExt = 0;
    };
    
    // ---
    
    // Window manager - Win32 impl
    class __WindowImpl final {
    public:
      __WindowImpl(Window& container, const wchar_t* caption, const wchar_t* windowClassName, 
                   const Window::Builder::Params& params, HWND parent); // throws
      __WindowImpl(Window& container, HWND existingHandle, const Window::Builder::Params& params, 
                   bool callExistingEventProc); // throws
      ~__WindowImpl() noexcept;
      
      static std::string formatLastError(const char* prefix);
      
      // -- accessors --
      
      inline HWND handle() const noexcept { return this->_handle; }
      static int32_t getWindowInstanceCount() noexcept;
      
      inline WindowType displayMode() const noexcept { return this->_mode; }
      inline bool isFullscreen() const noexcept { return (this->_mode == WindowType::fullscreen); }
      inline bool hasMenu() const noexcept { return (this->_menuHandle != nullptr); }
      inline bool hasParent() const noexcept { return (this->_parent != nullptr); }
      static inline bool isFullscreen(WindowType mode) noexcept { return (mode == WindowType::fullscreen); }
      
      inline ResizeMode resizeMode() const noexcept { return this->_resizeMode; }
      inline bool isResizable() const noexcept { return ((this->_resizeMode & ResizeMode::resizable) == true); }
      inline bool isResizableX() const noexcept { return ((this->_resizeMode & ResizeMode::resizableX) == true); }
      inline bool isResizableY() const noexcept { return ((this->_resizeMode & ResizeMode::resizableY) == true); }
      inline bool isResizableXY() const noexcept { return ((this->_resizeMode & ResizeMode::resizable) == ResizeMode::resizable); }
      inline bool hasHomothety() const noexcept { return ((this->_resizeMode & ResizeMode::homothety) == true); }
      static inline bool isResizable(ResizeMode mode) noexcept { return ((mode & ResizeMode::resizable) == true); }
      
      inline WindowBehavior behavior() const noexcept { return this->_behavior; }
      inline bool isScrollable() const noexcept { return (this->_currentStyleFlag & (WS_HSCROLL|WS_VSCROLL)); }
      inline bool isScrollableH() const noexcept { return (this->_currentStyleFlag & WS_HSCROLL); }
      inline bool isScrollableV() const noexcept { return (this->_currentStyleFlag & WS_VSCROLL); }
      inline bool isAboveTaskbar() const noexcept { return (this->_currentStyleExtFlag & WS_EX_APPWINDOW); }
      inline Window::VisibleState getVisibleState() const noexcept;
      
      float contentScale() const noexcept;
      inline pandora::hardware::DisplayArea lastWindowArea() const noexcept;
      inline pandora::hardware::DisplayArea lastClientArea() const noexcept;
      inline void lastRelativeClientRect(RECT& outRect) const noexcept;
      inline PixelPosition lastClientPosition() const noexcept;
      inline PixelSize lastClientSize() const noexcept;
      
      inline PixelPosition lastScrollPosition() const noexcept;
      inline int32_t lastScrollPositionH() const noexcept;
      inline int32_t lastScrollPositionV() const noexcept;
      
      // -- style flag conversions --
      
      static void toWindowStyleFlags(WindowType type, WindowBehavior behavior, ResizeMode resizeMode, 
                                     DWORD& outWindowStyle, DWORD& outWindowStyleExt) noexcept;
      static UINT toWindowClassStyleFlag(WindowBehavior behavior) noexcept;
      
      static int toWindowVisibilityFlag(Window::VisibilityCommand state) noexcept;
      static int toFullscreenVisibilityFlag(Window::VisibilityCommand state) noexcept;
      
      // -- change display mode --
      
      bool show(int visibilityFlag) noexcept;
      bool setDisplayMode(WindowType type, WindowBehavior behavior, ResizeMode resizeMode,
                          const pandora::hardware::DisplayArea& userClientArea, uint32_t rate);
      
      bool setMenu(HMENU menuHandle) noexcept;
      
      // -- display size/position
      
      bool move(int32_t x, int32_t y) noexcept;
      bool resize(uint32_t width, uint32_t height) noexcept;
      bool resize(const pandora::hardware::DisplayArea& clientArea) noexcept;
      
      bool setMinClientAreaSize(uint32_t minWidth, uint32_t minHeight) noexcept;
      
      bool setCursorPosition(int32_t x, int32_t y, Window::CursorPositionType mode) noexcept;
      
      bool setScrollbarRange(uint32_t posH, uint32_t posV, uint32_t horizontalMax, uint32_t verticalMax, uint32_t pixelsPerUnit) noexcept;
      bool setScrollPositionH(uint32_t pos) noexcept;
      bool setScrollPositionV(uint32_t pos) noexcept;
      
      // -- window events --
      
      void setWindowHandler(WindowEventHandler handler) noexcept;
      void setPositionHandler(PositionEventHandler handler) noexcept;
      void setKeyboardHandler(KeyboardEventHandler handler) noexcept;
      void setMouseHandler(MouseEventHandler handler, Window::CursorMode cursor) noexcept;
    
      static LRESULT CALLBACK windowEventProcessor(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
      
    private:
      void _destroy() noexcept;
      
      // -- window metrics --
      
      void computeWindowDecorations(DWORD windowStyle, DWORD windowStyleExt, 
                                    bool hasMenu, WindowDecorationSize& outSizes) noexcept;
      void readVisibleWindowDecorations(const pandora::hardware::DisplayArea& clientArea, 
                                        WindowDecorationSize& outSizes) noexcept;
      
      bool readVisibleWindowArea(pandora::hardware::DisplayArea& outWindowArea) noexcept;
      bool readVisibleClientArea(pandora::hardware::DisplayArea& outClientArea) noexcept;
      
      void adjustScrollbarPageSize(const pandora::hardware::DisplayArea& clientArea, bool repaint) noexcept;
      
      // -- user area computation --
      
      void computeUserPosition(int32_t x, int32_t y, const WindowDecorationSize& decorationSizes, 
                               pandora::hardware::DisplayArea& inOutClientArea, pandora::hardware::DisplayArea& outWindowPos) noexcept;
      void computeUserSize(uint32_t width, uint32_t height, const WindowDecorationSize& decorationSizes, 
                           pandora::hardware::DisplayArea& outClientSize, pandora::hardware::DisplayArea& outWindowSize) noexcept;
      void computeUserArea(WindowType mode, WindowBehavior behavior, const pandora::hardware::DisplayArea& userArea, 
                           const WindowDecorationSize& decorationSizes, pandora::hardware::DisplayArea& outClientArea, 
                           pandora::hardware::DisplayArea& outWindowArea) noexcept;
      void adjustVisibleClientSize() noexcept;
      
      // -- display resolution/mode --
      
      void enterFullscreenMode(pandora::hardware::DisplayArea windowArea, uint32_t rate) noexcept;
      void exitFullscreenMode() noexcept;
      
      bool firstShow(int visibilityFlag) noexcept;    
      
      friend class __WindowImplEventProc;
    
    private:
      // resources
      std::wstring _windowClassName;
      std::shared_ptr<pandora::hardware::DisplayMonitor> _monitor = nullptr; // parent display monitor
      std::unique_ptr<WindowStyle> _originalStyle = nullptr;                 // backup of original style (builder.update existing window)
      RAWINPUT* _mouseInputBuffer = nullptr; // buffer for raw motion events
      size_t _mouseInputBufferSize = 0;
      HINSTANCE _moduleInstance = nullptr;
      HWND _handle = nullptr;     // native window handle
      HWND _parent = nullptr;     // native parent window handle
      HMENU _menuHandle = nullptr;// native menu handle
      Window& _container;         // Window instance containing implementation
      
      // input event handlers
      WindowEventHandler _onWindowEvent = nullptr;
      PositionEventHandler _onPositionEvent = nullptr;
      KeyboardEventHandler _onKeyboardEvent = nullptr;
      MouseEventHandler _onMouseEvent = nullptr;
      
      // window size/position
      WindowDecorationSize _decorationSizes{ 0,0,0,0 }; // size of window decoration (caption/borders/scrollbars)
      pandora::hardware::DisplayArea _lastClientArea{ 0,0,__P_DEFAULT_WINDOW_WIDTH,__P_DEFAULT_WINDOW_HEIGHT }; // last known client-area size/position
      PixelSize     _minClientSize{ __P_DEFAULT_MIN_WINDOW_WIDTH,__P_DEFAULT_MIN_WINDOW_HEIGHT }; // minimum client-area size
      PixelPosition _lastCursorPosition{ __P_UNKNOWN_CURSOR_POS,__P_UNKNOWN_CURSOR_POS }; // last known cursor position (for raw motion mode only)
      PixelPosition _lastScrollPosition{ 0,0 };   // last known scroll position
      PixelSize     _maxScrollPosition{ 0,0 };    // scroll range limits
      double        _clientAreaRatio = 4.0/3.0;   // ratio used for homothety
      uint32_t      _scrollUnit = 1;              // number of pixels per scroll unit
      mutable pandora::thread::RecursiveSpinLock _sizePositionLock;
    
      // window properties
      __P_FLAGS_TYPE _statusFlags = 0;
      DWORD _currentStyleFlag = 0;
      DWORD _currentStyleExtFlag = 0;
      WindowType _mode = WindowType::window;
      WindowBehavior _behavior = WindowBehavior::none;
      ResizeMode _resizeMode = ResizeMode::fixed;
      uint32_t _refreshRate = 0; // milli-Hz
    };
  }
}

// ---

inline pandora::video::Window::VisibleState pandora::video::__WindowImpl::getVisibleState() const noexcept {
  return (Window::VisibleState)((this->_statusFlags & __P_FLAGS_VISIBILITY_MASK) >> __P_FLAGS_VISIBILITY_OFFSET);
}

inline pandora::hardware::DisplayArea pandora::video::__WindowImpl::lastWindowArea() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return pandora::hardware::DisplayArea{ this->_lastClientArea.x - this->_decorationSizes.left, 
                                         this->_lastClientArea.y - this->_decorationSizes.top, 
                                         this->_lastClientArea.width + this->_decorationSizes.left + this->_decorationSizes.right, 
                                         this->_lastClientArea.height + this->_decorationSizes.top + this->_decorationSizes.bottom };
}

//not a reference: make a copy -> lock can be freed
inline pandora::hardware::DisplayArea pandora::video::__WindowImpl::lastClientArea() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return this->_lastClientArea;
}
inline void pandora::video::__WindowImpl::lastRelativeClientRect(RECT& outRect) const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  outRect.left = 0;
  outRect.top = 0;
  outRect.right = (int)this->_lastClientArea.x + (int)this->_lastClientArea.width;
  outRect.bottom = (int)this->_lastClientArea.y + (int)this->_lastClientArea.height;
}
inline pandora::video::PixelPosition pandora::video::__WindowImpl::lastClientPosition() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return pandora::video::PixelPosition{ this->_lastClientArea.x, this->_lastClientArea.y };
}
inline pandora::video::PixelSize pandora::video::__WindowImpl::lastClientSize() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return pandora::video::PixelSize{ this->_lastClientArea.width, this->_lastClientArea.height };
}

inline pandora::video::PixelPosition pandora::video::__WindowImpl::lastScrollPosition() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return pandora::video::PixelPosition{ this->_lastScrollPosition.x, this->_lastScrollPosition.y };
}
inline int32_t pandora::video::__WindowImpl::lastScrollPositionH() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return this->_lastScrollPosition.x;
}
inline int32_t pandora::video::__WindowImpl::lastScrollPositionV() const noexcept {
  std::lock_guard<pandora::thread::RecursiveSpinLock> guard(_sizePositionLock);
  return this->_lastScrollPosition.y;
}

#endif
