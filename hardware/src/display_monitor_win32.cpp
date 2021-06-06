/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# ifndef __MINGW32__
#   pragma warning(push)
#   pragma warning(disable : 26812)
# endif
# include <cstdint>
# include <string>
# include <stdexcept>
# include <vector>
# include "hardware/_private/_libraries_win32.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;


// -- monitor attributes - id/area/description/primary -- ----------------------

  namespace attributes {
    // verify if a monitor is the primary device
    static inline bool _isPrimaryDevice(const MONITORINFOEXW& info) {
      return ((info.dwFlags & MONITORINFOF_PRIMARY) != 0);
    }
    
    // read brand/description string of monitor
    static inline std::wstring _readDeviceDescription(const DisplayMonitor::DeviceId& id) {
      DISPLAY_DEVICEW device;
      ZeroMemory(&device, sizeof(device));
      device.cb = sizeof(device);
      return (!id.empty() && EnumDisplayDevicesW(id.c_str(), 0, &device, 0) != FALSE)
            ? device.DeviceString 
            : L"Generic PnP Monitor";
    }
    // read brand/description string of adapter
    static inline std::wstring _readAdapterName(const DisplayMonitor::DeviceId& id) {
      DISPLAY_DEVICEW device;
      ZeroMemory(&device, sizeof(device));
      device.cb = sizeof(device);
      
      if (!id.empty()) {
        BOOL result = TRUE;
        for (uint32_t index = 0; result; ++index) {
          result = (EnumDisplayDevicesW(nullptr, index, &device, 0) != FALSE);
          if (result != FALSE && id == device.DeviceName)
            return device.DeviceString;
        }
      }
      else if (EnumDisplayDevicesW(nullptr, 0, &device, 0) != FALSE)
        return device.DeviceString;
      return L"";
    }
    
    // read screen area of a monitor (screen position/size + work area)
    static inline void _readScreenArea(const MONITORINFOEXW& info, DisplayArea& outScreenArea, DisplayArea& outWorkArea) {
      outScreenArea.x = info.rcMonitor.left;
      outScreenArea.y = info.rcMonitor.top;
      outScreenArea.width  = static_cast<uint32_t>(info.rcMonitor.right - info.rcMonitor.left);
      outScreenArea.height = static_cast<uint32_t>(info.rcMonitor.bottom - info.rcMonitor.top);
      outWorkArea.x = info.rcWork.left;
      outWorkArea.y = info.rcWork.top;
      outWorkArea.width  = static_cast<uint32_t>(info.rcWork.right - info.rcWork.left);
      outWorkArea.height = static_cast<uint32_t>(info.rcWork.bottom - info.rcWork.top);
    }
    // read primary screen area with default metrics (fallback if _readScreenArea can't be used)
    static void _readDefaultPrimaryScreenArea(DisplayArea& out) {
      out.x = out.y = 0;
      
      DEVMODEW deviceInfo;
      ZeroMemory(&deviceInfo, sizeof(deviceInfo));
      deviceInfo.dmSize = sizeof(deviceInfo);
      if (EnumDisplaySettingsExW(nullptr, ENUM_CURRENT_SETTINGS, &deviceInfo, 0) != FALSE && deviceInfo.dmPelsWidth > 0u && deviceInfo.dmPelsHeight > 0u) {
        out.width  = static_cast<uint32_t>(deviceInfo.dmPelsWidth);
        out.height = static_cast<uint32_t>(deviceInfo.dmPelsHeight);
      }
      else {
        HDC screenDC = GetDC(nullptr);
        int width = GetDeviceCaps(screenDC, HORZRES);
        int height = GetDeviceCaps(screenDC, VERTRES);
        ReleaseDC(nullptr, screenDC);
        if (width <= 0 || height <= 0) {
          width = GetSystemMetrics(SM_CXSCREEN);
          height = GetSystemMetrics(SM_CYSCREEN);
        }
        
        out.width  = (width >= 0) ?  static_cast<uint32_t>(width)  : 0; // 0 == no display device connected
        out.height = (height >= 0) ? static_cast<uint32_t>(height) : 0;
      }
    }
    // read primary work area size/position (fallback if _readScreenArea can't be used)
    static bool _readDefaultPrimaryWorkArea(DisplayArea& out) {
      RECT workArea;
      if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0) != FALSE && workArea.right > workArea.left) {
        out.x = workArea.left;
        out.y = workArea.top;
        out.width  = static_cast<uint32_t>(workArea.right - workArea.left);
        out.height = static_cast<uint32_t>(workArea.bottom - workArea.top);
        return true;
      }
      return false;
    }
    
    // ---
    
    // read all attributes of a monitor handle
    static bool read(HMONITOR handle, DisplayMonitor::Attributes& outAttr) {
      MONITORINFOEXW info;
      ZeroMemory(&info, sizeof(info));
      info.cbSize = sizeof(info);

      if (GetMonitorInfoW(handle, (MONITORINFO*)&info) != FALSE && info.rcMonitor.right > info.rcMonitor.left) {
        outAttr.id = info.szDevice;
        outAttr.isPrimary = ::attributes::_isPrimaryDevice(info);
        outAttr.description = ::attributes::_readDeviceDescription(outAttr.id);
        ::attributes::_readScreenArea(info, outAttr.screenArea, outAttr.workArea);
        return true;
      }
      return false;
    }
    // verify if monitor handle's ID equals the argument ID + read all attributes (if outAttr not null)
    static inline bool readForId(HMONITOR handle, const DisplayMonitor::DeviceId& id, DisplayMonitor::Attributes* outAttr) {
      MONITORINFOEXW info;
      ZeroMemory(&info, sizeof(info));
      info.cbSize = sizeof(info);

      if (GetMonitorInfoW(handle, (MONITORINFO*)&info) != FALSE && wcscmp(info.szDevice, id.c_str()) == 0) {
        if (outAttr != nullptr) {
          outAttr->id = info.szDevice;
          outAttr->isPrimary = ::attributes::_isPrimaryDevice(info);
          outAttr->description = ::attributes::_readDeviceDescription(outAttr->id);
          ::attributes::_readScreenArea(info, outAttr->screenArea, outAttr->workArea);
        }
        return true;
      }
      return false;
    }
  }
  

