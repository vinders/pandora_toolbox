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
List of x86 / ARM register IDs and properties
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <string>
#include <system/cpu_arch.h>
#include "./cpuid_property_location.h"

namespace pandora {
  namespace hardware {
    using CpuRegister128 = std::array<uint32_t, CpuRegisterPart_size()>; ///< 128-bit register (x86, ...)
    using CpuRegister64 = uint64_t; ///< 32-bit register (ARM, ...)
    
#   if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
      // -- x86 / X86_64 --
      namespace cpuid_x86 {
        // -- register IDs --
    
        /// @brief Register identifiers for x86 CPUID
        enum class RegisterId : int32_t {
          sizeAndVendor    = 0x0,
          baseFeatures     = 0x1,
          tlb              = 0x2,
          cache            = 0x4,
          advancedFeatures = 0x7,
          extendedTopology = 0xB,
          extendedSize     = static_cast<int32_t>(0x80000000),
          extendedFeatures = static_cast<int32_t>(0x80000001),
          brand1           = static_cast<int32_t>(0x80000002),
          brand2           = static_cast<int32_t>(0x80000003),
          brand3           = static_cast<int32_t>(0x80000004),
          vendorReserved   = static_cast<int32_t>(0x80000008)
        };

        // -- register properties --
      
        constexpr inline CpuidPropertyLocation baseSize() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::sizeAndVendor, CpuRegisterPart::eax, 0xFFFFFFFF, 0); }
        constexpr inline CpuidPropertyLocation extendedSize() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::extendedSize, CpuRegisterPart::eax, 0xFFFFFFFF, 0); }
        
        constexpr inline CpuidPropertyLocation vendorPart1() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::sizeAndVendor, CpuRegisterPart::ebx, 0xFFFFFFFF, 0); }
        constexpr inline CpuidPropertyLocation vendorPart2() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::sizeAndVendor, CpuRegisterPart::edx, 0xFFFFFFFF, 0); }
        constexpr inline CpuidPropertyLocation vendorPart3() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::sizeAndVendor, CpuRegisterPart::ecx, 0xFFFFFFFF, 0); }

        constexpr inline CpuidPropertyLocation baseFamily() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::eax, 0x00000F00, 8, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation extendedFamily() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::eax, 0xFFu<<20, 20, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation brandIndex() noexcept   { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ebx, 0x000000FF, 0); }
        constexpr inline CpuidPropertyLocation logicalProcessorsPerCpu() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ebx, 0x00FF0000, 16); }
        constexpr inline CpuidPropertyLocation localApicId() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ebx, 0xFF000000, 24); }
        constexpr inline CpuidPropertyLocation sse3() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u,     0); }
        constexpr inline CpuidPropertyLocation pclMulQdq() noexcept    { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<1,  1); }
        constexpr inline CpuidPropertyLocation monitor() noexcept      { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<3,  3); }
        constexpr inline CpuidPropertyLocation ssse3() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<9,  9); }
        constexpr inline CpuidPropertyLocation fma() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<12, 12); }
        constexpr inline CpuidPropertyLocation cmpXchg16b() noexcept   { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<13, 13); }
        constexpr inline CpuidPropertyLocation sse41() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<19, 19); }
        constexpr inline CpuidPropertyLocation sse42() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<20, 20); }
        constexpr inline CpuidPropertyLocation movBE() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<22, 22); }
        constexpr inline CpuidPropertyLocation popCnt() noexcept       { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<23, 23); }
        constexpr inline CpuidPropertyLocation aes() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<25, 25); }
        constexpr inline CpuidPropertyLocation extendedSave() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<26, 26); }
        constexpr inline CpuidPropertyLocation osExtendedSave() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<27, 27); }
        constexpr inline CpuidPropertyLocation avx() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<28, 28); }
        constexpr inline CpuidPropertyLocation f16c() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<29, 29); }
        constexpr inline CpuidPropertyLocation rdRand() noexcept       { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::ecx, 1u<<30, 30); }
        constexpr inline CpuidPropertyLocation msr() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<5,  5); }
        constexpr inline CpuidPropertyLocation cx8() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<8,  8); }
        constexpr inline CpuidPropertyLocation sep() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<11, 11); }
        constexpr inline CpuidPropertyLocation cmov() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<15, 15); }
        constexpr inline CpuidPropertyLocation clFsh() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<19, 19); }
        constexpr inline CpuidPropertyLocation mmx() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<23, 23); }
        constexpr inline CpuidPropertyLocation fxSr() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<24, 24); }
        constexpr inline CpuidPropertyLocation sse() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<25, 25); }
        constexpr inline CpuidPropertyLocation sse2() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<26, 26); }
        constexpr inline CpuidPropertyLocation hyperThreading() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::baseFeatures, CpuRegisterPart::edx, 1u<<28, 28); }

        constexpr inline CpuidPropertyLocation fsgsBase() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u,     0); }
        constexpr inline CpuidPropertyLocation bmi1() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<3,  3); }
        constexpr inline CpuidPropertyLocation hle() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<4,  4, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation avx2() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<5,  5); }
        constexpr inline CpuidPropertyLocation bmi2() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<8,  8); }
        constexpr inline CpuidPropertyLocation erms() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<9,  9); }
        constexpr inline CpuidPropertyLocation invPcid() noexcept      { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<10, 10); }
        constexpr inline CpuidPropertyLocation rtm() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<11, 11, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation avx512f() noexcept      { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<16, 16); }
        constexpr inline CpuidPropertyLocation rdSeed() noexcept       { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<18, 18); }
        constexpr inline CpuidPropertyLocation adx() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<19, 19); }
        constexpr inline CpuidPropertyLocation avx512pf() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<26, 26); }
        constexpr inline CpuidPropertyLocation avx512er() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<27, 27); }
        constexpr inline CpuidPropertyLocation avx512cd() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<28, 28); }
        constexpr inline CpuidPropertyLocation sha() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ebx, 1u<<29, 29); }
        constexpr inline CpuidPropertyLocation prefetchWt1() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::advancedFeatures, CpuRegisterPart::ecx, 1u,     0); }

        constexpr inline CpuidPropertyLocation lahf() noexcept         { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u,     0); }
        constexpr inline CpuidPropertyLocation amdCmpLegacy() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u<<1,  1, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation lzCnt() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u<<5,  5, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation abm() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u<<5,  5, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation sse4a() noexcept        { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u<<6,  6, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation xop() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u<<11, 11, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation tbm() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::ecx, 1u<<21, 21, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation sysCall() noexcept      { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::edx, 1u<<11, 11, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation mmxExt() noexcept       { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::edx, 1u<<22, 22, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation rdTscp() noexcept       { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::edx, 1u<<27, 27, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation amd3dNowExt() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::edx, 1u<<30, 30, CpuVendor::amd); }
        constexpr inline CpuidPropertyLocation amd3dNow() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::extendedFeatures, CpuRegisterPart::edx, 1u<<31, 31, CpuVendor::amd); }

        constexpr inline CpuidPropertyLocation multiCoresInfo(){ return CpuidPropertyLocation((int32_t)RegisterId::cache, CpuRegisterPart::eax, 0xFC000000, 26, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation amdCoresInfo() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::vendorReserved, CpuRegisterPart::ecx, 0x000000FF, 0, CpuVendor::amd); }

        constexpr inline int32_t _maxIntelTopologyIndex() noexcept { return 4; }
        constexpr inline CpuidPropertyLocation intelTopologyType() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::extendedTopology, CpuRegisterPart::ecx, 0x0000FF00, 8, CpuVendor::intel); }
        constexpr inline CpuidPropertyLocation intelTopologyValue() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::extendedTopology, CpuRegisterPart::ebx, 0x0000FFFF, 0, CpuVendor::intel); }
        
        /// @brief Type of topology value for Intel x86 CPU
        enum class IntelTopologyType {
          unknown = 0,
          threadsPerCore = 0x01,
          logicalCpuCount = 0x02
        };
      }

