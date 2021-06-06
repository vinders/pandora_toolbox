/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : WindowResource - Wayland implementation (Linux)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstdint>
# include <cstdlib>
# include <cstring>
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "video/window_resource.h"

# define __P_MAKE_SHARED_RES(handle, categ) \
         std::make_shared<WindowResource>((WindowResourceHandle) handle, categ)

  using namespace pandora::video;


// -- WindowResource destruction -- --------------------------------------------

  WindowResource::~WindowResource() noexcept {
    if (this->_handle) {
      switch (this->_type) {
        case WindowResource::Category::icon:       free(this->_handle); break;
        case WindowResource::Category::cursor:     free(this->_handle); break;
        case WindowResource::Category::colorBrush: free(this->_handle); break;
        default: break;
      }
    }
  }


// -- Icon builder -- ----------------------------------------------------------

  // system icon
  std::shared_ptr<WindowResource> WindowResource::buildIcon(SystemIcon id) {
    void* handle = nullptr;
    switch (id) {
      case SystemIcon::app:      handle = malloc(sizeof(uint32_t)); break;//TODO
      case SystemIcon::info:     handle = malloc(sizeof(uint32_t)); break;
      case SystemIcon::question: handle = malloc(sizeof(uint32_t)); break;
      case SystemIcon::warning:  handle = malloc(sizeof(uint32_t)); break;
      case SystemIcon::error:    handle = malloc(sizeof(uint32_t)); break;
      case SystemIcon::security: handle = malloc(sizeof(uint32_t)); break;
      case SystemIcon::system:   handle = malloc(sizeof(uint32_t)); break;
      default: break;
    }
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }

  // extern icon file
  std::shared_ptr<WindowResource> WindowResource::buildIconFromFile(const char* uri, uint32_t width, uint32_t height) {
    void* handle = malloc(sizeof(uint32_t));
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }

  // embedded resource icon file
  std::shared_ptr<WindowResource> WindowResource::buildIconFromPackage(const char* id) {
    void* handle = malloc(sizeof(uint32_t));
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }


// -- Cursor builder -- --------------------------------------------------------

  // system cursor
  std::shared_ptr<WindowResource> WindowResource::buildCursor(SystemCursor id) {
    void* handle = nullptr;
    switch (id) {
      case SystemCursor::pointer:     handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::hand:        handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::wait:        handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::waitPointer: handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::textIBeam:   handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::help:        handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::forbidden:   handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::crosshair:   handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::arrowV:      handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::doubleArrowV:      handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::doubleArrowH:      handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::doubleArrow_NE_SW: handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::doubleArrow_NW_SE: handle = malloc(sizeof(uint32_t)); break;
      case SystemCursor::fourPointArrow:    handle = malloc(sizeof(uint32_t)); break;
      default: break;
    }
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor) : nullptr;
  }
  
  // extern cursor file
  std::shared_ptr<WindowResource> WindowResource::buildCursorFromFile(const char* uri) {
    void* handle = malloc(sizeof(uint32_t));
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor) : nullptr;
  }
  
  // embedded resource cursor file
  std::shared_ptr<WindowResource> WindowResource::buildCursorFromPackage(const char* id) {
    void* handle = malloc(sizeof(uint32_t));
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor) : nullptr;
  }


// -- Color brush builder -- ---------------------------------------------------

  // system color brush
  std::shared_ptr<WindowResource> WindowResource::buildColorBrush(WindowResource::Color color) {
    void* handle = nullptr;
    switch (color) {
      case WindowResource::systemWindowColor(): handle = malloc(sizeof(uint32_t)); break;
      case WindowResource::systemTextColor():   handle = malloc(sizeof(uint32_t)); break;
      case WindowResource::transparentColor():  handle = malloc(sizeof(uint32_t)); break;
      case 0x00000000: handle = malloc(sizeof(uint32_t)); break;
      case 0x00FFFFFF: handle = malloc(sizeof(uint32_t)); break;
      default:         handle = malloc(sizeof(uint32_t)); break;
    }
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::colorBrush) : nullptr;
  }


// -- Menu container builder -- ------------------------------------------------

  // create main menubar (isSubMenu==false) or popup sub-menu (isSubMenu==true)
  WindowMenu::WindowMenu(bool isSubMenu) 
    : _handle(malloc(sizeof(uint32_t))), 
      _isSubMenu(isSubMenu) {
    if (this->_handle == nullptr)
      throw std::runtime_error("WindowMenu: menu creation failed");
  }
  WindowMenu::~WindowMenu() noexcept {
    if (this->_handle != nullptr)
      free(this->_handle);
  }

  // add menu command item
  bool WindowMenu::insertItem(uint32_t id, const char* label, WindowMenu::ItemType type, bool isEnabled) noexcept {
    return true;
  }
  // add sub-menu
  bool WindowMenu::insertSubMenu(WindowMenu&& item, const char* label) noexcept {
    return (item.isSubMenu() && item._handle);
  }
  // insert item separator (vertical separator in sub-menus, horizontal separator in main menu)
  void WindowMenu::insertSeparator() noexcept {}
  
  // ---

  // change state of a checkbox/radio-button
  void WindowMenu::changeCheckItemState(MenuHandle menu, uint32_t id, bool isChecked, bool isEnabled) noexcept {}
  // change checkboxes/radio-buttons selection
  void WindowMenu::changeCheckItemState(MenuHandle menu, uint32_t checkId, uint32_t uncheckId, bool isUncheckedEnabled) noexcept {}
  // change state of any other item type
  void WindowMenu::changeItemState(MenuHandle menu, uint32_t id, bool isEnabled) noexcept {}

  // ---

  // create portable window menu resource
  std::shared_ptr<WindowResource> WindowResource::buildMenu(WindowMenu&& mainMenu) {
    if (mainMenu._handle != nullptr && !mainMenu.isSubMenu()) {
      auto resource = __P_MAKE_SHARED_RES(mainMenu._handle, WindowResource::Category::menu);
      mainMenu._handle = nullptr;
#     if defined(_CPP_REVISION) && _CPP_REVISION == 14
        return std::move(resource);
#     else
        return resource;
#     endif
    }
    return nullptr;
  }
  // native menu container
  std::shared_ptr<WindowResource> WindowResource::buildMenu(MenuHandle handle) {
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::menu) : nullptr;
  }

#endif
