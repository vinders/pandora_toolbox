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
Description : WindowResource - Win32 implementation (Windows)
--------------------------------------------------------------------------------
Implementation included in window_win32.cpp
(grouped object improves compiler optimizations + reduces executable size)
*******************************************************************************/
#ifdef _WINDOWS
// includes + namespaces: in window_win32.cpp

# define __P_MAKE_SHARED_RES(handle, categ) \
         std::make_shared<WindowResource>((WindowResourceHandle) handle, categ)


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
      throw std::runtime_error("WindowMenu: creation error");
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
  
  
// -- System string converter -- -----------------------------------------------

# define __P_UTF8_BUFFER_SIZE 128
# define __P_UNKNOWN_CHAR 0xFFFDu

  static __forceinline uint32_t __decodeUtf16(char highBitsWord1, char lowBitsWord1, 
                                              char highBitsWord2, char lowBitsWord2, size_t& outWordLength) noexcept {
    if ((highBitsWord1 & 0xFC) == 0xD8) {
      if ((highBitsWord2 & 0xFC) != 0xDC) {
        outWordLength = size_t{1u};
        return __P_UNKNOWN_CHAR; // report invalid character
      }
      outWordLength = size_t{2u};
      return ( (((uint32_t)highBitsWord1 & 0x3u) << 18) | (((uint32_t)lowBitsWord1 & 0xFF) << 10) 
             | (((uint32_t)highBitsWord2 & 0x3u) << 8)  | ((uint32_t)lowBitsWord2 & 0xFF) ) + 0x10000u;
    }
    outWordLength = size_t{1u};
    return ((((uint32_t)highBitsWord1 & 0xFF) << 8) | ((uint32_t)lowBitsWord1 & 0xFF)); // masks: to avoid duplication of most-significant-bit
  }
  static __forceinline size_t __encodeUtf8(uint32_t charCode, char* outBuffer) noexcept {
    if (charCode > (uint32_t)0x7Fu) {
      if (charCode > (uint32_t)0x7FFu) {
        if (charCode > (uint32_t)0xFFFFu) {
          outBuffer[0] = static_cast<char>(0xF0u | ((charCode >> 18) & 0x07)); // truncate values out of range
          outBuffer[1] = static_cast<char>(0x80u | ((charCode >> 12) & 0x3F));
          outBuffer[2] = static_cast<char>(0x80u | ((charCode >> 6) & 0x3F));
          outBuffer[3] = static_cast<char>(0x80u | (charCode & 0x3F));
          return size_t{4u};
        }
        outBuffer[0] = static_cast<char>(0xE0u | (charCode >> 12));
        outBuffer[1] = static_cast<char>(0x80u | ((charCode >> 6) & 0x3F));
        outBuffer[2] = static_cast<char>(0x80u | (charCode & 0x3F));
        return size_t{3u};
      }
      outBuffer[0] = static_cast<char>(0xC0u | (charCode >> 6));
      outBuffer[1] = static_cast<char>(0x80u | (charCode & 0x3F));
      return size_t{2u};
    }
    *outBuffer = (char)charCode;
    return size_t{1u};
  }

  pandora::memory::LightString WindowResource::systemStringToUtf8(const wchar_t* data, size_t length) {
    pandora::memory::LightString result;
    char buffer[__P_UTF8_BUFFER_SIZE];
    char* bufferIt = &buffer[0];
    size_t bufferLength = 0;

    uint32_t charCode = 0;
    size_t sourceWordLength = 0, outputCharLength = 0;
    char nextSourceWordHighBits = 0, nextSourceWordLowBits = 0;
    while (length) {
      // decode UTF-16
      if (length >= size_t{2u}) {
        nextSourceWordHighBits = static_cast<char>(((int)data[1] >> 8) & 0xFF);
        nextSourceWordLowBits = static_cast<char>((int)data[1] & 0xFF);
      }
      else
        nextSourceWordHighBits = nextSourceWordLowBits = (char)0;

      charCode = __decodeUtf16(static_cast<char>(((int)data[0] >> 8) & 0xFF), static_cast<char>((int)data[0] & 0xFF),
                               nextSourceWordHighBits, nextSourceWordLowBits, sourceWordLength);
      data += sourceWordLength;
      length -= sourceWordLength;

      // encode UTF-8
      outputCharLength = __encodeUtf8(charCode, bufferIt);
      bufferLength += outputCharLength;
      bufferIt += outputCharLength;

      if (bufferLength + 5u >= __P_UTF8_BUFFER_SIZE) { // limit number of concats as much as possible (to avoid reallocs)
        result.append(&buffer[0], bufferLength);
        bufferLength = 0;
        bufferIt = &buffer[0];
      }
    }
    if (bufferLength > 0)
      result.append(&buffer[0], bufferLength);
    return result;
  }

# undef __P_MAKE_SHARED_RES
#endif
