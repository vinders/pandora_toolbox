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
