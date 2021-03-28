/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Display monitor - Win32 implementation (Windows)
*******************************************************************************/
#ifdef _WINDOWS
# pragma warning(push)
# pragma warning(disable : 26812)
# include <system/api/windows_version.h>
# include <cstdint>
# include <string>
# include <stdexcept>
# include <vector>
# include "hardware/_private/_libraries_win32.h"
# include "hardware/display_monitor.h"

  using namespace pandora::hardware;


// -- monitor handle & description/attributes -- -------------------------------

  // read brand/description string of monitor and associated adapter
  static inline void __readDeviceDescription(const DisplayMonitor::DeviceId& id, std::wstring& outDescription, std::wstring& outAdapter) {
    DISPLAY_DEVICEW device;
    ZeroMemory(&device, sizeof(device));
    device.cb = sizeof(device);

    if (!id.empty() && EnumDisplayDevicesW(id.c_str(), 0, &device, 0) != FALSE)
      outDescription = device.DeviceString;
    else
      outDescription = L"Generic PnP Monitor";

    if (!id.empty()) {
      BOOL result = TRUE;
      for (uint32_t index = 0; result; ++index) {
        result = (EnumDisplayDevicesW(nullptr, index, &device, 0) != FALSE);
        if (result != FALSE && id == device.DeviceName)
          outAdapter = device.DeviceString;
      }
      if (outAdapter.empty() && EnumDisplayDevicesW(nullptr, 0, &device, 0) != FALSE)
        outAdapter = device.DeviceString;
    }
    else if (EnumDisplayDevicesW(nullptr, 0, &device, 0) != FALSE)
      outAdapter = device.DeviceString;
  }

  // convert and fill attributes of a monitor (id, position/size, primary)
  static inline void __fillMonitorAttributes(MONITORINFOEXW& info, DisplayMonitor::Attributes& outAttr) {
    outAttr.id = info.szDevice;
    outAttr.screenArea.x = info.rcMonitor.left;
    outAttr.screenArea.y = info.rcMonitor.top;
    outAttr.screenArea.width  = static_cast<uint32_t>(info.rcMonitor.right - info.rcMonitor.left);
    outAttr.screenArea.height = static_cast<uint32_t>(info.rcMonitor.bottom - info.rcMonitor.top);
    outAttr.workArea.x = info.rcWork.left;
    outAttr.workArea.y = info.rcWork.top;
    outAttr.workArea.width  = static_cast<uint32_t>(info.rcWork.right - info.rcWork.left);
    outAttr.workArea.height = static_cast<uint32_t>(info.rcWork.bottom - info.rcWork.top);
    outAttr.isPrimary = ((info.dwFlags & MONITORINFOF_PRIMARY) != 0);
  }
  // read attributes of a monitor handle
  static bool _readDisplayMonitorAttributes(DisplayMonitor::Handle monitorHandle, DisplayMonitor::Attributes& outAttr) {
    MONITORINFOEXW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);

    if (GetMonitorInfoW((HMONITOR)monitorHandle, (MONITORINFO*)&info) != FALSE && info.rcMonitor.right > info.rcMonitor.left) {
      __fillMonitorAttributes(info, outAttr);
      __readDeviceDescription(outAttr.id, outAttr.description, outAttr.adapter);
      return true;
    }
    return false;
  }
  
  // ---
  
  // read primary monitor size/position (fallback if no handle or if _readDisplayMonitorAttributes failed)
  static bool __readPrimaryDisplayMonitorScreenArea_fallback(DisplayArea& out) {
    DEVMODEW deviceInfo;
    ZeroMemory(&deviceInfo, sizeof(deviceInfo));
    deviceInfo.dmSize = sizeof(deviceInfo);

    out.x = 0;
    out.y = 0;
    if (EnumDisplaySettingsExW(nullptr, ENUM_CURRENT_SETTINGS, &deviceInfo, 0) != FALSE && deviceInfo.dmPelsWidth > 0u && deviceInfo.dmPelsHeight > 0u) {
      out.width  = static_cast<uint32_t>(deviceInfo.dmPelsWidth);
      out.height = static_cast<uint32_t>(deviceInfo.dmPelsHeight);
    }
    else {
      HDC screenDC = GetDC(nullptr);
      int x = GetDeviceCaps(screenDC, HORZRES);
      int y = GetDeviceCaps(screenDC, VERTRES);
      ReleaseDC(nullptr, screenDC);
      if (x > 0 && y > 0) {
        out.width = static_cast<uint32_t>(x);
        out.height = static_cast<uint32_t>(y);
      }
      else {
        x = GetSystemMetrics(SM_CXSCREEN);
        y = GetSystemMetrics(SM_CYSCREEN);
        if (x > 0 && y > 0) {
          out.width = static_cast<uint32_t>(x);
          out.height = static_cast<uint32_t>(y);
        }
        else
          return false;
      }
    }
    return true;
  }
  // read primary monitor work area size/position (fallback if no handle or if _readDisplayMonitorAttributes failed)
  static bool __readPrimaryDisplayMonitorWorkArea_fallback(DisplayArea& out) {
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
  // read handle + attributes of primary/default monitor
  static inline void _readPrimaryDisplayMonitorInfo(DisplayMonitor::Handle& outHandle, DisplayMonitor::Attributes& outAttr) {
    outHandle = (DisplayMonitor::Handle)MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
    if (outHandle == nullptr || !_readDisplayMonitorAttributes(outHandle, outAttr)) {
      outAttr.isPrimary = true;
      outAttr.id.clear();
      __readDeviceDescription(outAttr.id, outAttr.description, outAttr.adapter);
      if (!__readPrimaryDisplayMonitorScreenArea_fallback(outAttr.screenArea)) {
        outAttr.screenArea.width = 0u; // no display device connected
        outAttr.screenArea.height = 0u;
      }
      if (!__readPrimaryDisplayMonitorWorkArea_fallback(outAttr.workArea))
        outAttr.workArea = outAttr.screenArea;
    }
  }
  
  // ---

  struct __DisplayMonitorHandleSearch final {
    const DisplayMonitor::DeviceId* id;
    DisplayMonitor::Handle handle;
    DisplayMonitor::Attributes* attributes;
  };
  static BOOL CALLBACK __getDisplayMonitorByIdCallback(HMONITOR handle, HDC, RECT*, LPARAM data) {
    MONITORINFOEXW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);

    if (handle != nullptr && GetMonitorInfoW(handle, (MONITORINFO*)&info) != FALSE) {
      __DisplayMonitorHandleSearch* query = (__DisplayMonitorHandleSearch*)data;
      if (query != nullptr && query->id != nullptr && wcscmp(info.szDevice, query->id->c_str()) == 0) {
        query->handle = (DisplayMonitor::Handle)handle;
        if (query->attributes != nullptr)
          __fillMonitorAttributes(info, *(query->attributes));
      }
    }
    return TRUE;
  }
  // get handle/attributes of any monitor by ID
  static inline DisplayMonitor::Handle _getDisplayMonitorById(const DisplayMonitor::DeviceId& id, DisplayMonitor::Attributes* outAttr) {
    __DisplayMonitorHandleSearch query{ &id, nullptr, outAttr };
    if (EnumDisplayMonitors(nullptr, nullptr, __getDisplayMonitorByIdCallback, (LPARAM)&query) != FALSE)
      return query.handle;
    return nullptr;
  }

  static BOOL CALLBACK __listDisplayMonitorsCallback(HMONITOR handle, HDC, RECT*, LPARAM data) {
    std::vector<DisplayMonitor::Handle>* handleList = (std::vector<DisplayMonitor::Handle>*)data;
    if (handle != nullptr && handleList != nullptr)
      handleList->emplace_back((DisplayMonitor::Handle)handle);
    return TRUE;
  }
  // list handles of all active monitors
  static inline bool _listDisplayMonitors(std::vector<DisplayMonitor::Handle>& out) {
    return (EnumDisplayMonitors(nullptr, nullptr, __listDisplayMonitorsCallback, (LPARAM)&out) != FALSE);
  }


