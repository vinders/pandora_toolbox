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
Location identification for CPUID property
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>
#include <system/cpu_arch.h>
#include "./cpu_vendor.h"

namespace pandora {
  namespace hardware {
    /// @brief CPU register memory part
    enum class CpuRegisterPart : uint32_t {
      eax = 0,
      ebx = 1,
      ecx = 2,
      edx = 3
    };
    constexpr inline size_t CpuRegisterPart_size() noexcept { return static_cast<size_t>(CpuRegisterPart::edx) + static_cast<size_t>(1u); }
    
    // ---
    
#   if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
#     define __P_CPUID_MASK uint64_t
#   else
#     define __P_CPUID_MASK uint32_t
#   endif
    
    /// @class CpuidPropertyLocation
    /// @brief Location of a readable property of a CPUID register
    class CpuidPropertyLocation final {
    public:
      CpuidPropertyLocation() = default;
      CpuidPropertyLocation(const CpuidPropertyLocation&) = default;
      CpuidPropertyLocation(CpuidPropertyLocation&&) noexcept = default;
      CpuidPropertyLocation& operator=(const CpuidPropertyLocation&) = default;
      CpuidPropertyLocation& operator=(CpuidPropertyLocation&&) noexcept = default;
      
      constexpr CpuidPropertyLocation(int32_t registerId, CpuRegisterPart index, __P_CPUID_MASK mask, uint32_t offset, CpuVendor exclusivity = CpuVendor::unknown) noexcept
        : _registerId(registerId),
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
          _index(index),
#     endif
          _mask(mask),
          _offset(offset), 
          _exclusivity(exclusivity) {
#       if !defined(_CPP_REVISION) || _CPP_REVISION != 14
          assert(static_cast<size_t>(index) < CpuRegisterPart_size() && offset < 32u);
#       endif
      }
      
      // -- getters --
      
      constexpr inline int32_t registerId() const noexcept { return this->_registerId; }
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        constexpr inline CpuRegisterPart index() const noexcept { return this->_index; }
#     else
        constexpr inline CpuRegisterPart index() const noexcept { return CpuRegisterPart::eax; }
#     endif

      constexpr inline __P_CPUID_MASK mask() const noexcept { return this->_mask; }
      constexpr inline uint32_t offset() const noexcept { return this->_offset; }
      
      constexpr inline bool hasExclusivity() const noexcept { return (this->_exclusivity != CpuVendor::unknown); }
      constexpr inline CpuVendor exclusivity() const noexcept { return this->_exclusivity; }
      
    private:
      int32_t _registerId = 0; // identifier of CPU register (_CPU_X86_CPUID, _CPU_X86_ADVANCED, _CPU_ARM_CPUID, ...)
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        CpuRegisterPart _index = CpuRegisterPart::eax; // part index in register (__REG_EAX to __REG_EDX)
#     endif
      __P_CPUID_MASK _mask = 0u;     // bit-mask to identify property in register part
      uint32_t _offset = 0u;    // offset to use after applying bit-mask to get the property value
      CpuVendor _exclusivity = CpuVendor::unknown; // optional: feature exclusivity for certain vendors
    };

#   undef __P_CPUID_MASK
  }
}
