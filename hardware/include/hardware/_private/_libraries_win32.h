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
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
# include <cstdint>
# include <system/api/windows_api.h>
# if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WINBLUE) && !defined(__MINGW32__) && defined(_MSC_VER) && (_MSC_VER >= 1920)
#   include <shellscalingapi.h>
# endif

// windows 8.1+ features
# if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WINBLUE) && !defined(__MINGW32__) && defined(_MSC_VER) && (_MSC_VER >= 1920)
#   define _P_WIN32_SetProcessDpiAwareness_SUPPORTED 1
#   define _P_WIN32_GetDpiForMonitor_SUPPORTED 1
# endif
# if !defined(DPI_ENUMS_DECLARED)
    typedef enum PROCESS_DPI_AWARENESS {
      PROCESS_DPI_UNAWARE = 0,
      PROCESS_SYSTEM_DPI_AWARE = 1,
      PROCESS_PER_MONITOR_DPI_AWARE = 2
    } PROCESS_DPI_AWARENESS;
    typedef enum MONITOR_DPI_TYPE {
      MDT_EFFECTIVE_DPI = 0,
      MDT_ANGULAR_DPI = 1,
      MDT_RAW_DPI = 2,
      MDT_DEFAULT = MDT_EFFECTIVE_DPI
    } MONITOR_DPI_TYPE;
#   define DPI_ENUMS_DECLARED
# endif

// windows 10+ features
# if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WIN10_RS1) && !defined(__MINGW32__) && defined(_MSC_VER) && (_MSC_VER >= 1920)
#   define _P_WIN32_EnableNonClientDpiScaling_SUPPORTED 1
#   define _P_WIN32_GetDpiForWindow_SUPPORTED 1
#   define _P_WIN32_GetSystemMetricsForDpi_SUPPORTED 1
#   define _P_WIN32_AdjustWindowRectExForDpi_SUPPORTED 1
# endif
# if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WIN10_RS2) && !defined(__MINGW32__) && defined(_MSC_VER) && (_MSC_VER >= 1920)
#   define _P_WIN32_SetProcessDpiAwarenessContext_SUPPORTED 1
# endif
# ifndef DPI_AWARENESS_CONTEXT_UNAWARE
#   define DPI_AWARENESS_CONTEXT_UNAWARE ((HANDLE)-1)
# endif
# ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#   define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)
# endif

// windows build versions
# define __P_WIN_10_RS2_BUILD 15063
# define __P_WIN_10_RS1_BUILD 14393
# define __P_WIN_8_1_BLUE_BUILD 9600
# define __P_WIN_7_BUILD 7601
# define __P_WIN_VISTA_SP1_BUILD 6001

# if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_WIN10_RS2) || defined(__MINGW32__) || !defined(_MSC_VER) || (_MSC_VER < 1920)
    namespace pandora {
      namespace hardware {
        // user32.dll functions
        typedef BOOL (WINAPI * __win32_SetProcessDpiAwarenessContext)(HANDLE);
        typedef BOOL (WINAPI * __win32_EnableNonClientDpiScaling)(HWND);
        typedef UINT (WINAPI * __win32_GetDpiForWindow)(HWND);
        typedef int  (WINAPI * __win32_GetSystemMetricsForDpi)(int,UINT);
        typedef BOOL (WINAPI * __win32_AdjustWindowRectExForDpi)(LPRECT,DWORD,BOOL,DWORD,UINT);

#       if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_WINBLUE) || defined(__MINGW32__) || !defined(_MSC_VER) || (_MSC_VER < 1920)
          // shcore.dll functions
          typedef HRESULT (WINAPI * __win32_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
          typedef HRESULT (WINAPI * __win32_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
#       endif


        // library loader - Win32
        struct LibrariesWin32 final {
          LibrariesWin32() = default;
          ~LibrariesWin32() noexcept { shutdown(); }

          struct {
            HINSTANCE                             instance = nullptr;
            __win32_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_ = nullptr;
            __win32_EnableNonClientDpiScaling     EnableNonClientDpiScaling_ = nullptr;
            __win32_GetDpiForWindow               GetDpiForWindow_ = nullptr;
            __win32_GetSystemMetricsForDpi        GetSystemMetricsForDpi_ = nullptr;
            __win32_AdjustWindowRectExForDpi      AdjustWindowRectExForDpi_ = nullptr;
          } user32;
#         if !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_WINBLUE) || defined(__MINGW32__) || !defined(_MSC_VER) || (_MSC_VER < 1920)
            struct {
              HINSTANCE                       instance = nullptr;
              __win32_SetProcessDpiAwareness  SetProcessDpiAwareness_ = nullptr;
              __win32_GetDpiForMonitor        GetDpiForMonitor_ = nullptr;
            } shcore;
#         endif
          
          // verify if Windows version >= Windows 10 Creators RS2
          inline bool isAtLeastWindows10_RS2() const noexcept { return (this->_windowsReferenceBuild >= __P_WIN_10_RS2_BUILD); }
          // verify if Windows version >= Windows 10 Anniversary RS1
          inline bool isAtLeastWindows10_RS1() const noexcept { return (this->_windowsReferenceBuild >= __P_WIN_10_RS1_BUILD); }
          // verify if Windows version >= Windows 8.1 Blue
          inline bool isAtLeastWindows8_1_Blue() const noexcept { return (this->_windowsReferenceBuild >= __P_WIN_8_1_BLUE_BUILD); }
          // verify if Windows version >= Windows 7
          inline bool isAtLeastWindows7() const noexcept { return (this->_windowsReferenceBuild >= __P_WIN_7_BUILD); }

          
          // initialize available libraries
          void init() noexcept;
          // close libraries
          void shutdown() noexcept;
          
          // get global instance
          static inline LibrariesWin32& instance() noexcept {
            if (!_libs._isInit)
              _libs.init();
            return _libs;
          }

        private:
          bool _isInit = false;
          uint32_t _windowsReferenceBuild = __P_WIN_7_BUILD;
          static LibrariesWin32 _libs;
        };
      }
    }
# endif
#endif
