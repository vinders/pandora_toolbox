/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <string>

#ifdef _WINDOWS
# include <io.h>
# include <tchar.h>
# ifndef NOMINMAX
#   define NOMINMAX // no min/max macros
#   define WIN32_LEAN_AND_MEAN // exclude rare MFC libraries
# endif
# include <Windows.h>
# include <shlobj.h>

#elif defined(__APPLE__)
# include <TargetConditionals.h>
# if !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE
#   include <vector>
#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <limits.h>
#   include <Availability.h>
#   if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_12
#     include <sysdir.h>
#   else
#     include <NSSystemDirectories.h>
#   endif
# endif
#endif

namespace pandora { 
  namespace io {
  
#   ifdef _WINDOWS
      /// @brief Find known system location
      inline bool _findLocation(KNOWNFOLDERID location, std::wstring& outPath, uint32_t systemFlags = 0u) noexcept {
        PWSTR buffer = nullptr;
        bool isSuccess = (SHGetKnownFolderPath(location, static_cast<DWORD>(systemFlags), nullptr, &buffer) == S_OK);
        if (isSuccess)
          outPath = (wchar_t*)buffer;
        CoTaskMemFree((LPVOID)buffer);
        return isSuccess;
      }
    
      /// @brief Find known system location
      inline std::wstring _findLocation(KNOWNFOLDERID location, uint32_t systemFlags = 0u, const std::wstring& fallbackValue = L"") noexcept {
        std::wstring path;
        return (_findLocation(location, path, systemFlags))
              ? path
              : fallbackValue;
      }
      
#   elif defined(__APPLE__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
#     if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_12
        using NativeLocationId = sysdir_search_path_directory_t;
        using NativeDomainMask = sysdir_search_path_domain_mask_t;
        using LocationSearchState = sysdir_search_path_enumeration_state;
#       define __P_START_PATH_SEARCH          sysdir_start_search_path_enumeration
#       define __P_GET_NEXT_PATH_VALUE        sysdir_get_next_search_path_enumeration
#       define __P_DEFAULT_DOMAIN_MASK_VALUE  SYSDIR_DOMAIN_MASK_ALL
#       define SYSDIR_DOMAIN_MASK__USER_LOCAL        (sysdir_search_path_domain_mask_t)((unsigned int)SYSDIR_DOMAIN_MASK_USER | (unsigned int)SYSDIR_DOMAIN_MASK_LOCAL)
#       define SYSDIR_DOMAIN_MASK__SYSTEM_LOCAL      (sysdir_search_path_domain_mask_t)((unsigned int)SYSDIR_DOMAIN_MASK_LOCAL | (unsigned int)SYSDIR_DOMAIN_MASK_SYSTEM)
#       define SYSDIR_DOMAIN_MASK__SYSTEM_USER_LOCAL (sysdir_search_path_domain_mask_t)((unsigned int)SYSDIR_DOMAIN_MASK_USER | (unsigned int)SYSDIR_DOMAIN_MASK_LOCAL | (unsigned int)SYSDIR_DOMAIN_MASK_SYSTEM)
#       define SYSDIR_DOMAIN_MASK__SYSTEM_NET_LOCAL  (sysdir_search_path_domain_mask_t)((unsigned int)SYSDIR_DOMAIN_MASK_NETWORK | (unsigned int)SYSDIR_DOMAIN_MASK_LOCAL | (unsigned int)SYSDIR_DOMAIN_MASK_SYSTEM)

#     else
        using NativeLocationId = NSSearchPathDirectory;
        using NativeDomainMask = NSSearchPathDomainMask;
        using LocationSearchState = NSSearchPathEnumerationState;
#       define __P_START_PATH_SEARCH          NSStartSearchPathEnumeration
#       define __P_GET_NEXT_PATH_VALUE        NSGetNextSearchPathEnumeration
#       define __P_DEFAULT_DOMAIN_MASK_VALUE  NSAllDomainsMask
#       define NSUserLocalDomainMask       (NSSearchPathDomainMask)((unsigned int)NSUserDomainMask | (unsigned int)NSLocalDomainMask)
#       define NSLocalSystemDomainMask     (NSSearchPathDomainMask)((unsigned int)NSLocalDomainMask | (unsigned int)NSSystemDomainMask)
#       define NSSystemUserLocalDomainMask (NSSearchPathDomainMask)((unsigned int)NSUserDomainMask | (unsigned int)NSLocalDomainMask | (unsigned int)NSSystemDomainMask)
#       define NSSystemNetLocalDomainMask (NSSearchPathDomainMask)((unsigned int)NSNetworkDomainMask | (unsigned int)NSLocalDomainMask | (unsigned int)NSSystemDomainMask)

#       define SYSDIR_DIRECTORY_APPLICATION           NSApplicationDirectory
#       define SYSDIR_DIRECTORY_DEMO_APPLICATION      NSDemoApplicationDirectory
#       define SYSDIR_DIRECTORY_DEVELOPER_APPLICATION NSDeveloperApplicationDirectory
#       define SYSDIR_DIRECTORY_ADMIN_APPLICATION     NSAdminApplicationDirectory
#       define SYSDIR_DIRECTORY_LIBRARY               NSLibraryDirectory
#       define SYSDIR_DIRECTORY_DEVELOPER             NSDeveloperDirectory
#       define SYSDIR_DIRECTORY_USER                  NSUserDirectory
#       define SYSDIR_DIRECTORY_DOCUMENTATION         NSDocumentationDirectory
#       define SYSDIR_DIRECTORY_DOCUMENT              NSDocumentDirectory
#       define SYSDIR_DIRECTORY_CORESERVICE           NSCoreServiceDirectory
#       define SYSDIR_DIRECTORY_AUTOSAVED_INFORMATION NSAutosavedInformationDirectory
#       define SYSDIR_DIRECTORY_DESKTOP               NSDesktopDirectory
#       define SYSDIR_DIRECTORY_CACHES                NSCachesDirectory
#       define SYSDIR_DIRECTORY_APPLICATION_SUPPORT   NSApplicationSupportDirectory
#       define SYSDIR_DIRECTORY_DOWNLOADS             NSDownloadsDirectory
#       define SYSDIR_DIRECTORY_INPUT_METHODS         NSInputMethodsDirectory
#       define SYSDIR_DIRECTORY_MOVIES                NSMoviesDirectory
#       define SYSDIR_DIRECTORY_MUSIC                 NSMusicDirectory
#       define SYSDIR_DIRECTORY_PICTURES              NSPicturesDirectory
#       define SYSDIR_DIRECTORY_PRINTER_DESCRIPTION   NSPrinterDescriptionDirectory
#       define SYSDIR_DIRECTORY_SHARED_PUBLIC         NSSharedPublicDirectory
#       define SYSDIR_DIRECTORY_PREFERENCE_PANES      NSPreferencePanesDirectory
#       define SYSDIR_DIRECTORY_ALL_APPLICATIONS      NSAllApplicationsDirectory
#       define SYSDIR_DIRECTORY_ALL_LIBRARIES         NSAllLibrariesDirectory

#       define SYSDIR_DOMAIN_MASK_USER               NSUserDomainMask
#       define SYSDIR_DOMAIN_MASK_LOCAL              NSLocalDomainMask
#       define SYSDIR_DOMAIN_MASK_NETWORK            NSNetworkDomainMask
#       define SYSDIR_DOMAIN_MASK_SYSTEM             NSSystemDomainMask
#       define SYSDIR_DOMAIN_MASK__USER_LOCAL        NSUserLocalDomainMask    
#       define SYSDIR_DOMAIN_MASK__SYSTEM_LOCAL      NSLocalSystemDomainMask
#       define SYSDIR_DOMAIN_MASK__SYSTEM_USER_LOCAL NSSystemUserLocalDomainMask
#       define SYSDIR_DOMAIN_MASK__SYSTEM_NET_LOCAL  NSSystemNetLocalDomainMask
#       define SYSDIR_DOMAIN_MASK_ALL                NSAllDomainsMask
#     endif

