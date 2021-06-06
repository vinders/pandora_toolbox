/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : WindowResource - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# include <cstdint>
# include <cstring>
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "video/window_resource.h"

# define __P_MAKE_SHARED_RES(handle, categ) \
         std::make_shared<WindowResource>((WindowResourceHandle) handle, categ)

  using namespace pandora::video;
  using pandora::system::AppInstanceHandle;
  using pandora::system::WindowsApp;


// -- WindowResource destruction -- --------------------------------------------

  WindowResource::~WindowResource() noexcept {
    if (this->_handle) {
      switch (this->_type) {
        case WindowResource::Category::icon:       DestroyIcon((HICON)this->_handle); break;
        case WindowResource::Category::cursor:     DestroyCursor((HCURSOR)this->_handle); break;
        case WindowResource::Category::colorBrush: DeleteObject((HBRUSH)this->_handle); break;
        case WindowResource::Category::menu:       DestroyMenu((HMENU)this->_handle); break;
        default: break;
      }
    }
  }


// -- Icon builder -- ----------------------------------------------------------

  // system icon
  std::shared_ptr<WindowResource> WindowResource::buildIcon(SystemIcon id) {
    HICON handle = nullptr;
    switch (id) {
      case SystemIcon::app:      handle = LoadIcon(nullptr, IDI_APPLICATION); break;
      case SystemIcon::info:     handle = LoadIcon(nullptr, IDI_INFORMATION); break;
      case SystemIcon::question: handle = LoadIcon(nullptr, IDI_QUESTION); break;
      case SystemIcon::warning:  handle = LoadIcon(nullptr, IDI_WARNING); break;
      case SystemIcon::error:    handle = LoadIcon(nullptr, IDI_ERROR); break;
      case SystemIcon::security: handle = LoadIcon(nullptr, IDI_SHIELD); break;
      case SystemIcon::system:   handle = LoadIcon(nullptr, IDI_WINLOGO); break;
      default: break;
    }
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }

  // extern icon file
  std::shared_ptr<WindowResource> WindowResource::buildIconFromFile(const char* uri, uint32_t width, uint32_t height) {
    UINT flags = (width == 0u || height == 0u) ? (LR_LOADFROMFILE | LR_DEFAULTSIZE) : LR_LOADFROMFILE;
    
    HANDLE handle = LoadImageA(nullptr, uri, IMAGE_ICON, width, height, flags);
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }
  std::shared_ptr<WindowResource> WindowResource::buildIconFromFile(const wchar_t* uri, uint32_t width, uint32_t height) {
    UINT flags = (width == 0u || height == 0u) ? (LR_LOADFROMFILE | LR_DEFAULTSIZE) : LR_LOADFROMFILE;
    
    HANDLE handle = LoadImageW(nullptr, uri, IMAGE_ICON, width, height, flags);
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }

  // embedded resource icon file
  std::shared_ptr<WindowResource> WindowResource::buildIconFromPackage(const char* id, pandora::system::AppInstanceHandle instance) {
    HICON handle = LoadIconA((instance) ? (HINSTANCE)instance : (HINSTANCE)WindowsApp::instance().handle(), id);
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }
  std::shared_ptr<WindowResource> WindowResource::buildIconFromPackage(const wchar_t* id, pandora::system::AppInstanceHandle instance) {
    HICON handle = LoadIconW((instance) ? (HINSTANCE)instance : (HINSTANCE)WindowsApp::instance().handle(), id);
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::icon) : nullptr;
  }


// -- Cursor builder -- --------------------------------------------------------

  // system cursor
  std::shared_ptr<WindowResource> WindowResource::buildCursor(SystemCursor id) {
    HCURSOR handle = nullptr;
    switch (id) {
      case SystemCursor::pointer:     handle = LoadCursor(nullptr, IDC_ARROW); break;
      case SystemCursor::hand:        handle = LoadCursor(nullptr, IDC_HAND); break;
      case SystemCursor::wait:        handle = LoadCursor(nullptr, IDC_WAIT); break;
      case SystemCursor::waitPointer: handle = LoadCursor(nullptr, IDC_APPSTARTING); break;
      case SystemCursor::textIBeam:   handle = LoadCursor(nullptr, IDC_IBEAM); break;
      case SystemCursor::help:        handle = LoadCursor(nullptr, IDC_HELP); break;
      case SystemCursor::forbidden:   handle = LoadCursor(nullptr, IDC_NO); break;
      case SystemCursor::crosshair:   handle = LoadCursor(nullptr, IDC_CROSS); break;
      case SystemCursor::arrowV:      handle = LoadCursor(nullptr, IDC_UPARROW); break;
      case SystemCursor::doubleArrowV:      handle = LoadCursor(nullptr, IDC_SIZENS); break;
      case SystemCursor::doubleArrowH:      handle = LoadCursor(nullptr, IDC_SIZEWE); break;
      case SystemCursor::doubleArrow_NE_SW: handle = LoadCursor(nullptr, IDC_SIZENESW); break;
      case SystemCursor::doubleArrow_NW_SE: handle = LoadCursor(nullptr, IDC_SIZENWSE); break;
      case SystemCursor::fourPointArrow:    handle = LoadCursor(nullptr, IDC_SIZEALL); break;
      default: break;
    }
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor) : nullptr;
  }
  
  // extern cursor file
  std::shared_ptr<WindowResource> WindowResource::buildCursorFromFile(const char* uri) {
    HANDLE handle = LoadImageA(nullptr, uri, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor) : nullptr;
  }
  std::shared_ptr<WindowResource> WindowResource::buildCursorFromFile(const wchar_t* uri) {
    HANDLE handle = LoadImageW(nullptr, uri, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor) : nullptr;
  }
  
  // embedded cursor reader
