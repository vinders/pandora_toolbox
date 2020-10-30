/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
CPU core specs detection
Types: CpuCoreInfo
Functions: _isHyperThreadingSupported, _readCpuCoresFromOS, [_readCpuCoresFromCpuid](only x86)
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstring>
#include <string>

# if defined(_WINDOWS)
#   include <system/api/windows_api.h>
# elif defined(__APPLE__)
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <system/operating_system.h>
# elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__)
#  include <sys/sysinfo.h>
#  include <unistd.h>
# endif
# include <system/cpu_arch.h>
# include <system/trace.h>
# include "hardware/cpuid_registers.h"
# include "./_cpuid_register_reader.h"

namespace pandora {
  namespace hardware {
    // CPU core description
    struct CpuCoreInfo {
      uint32_t physicalCores;
      uint32_t logicalCores;
      constexpr inline bool isHyperThreadingCapable() const noexcept { return (logicalCores > physicalCores); }
    };

    // ---

    // Verify hyper-threading capability of current system
    inline bool _isHyperThreadingSupported(CpuVendor vendorType) noexcept {
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        CpuRegister128 buffer;
        if (CpuidRegisterReader::hasRegisterBits(buffer, cpuid_x86::hyperThreading())) {
          if (vendorType != CpuVendor::amd)
            return true;

          int32_t amdFamily = static_cast<int32_t>(CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::baseFamily()))
                         + static_cast<int32_t>(CpuidRegisterReader::extractRegisterBits(buffer, cpuid_x86::extendedFamily()));
          if (amdFamily >= 0x17)
            return true;

          int32_t extendedSize = static_cast<int32_t>(CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::extendedSize()));
          return (extendedSize >= cpuid_x86::amdCmpLegacy().registerId()
                && CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::amdCmpLegacy()) == 1u);
        }
#     endif
      return false;
    }

    // Count CPU physical cores using system API
    inline CpuCoreInfo _readCpuCoresFromOS(CpuVendor vendorType) noexcept {
      TRACE("CPU cores: call to _readCpuCoresFromSystem()");
      CpuCoreInfo coresCount{ 1u, 1u };
      
#     if defined(_WINDOWS)
        SYSTEM_INFO systemInfo;
        systemInfo.dwNumberOfProcessors = 0u;
        GetSystemInfo(&systemInfo);
        if (systemInfo.dwNumberOfProcessors != 0u && systemInfo.dwNumberOfProcessors != 0xFFFFFFFFu)
          coresCount.logicalCores = static_cast<uint32_t>(systemInfo.dwNumberOfProcessors);

#     elif defined __APPLE__
        int32_t buffer = 0u;
        size_t bufferSize = sizeof(buffer);
          
#       if _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_IOS
          sysctlbyname("hw.ncpu", &buffer, &bufferSize, nullptr, 0);
#       elif _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_MAC_OSX
          int32_t controlId[2] { CTL_HW, HW_AVAILCPU };
          sysctl(controlId, 2, &buffer, &bufferSize, nullptr, 0);
          if (buffer < 1) {
            controlId[1] = HW_NCPU;
            sysctl(controlId, 2, &buffer, &bufferSize, nullptr, 0);
          }
#       endif
        if (buffer > 1)
          coresCount.logicalCores = static_cast<uint32_t>(buffer);

#     elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__) // linux
        int32_t buffer = get_nprocs();
        if (buffer > 1)
          coresCount.logicalCores = static_cast<uint32_t>(buffer);
        else { // error (0 or -1) or supposedly 1
          coresCount.logicalCores = sysconf(_SC_NPROCESSORS_ONLN);
          if (coresCount.logicalCores < 1u)
            coresCount.logicalCores = 1u;
        }
#     endif

      coresCount.physicalCores = (coresCount.logicalCores > 1u && _isHyperThreadingSupported(vendorType)) ? coresCount.logicalCores / 2u : coresCount.logicalCores;
      return coresCount;
    }
    
    // -- architecture specific functions --

#   if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
      inline bool __readIntelCoreTopology(CpuRegister128& buffer, int32_t maxBaseRegisterId, CpuCoreInfo& outCoreInfo) noexcept {
        if (maxBaseRegisterId >= cpuid_x86::intelTopologyType().registerId()) {
          TRACE("CPU cores: call to __readIntelCoreTopology");
          uint32_t threadsPerCore = 0u;
          uint32_t logicalCores = 0u;
          
          for (int index = 0; index < cpuid_x86::_maxIntelTopologyIndex(); ++index) {
            CpuidRegisterReader::fillCpuid(buffer, static_cast<cpuid_x86::RegisterId>(cpuid_x86::intelTopologyType().registerId()), index);
            cpuid_x86::IntelTopologyType topologyType = static_cast<cpuid_x86::IntelTopologyType>(
                                         CpuidRegisterReader::extractRegisterBits(buffer, cpuid_x86::intelTopologyType()) );

            switch (topologyType) {
              case cpuid_x86::IntelTopologyType::threadsPerCore:
                threadsPerCore = CpuidRegisterReader::extractRegisterBits(buffer, cpuid_x86::intelTopologyValue()); 
                break;
              case cpuid_x86::IntelTopologyType::logicalCpuCount: 
                logicalCores = CpuidRegisterReader::extractRegisterBits(buffer, cpuid_x86::intelTopologyValue()); 
                break;
              default: break;
            }
          }
          
          if (logicalCores && threadsPerCore) {
            outCoreInfo.physicalCores = (outCoreInfo.logicalCores / threadsPerCore);
            outCoreInfo.logicalCores = logicalCores;
            return true;
          }
        }
        return false;
      }

      // Count physical/logical cores on x86 CPU
      inline CpuCoreInfo _readCpuCoresFromCpuid(int32_t maxBaseRegisterId, int32_t maxExtendedRegisterId, CpuVendor vendorType) noexcept {
        CpuRegister128 buffer{ 0 };
        CpuCoreInfo coresCount{ 1u, 1u };

        // intel
        if (vendorType == CpuVendor::intel) {
          // modern intel topology
          if (__readIntelCoreTopology(buffer, maxBaseRegisterId, coresCount))
            return coresCount;

          // topology not available -> legacy
          if (maxBaseRegisterId >= cpuid_x86::multiCoresInfo().registerId()) {
            TRACE("CPU cores: read intel legacy core info");
            coresCount.physicalCores = (CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::multiCoresInfo()) + 1u);
            coresCount.logicalCores = CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::logicalProcessorsPerCpu());
            if (coresCount.physicalCores >= coresCount.logicalCores)
              coresCount.logicalCores = (_isHyperThreadingSupported(vendorType)) ? coresCount.physicalCores * 2u : coresCount.physicalCores;
          }
        }
        // AMD
        else if (vendorType == CpuVendor::amd) {
          if (maxExtendedRegisterId >= cpuid_x86::amdCoresInfo().registerId()) {
            TRACE("CPU cores: read amd core info");
            coresCount.physicalCores = (CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::amdCoresInfo()) + 1u);
            coresCount.logicalCores = CpuidRegisterReader::readRegisterBits(buffer, cpuid_x86::logicalProcessorsPerCpu());
            if (coresCount.physicalCores >= coresCount.logicalCores) {
              coresCount.logicalCores = coresCount.physicalCores;
              if (coresCount.logicalCores > 1u && _isHyperThreadingSupported(vendorType))
                coresCount.physicalCores = coresCount.logicalCores / 2u;
            }
          }
        }

        // logical cores based on hyperthreading capability
        
        return coresCount;
      }
#   endif

  }
}
