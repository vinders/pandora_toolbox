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
