/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <string>
#include "./_system_location_finder.h"

#ifndef _WINDOWS
# include <cstdlib>
# include <cerrno>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <pwd.h>
#endif

namespace pandora { 
  namespace io {
  
#   ifdef _WINDOWS
      /// @brief Get path of user home directory
      inline std::wstring _getUserHomePath() {
        return _findLocation(FOLDERID_Profile, 0u, L"%USERPROFILE%");
      }
      
      /// @brief Get path of user local config directory
      inline std::wstring _getUserLocalConfigPath() {
        return _findLocation(FOLDERID_LocalAppData, 0u, L"%USERPROFILE%\\AppData\\Local");
      }
    
#   elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
#     if !defined(__APPLE__) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
        inline char* __readUserDirectoryInPasswordFile() noexcept {
          struct passwd* wuid = getpwuid(getuid());
          errno = 0;
          return (wuid != nullptr) ? wuid->pw_dir : nullptr;
        }
    
        /// @brief Get path of user home directory
        inline std::string _getUserHomePath() {
          char* buffer = getenv("HOME");
          if (buffer != nullptr)
            return std::string(buffer);

          buffer = __readUserDirectoryInPasswordFile();
          return (buffer != nullptr) ? std::string(buffer) : "$HOME";
        }
    
#     else
        /// @brief Get path of user home directory
        inline std::string _getUserHomePath() {
          std::string homePath = _findFirstLocation(SYSDIR_DIRECTORY_USER, SYSDIR_DOMAIN_MASK_USER);
          if (!homePath.empty())
            return homePath;
          
          char* envBuffer = getenv("HOME");
          return (envBuffer != nullptr) ? std::string(envBuffer) : "$HOME";
        }
#     endif
      
      /// @brief Get path of user local config directory
      inline std::string _getUserLocalConfigPath() {
        char *envBuffer = getenv("XDG_CONFIG_HOME");
        return (envBuffer != nullptr) ? std::string(envBuffer) : _getUserHomePath() + "/.config";
      }
#   endif

  }
}
