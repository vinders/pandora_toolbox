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

#include <cstdint>
#include <string>
#include <vector>

namespace pandora { 
  namespace io {
    /// @brief Standard file system directories
    enum class FileSystemLocation: uint32_t {
      // user data directories
      home = 0u, ///< User home directory (may be application-specific on embedded/mobile systems)
      desktop,   ///< User desktop directory (or home directory, if system doesn't have a desktop)
      documents, ///< User document files directory
      pictures,  ///< User image files directory (or user documents, if system doesn't have a picture directory)
      music,     ///< User audio/music files directory (or user documents, if system doesn't have a music directory)
      movies,    ///< User video/movie files directory (or user documents, if system doesn't have a movie directory)
      downloads, ///< User downloaded files directory (may be application-specific on embedded/mobile systems)
      
      // application directories
      applications, ///< User installed application links directory (right access is platform-specific)
      sharedApps,   ///< System-wide application links directory (may be the same as 'applications' on some systems)
      appMenu,      ///< Application menu directories (start menu, ...)
      fonts,        ///< System directories containing available fonts (read-only)

      // app data directories
      appData,        ///< Directory where persistent application-specific data can be written (profiles, settings, ...)
      localAppData,   ///< Directory where local application data can be written
      appConfig,      ///< Directory where application-specific configuration files for current user can be stored
      commonAppConfig,///< Directory where configuration files for current user can be stored
      cache,          ///< Directory where non-essential application-specific data for current user can be written
      localCache,     ///< Directory where non-essential application data for current user can be written
      sharedCache,    ///< Directory where shared non-essential application data can be written
      temp,           ///< Directory where short-term temporary files can be written

      // binary/system directories
      system,           ///< System binary files directories (may not exist on embedded/mobile systems)
      localBinaries,    ///< User-specific application binary files directories (may not exist on embedded/mobile systems)
      sharedBinaries,   ///< Installed application binary files directories (may not exist on embedded/mobile systems)
      sharedLibraries,  ///< Installed library/framework files directories (may be application-specific on embedded/mobile systems)
      commonBinaryFiles,///< Directory to store common binary/library files shared by multiple applications
      printers,         ///< Directory containing symbolic links to available printers (may not exist on embedded/mobile systems)
      trash             ///< System directory containing deleted files (may not exist on embedded/mobile systems)
    };
    
    // ---
    
#   ifdef _WINDOWS
      using SystemPath = std::wstring;
      using SystemPathChar = wchar_t;
#     define __P_DEFAULT_APP_SUBDIR L"common"
#   else
      using SystemPath = std::string;
      using SystemPathChar = char;
#     define __P_DEFAULT_APP_SUBDIR "common"
#   endif
  
    /// @brief Standard file system location finding toolbox
    class FileSystemLocationFinder final {
    public:
      FileSystemLocationFinder() = delete;

      /// @brief Get absolute path of current directory
      /// @warning: in special environments (sandboxes,...), this may return the relative location of the app (empty string)
      static SystemPath currentLocation();
      /// @brief Get absolute path of user home directory
      /// @warning: in special environments (sandboxes,...), this may return the relative location of the app (empty string)
      static SystemPath homeLocation();

      /// @brief Get absolute path(s) of standard user/app/system location
      /// @warning: - may contain multiple results
      ///           - may be empty if not found on current system
      ///           - returns all potential known locations paths: some of them may currently not exist.
      static std::vector<SystemPath> standardLocation(FileSystemLocation location, const SystemPathChar* appDirLabel = __P_DEFAULT_APP_SUBDIR, uint32_t systemFlags = 0u);
    };

  }
}
#undef __P_DEFAULT_APP_SUBDIR
