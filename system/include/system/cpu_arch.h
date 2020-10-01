/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
------------------------------------------------------------------------
Description : CPU architecture detection
Constants : _SYSTEM_CPU_ARCH    : CPU architecture type
            _SYSTEM_CPU_REGSIZE : register size (16/32/64 bits)
Functions : getCpuArchitecture, getCpuRegisterSize
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include "./preprocessor_tools.h"

namespace pandora { 
  namespace system {
#   define _SYSTEM_CPU_ARCH_UNK 0x00
#   define _SYSTEM_CPU_ARCH_X86 0x01
#   define _SYSTEM_CPU_ARCH_PPC 0x02
#   define _SYSTEM_CPU_ARCH_ARM 0x04

    /// @enum CpuArchitecture
    /// @brief Type of central processor architecture
    enum class CpuArchitecture : uint32_t {
      unknown = _SYSTEM_CPU_ARCH_UNK,
      x86 = _SYSTEM_CPU_ARCH_X86,
      powerpc = _SYSTEM_CPU_ARCH_PPC,
      arm = _SYSTEM_CPU_ARCH_ARM,
      all = 0xFF
    };
    _P_SERIALIZABLE_ENUM(CpuArchitecture, unknown, x86, powerpc, arm)

    
#   if !defined(_SYSTEM_CPU_ARCH) || !defined(_SYSTEM_CPU_REGSIZE) // if manually specified in project, do not overwrite values

      // -- architecture detection --

#     if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__) || defined(_IA64_)
#       define _SYSTEM_CPU_ARCH     _SYSTEM_CPU_ARCH_X86
#       define _SYSTEM_CPU_REGSIZE  64
    
#     elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__IA32__) || defined(_M_I86) || defined(_M_IX86) || defined(__X86__) || defined(_X86_) || defined (__I86__)
#       define _SYSTEM_CPU_ARCH      _SYSTEM_CPU_ARCH_X86
#       define _SYSTEM_CPU_REGSIZE   32
#       if defined(MSDOS) && !defined(_WINDOWS) && !defined(WIN32) && !defined(_WIN32) && !defined(_WIN64) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(__linux__) && !defined(__linux) && !defined(__unix__) && !defined(_POSIX_VERSION)
#         define _SYSTEM_CPU_REGSIZE 16
#       endif

#     elif defined(__aarch64__) || defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__arm) || defined(_ARM) || defined(_M_ARMT) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB)
#       define _SYSTEM_CPU_ARCH      _SYSTEM_CPU_ARCH_ARM
#       if defined(__aarch64__) || defined(_M_ARM64)
#         define _SYSTEM_CPU_REGSIZE 64
#       else
#         define _SYSTEM_CPU_REGSIZE 32
#       endif

#     elif defined(__powerpc) || defined(__powerpc__) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(__powerpc64__)
#       define _SYSTEM_CPU_ARCH     _SYSTEM_CPU_ARCH_PPC
#       if defined(_ARCH_PPC64) || defined(__powerpc64__)
#         define _SYSTEM_CPU_REGSIZE 64
#       else
#         define _SYSTEM_CPU_REGSIZE 32
#       endif

#     else
#       define _SYSTEM_CPU_ARCH     _SYSTEM_CPU_ARCH_UNK
#       define _SYSTEM_CPU_REGSIZE  32
#     endif

#   endif

    /// @brief Verify the architecture type of current system
    constexpr inline CpuArchitecture getCpuArchitecture() noexcept { return static_cast<CpuArchitecture>(_SYSTEM_CPU_ARCH); }
    /// @brief Verify the architecture register size of current system
    constexpr inline size_t getCpuRegisterSize() noexcept { return static_cast<size_t>(_SYSTEM_CPU_REGSIZE); }
  }
}
#undef __CPU_ARCH_ENUM
_P_FLAGS_OPERATORS(::pandora::system::CpuArchitecture, uint32_t)
