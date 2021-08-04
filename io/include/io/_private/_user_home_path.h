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
