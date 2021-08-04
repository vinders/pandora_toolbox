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
Description : Microsoft Windows feature version constants
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
// Windows constants (to enable all features available on platform versions up to the one specified)
# include <sdkddkver.h>
# ifdef WINVER
#   undef WINVER
# endif
# ifdef _WIN32_WINNT
#   undef _WIN32_WINNT
# endif
# ifdef _WIN32_WINDOWS
#   undef _WIN32_WINDOWS
# endif
# ifdef _WIN32_IE
#   undef _WIN32_IE
# endif
# ifdef NTDDI_VERSION
#   undef NTDDI_VERSION
# endif

# if defined(_P_MIN_WINDOWS_VERSION) && _P_MIN_WINDOWS_VERSION == 10 // only Windows 10.RS2+
#   define WINVER _WIN32_WINNT_WIN10
#   define _WIN32_WINNT _WIN32_WINNT_WIN10
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN10
#   define _WIN32_IE _WIN32_IE_IE110
#   define NTDDI_VERSION NTDDI_WIN10_RS2

# elif !defined(_P_MIN_WINDOWS_VERSION) || _P_MIN_WINDOWS_VERSION == 8 // Windows 8.1/10+
#   define WINVER _WIN32_WINNT_WINBLUE
#   define _WIN32_WINNT _WIN32_WINNT_WINBLUE
#   define _WIN32_WINDOWS _WIN32_WINNT_WINBLUE
#   define _WIN32_IE _WIN32_IE_IE90
#   define NTDDI_VERSION NTDDI_WINBLUE

# elif defined(_P_MIN_WINDOWS_VERSION) && _P_MIN_WINDOWS_VERSION == 7 // Windows 7/8/10+
#   define WINVER _WIN32_WINNT_WIN7
#   define _WIN32_WINNT _WIN32_WINNT_WIN7
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN7
#   define _WIN32_IE _WIN32_IE_IE80
#   define NTDDI_VERSION NTDDI_WIN7

# else // Windows Vista.SP1/7/8/10+
#   define WINVER _WIN32_WINNT_VISTA
#   define _WIN32_WINNT _WIN32_WINNT_VISTA
#   define _WIN32_WINDOWS _WIN32_WINNT_VISTA
#   define _WIN32_IE _WIN32_IE_IE70
#   define NTDDI_VERSION NTDDI_VISTASP1
# endif

#endif