      /// @brief Find known system locations
      inline std::vector<std::string> _findLocation(NativeLocationId location, NativeDomainMask domainMask = __P_DEFAULT_DOMAIN_MASK_VALUE) noexcept {
        std::vector<std::string> paths;
        char buffer[PATH_MAX];
        LocationSearchState state = __P_START_PATH_SEARCH(location, domainMask);
        while ((state = __P_GET_NEXT_PATH_VALUE(state, buffer)) != 0)
          paths.emplace_back(buffer);
        return paths;
      }
    
      /// @brief Find known system locations
      inline std::vector<std::string> _findLocation(NativeLocationId location, NativeDomainMask domainMask, const std::string& fallbackValue) noexcept {
        std::vector<std::string> paths = _findLocation(location, domainMask);
        if (paths.empty())
          paths.emplace_back(fallbackValue);
        return paths;
      }

      /// @brief Find known system locations
      inline std::vector<std::string> _findLocation(NativeLocationId location, NativeDomainMask domainMask, const std::string& fallbackValue, const std::string& suffix) noexcept {
        std::vector<std::string> paths;
        char buffer[PATH_MAX];
        LocationSearchState state = __P_START_PATH_SEARCH(location, domainMask);
        while ((state = __P_GET_NEXT_PATH_VALUE(state, buffer)) != 0)
          paths.emplace_back(buffer + suffix);
        if (paths.empty())
          paths.emplace_back(fallbackValue + suffix);
        return paths;
      }
    
      /// @brief Find known system location - only first result
      inline std::string _findFirstLocation(NativeLocationId location, NativeDomainMask domainMask = __P_DEFAULT_DOMAIN_MASK_VALUE, const std::string& fallbackValue = "") noexcept {
        char buffer[PATH_MAX];
        LocationSearchState state = __P_START_PATH_SEARCH(location, domainMask);
        return ((state = __P_GET_NEXT_PATH_VALUE(state, buffer)) != 0)
              ? std::string(buffer)
              : fallbackValue;
      }

#     undef __P_START_PATH_SEARCH
#     undef __P_GET_NEXT_PATH_VALUE
#     undef __P_DEFAULT_DOMAIN_MASK_VALUE
#   endif

  }
}
