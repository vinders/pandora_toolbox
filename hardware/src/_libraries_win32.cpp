/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifdef _WINDOWS
# pragma warning(push)
# pragma warning(disable : 26812)
# include "hardware/_private/_libraries_win32.h"

# if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_WIN10_RS2) || defined(__MINGW32__)
    pandora::hardware::LibrariesWin32 pandora::hardware::LibrariesWin32::_libs{};

    // ntdll.dll functions
    typedef LONG (WINAPI * __win32_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,ULONG,ULONGLONG);

    using namespace pandora::hardware;


    // -- utilities --

    // verify Windows version on running system
    static inline bool _isWindowsVersionGreaterOrEqual(__win32_RtlVerifyVersionInfo verifyCommand, DWORD major, DWORD minor, DWORD build) noexcept {
      if (verifyCommand != nullptr) {
        OSVERSIONINFOEXW versionInfo = { sizeof(versionInfo), major, minor, build };
        DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
        ULONGLONG check = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
        check = VerSetConditionMask(check, VER_MINORVERSION, VER_GREATER_EQUAL);
        check = VerSetConditionMask(check, VER_BUILDNUMBER, VER_GREATER_EQUAL);
        return (verifyCommand(&versionInfo, mask, check) == 0);
      }
      else {
#       if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
          return (major < 10u || (major == 10u && minor == 0u && (build == 0 || build <= __P_WIN_10_RS2_BUILD)) );
#       elif defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WIN10_RS1)
          return (major < 10u || (major == 10u && minor == 0u && (build == 0 || build <= __P_WIN_10_RS1_BUILD)) );
#       elif defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WINBLUE)
          return (major < HIBYTE(_WIN32_WINNT_WINBLUE) 
             || (major == HIBYTE(_WIN32_WINNT_WINBLUE) && minor <= LOBYTE(_WIN32_WINNT_WINBLUE) && (build == 0 || build <= __P_WIN_8_1_BLUE_BUILD)) );
#       elif defined(WINVER) && (WINVER >= _WIN32_WINNT_WIN7)
          return (major < HIBYTE(_WIN32_WINNT_WIN7)
             || (major == HIBYTE(_WIN32_WINNT_WIN7) && minor <= LOBYTE(_WIN32_WINNT_WIN7) && (build == 0 || build <= __P_WIN_7_BUILD)) );
#       else
          return (major < HIBYTE(_WIN32_WINNT_VISTA)
             || (major == HIBYTE(_WIN32_WINNT_VISTA) && minor == LOBYTE(_WIN32_WINNT_VISTA) && (build == 0 || build <= __P_WIN_VISTA_SP1_BUILD)) );
#       endif
      }
    }

    // -- init --

    void LibrariesWin32::init() noexcept {
      if (this->_isInit)
        return;
    
      // load system libraries
      this->user32.instance = LoadLibraryA("user32.dll");
      if (this->user32.instance != nullptr) {
        this->user32.SetProcessDpiAwarenessContext_ = (__win32_SetProcessDpiAwarenessContext)GetProcAddress(this->user32.instance, "SetProcessDpiAwarenessContext");
        this->user32.EnableNonClientDpiScaling_ = (__win32_EnableNonClientDpiScaling)GetProcAddress(this->user32.instance, "EnableNonClientDpiScaling");
        this->user32.GetDpiForWindow_ = (__win32_GetDpiForWindow)GetProcAddress(this->user32.instance, "GetDpiForWindow");
        this->user32.GetSystemMetricsForDpi_ = (__win32_GetSystemMetricsForDpi)GetProcAddress(this->user32.instance, "GetSystemMetricsForDpi");
        this->user32.AdjustWindowRectExForDpi_ = (__win32_AdjustWindowRectExForDpi)GetProcAddress(this->user32.instance, "AdjustWindowRectExForDpi");
      }

#     if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_WINBLUE) || defined(__MINGW32__)
        this->shcore.instance = LoadLibraryA("shcore.dll");
        if (this->shcore.instance != nullptr) {
          this->shcore.SetProcessDpiAwareness_ = (__win32_SetProcessDpiAwareness)GetProcAddress(this->shcore.instance, "SetProcessDpiAwareness");
          this->shcore.GetDpiForMonitor_ = (__win32_GetDpiForMonitor)GetProcAddress(this->shcore.instance, "GetDpiForMonitor");
        }
#     endif

      // identify windows compatibility
      HINSTANCE ntdll_instance = LoadLibraryA("ntdll.dll");
      __win32_RtlVerifyVersionInfo verifyVersionInfo_ = nullptr;
      if (ntdll_instance != nullptr)
        verifyVersionInfo_ = (__win32_RtlVerifyVersionInfo)GetProcAddress(ntdll_instance, "RtlVerifyVersionInfo");

      if (_isWindowsVersionGreaterOrEqual(verifyVersionInfo_, 10u, 0u, __P_WIN_10_RS2_BUILD))
        this->_windowsReferenceBuild = __P_WIN_10_RS2_BUILD;
      else if (_isWindowsVersionGreaterOrEqual(verifyVersionInfo_, 10u, 0u, __P_WIN_10_RS1_BUILD))
        this->_windowsReferenceBuild = __P_WIN_10_RS1_BUILD;
      else if (_isWindowsVersionGreaterOrEqual(verifyVersionInfo_, HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), __P_WIN_8_1_BLUE_BUILD))
        this->_windowsReferenceBuild = __P_WIN_8_1_BLUE_BUILD;
      else if (_isWindowsVersionGreaterOrEqual(verifyVersionInfo_, HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), __P_WIN_7_BUILD))
        this->_windowsReferenceBuild = __P_WIN_7_BUILD;
      else
        this->_windowsReferenceBuild = __P_WIN_VISTA_SP1_BUILD;

      if (ntdll_instance != nullptr)
        FreeLibrary(ntdll_instance);

      this->_isInit = true;
    }

    // -- shutdown --

    void LibrariesWin32::shutdown() noexcept {
      if (this->user32.instance != nullptr) {
        FreeLibrary(user32.instance);
        this->user32.instance = nullptr;
      }
#     if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_WINBLUE) || defined(__MINGW32__)
        if (this->shcore.instance != nullptr) {
          FreeLibrary(shcore.instance);
          this->shcore.instance = nullptr;
        }
#     endif
      
      this->_isInit = false;
    }
# endif
# pragma warning(pop)
#endif
