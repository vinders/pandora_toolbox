/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

# include <ctime>
#include "./_filesystem_api.h"

#ifdef _WINDOWS
# include <sys/types.h>
# include <sys/stat.h>
# ifdef _USE_NATIVE_FILESYSTEM
#   include <fileapi.h>
# endif

#else
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

namespace pandora { 
  namespace io {
  
#   ifndef _USE_NATIVE_FILESYSTEM
      /// @brief convert STL-FS file update time to time_t
      inline time_t _toWriteTime(const std::filesystem::directory_entry& entry) noexcept {
#       ifdef _WINDOWS
          struct _stat64 fileInfo;
          return (_wstati64(entry.path().wstring().c_str(), &fileInfo) == 0) ? fileInfo.st_mtime : time_t(0);
#       elif defined(__GNUC__) || defined(__clang__)
          std::error_code errorCode;
          auto timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(entry.last_write_time(errorCode) - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
          return std::chrono::system_clock::to_time_t(timePoint);
#       else
          std::error_code errorCode;
          auto time = entry.last_write_time(errorCode);
          return (!errorCode) ? decltype(time)::clock::to_time_t(time) : time_t(0);
#       endif
      }
      
#   elif defined(_WINDOWS)
      /// @brief Convert windows file update time to time_t
      inline time_t _toWriteTime(const FILETIME& timeData) noexcept {
        ULARGE_INTEGER buffer;
        buffer.LowPart = timeData.dwLowDateTime;
        buffer.HighPart = timeData.dwHighDateTime;
        return time_t(static_cast<uint64_t>(buffer.QuadPart) / 10000000uLL - 11644473600uLL);
      }
#   endif

  }
}