# ifdef UNICODE
#   define RT_CURSOR__A       MAKEINTRESOURCEA(1)
#   define RT_GROUP_CURSOR__A MAKEINTRESOURCEA((ULONG_PTR)(RT_CURSOR__A) + 11)
#   define RT_RCDATA__A       MAKEINTRESOURCEA(10)
#   define RT_CURSOR__W       RT_CURSOR
#   define RT_GROUP_CURSOR__W RT_GROUP_CURSOR
#   define RT_RCDATA__W       RT_RCDATA
# else
#   define RT_CURSOR__A       RT_CURSOR
#   define RT_GROUP_CURSOR__A RT_GROUP_CURSOR
#   define RT_RCDATA__A       RT_RCDATA
#   define RT_CURSOR__W       MAKEINTRESOURCEW(1)
#   define RT_GROUP_CURSOR__W MAKEINTRESOURCEW((ULONG_PTR)(RT_CURSOR__W) + 11)
#   define RT_RCDATA__W       MAKEINTRESOURCEW(10)
# endif
  
  static HCURSOR __readSpecialCursor_win32(HINSTANCE instance, HRSRC res) {
    if (!res)
      return nullptr;
    
    DWORD size = SizeofResource(instance, res);
    HGLOBAL globalRes = LoadResource(instance, res);
    if (size == 0 || !globalRes)
      return nullptr;
    
    LPBYTE resBytes = (LPBYTE)LockResource(globalRes);
    return (HCURSOR)CreateIconFromResource(resBytes, size, FALSE, 0x00030000);
  }
  
  // embedded resource cursor file
  std::shared_ptr<WindowResource> WindowResource::buildCursorFromPackage(const char* id, pandora::system::AppInstanceHandle instance) {
    HINSTANCE hInstance = (instance) ? (HINSTANCE)instance : (HINSTANCE)WindowsApp::instance().handle();
    HCURSOR handle = LoadCursorA(hInstance, id);
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    
    handle = __readSpecialCursor_win32(hInstance, FindResourceA(hInstance, id, RT_CURSOR__A));
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    
    handle = __readSpecialCursor_win32(hInstance, FindResourceA(hInstance, id, RT_GROUP_CURSOR__A));
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    
    handle = __readSpecialCursor_win32(hInstance, FindResourceA(hInstance, id, RT_RCDATA__A));
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    return nullptr;
  }
  std::shared_ptr<WindowResource> WindowResource::buildCursorFromPackage(const wchar_t* id, pandora::system::AppInstanceHandle instance) {
    HINSTANCE hInstance = (instance) ? (HINSTANCE)instance : (HINSTANCE)WindowsApp::instance().handle();
    HCURSOR handle = LoadCursorW(hInstance, id);
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    
    handle = __readSpecialCursor_win32(hInstance, FindResourceW(hInstance, id, RT_CURSOR__W));
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    
    handle = __readSpecialCursor_win32(hInstance, FindResourceW(hInstance, id, RT_GROUP_CURSOR__W));
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    
    handle = __readSpecialCursor_win32(hInstance, FindResourceW(hInstance, id, RT_RCDATA__W));
    if (handle != nullptr)
      return __P_MAKE_SHARED_RES(handle, WindowResource::Category::cursor);
    return nullptr;
  }


// -- Color brush builder -- ---------------------------------------------------

  // system color brush
  std::shared_ptr<WindowResource> WindowResource::buildColorBrush(WindowResource::Color color) {
    HBRUSH handle = nullptr;
    switch (color) {
      case WindowResource::systemWindowColor(): handle = GetSysColorBrush(COLOR_WINDOW+1); break;
      case WindowResource::systemTextColor():   handle = GetSysColorBrush(COLOR_WINDOWTEXT+1); break;
      case WindowResource::transparentColor():  handle = (HBRUSH)GetStockObject(NULL_BRUSH); break;
      case 0x00000000: handle = (HBRUSH)GetStockObject(BLACK_BRUSH); break;
      case 0x00FFFFFF: handle = (HBRUSH)GetStockObject(WHITE_BRUSH); break;
      default:         handle = CreateSolidBrush((COLORREF)(color & 0x00FFFFFFu)); break;
    }
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::colorBrush) : nullptr;
  }