// -- contructors/list -- ------------------------------------------------------

  DisplayMonitor::DisplayMonitor() {
    _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
  }
  DisplayMonitor::DisplayMonitor(Handle monitorHandle, bool usePrimaryAsDefault)
    : _handle(monitorHandle) {
    if (this->_handle == nullptr || !_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor handle is invalid or can't be used.");
    }
  }
  DisplayMonitor::DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault) {
    this->_handle = _getDisplayMonitorById(id, &(this->_attributes));
    if (this->_handle == nullptr) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor ID was not found on system.");
    }
  }
  DisplayMonitor::DisplayMonitor(uint32_t index, bool usePrimaryAsDefault) {
    std::vector<DisplayMonitor::Handle> handles;
    if (_listDisplayMonitors(handles) && index < handles.size())
      this->_handle = handles[index];
    if (this->_handle == nullptr || !_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
      if (usePrimaryAsDefault)
        _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
      else
        throw std::invalid_argument("DisplayMonitor: monitor index was not found on system.");
    }
  }

  std::vector<DisplayMonitor> DisplayMonitor::listAvailableMonitors() {
    std::vector<DisplayMonitor> monitors;

    std::vector<DisplayMonitor::Handle> handles;
    if (_listDisplayMonitors(handles)) {
      for (auto it : handles) {
        try {
          monitors.emplace_back(it, false);
        }
        catch (...) {}
      }
    }

    if (monitors.empty()) { // primary monitor as default
      monitors.emplace_back();
      if (monitors[0].attributes().screenArea.width == 0) // no display monitor
        monitors.clear();
    }
    return monitors;
  }


