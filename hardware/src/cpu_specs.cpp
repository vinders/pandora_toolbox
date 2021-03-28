/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cstddef>
#include <cstring>
#include <string>

# include <system/cpu_arch.h>
#if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
# include <sstream>
# include <ios>
#endif
# include "hardware/cpuid_registers.h"
# include "hardware/_private/_cpuid_register_reader.h"
# include "hardware/_private/_cpu_cores_specs.h"

#include "hardware/cpu_specs.h"

using namespace pandora::hardware;


#if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
  // -- x86 / x86_64 --
  
  // Convert raw CPUID register value to vendor string
  static inline std::string _toCpuVendorString(const CpuRegister128& rawValue) {
    char vendor[3*sizeof(uint32_t) + 1];
    memset(vendor, 0, sizeof(vendor));
    uint32_t* cursor = reinterpret_cast<uint32_t*>(vendor);

    *cursor = rawValue[static_cast<uint32_t>(cpuid_x86::vendorPart1().index())];
    ++cursor;
    *cursor = rawValue[static_cast<uint32_t>(cpuid_x86::vendorPart2().index())];
    ++cursor;
    *cursor = rawValue[static_cast<uint32_t>(cpuid_x86::vendorPart3().index())];
    return std::string(vendor);
  }
  // Read brand name in CPUID register  and convert raw value to brand string
  static inline std::string _readCpuBrandString(int32_t maxExtendedRegisterId) {
    char brand[3*4*sizeof(uint32_t) + 1];
    memset(brand, 0, sizeof(brand));
      
    if (maxExtendedRegisterId >= static_cast<int32_t>(cpuid_x86::RegisterId::brand3)) {
      CpuRegister128 buffer{ 0 };
      CpuidRegisterReader::fillCpuid(buffer, cpuid_x86::RegisterId::brand1, 0);
      memcpy(reinterpret_cast<void*>(brand), reinterpret_cast<void*>(buffer.data()), 4*sizeof(uint32_t));

      CpuidRegisterReader::fillCpuid(buffer, cpuid_x86::RegisterId::brand2, 0);
      memcpy(reinterpret_cast<void*>(brand + 4*sizeof(uint32_t)), reinterpret_cast<void*>(buffer.data()), 4*sizeof(uint32_t));

      CpuidRegisterReader::fillCpuid(buffer, cpuid_x86::RegisterId::brand3, 0);
      memcpy(reinterpret_cast<void*>(brand + 8*sizeof(uint32_t)), reinterpret_cast<void*>(buffer.data()), 4*sizeof(uint32_t));
    }
    return std::string(brand);
  }

  // Process CPU specifications detection (x86)
  CpuSpecs::CpuSpecs(CpuSpecs::SpecMode mode) {
    CpuRegister128 buffer{ 0 };
    CpuidRegisterReader::fillCpuid(buffer, cpuid_x86::RegisterId::sizeAndVendor);
    this->_maxBaseRegisterId = CpuidRegisterReader::extractRegisterBits(buffer, cpuid_x86::baseSize()); // number of base register IDs
    
    if ((mode & SpecMode::vendor) == true || (mode & SpecMode::cores) == true) { // vendor info required for cores count too
      this->_vendor = _toCpuVendorString(buffer);
      
      if (this->_vendor == toString(CpuVendor::intel))
        this->_vendorId = CpuVendor::intel;
      else if (this->_vendor == toString(CpuVendor::amd))
        this->_vendorId = CpuVendor::amd;
    }
    
    CpuidRegisterReader::fillCpuid(buffer, cpuid_x86::RegisterId::extendedSize, 0);
    this->_maxExtendedRegisterId = CpuidRegisterReader::extractRegisterBits(buffer, cpuid_x86::extendedSize()); // number of extended register IDs
    
    if ((mode & SpecMode::brandName) == true) {
      this->_brand = _readCpuBrandString(this->_maxExtendedRegisterId);
    }

    if ((mode & SpecMode::cores) == true) {
      auto cpuCores = _readCpuCoresFromCpuid(this->_maxBaseRegisterId, this->_maxExtendedRegisterId, this->_vendorId);
      if (cpuCores.physicalCores <= 1u)
        cpuCores = _readCpuCoresFromOS(this->_vendorId);
      this->_physicalCores = cpuCores.physicalCores;
      this->_logicalCores = cpuCores.logicalCores;
    }
    
    if ((mode & SpecMode::registers) == true) {
      if (this->_maxBaseRegisterId >= static_cast<int32_t>(cpuid_x86::RegisterId::baseFeatures)) {
        CpuidRegisterReader::fillCpuid(this->_regBase, cpuid_x86::RegisterId::baseFeatures);
        
        if (this->_maxBaseRegisterId >= static_cast<int32_t>(cpuid_x86::RegisterId::advancedFeatures))
          CpuidRegisterReader::fillCpuid(this->_regAdvanced, cpuid_x86::RegisterId::advancedFeatures);
      }
      if (this->_maxExtendedRegisterId >= static_cast<int32_t>(cpuid_x86::RegisterId::extendedFeatures))
        CpuidRegisterReader::fillCpuid(this->_regExtended, cpuid_x86::RegisterId::extendedFeatures, 0);
    }
  }

  uint32_t CpuSpecs::getProperty(const CpuidPropertyLocation& prop) const noexcept {
    switch (static_cast<cpuid_x86::RegisterId>(prop.registerId())) {
      case cpuid_x86::RegisterId::baseFeatures:     return static_cast<uint32_t>(hasBaseProperty(prop));
      case cpuid_x86::RegisterId::advancedFeatures: return static_cast<uint32_t>(hasAdvancedProperty(prop));
      case cpuid_x86::RegisterId::extendedFeatures: return static_cast<uint32_t>(hasExtendedProperty(prop));
      default: return (prop.exclusivity() == CpuVendor::unknown || prop.exclusivity() == this->_vendorId)
                      ? CpuidRegisterReader::readRegisterBits(prop) : 0u;
    }
  }

  // Verify CPU instruction set support
  bool CpuSpecs::isInstructionSetSupported(CpuInstructionSet type) const noexcept {
    switch (type) {
      case CpuInstructionSet::cpp:    return true;
      case CpuInstructionSet::mmx:    return hasBaseProperty(cpuid_x86::mmx());
      case CpuInstructionSet::sse:    return hasBaseProperty(cpuid_x86::sse());
      case CpuInstructionSet::sse2:   return hasBaseProperty(cpuid_x86::sse2());
      case CpuInstructionSet::sse3:   return hasBaseProperty(cpuid_x86::sse3());
      case CpuInstructionSet::ssse3:  return hasBaseProperty(cpuid_x86::ssse3());
      case CpuInstructionSet::sse4_1: return hasBaseProperty(cpuid_x86::sse41());
      case CpuInstructionSet::sse4_2: return hasBaseProperty(cpuid_x86::sse42());
      case CpuInstructionSet::avx:    return (hasBaseProperty(cpuid_x86::avx())
                                           && hasBaseProperty(cpuid_x86::osExtendedSave())
                                           && (CpuidRegisterReader::readExtendedControlRegister(0) & 0x6uLL) == 0x6uLL);
      case CpuInstructionSet::avx2:    return hasAdvancedProperty(cpuid_x86::avx2());
      case CpuInstructionSet::avx512f: return hasAdvancedProperty(cpuid_x86::avx512f());
      default: return false;
    }
  }


#elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
  // -- arm / arm64 --

  static inline std::string _toHexString(uint32_t value) noexcept {
    std::stringstream formatter;
    formatter << std::hex << value;
    return formatter.str(); 
  }
   
  // Process CPU specifications detection (ARM)
  CpuSpecs::CpuSpecs(SpecMode mode) {
    CpuRegister64 cpuInfo = CpuidRegisterReader::readCpuid(static_cast<cpuid_arm::RegisterId>(cpuid_arm::id().registerId()));
    
    if ((mode & SpecMode::vendor) == true || (mode & SpecMode::cores) == true) { // vendor info required for cores count too
      switch (CpuidRegisterReader::readRegisterBits(cpuInfo, cpuid_arm::implementer())) {
        case 0x41: this->_vendorId = CpuVendor::armLtd; break;
        case 0x42: this->_vendorId = CpuVendor::broadcom; break;
        case 0x43: this->_vendorId = CpuVendor::cavium; break;
        case 0x44: this->_vendorId = CpuVendor::dec; break;
        case 0x46: this->_vendorId = CpuVendor::fujitsu; break;
        case 0x49: this->_vendorId = CpuVendor::infineon; break;
        case 0x4D: this->_vendorId = CpuVendor::motorola; break;
        case 0x4E: this->_vendorId = CpuVendor::nvidia; break;
        case 0x50: this->_vendorId = CpuVendor::amcc; break;
        case 0x51: this->_vendorId = CpuVendor::qualcomm; break;
        case 0x56: this->_vendorId = CpuVendor::marvell; break;
        case 0x69: this->_vendorId = CpuVendor::intel; break;
        case 0xC0: this->_vendorId = CpuVendor::ampere; break;
        default: this->_vendorId = CpuVendor::unknown; break;
      }
        
#     if !defined(_WINDOWS) && !defined(__APPLE__) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__))
        this->_vendor = CpuidRegisterReader::readCpuInfoFile("vendor_id");
        if (this->_vendor.empty())
#     endif
      {
        if (this->_vendorId != CpuVendor::unknown)
          this->_vendor = toString(this->_vendorId);
        else
          this->_vendor = _toHexString(CpuidRegisterReader::extractRegisterBits(cpuInfo, cpuid_arm::implementer()));
      }
    }
    
    if ((mode & SpecMode::brandName) == true) {
#     if !defined(_WINDOWS) && !defined(__APPLE__) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__))
        this->_brand = CpuidRegisterReader::readCpuInfoFile("model name");
        if (this->_brand.empty())
#     endif
      {
        this->_brand = toString(static_cast<cpuid_arm::ModelNumber>( CpuidRegisterReader::readRegisterBits(cpuInfo, cpuid_arm::modelNumber()) ));
        if (this->_brand.empty())
          this->_brand = _toHexString(CpuidRegisterReader::extractRegisterBits(cpuInfo, cpuid_arm::partNumber()));
      }
    }
    
    if ((mode & SpecMode::cores) == true) {
      auto cpuCores = _readCpuCoresFromOS(this->_vendorId);
      this->_physicalCores = cpuCores.physicalCores;
      this->_logicalCores = cpuCores.logicalCores;
    }
    
    if ((mode & SpecMode::registers) == true) {
      this->_regBase = cpuInfo;
    }
  }

