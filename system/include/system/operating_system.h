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
------------------------------------------------------------------------
Description : operating system detection
Constants : _SYSTEM_OPERATING_SYSTEM : operating system type
Functions : getOperatingSystem
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include "./preprocessor_tools.h"

namespace pandora { 
  namespace system {
#   define _SYSTEM_OS_UNKNOWN 0x00
#   define _SYSTEM_OS_DOS     0x01
#   define _SYSTEM_OS_WINDOWS 0x02
#   define _SYSTEM_OS_MAC_PPC 0x04
#   define _SYSTEM_OS_MAC_OSX 0x08
#   define _SYSTEM_OS_IOS     0x10
#   define _SYSTEM_OS_UNIX    0x20
#   define _SYSTEM_OS_LINUX   0x40
#   define _SYSTEM_OS_ANDROID 0x80

    /// @enum OperatingSystem
    /// @brief Type of operating system
    enum class OperatingSystem : uint32_t {
      unknown    = _SYSTEM_OS_UNKNOWN,
      ms_dos     = _SYSTEM_OS_DOS,
      windows    = _SYSTEM_OS_WINDOWS,
      mac_os_ppc = _SYSTEM_OS_MAC_PPC,
      mac_os_x   = _SYSTEM_OS_MAC_OSX,
      ios        = _SYSTEM_OS_IOS,
      unix_os    = _SYSTEM_OS_UNIX,
      gnu_linux  = _SYSTEM_OS_LINUX,
      android    = _SYSTEM_OS_ANDROID
    };


#   if !defined(_SYSTEM_OPERATING_SYSTEM) // if manually specified in project, do not overwrite values

      // -- operating system detection --
    
#     if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINNT)
#       undef __APPLE__
#       undef __ANDROID__
#       undef __linux__
#       undef __unix__
#       define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_WINDOWS

#     elif defined(__APPLE__)
#       include <TargetConditionals.h>
#       undef __ANDROID__
#       undef __linux__
#       if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#         define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_IOS
#       elif defined(TARGET_OS_MAC) && TARGET_OS_MAC
#         define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_MAC_OSX
#       else
#         define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_MAC_PPC
#       endif

#     elif defined(__ANDROID__)
#       define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_ANDROID

#     elif defined(__linux__) || defined(__linux)
#       define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_LINUX

#     elif defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
#       define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_UNIX
      
#     elif defined(MSDOS)
#       define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_DOS

#     else
#       define _SYSTEM_OPERATING_SYSTEM _SYSTEM_OS_UNKNOWN
#     endif

#   endif

    /// @brief Verify the operating system type of current system
    constexpr inline OperatingSystem getOperatingSystem() noexcept { return static_cast<OperatingSystem>(_SYSTEM_OPERATING_SYSTEM); }
  }
}
_P_FLAGS_OPERATORS(::pandora::system::OperatingSystem, uint32_t)
