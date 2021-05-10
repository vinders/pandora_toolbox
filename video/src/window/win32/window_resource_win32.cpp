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

  // native menu container
  std::shared_ptr<WindowResource> WindowResource::buildMenu(MenuHandle handle) {
    return (handle != nullptr) ? __P_MAKE_SHARED_RES(handle, WindowResource::Category::menu) : nullptr;
  }

#endif