// -- display modes -- ---------------------------------------------------------

  // read display resolution/depth/rate of a monitor
  static inline bool _getMonitorDisplayMode(const DisplayMonitor::DeviceId& id, DisplayMode& out) noexcept {
    DEVMODEW screenMode;
    ZeroMemory(&screenMode, sizeof(screenMode));
    screenMode.dmSize = sizeof(screenMode);
    if (EnumDisplaySettingsExW(id.c_str(), ENUM_CURRENT_SETTINGS, &screenMode, 0) != FALSE && screenMode.dmPelsWidth != 0) {
      out.width = screenMode.dmPelsWidth;
      out.height = screenMode.dmPelsHeight;    
      out.bitDepth = screenMode.dmBitsPerPel;
      out.refreshRate = screenMode.dmDisplayFrequency;
      return true;
    }
    return false;
  }
  DisplayMode DisplayMonitor::getDisplayMode() const noexcept {
    DisplayMode mode;
    if (!_getMonitorDisplayMode(this->_attributes.id, mode)) {
      mode.width = this->_attributes.screenArea.width;
      mode.height = this->_attributes.screenArea.height;
      mode.bitDepth = 32;
      mode.refreshRate = undefinedRefreshRate();
    }
    return mode;
  }
  
  // set display resolution/depth/rate of a monitor
  static inline bool _setMonitorDisplayMode(const DisplayMonitor::DeviceId& id, const DisplayMode& mode) noexcept {
    DEVMODEW screenMode;
    ZeroMemory(&screenMode, sizeof(screenMode));
    screenMode.dmSize = sizeof(screenMode);   

    screenMode.dmPelsWidth = mode.width;
    screenMode.dmPelsHeight = mode.height;    
    screenMode.dmBitsPerPel = mode.bitDepth;
    screenMode.dmDisplayFrequency = mode.refreshRate;
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
  bool DisplayMonitor::setDisplayMode(const DisplayMode& mode, bool refreshAttributes) {
    if (_setMonitorDisplayMode(this->_attributes.id, mode)) {
      if (refreshAttributes) {
        if (this->_attributes.isPrimary) {
          _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
        }
        else if (!_readDisplayMonitorAttributes(this->_handle, this->_attributes)) {
          this->_attributes.screenArea.width = mode.width;
          this->_attributes.screenArea.height = mode.height;
        }
      }
      return true;
    }
    return false;
  }
  
  // reset display resolution/depth/rate of a monitor to default values
  static inline bool _setDefaultMonitorDisplayMode(const DisplayMonitor::DeviceId& id) noexcept {
    if (!id.empty())
      return (ChangeDisplaySettingsExW(id.c_str(), nullptr, nullptr, 0, nullptr) == DISP_CHANGE_SUCCESSFUL);
    else // primary monitor
      return (ChangeDisplaySettingsW(nullptr, 0) == DISP_CHANGE_SUCCESSFUL);
  }
  bool DisplayMonitor::setDefaultDisplayMode(bool refreshAttributes) {
    if (_setDefaultMonitorDisplayMode(this->_attributes.id)) {
      if (refreshAttributes) {
        if (this->_attributes.isPrimary) {
          _readPrimaryDisplayMonitorInfo(this->_handle, this->_attributes);
        }
        else
          _readDisplayMonitorAttributes(this->_handle, this->_attributes);
      }
      return true;
    }
    return false;
  }

  // list all display modes of a monitor
  std::vector<DisplayMode> DisplayMonitor::listAvailableDisplayModes() const {
    std::vector<DisplayMode> modes;
    BOOL result = TRUE;
    for (DWORD index = 0; result != FALSE; ++index) {
      DEVMODEW info;
      ZeroMemory(&info, sizeof(info));
      info.dmSize = sizeof(info);

      result = EnumDisplaySettingsExW(this->_attributes.id.c_str(), index, &info, 0);
      if (result != FALSE && info.dmPelsWidth != 0u && info.dmPelsHeight != 0u && (info.dmBitsPerPel >= 15u || info.dmBitsPerPel == 0))
        modes.push_back(DisplayMode{ info.dmPelsWidth, info.dmPelsHeight, info.dmBitsPerPel, info.dmDisplayFrequency });
    }

    if (modes.empty())
      modes.emplace_back(getDisplayMode());
    return modes;
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

# pragma warning(pop)
#endif
