/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
CPU vendor identifiers + string labels
*******************************************************************************/
#pragma once

#include <cstdint>
#include <string>

namespace pandora {
  namespace hardware {
    /// @brief CPU vendor identifier
    enum class CpuVendor : uint32_t {
      unknown  = 0,
      intel,
      amd,
      armLtd,
      broadcom,
      dec,
      ampere,
      cavium,
      fujitsu,
      infineon,
      motorola,
      nvidia,
      amcc,
      qualcomm,
      marvell
    };
    /// @brief Convert CPU vendor enumeration ID to vendor string
    inline std::string toString(CpuVendor vendor) {
      switch (vendor) {
        case CpuVendor::intel:    return "GenuineIntel";
        case CpuVendor::amd:      return "AuthenticAMD";
        case CpuVendor::armLtd:   return "ARM Limited";
        case CpuVendor::broadcom: return "Broadcom Corporation";
        case CpuVendor::dec:      return "Digital Equipment Corporation";
        case CpuVendor::ampere:   return "Ampere Computing";
        case CpuVendor::cavium:   return "Cavium Inc";
        case CpuVendor::fujitsu:  return "Fujitsu Ltd";
        case CpuVendor::infineon: return "Infineon Technologies AG";
        case CpuVendor::motorola: return "Motorola";
        case CpuVendor::nvidia:   return "NVIDIA Corporation";
        case CpuVendor::amcc:     return "Applied Micro Circuits Corporation";
        case CpuVendor::qualcomm: return "Qualcomm Inc";
        case CpuVendor::marvell:  return "Marvell International Ltd";
        case CpuVendor::unknown:
        default: return "";
      }
    }

  }
}
