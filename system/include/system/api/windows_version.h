/*******************************************************************************
Description : Microsoft Windows feature version constants
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
// Windows constants (to enable all features available on platform versions up to the one specified)
# include <sdkddkver.h>

# if defined(CWORK_WINDOWS_SUPPORT) && CWORK_WINDOWS_SUPPORT == 10 // only Windows 10+
#   define WINVER _WIN32_WINNT_WIN10 
#   define _WIN32_WINNT _WIN32_WINNT_WIN10 
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN10   
#   define _WIN32_IE _WIN32_IE_IE110  
#   define NTDDI_VERSION NTDDI_WIN10 

# elif defined(CWORK_WINDOWS_SUPPORT) && CWORK_WINDOWS_SUPPORT == 8 // Windows 8/10+
#   define WINVER _WIN32_WINNT_WIN8 
#   define _WIN32_WINNT _WIN32_WINNT_WIN8 
#   define _WIN32_WINDOWS _WIN32_WINNT_WIN8   
#   define _WIN32_IE _WIN32_IE_IE90  
#   define NTDDI_VERSION NTDDI_WIN8 

# elif !defined(CWORK_WINDOWS_SUPPORT) || CWORK_WINDOWS_SUPPORT == 7 // Windows 7/8/10+
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