// -- Menu container builder -- ------------------------------------------------

  // create main menubar (isSubMenu==false) or popup sub-menu (isSubMenu==true)
  WindowMenu::WindowMenu(bool isSubMenu) 
    : _handle(isSubMenu ? (MenuHandle)CreatePopupMenu() : (MenuHandle)CreateMenu()), 
      _isSubMenu(isSubMenu) {
    if (this->_handle == nullptr)
      throw std::runtime_error("WindowMenu: menu creation failed");
  }
  WindowMenu::~WindowMenu() noexcept {
    if (this->_handle != nullptr)
      DestroyMenu((HMENU)this->_handle);
  }

  // add menu command item
  bool WindowMenu::insertItem(uint32_t id, const wchar_t* label, WindowMenu::ItemType type, bool isEnabled) noexcept {
    UINT flags = isEnabled ? MF_ENABLED : MF_GRAYED;
    switch (type) {
      case WindowMenu::ItemType::radioOn:
      case WindowMenu::ItemType::radioOff: {
        MENUITEMINFOW menuItemInfo;
        ZeroMemory(&menuItemInfo, sizeof(menuItemInfo));
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = (MIIM_ID|MIIM_FTYPE|MIIM_STATE|MIIM_STRING);
        menuItemInfo.fType = MFT_RADIOCHECK;
        menuItemInfo.fState = isEnabled ? MFS_ENABLED : MFS_GRAYED;
        menuItemInfo.fState |= (type == WindowMenu::ItemType::radioOn) ? MFS_CHECKED : MFS_UNCHECKED;
        menuItemInfo.wID = (UINT)id;
        menuItemInfo.dwTypeData = (LPWSTR)label;
        menuItemInfo.cch = (UINT)wcslen(label);
        return (InsertMenuItemW((HMENU)this->_handle, (UINT)-1, TRUE, &menuItemInfo) != FALSE);
      }
      case WindowMenu::ItemType::checkboxOn: flags |= (MF_STRING|MF_CHECKED); break;
      case WindowMenu::ItemType::checkboxOff: flags |= (MF_STRING|MF_UNCHECKED); break;
      default: flags |= MF_STRING; break;
    }
    return (AppendMenuW((HMENU)this->_handle, flags, (UINT)id, label) != FALSE);
  }
  // add sub-menu
  bool WindowMenu::insertSubMenu(WindowMenu&& item, const wchar_t* label) noexcept {
    if (item.isSubMenu() && item._handle != nullptr 
    && AppendMenuW((HMENU)this->_handle, MF_POPUP, (UINT_PTR)item._handle, label) != FALSE) {
      item._handle = nullptr;
      return true;
    }
    return false;
  }
  // insert item separator (vertical separator in sub-menus, horizontal separator in main menu)
  void WindowMenu::insertSeparator() noexcept {
    AppendMenuW((HMENU)this->_handle, isSubMenu() ? MF_SEPARATOR : MF_MENUBARBREAK, 0, nullptr);
  }

  // ---

  // change state of a checkbox/radio-button
  void WindowMenu::changeCheckItemState(MenuHandle menu, uint32_t id, bool isChecked, bool isEnabled) noexcept {
    MENUITEMINFOW menuItemInfo;
    ZeroMemory(&menuItemInfo, sizeof(menuItemInfo));
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_STATE;
    menuItemInfo.fState = isEnabled ? MFS_ENABLED : MFS_GRAYED;
    menuItemInfo.fState |= isChecked ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfoW((HMENU)menu, (UINT)id, FALSE, &menuItemInfo);
  }
  // change checkboxes/radio-buttons selection
  void WindowMenu::changeCheckItemState(MenuHandle menu, uint32_t checkId, uint32_t uncheckId, bool isUncheckedEnabled) noexcept {
    MENUITEMINFOW menuItemInfo;
    ZeroMemory(&menuItemInfo, sizeof(menuItemInfo));
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_STATE;
    menuItemInfo.fState = MFS_CHECKED;
    SetMenuItemInfoW((HMENU)menu, (UINT)checkId, FALSE, &menuItemInfo);
    menuItemInfo.fMask = MIIM_STATE;
    menuItemInfo.fState = isUncheckedEnabled ? MFS_UNCHECKED|MFS_ENABLED : MFS_UNCHECKED|MFS_GRAYED;
    SetMenuItemInfoW((HMENU)menu, (UINT)uncheckId, FALSE, &menuItemInfo);
  }
  // change state of any other item type
  void WindowMenu::changeItemState(MenuHandle menu, uint32_t id, bool isEnabled) noexcept {
    MENUITEMINFOW menuItemInfo;
    ZeroMemory(&menuItemInfo, sizeof(menuItemInfo));
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = (MIIM_ID|MIIM_FTYPE|MIIM_STATE|MIIM_STRING);
    menuItemInfo.fType = MFT_RADIOCHECK;
    menuItemInfo.fState = isEnabled ? MFS_ENABLED : MFS_GRAYED;
    SetMenuItemInfoW((HMENU)menu, (UINT)id, FALSE, &menuItemInfo);
  }

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
