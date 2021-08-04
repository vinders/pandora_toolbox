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
--------------------------------------------------------------------------------
Enumerations : CpuInstructionSet, CpuInstructionFamily
Functions : toInstructionSet, toInstructionSetFamily, toCpuArchitecture
*******************************************************************************/
#pragma once

#include <cstdint>
#include <system/preprocessor_tools.h>
#include <system/cpu_arch.h>

namespace pandora { 
  namespace hardware {
#   define __P_toCpuInstructionSetValue(arch, family, code)  ((static_cast<uint32_t>(arch) << 24) | (static_cast<uint32_t>(family) << 16) | code)
    
    /// @enum CpuInstructionFamily
    /// @brief Supported family of instruction sets
    enum class CpuInstructionFamily : uint32_t {
      assembly = 0,
      mmx = 0x01,
      sse = 0x02,
      avx = 0x04,
      neon = 0x08,
    };
    _P_SERIALIZABLE_ENUM(CpuInstructionFamily, assembly, mmx, sse, avx, neon)
    
    /// @enum CpuInstructionSet
    /// @brief Supported set of instructions
    enum class CpuInstructionSet : uint32_t {
      cpp     = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::all, CpuInstructionFamily::assembly, 0x0),
      mmx     = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::mmx, 0x1),
      sse     = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x2),
      sse2    = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x4),
      sse3    = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x8),
      ssse3   = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x10),
      sse4_1  = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x20),
      sse4_2  = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x40),
      avx     = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::avx, 0x80),
      avx2    = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::avx, 0x100),
      avx512f = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::avx, 0x200),
      neon    = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::arm, CpuInstructionFamily::neon, 0x1),
      neon64  = __P_toCpuInstructionSetValue(pandora::system::CpuArchitecture::arm, CpuInstructionFamily::neon, 0x2),
    };
    _P_SERIALIZABLE_ENUM(CpuInstructionSet, cpp, mmx, sse, sse2, sse3, ssse3, sse4_1, sse4_2, avx, avx2, avx512f, neon, neon64)
    _P_LIST_ENUM_VALUES(CpuInstructionSet, x86_values, mmx, sse, sse2, sse3, ssse3, sse4_1, sse4_2, avx, avx2, avx512f)
    _P_LIST_ENUM_VALUES(CpuInstructionSet, x86_rvalues, avx512f, avx2, avx, sse4_2, sse4_1, ssse3, sse3, sse2, sse, mmx)
    _P_LIST_ENUM_VALUES(CpuInstructionSet, arm_values, neon, neon64)
    _P_LIST_ENUM_VALUES(CpuInstructionSet, arm_rvalues, neon64, neon)
    // !!! on enum update, also adapt 'cpu_specs.cpp' and 'cpuid_registers.h'


    /// @brief Build instruction-set value from CPU architecture, instruction family and instruction-set code
    constexpr inline CpuInstructionSet toCpuInstructionSet(pandora::system::CpuArchitecture arch, CpuInstructionFamily family, uint32_t code) noexcept {
      return static_cast<CpuInstructionSet>(__P_toCpuInstructionSetValue(arch, family, code));
    }
    /// @brief Extract instruction family from instruction-set
    constexpr inline CpuInstructionFamily toCpuInstructionFamily(CpuInstructionSet instr) noexcept {
      return static_cast<CpuInstructionFamily>((static_cast<uint32_t>(instr) >> 16) & 0xFFu);
    }
    /// @brief Extract CPU architecture from instruction-set
    constexpr inline pandora::system::CpuArchitecture toCpuArchitecture(CpuInstructionSet instr) noexcept {
      return static_cast<pandora::system::CpuArchitecture>(static_cast<uint32_t>(instr) >> 24);
    }
#   undef __P_toCpuInstructionSetValue
  }
}
_P_FLAGS_OPERATORS(::pandora::hardware::CpuInstructionFamily, uint32_t)
_P_FLAGS_OPERATORS(::pandora::hardware::CpuInstructionSet, uint32_t) // note: removeFlag is not supported
