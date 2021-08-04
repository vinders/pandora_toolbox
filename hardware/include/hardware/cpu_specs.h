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
#include <cstdint>
#include <cassert>
#include <string>
#include <array>
#include <vector>
#include <system/cpu_arch.h>
#include <system/preprocessor_tools.h>
#include "./cpu_instruction_set.h"
#include "./cpuid_property_location.h"
#include "./cpuid_registers.h"
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace hardware {
    /// @class CpuSpecs
    /// @brief CPU specifications reader
    class CpuSpecs final {
    public:
      /// @brief Type of specifications to process ('all' for complete CPU specs, others for lazy modes)
      enum class SpecMode: uint32_t {
        none = 0,
        vendor = 0x1,
        brandName = 0x2,
        cores = 0x4,
        registers = 0x8,
        all = 0xF
      };
      
      /// @brief Verify all the CPU specifications requested in 'mode' (SpecMode bit-map) and store results
      explicit CpuSpecs(SpecMode mode = SpecMode::all);
      CpuSpecs(const CpuSpecs&) = default;
      CpuSpecs(CpuSpecs&&) noexcept = default;
      CpuSpecs& operator=(const CpuSpecs&) = default;
      CpuSpecs& operator=(CpuSpecs&&) noexcept = default;

      // -- getters --
      
      constexpr inline CpuVendor vendorId() const noexcept { return this->_vendorId; }
      constexpr inline pandora::system::CpuArchitecture archType() const noexcept { return pandora::system::getCpuArchitecture(); }
      inline const std::string& vendor() const noexcept { return this->_vendor; }
      inline const std::string& brand() const noexcept { return this->_brand; }
      
      constexpr inline uint32_t physicalCores() const noexcept { return this->_physicalCores; }
      constexpr inline uint32_t logicalCores() const noexcept { return this->_logicalCores; }
      constexpr inline bool isHyperThreadingCapable() const noexcept { return (this->_logicalCores > this->_physicalCores); }

#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        /// @brief Verify if a basic function is supported on current system
        inline bool hasBaseProperty(const CpuidPropertyLocation& prop) const noexcept {
          assert(prop.registerId() == static_cast<int32_t>(cpuid_x86::RegisterId::baseFeatures));
          return CpuSpecs::_hasRegisterBits(this->_regBase, prop);
        }
        /// @brief Verify if an advanced function is supported on current system (AVX2+, SHA, ...)
        inline bool hasAdvancedProperty(const CpuidPropertyLocation& prop) const noexcept {
          assert(prop.registerId() == static_cast<int32_t>(cpuid_x86::RegisterId::advancedFeatures));
          return (CpuSpecs::_hasRegisterBits(this->_regAdvanced, prop) 
               && (prop.exclusivity() == CpuVendor::unknown || prop.exclusivity() == this->_vendorId));
        }
        /// @brief Verify if an extended feature is supported on current system (intel/amd specific features)
        inline bool hasExtendedProperty(const CpuidPropertyLocation& prop) const noexcept {
          assert(prop.registerId() == static_cast<int32_t>(cpuid_x86::RegisterId::extendedFeatures));
          return (CpuSpecs::_hasRegisterBits(this->_regExtended, prop)
               && (prop.exclusivity() == CpuVendor::unknown || prop.exclusivity() == this->_vendorId));
        }
        
#     else
#       if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
          inline bool hasBaseProperty(const CpuidPropertyLocation& prop) const noexcept {
            assert(prop.registerId() == static_cast<int32_t>(cpuid_arm::RegisterId::id));
            return CpuSpecs::_hasRegisterBits(this->_regBase, prop);
          }
#       else
          inline bool hasBaseProperty(const CpuidPropertyLocation& prop) const noexcept { return false; }
#       endif
        inline bool hasAdvancedProperty(const CpuidPropertyLocation& prop) const noexcept { return false; }
        inline bool hasExtendedProperty(const CpuidPropertyLocation& prop) const noexcept { return false; }
#     endif

#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
        /// @brief Verify any property on current system 
        ///        (by calling hasBaseProperty/hasAdvancedProperty/hasExtendedProperty, or making a cpuid query for other registers)
        uint64_t getProperty(const CpuidPropertyLocation& prop) const noexcept;
#     else
        /// @brief Verify any property on current system 
        ///        (by calling hasBaseProperty/hasAdvancedProperty/hasExtendedProperty, or making a cpuid query for other registers)
        uint32_t getProperty(const CpuidPropertyLocation& prop) const noexcept;
#     endif
      
      /// @brief Verify if a CPU instruction-set is supported on current system
      bool isInstructionSetSupported(CpuInstructionSet type) const noexcept;

      /// @brief Get a list of all CPU instruction-sets available on current system
      inline std::vector<CpuInstructionSet> getAvailableCpuInstructionSets() const noexcept {
        std::vector<CpuInstructionSet> results{ CpuInstructionSet::cpp };
#       if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86 // arm / arm64
          for (const auto type : CpuInstructionSet_x86_values())
            if (isInstructionSetSupported(type))
              results.emplace_back(type);
#       elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM // general / x86 / x86_64
          for (const auto type : CpuInstructionSet_arm_values())
            if (isInstructionSetSupported(type))
              results.emplace_back(type);
#       endif
        return results;
      }
      
    private:
      // -- private helpers --
      
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        static inline bool _hasRegisterBits(const CpuRegister128& reg, const CpuidPropertyLocation& prop) noexcept { return ((reg[static_cast<uint32_t>(prop.index())] & prop.mask()) != 0u); }
#     else
        static inline bool _hasRegisterBits(const CpuRegister64& reg, const CpuidPropertyLocation& prop) noexcept { return ((static_cast<uint64_t>(reg) & prop.mask()) != 0u); }
#     endif

    private:
      std::string _vendor;
      std::string _brand;
      CpuVendor _vendorId = CpuVendor::unknown;
      
      uint32_t _physicalCores = 1u;
      uint32_t _logicalCores = 1u;
      
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        int32_t _maxBaseRegisterId = 0u;
        int32_t _maxExtendedRegisterId = 0u;
        CpuRegister128 _regBase = CpuRegister128{ 0 };
        CpuRegister128 _regAdvanced = CpuRegister128{ 0 };
        CpuRegister128 _regExtended = CpuRegister128{ 0 };
#     elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
        CpuRegister64 _regBase{ 0 };
#     endif
    };
  }
}
_P_FLAGS_OPERATORS(::pandora::hardware::CpuSpecs::SpecMode, uint32_t)

#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
