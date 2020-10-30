/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#if !defined(_WINDOWS) && !defined(__APPLE__) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__))
# include <cstdio>
# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>
# include <elf.h>
#endif
#include <system/cpu_arch.h>
#include "hardware/cpuid_property_location.h"
#include "hardware/cpuid_registers.h"
#if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
# include <system/api/intrinsics.h>
#endif

namespace pandora {
  namespace hardware {
    /// @class CpuidRegisterReader
    /// @brief Low-level utility to read the value of a CPUID register (or CPU info file, on Linux)
    class CpuidRegisterReader final {
    public:
      CpuidRegisterReader() = delete;

      // -- CPUID registers --
      
#     if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
        /// @brief Read register value of a CPUID register - buffer returned
        static inline CpuRegister128 readCpuid(cpuid_x86::RegisterId registerId, int32_t extendedIndex = 0) noexcept {
          CpuRegister128 output{ 0 };
          CpuidRegisterReader::fillCpuid(output, registerId, extendedIndex);
          return output;
        }
        /// @brief Read register value of a CPUID register - preallocated buffer
        static inline void fillCpuid(CpuRegister128& buffer, cpuid_x86::RegisterId registerId, int32_t extendedIndex = 0) noexcept {
#         if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINNT)
            __cpuidex(reinterpret_cast<int32_t*>(buffer.data()), static_cast<int32_t>(registerId), extendedIndex);
#         else
            auto outputRef = reinterpret_cast<int32_t*>(buffer.data());
            __asm__ __volatile__(
              "cpuid;" 
              : "=a" (outputRef[0]), "=b" (outputRef[1]), "=c" (outputRef[2]), "=d" (outputRef[3])
              : "a" (registerId), "c" (extendedIndex)
            );
#         endif
        }

        /// @brief Read value of the XGETBV extended control register
        static inline uint64_t readExtendedControlRegister(uint32_t index) noexcept {
#         if defined(_WINDOWS) && defined(_MSC_VER)
            return _xgetbv(index);
#         else
            uint32_t eax, edx;
            __asm__ __volatile__(
              "xgetbv;"
              :"=a" (eax), "=d"(edx)
              : "c" (index)
            );
            return ((static_cast<uint64_t>(edx) << 32u) | static_cast<uint64_t>(eax));
#         endif
        }

