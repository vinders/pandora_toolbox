/*******************************************************************************
Description : Microsoft Windows feature version constants
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
  // Windows constants (to enable all features available on platform versions up to the one specified)
# include <WinSDKVer.h>
# if !defined(WINVER) || WINVER < 0x0600 // required Windows
#   define WINVER 0x0600
# endif
# if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600 // required NT platform
#   define _WIN32_WINNT 0x0600
# endif
# if !defined(_WIN32_WINDOWS) || _WIN32_WINDOWS < 0x0600 // required DOS-based platform
#  define _WIN32_WINDOWS 0x0600  
# endif
# ifndef _WIN32_IE // required browser (Internet Explorer 7.0+)
#   define _WIN32_IE 0x0700 
# endif
# include <SDKDDKVer.h>

#endif