// -- get display monitors (handle + attributes) -- ----------------------------

  namespace monitors {
    // read handle/attributes of primary/default monitor
    static DisplayMonitor::Handle getPrimary(DisplayMonitor::Attributes& outAttr) {
      HMONITOR handle = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
      if (handle == nullptr || !attributes::read(handle, outAttr)) {
        outAttr.id.clear();
        outAttr.description = attributes::_readDeviceDescription(outAttr.id);
        attributes::_readDefaultPrimaryScreenArea(outAttr.screenArea);
        if (!attributes::_readDefaultPrimaryWorkArea(outAttr.workArea))
          outAttr.workArea = outAttr.screenArea;
      }
      outAttr.isPrimary = true;
      return (DisplayMonitor::Handle)handle;
    }
    
    // ---

    struct __DisplayMonitorHandleSearch final {
      const DisplayMonitor::DeviceId* id;
      DisplayMonitor::Handle handle;
      DisplayMonitor::Attributes* attributes;
    };
    static BOOL CALLBACK __getById_callback(HMONITOR handle, HDC, RECT*, LPARAM data) {
      __DisplayMonitorHandleSearch* query = (__DisplayMonitorHandleSearch*)data;
      if (handle != nullptr && query != nullptr && query->id != nullptr) {
        if (attributes::readForId(handle, *(query->id), query->attributes))
          query->handle = (DisplayMonitor::Handle)handle;
      }
      return TRUE;
    }
    // get handle/attributes of any monitor by ID
    static inline DisplayMonitor::Handle getById(const DisplayMonitor::DeviceId& id, DisplayMonitor::Attributes* outAttr) {
      __DisplayMonitorHandleSearch query{ &id, nullptr, outAttr };
      if (EnumDisplayMonitors(nullptr, nullptr, __getById_callback, (LPARAM)&query) != FALSE)
        return query.handle;
      return nullptr;
    }
    
    // ---

    static BOOL CALLBACK __list_callback(HMONITOR handle, HDC, RECT*, LPARAM data) {
      std::vector<DisplayMonitor::Handle>* handleList = (std::vector<DisplayMonitor::Handle>*)data;
      if (handle != nullptr && handleList != nullptr)
        handleList->emplace_back((DisplayMonitor::Handle)handle);
      return TRUE;
    }
    // list handles of all active monitors
    static inline bool listHandles(std::vector<DisplayMonitor::Handle>& out) {
      return (EnumDisplayMonitors(nullptr, nullptr, __list_callback, (LPARAM)&out) != FALSE);
    }
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    this->_handle = monitors::getPrimary(this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(DisplayMonitor::Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (this->_handle == nullptr || !attributes::read((HMONITOR)this->_handle, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = monitors::getById(id, &(this->_attributes));
    if (this->_handle == nullptr) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(bool usePrimaryAsDefault, uint32_t index) {
    std::vector<DisplayMonitor::Handle> handles;
    if (monitors::listHandles(handles) && index < handles.size())
      this->_handle = handles[index];
    if (this->_handle == nullptr || !attributes::read((HMONITOR)this->_handle, this->_attributes)) {
      if (usePrimaryAsDefault)
        this->_handle = monitors::getPrimary(this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitorList;

    std::vector<DisplayMonitor::Handle> handles;
    if (monitors::listHandles(handles)) {
      for (auto it : handles) {
        try {
          monitorList.emplace_back(it, false);
        }
        catch (const std::bad_alloc&) { throw; }
        catch (...) {} // ignore invalid_argument
      }
    }

    if (monitorList.empty()) { // primary monitor as default
      monitorList.emplace_back();
      if (monitorList[0].attributes().screenArea.width == 0) // no display monitor
        monitorList.clear();
    }
    return monitorList;
  }


// -- accessors -- -------------------------------------------------------------

  DisplayMonitor::String DisplayMonitor::adapterName() const {
    return attributes::_readAdapterName(this->_attributes.id);
  }


// -- display modes -- ---------------------------------------------------------

  namespace monitors {
    // convert frequency flag to refresh rate (mHz)
    static inline uint32_t _getRefreshRate(DWORD displayFrequency) {
      uint32_t refreshRate;
      switch (displayFrequency) {
        case 59u: refreshRate = 59940u; break;
        case 29u: refreshRate = 29970u; break;
        case 27u: refreshRate = 27500u; break;
        case 23u: refreshRate = 23976u; break;
        default:  refreshRate = displayFrequency * 1000u; break;
      }
      return refreshRate;
    }
    
    // read display resolution/depth/rate of a monitor
    static inline bool getDisplayMode(const DisplayMonitor::DeviceId& id, DisplayMode& out) noexcept {
      DEVMODEW screenMode;
      ZeroMemory(&screenMode, sizeof(screenMode));
      screenMode.dmSize = sizeof(screenMode);
      if (EnumDisplaySettingsExW(id.c_str(), ENUM_CURRENT_SETTINGS, &screenMode, 0) != FALSE && screenMode.dmPelsWidth != 0) {
        out.width = screenMode.dmPelsWidth;
        out.height = screenMode.dmPelsHeight;    
        out.bitDepth = screenMode.dmBitsPerPel;
        out.refreshRate = _getRefreshRate(screenMode.dmDisplayFrequency);
        return true;
      }
      return false;
    }
    
    // set display resolution/depth/rate of a monitor
    static inline bool setDisplayMode(const DisplayMonitor::DeviceId& id, const DisplayMode& mode) noexcept {
      DEVMODEW screenMode;
      ZeroMemory(&screenMode, sizeof(screenMode));
      screenMode.dmSize = sizeof(screenMode);   

      screenMode.dmPelsWidth = mode.width;
      screenMode.dmPelsHeight = mode.height;    
      screenMode.dmBitsPerPel = mode.bitDepth;
      screenMode.dmDisplayFrequency = mode.refreshRate/1000u;
      screenMode.dmFields = (DM_PELSWIDTH | DM_PELSHEIGHT);
      if (mode.bitDepth != 0)
        screenMode.dmFields |= DM_BITSPERPEL;
      if (mode.refreshRate != undefinedRefreshRate())
        screenMode.dmFields |= DM_DISPLAYFREQUENCY;

      if (!id.empty())
        return (ChangeDisplaySettingsExW(id.c_str(), &screenMode, nullptr, CDS_FULLSCREEN, nullptr) == DISP_CHANGE_SUCCESSFUL);
      else // primary monitor
        return (ChangeDisplaySettingsW(&screenMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
    }
    
    // reset display resolution/depth/rate of a monitor to default values
    static inline bool setDefaultDisplayMode(const DisplayMonitor::DeviceId& id) noexcept {
      if (!id.empty())
        return (ChangeDisplaySettingsExW(id.c_str(), nullptr, nullptr, 0, nullptr) == DISP_CHANGE_SUCCESSFUL);
      else // primary monitor
        return (ChangeDisplaySettingsW(nullptr, 0) == DISP_CHANGE_SUCCESSFUL);
    }
    
    // ---
    
    // list all display modes of a monitor
    static inline std::vector<DisplayMode> listDisplayModes(const DisplayMonitor::DeviceId& id) {
      std::vector<DisplayMode> modes;
      BOOL result = TRUE;
      for (DWORD index = 0; result != FALSE; ++index) {
        DEVMODEW info;
        ZeroMemory(&info, sizeof(info));
        info.dmSize = sizeof(info);

        result = EnumDisplaySettingsExW(id.c_str(), index, &info, 0);
        if (result != FALSE && info.dmPelsWidth != 0u && info.dmPelsHeight != 0u && (info.dmBitsPerPel >= 15u || info.dmBitsPerPel == 0)) {
          uint32_t refreshRate = _getRefreshRate(info.dmDisplayFrequency);
          
          bool isAlreadyListed = false;
          for (auto& it : modes) {
            if (it.width == info.dmPelsWidth && it.height == info.dmPelsHeight && it.bitDepth == info.dmBitsPerPel && it.refreshRate == refreshRate) {
              isAlreadyListed = true;
              break;
            }
          }
          if (!isAlreadyListed)
            modes.push_back(DisplayMode{ info.dmPelsWidth, info.dmPelsHeight, info.dmBitsPerPel, refreshRate });
        }
      }
      return modes;
    }
  }
  
  // ---
  
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!monitors::getDisplayMode(this->_attributes.id, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode) {
    return monitors::setDisplayMode(this->_attributes.id, mode);
  }
  bool DisplayMonitor::setDefaultDisplayMode() {
    return monitors::setDefaultDisplayMode(this->_attributes.id);
  }

  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    return monitors::listDisplayModes(this->_attributes.id);
  }


// -- DPI awareness -- ---------------------------------------------------------

  bool DisplayMonitor::setDpiAwareness(bool isEnabled) noexcept {
#   ifdef _P_WIN32_SetProcessDpiAwarenessContext_SUPPORTED
      return (SetProcessDpiAwarenessContext(isEnabled ? DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 : DPI_AWARENESS_CONTEXT_UNAWARE) != FALSE
           || SetProcessDpiAwareness(isEnabled ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE) == S_OK);
#   else
      LibrariesWin32& libs = LibrariesWin32::instance();
      if (libs.isAtLeastWindows10_RS2() && libs.user32.SetProcessDpiAwarenessContext_
      && libs.user32.SetProcessDpiAwarenessContext_(isEnabled ? DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 : DPI_AWARENESS_CONTEXT_UNAWARE) != FALSE)
        return true;

#     ifdef _P_WIN32_SetProcessDpiAwareness_SUPPORTED
        return (SetProcessDpiAwareness(isEnabled ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE) == S_OK);
#     else
        if (libs.isAtLeastWindows8_1_Blue() && libs.shcore.SetProcessDpiAwareness_
        && libs.shcore.SetProcessDpiAwareness_(isEnabled ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE) == S_OK)
          return true;
        return (isEnabled && SetProcessDPIAware() != FALSE);
#     endif
#   endif
  }

  // read per-window/per-monitor DPI if supported (Win10.RS2+ and valid windowHandle), or system DPI (if DPI aware process)
  void DisplayMonitor::getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, DisplayMonitor::WindowHandle windowHandle) const noexcept {
#   if !defined(_P_WIN32_GetDpiForWindow_SUPPORTED) || !defined(_P_WIN32_GetDpiForMonitor_SUPPORTED)
      LibrariesWin32& libs = LibrariesWin32::instance();
#   endif

    // per monitor DPI (better, but only if Win10 RS1+ and valid window handle)
    if (windowHandle != nullptr) {
#     ifdef _P_WIN32_GetDpiForWindow_SUPPORTED
        UINT dpi = GetDpiForWindow((HWND)windowHandle);
        if (dpi > 0)
          outDpiX = outDpiY = static_cast<uint32_t>(dpi);
#     else
        if (libs.isAtLeastWindows10_RS1() && libs.user32.GetDpiForWindow_) {
          UINT dpi = libs.user32.GetDpiForWindow_((HWND)windowHandle);
          if (dpi > 0)
            outDpiX = outDpiY = static_cast<uint32_t>(dpi);
        }
#     endif
    }
    // per system DPI (fallback, if Win8.1+)
    UINT dpiX, dpiY;
#   ifdef _P_WIN32_GetDpiForMonitor_SUPPORTED
      if (GetDpiForMonitor((HMONITOR)(this->_handle), MDT_EFFECTIVE_DPI, &dpiX, &dpiY) == S_OK) {
        outDpiX = static_cast<uint32_t>(dpiX);
        outDpiY = static_cast<uint32_t>(dpiY);
      }
#   else
      if (libs.isAtLeastWindows8_1_Blue() && libs.shcore.GetDpiForMonitor_) {
        if (libs.shcore.GetDpiForMonitor_((HMONITOR)(this->_handle), MDT_EFFECTIVE_DPI, &dpiX, &dpiY) == S_OK) {
          outDpiX = static_cast<uint32_t>(dpiX);
          outDpiY = static_cast<uint32_t>(dpiY);
        }
      }
#   endif

    // per system DPI (legacy)
    HDC hdc = GetDC(nullptr);
    int pxPerLogicInchX = GetDeviceCaps(hdc, LOGPIXELSX);
    int pxPerLogicInchY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(nullptr, hdc);
    outDpiX = (pxPerLogicInchX > 0) ? pxPerLogicInchX : USER_DEFAULT_SCREEN_DPI;
    outDpiY = (pxPerLogicInchY > 0) ? pxPerLogicInchY : USER_DEFAULT_SCREEN_DPI;
  }
  // read window/monitor scaling factor if supported (Win10.RS2+ and valid windowHandle), or system scaling factor (if DPI aware process)
  void DisplayMonitor::getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle windowHandle) const noexcept {
    uint32_t dpiX, dpiY;
    getMonitorDpi(dpiX, dpiY, windowHandle);
    outScaleX = static_cast<float>(dpiX) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
    outScaleY = static_cast<float>(dpiY) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
  }


// -- metrics -- ---------------------------------------------------------------

  // read system metrics with DPI, if supported
# ifdef _P_WIN32_GetSystemMetricsForDpi_SUPPORTED
    static inline int32_t __getSystemMetrics(int index, uint32_t dpi, int32_t defaultValue) noexcept {
      int value = GetSystemMetricsForDpi(index, dpi);
      if (value > 0)
        return static_cast<int32_t>(value);
      value = GetSystemMetrics(index);
      return (value > 0) ? static_cast<int32_t>(value) : defaultValue;
    }
# else
    static inline int32_t __getSystemMetrics_impl(int index, uint32_t dpi, int32_t defaultValue, LibrariesWin32& libs) noexcept {
      if (libs.isAtLeastWindows10_RS1() && libs.user32.GetSystemMetricsForDpi_) {
        int value = libs.user32.GetSystemMetricsForDpi_(index, dpi);
        if (value > 0)
          return static_cast<int32_t>(value);
      }
      int value = GetSystemMetrics(index);
      return (value > 0) ? static_cast<int32_t>(value) : defaultValue;
    }
#   define __getSystemMetrics(index,dpi,defaultValue) __getSystemMetrics_impl(index,dpi,defaultValue,libs)
# endif

  // manually calculate window area from client area
  static DisplayArea _calculateWindowArea(const DisplayArea& clientArea, DWORD styleFlags, DWORD styleExtendedFlags, bool hasMenu, int32_t dpiX, int32_t dpiY) noexcept {
#   ifndef _P_WIN32_GetSystemMetricsForDpi_SUPPORTED
      LibrariesWin32& libs = LibrariesWin32::instance(); // used by macro __getSystemMetrics (if GetSystemMetricsForDpi not supported)
#   endif

    int32_t borderLeft = 0;
    int32_t borderRight = 0;
    int32_t borderTop = 0;
    int32_t borderBottom = 0;
    if ((styleFlags & WS_POPUP) == 0) {
      if ((styleFlags & WS_THICKFRAME) != 0) {
        borderLeft = borderRight = __getSystemMetrics(SM_CXSIZEFRAME, dpiX, 4) + 4*dpiX/USER_DEFAULT_SCREEN_DPI; // additional invisible borders of (4*scaling_factor) pixels
        borderTop = borderBottom = __getSystemMetrics(SM_CYSIZEFRAME, dpiY, 4) + 4*dpiX/USER_DEFAULT_SCREEN_DPI; // for some reason, AdjustWindowRect counts a "top" border
      }
      else {
        borderLeft = borderRight = __getSystemMetrics(SM_CXFIXEDFRAME, dpiX, 4) + 4*dpiX/USER_DEFAULT_SCREEN_DPI;
        borderTop = borderBottom = __getSystemMetrics(SM_CYFIXEDFRAME, dpiY, 4) + 4*dpiX/USER_DEFAULT_SCREEN_DPI;
      }
    }
    if ((styleFlags & WS_CAPTION) != 0)
      borderTop += __getSystemMetrics((styleExtendedFlags & WS_EX_TOOLWINDOW) ? SM_CYSMCAPTION : SM_CYCAPTION, dpiY, 33);
    if ((styleFlags & WS_VSCROLL) != 0)
      borderRight += __getSystemMetrics(SM_CXVSCROLL, dpiX, 18);
    if ((styleFlags & WS_HSCROLL) != 0)
      borderBottom += __getSystemMetrics(SM_CYHSCROLL, dpiY, 18);
    if (hasMenu)
      borderTop += 20*dpiX/USER_DEFAULT_SCREEN_DPI;
    
    DisplayArea windowArea = clientArea;
    windowArea.x -= borderLeft;
    windowArea.width += static_cast<uint32_t>(borderLeft + borderRight);
    windowArea.y -= borderTop;
    windowArea.height += static_cast<uint32_t>(borderTop + borderBottom);
    return windowArea;
  }

  // client area to window area (DPI adjusted)
  DisplayArea DisplayMonitor::convertClientAreaToWindowArea(const DisplayArea& clientArea, DisplayMonitor::WindowHandle windowHandle, bool hasMenu, 
                                                            uint32_t nativeStyleFlags, uint32_t nativeSecondaryFlags) const noexcept {
    uint32_t dpiX, dpiY;
    getMonitorDpi(dpiX, dpiY, windowHandle);

    DWORD styleFlags;
    if (windowHandle != nullptr) {
      styleFlags = GetWindowLongW((HWND)windowHandle, GWL_STYLE);
      if (styleFlags == 0)
        styleFlags = nativeStyleFlags;
    }
    else
      styleFlags = nativeStyleFlags;
    DWORD styleExtendedFlags;
    if (windowHandle != nullptr) {
      styleExtendedFlags = GetWindowLongW((HWND)windowHandle, GWL_EXSTYLE);
      if (styleExtendedFlags == 0)
        styleExtendedFlags = nativeSecondaryFlags;
    }
    else
      styleExtendedFlags = nativeSecondaryFlags;

    RECT area;
    area.left = static_cast<LONG>(clientArea.x);
    area.top  = static_cast<LONG>(clientArea.y);
    area.right = area.left + static_cast<LONG>(clientArea.width);
    area.bottom = area.top + static_cast<LONG>(clientArea.height);

#   ifdef _P_WIN32_AdjustWindowRectExForDpi_SUPPORTED
      bool isSuccess = (AdjustWindowRectExForDpi(&area, styleFlags, hasMenu ? TRUE : FALSE, styleExtendedFlags, dpiY) != FALSE);
#   else
      LibrariesWin32& libs = LibrariesWin32::instance();
      bool isSuccess = (libs.isAtLeastWindows10_RS1() && libs.user32.AdjustWindowRectExForDpi_
                     && libs.user32.AdjustWindowRectExForDpi_(&area, styleFlags, hasMenu ? TRUE : FALSE, styleExtendedFlags, dpiY) != FALSE);
#   endif
    if (isSuccess && (area.bottom - area.top > static_cast<LONG>(clientArea.height) || (styleFlags & (WS_CAPTION | WS_SIZEBOX)) == 0)) {
      return DisplayArea{ area.left, area.top, static_cast<uint32_t>(area.right - area.left), static_cast<uint32_t>(area.bottom - area.top) };
    }
    else if (AdjustWindowRectEx(&area, styleFlags, hasMenu ? TRUE : FALSE, styleExtendedFlags) != FALSE 
             && (area.bottom - area.top > static_cast<LONG>(clientArea.height) || (styleFlags & (WS_CAPTION | WS_SIZEBOX)) == 0)) {
      return DisplayArea{ area.left, area.top, static_cast<uint32_t>(area.right - area.left), static_cast<uint32_t>(area.bottom - area.top) };
    }
    else
      return _calculateWindowArea(clientArea, styleFlags, styleExtendedFlags, hasMenu, static_cast<int32_t>(dpiX), static_cast<int32_t>(dpiY));
  }

# ifndef __MINGW32__
#   pragma warning(pop)
# endif
#endif