        /// @brief Extract a value from a pre-read 128-bit register
        static inline uint32_t extractRegisterBits(const CpuRegister128& rawValue, const CpuidPropertyLocation& location) noexcept {
          return ( (rawValue[static_cast<uint32_t>(location.index())] & location.mask()) >> location.offset());
        }
        /// @brief Read CPUID register and extract value bits - preallocated buffer
        static inline uint32_t readRegisterBits(CpuRegister128& buffer, const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept {
          CpuidRegisterReader::fillCpuid(buffer, static_cast<cpuid_x86::RegisterId>(location.registerId()), extendedIndex);
          return CpuidRegisterReader::extractRegisterBits(buffer, location);
        }
        /// @brief Read CPUID register and verify value bits - preallocated buffer
        static inline bool hasRegisterBits(CpuRegister128& buffer, const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept {
          CpuidRegisterReader::fillCpuid(buffer, static_cast<cpuid_x86::RegisterId>(location.registerId()), extendedIndex);
          return ((buffer[static_cast<uint32_t>(location.index())] & location.mask()) != 0u);
        }
        /// @brief Read CPUID register and verify value bits
        static inline bool hasRegisterBits(const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept {
          CpuRegister128 buffer = CpuRegister128{ 0 };
          return CpuidRegisterReader::hasRegisterBits(buffer, location, extendedIndex);
        }
        /// @brief Read CPUID register and extract value bits
        static inline uint32_t readRegisterBits(const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept {
          return CpuidRegisterReader::extractRegisterBits(
                   CpuidRegisterReader::readCpuid(static_cast<cpuid_x86::RegisterId>(location.registerId()), extendedIndex), location);
        }

#     elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
        /// @brief Read register value of a CPUID register - buffer returned
        static inline CpuRegister64 readCpuid(cpuid_arm::RegisterId registerId) noexcept {
          CpuRegister64 output{ 0 };
          CpuidRegisterReader::fillCpuid(output, registerId);
          return std::move(output);
        }
        /// @brief Read value of a CPUID register - preallocated buffer
        static void fillCpuid(CpuRegister64& buffer, cpuid_arm::RegisterId registerId) noexcept {
#         if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__MINGW32__)
#           if (defined(__ARM_ARCH) && __ARM_ARCH < 8) || (!defined(__ARM_ARCH) && (defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_5__)) )
              uint32_t regValue = 0;
              switch (registerId) {
                case cpuid_arm::RegisterId::id:    { __asm__ volatile("mrc p15, 0, %0, c0, c0, 0"
                                                                     :"=r"(regValue) : ); break; }
                case cpuid_arm::RegisterId::cache: { __asm__ volatile("mrc p15, 0, %0, c0, c0, 1"
                                                                     :"=r"(regValue) : ); break; }
                default: break;
              }
#           else
              uint64_t regValue = 0;
              switch (registerId) {
                case cpuid_arm::RegisterId::id:    { __asm__ volatile("mrs %0, MIDR_EL1"
                                                                     :"=r"(regValue) : ); break; }
                case cpuid_arm::RegisterId::cache: { __asm__ volatile("mrs %0, CTR_EL0"
                                                                     :"=r"(regValue) : ); break; }
                default: break;
              }
#           endif
            buffer = static_cast<CpuRegister64>(regValue);
#         elif defined(_MSC_VER)
            buffer = static_cast<CpuRegister64>(0x51000000u); // if windows ARM, supposedly qualcomm CPU
#         else
            buffer = static_cast<CpuRegister64>(0u);
#         endif
        }

        /// @brief Extract a value from a pre-read 32-bit register
        static inline uint32_t extractRegisterBits(CpuRegister64 rawValue, const CpuidPropertyLocation& location) noexcept {
          return static_cast<uint32_t>( (static_cast<uint64_t>(rawValue) & location.mask()) >> location.offset());
        }
        /// @brief Read CPUID register and extract value bits - preallocated buffer
        static inline uint32_t readRegisterBits(CpuRegister64& buffer, const CpuidPropertyLocation& location) noexcept {
          CpuidRegisterReader::fillCpuid(buffer, static_cast<cpuid_arm::RegisterId>(location.registerId()));
          return CpuidRegisterReader::extractRegisterBits(buffer, location);
        }
        /// @brief Read CPUID register and verify value bits - preallocated buffer
        static inline bool hasRegisterBits(CpuRegister64& buffer, const CpuidPropertyLocation& location) noexcept {
          CpuidRegisterReader::fillCpuid(buffer, static_cast<cpuid_arm::RegisterId>(location.registerId()));
          return ((static_cast<uint64_t>(buffer) & location.mask()) != 0u);
        }
        /// @brief Read CPUID register and verify value bits
        static inline bool hasRegisterBits(const CpuidPropertyLocation& location) noexcept {
          CpuRegister64 buffer{ 0 };
          return CpuidRegisterReader::hasRegisterBits(buffer, location);
        }
        /// @brief Read CPUID register and extract value bits
        static inline uint32_t readRegisterBits(const CpuidPropertyLocation& location) noexcept {
          return CpuidRegisterReader::extractRegisterBits(
                   CpuidRegisterReader::readCpuid(static_cast<cpuid_arm::RegisterId>(location.registerId())), location);
        }
 
#     else // unknown system
        template <typename _CpuRegister, typename _CpuRegisterId>
        static inline _CpuRegister readCpuid(_CpuRegisterId registerId, int32_t extendedIndex = 0) noexcept { return _CpuRegister{ 0 }; };
        template <typename _CpuRegister, typename _CpuRegisterId>
        static inline void fillCpuid(_CpuRegister& buffer, _CpuRegisterId registerId, int32_t extendedIndex = 0) noexcept { buffer = static_cast<CpuRegister32>(0u); }
        template <typename _CpuRegister>
        static inline uint32_t extractRegisterBits(_CpuRegister& rawValue, const CpuidPropertyLocation& location) noexcept { return 0u; }
        template <typename _CpuRegister>
        static inline uint32_t readRegisterBits(_CpuRegister& buffer, const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept { return 0u; }
        template <typename _CpuRegister>
        static inline bool hasRegisterBits(_CpuRegister& buffer, const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept { return false; }
        template <typename _CpuRegister>
        static inline bool hasRegisterBits(const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept { return false; }
        static inline uint32_t readRegisterBits(const CpuidPropertyLocation& location, int32_t extendedIndex = 0) noexcept { return 0u; }
#     endif

      // -- CPU info files (Linux) --
      
#     if !defined(_WINDOWS) && !defined(__APPLE__) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__))
        /// @brief Read value in CPU info file (Linux)
        static std::string readCpuInfoFile(const std::string& prop) noexcept {
          std::string result;
          FILE* cpuInfoFile = fopen("/proc/cpuinfo", "r");
          if (cpuInfoFile != nullptr) {
            char* line = nullptr;
            size_t len = 0;
            ssize_t readBytes;

            while (result.empty() && (readBytes = getline(&line, &len, cpuInfoFile)) != -1) {
              if (readBytes > 0 && line != nullptr) {
                std::string lineString(line);
                if (lineString.find(prop)) {
                  auto position = lineString.find(": ");
                  if (position != std::string::npos && position + 1 < lineString.size())
                    result = lineString.substr(position + 1);
                }

                free(line);
                line = nullptr;
              }
            }
            fclose(cpuInfoFile);
          }
          return result;
        }

        /// @brief Read value in aux vectors file (Linux)
        static bool readAuxVectorsFile(uint32_t auxVecType, uint32_t bits) noexcept {
          bool result = false;
          
          auto auxvFile = open("/proc/self/auxv", O_RDONLY);
          if (auxvFile > -1) {
            Elf32_auxv_t auxvTuple;
            while (read(auxvFile, &auxvTuple, sizeof(auxvTuple)) == sizeof(auxvTuple)) {
              if (auxvTuple.a_type == auxVecType) {
                result = ((auxvTuple.a_un.a_val & bits) != 0);
                break;
              }
            }
            close(auxvFile);
          }
          return result;
        }
#     endif
    };
    
  }
}