#   elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
      // -- arm / arm64 --
      namespace cpuid_arm {
        // -- register IDs --
    
        /// @brief Register identifiers for ARM CPUID
        enum class RegisterId : int32_t {
          id    = 0,
          cache = 1
        };

        // -- register properties --

        constexpr inline CpuidPropertyLocation id() noexcept          { return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0xFFFFFFFFuLL, 0); }
        constexpr inline CpuidPropertyLocation implementer() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0xFF000000uLL, 24); }
        constexpr inline CpuidPropertyLocation variant() noexcept     { return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0x00F00000uLL, 20); }
        constexpr inline CpuidPropertyLocation architecture() noexcept{ return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0x000F0000uLL, 16); }
        constexpr inline CpuidPropertyLocation modelNumber() noexcept { return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0xFF00FFF0uLL, 0); }
        constexpr inline CpuidPropertyLocation partNumber() noexcept  { return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0x0000FFF0uLL, 4); }
        constexpr inline CpuidPropertyLocation revision() noexcept    { return CpuidPropertyLocation((int32_t)RegisterId::id, CpuRegisterPart::eax, 0x0000000FuLL, 0); }

        /// @brief Model number of ARM CPUs
        enum class ModelNumber: uint32_t {
          arm1136     = 0x4100b360,
          arm1156     = 0x4100b560,
          arm1176     = 0x4100b760,
          arm11mpCore = 0x4100b020,
          cortex_A8   = 0x4100c080,
          cortex_A9   = 0x4100c090,
          cortex_A5   = 0x4100c050,
          cortex_A7   = 0x4100c070,
          cortex_A12  = 0x4100c0d0,
          cortex_A17  = 0x4100c0e0,
          cortex_A15  = 0x4100c0f0,
          cortex_A53  = 0x4100d030,
          cortex_A57  = 0x4100d070,
          cortex_A72  = 0x4100d080,
          cortex_A73  = 0x4100d090,
          cortex_A75  = 0x4100d0a0,
          brahma_B15  = 0x420000f0,
          brahma_B53  = 0x42001000,
          sa1100      = 0x4400a110,
          sa1110      = 0x6900b110,
          snapDragon  = 0x51002110,
          scorpion    = 0x510002d0
        };
      }
    
      /// @brief Convert model number of ARM CPUs to model brand string
      inline std::string toString(cpuid_arm::ModelNumber part) {
        switch (part) {
          case cpuid_arm::ModelNumber::arm1136:    return "ARM(R) ARM1136";
          case cpuid_arm::ModelNumber::arm1156:    return "ARM(R) ARM1156";
          case cpuid_arm::ModelNumber::arm1176:    return "ARM(R) ARM1176";
          case cpuid_arm::ModelNumber::arm11mpCore:return "ARM(R) ARM11 MPCore";
          case cpuid_arm::ModelNumber::cortex_A5:  return "ARM(R) Cortex-A5";
          case cpuid_arm::ModelNumber::cortex_A7:  return "ARM(R) Cortex-A7";
          case cpuid_arm::ModelNumber::cortex_A8:  return "ARM(R) Cortex-A8";
          case cpuid_arm::ModelNumber::cortex_A9:  return "ARM(R) Cortex-A9";
          case cpuid_arm::ModelNumber::cortex_A12: return "ARM(R) Cortex-A12";
          case cpuid_arm::ModelNumber::cortex_A15: return "ARM(R) Cortex-A15";
          case cpuid_arm::ModelNumber::cortex_A17: return "ARM(R) Cortex-A17";
          case cpuid_arm::ModelNumber::cortex_A53: return "ARM(R) Cortex-A53";
          case cpuid_arm::ModelNumber::cortex_A57: return "ARM(R) Cortex-A57";
          case cpuid_arm::ModelNumber::cortex_A72: return "ARM(R) Cortex-A72";
          case cpuid_arm::ModelNumber::cortex_A73: return "ARM(R) Cortex-A73";
          case cpuid_arm::ModelNumber::cortex_A75: return "ARM(R) Cortex-A75";
          case cpuid_arm::ModelNumber::brahma_B15: return "Broadcom(R) Brahma-B15";
          case cpuid_arm::ModelNumber::brahma_B53: return "Broadcom(R) Brahma-B53";
          case cpuid_arm::ModelNumber::sa1100:     return "Intel(R) StrongARM(R) SA-1100";
          case cpuid_arm::ModelNumber::sa1110:     return "Intel(R) StrongARM(R) SA-1110";
          case cpuid_arm::ModelNumber::snapDragon: return "Qualcomm(R) SnapDragon(R)";
          case cpuid_arm::ModelNumber::scorpion:   return "Qualcomm(R) Scorpion(R)";
          default: return "";
        }
      }
#   endif
    
  }
}
