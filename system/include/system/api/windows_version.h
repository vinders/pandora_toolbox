/*******************************************************************************
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

# if defined(_P_MIN_WINDOWS_VERSION) && _P_MIN_WINDOWS_VERSION == 10 // only Windows 10 RS3+
#   define WINVER _WIN32_WINNT_WIN10
#   define _WIN32_WINNT _WIN32_WINNT_WIN10
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN10
#   define _WIN32_IE _WIN32_IE_IE110
#   define NTDDI_VERSION NTDDI_WIN10_RS3

# elif defined(_P_MIN_WINDOWS_VERSION) && _P_MIN_WINDOWS_VERSION == 8 // Windows 8/10+
#   define WINVER _WIN32_WINNT_WIN8
#   define _WIN32_WINNT _WIN32_WINNT_WIN8
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN8
#   define _WIN32_IE _WIN32_IE_IE90
#   define NTDDI_VERSION NTDDI_WIN8

# elif !defined(_P_MIN_WINDOWS_VERSION) || _P_MIN_WINDOWS_VERSION == 7 // Windows 7/8/10+
#   define WINVER _WIN32_WINNT_WIN7
#   define _WIN32_WINNT _WIN32_WINNT_WIN7
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN7
#   define _WIN32_IE _WIN32_IE_IE80
#   define NTDDI_VERSION NTDDI_WIN7

# else // Windows Vista/7/8/10+
#   define WINVER _WIN32_WINNT_VISTA
#   define _WIN32_WINNT _WIN32_WINNT_VISTA
#   define _WIN32_WINDOWS _WIN32_WINNT_VISTA
#   define _WIN32_IE _WIN32_IE_IE70
#   define NTDDI_VERSION NTDDI_VISTASP1
# endif

#endif