# if !defined(_WINDOWS) && !defined(__APPLE__) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__))
    static inline bool _hasCpuNeonOnLinux() noexcept { return CpuidRegisterReader::readAuxVectorsFile(AT_HWCAP, 4096); }
# endif

  uint64_t CpuSpecs::getProperty(const CpuidPropertyLocation& prop) const noexcept {
    switch (static_cast<cpuid_arm::RegisterId>(prop.registerId())) {
      case cpuid_arm::RegisterId::id:
        return ((this->_regBase & prop.mask()) >> prop.offset());
      default:
        if (prop.exclusivity() == CpuVendor::unknown || prop.exclusivity() == this->_vendorId)
          return CpuidRegisterReader::readRegisterBits(prop);
        break;
    }
    return 0u;
  }

  // Verify CPU instruction set support
  bool CpuSpecs::isInstructionSetSupported(CpuInstructionSet type) const noexcept {
#   if defined(_MSC_VER) || defined(__MINGW32__) || defined(_WINDOWS)
#     if defined(_M_ARM64)
      return (type == CpuInstructionSet::neon64 || type == CpuInstructionSet::neon || type == CpuInstructionSet::cpp);
#     elif defined(_M_ARM)
      return (type == CpuInstructionSet::neon || type == CpuInstructionSet::cpp);
#     else
      return (type == CpuInstructionSet::cpp);
#     endif
#   elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__) || defined(__APPLE__)
#     if defined(__aarch64__)
        return (type == CpuInstructionSet::neon64 || type == CpuInstructionSet::neon || type == CpuInstructionSet::cpp);
#     elif defined(__clang__)
#       if defined(__ARM_NEON__)
          return (type == CpuInstructionSet::neon || type == CpuInstructionSet::cpp);
#       else
          return (type == CpuInstructionSet::cpp);
#       endif
#     else
        if (type == CpuInstructionSet::neon)
          return _hasCpuNeonOnLinux();
        return (type == CpuInstructionSet::cpp);
#     endif
#   else
      return (type == CpuInstructionSet::cpp);
#   endif
  }

#else
  // -- unknown --

  uint32_t CpuSpecs::getProperty(const CpuidPropertyLocation& prop) const noexcept { return 0u; }
  bool CpuSpecs::isInstructionSetSupported(CpuInstructionSet type) const noexcept { return (type == CpuInstructionSet::cpp); }
#endif
