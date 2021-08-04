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
 Endianness conversion utils
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>

namespace pandora {
  namespace memory {
#   pragma pack(push, 8)
    struct Int80 final { ///< 80-bit integer representation
      uint64_t high;
      uint16_t low;
    };
    struct Int128 final { ///< 128-bit integer representation
      uint64_t high;
      uint64_t low;
    };
#   pragma pack(pop)

    template <typename _ByteType, typename _FloatType>
    union swappableFloat { ///< byte-swappable buffer for floating point types
      _ByteType bytes;
      _FloatType value;
    };

    // -- swap integer types --

    /// @brief Convert little/big endian 16-bit integers
    constexpr inline uint16_t swapEndianInt16(uint16_t value) noexcept {
      return ((value >> 8) | (value << 8));
    }
    /// @brief Convert little/big endian 32-bit integers
    constexpr inline uint32_t swapEndianInt32(uint32_t value) noexcept {
      return ( (value >> 24) | (value << 24)
             | ((value >> 8) & 0xFF00u) | ((value & 0xFF00u) << 8) );
    }
    /// @brief Convert little/big endian 64-bit integers
    constexpr inline uint64_t swapEndianInt64(uint64_t value) noexcept {
      return ( (value >> 56) | (value << 56)
             | ((value >> 40) & 0xFF00uLL) | ((value & 0xFF00uLL) << 40)
             | ((value >> 24) & 0xFF0000uLL) | ((value & 0xFF0000uLL) << 24)
             | ((value >> 8) & 0xFF000000uLL) | ((value & 0xFF000000uLL) << 8) );
    }

    /// @brief Convert little/big endian 80-bit integers buffers
    inline void swapEndianInt80(const Int80& value, Int80& out) noexcept {
      out.high = ( static_cast<uint64_t>(swapEndianInt16(value.low)) << 48
                 | ((value.high >> 40) & 0xFFuLL) | ((value.high & 0xFFuLL) << 40)
                 | ((value.high >> 24) & 0xFF00uLL) | ((value.high & 0xFF00uLL) << 24)
                 | ((value.high >> 8) & 0xFF0000uLL) | ((value.high & 0xFF0000uLL) << 8) );
      out.low = swapEndianInt16(static_cast<uint16_t>(value.high >> 48));
    }
    /// @brief Convert little/big endian 80-bit integers buffers
    inline Int80 swapEndianInt80(const Int80& value) noexcept {
      Int80 buffer;
      swapEndianInt80(value, buffer);
      return buffer;
    }

    /// @brief Convert little/big endian 128-bit integers buffers
    inline void swapEndianInt128(const Int128& value, Int128& out) noexcept {
      out.high = swapEndianInt64(value.low);
      out.low = swapEndianInt64(value.high);
    }
    /// @brief Convert little/big endian 128-bit integers buffers
    inline Int128 swapEndianInt128(const Int128& value) noexcept {
      Int128 buffer;
      swapEndianInt128(value, buffer);
      return buffer;
    }

    // -- swap floating point types --

    /// @brief Convert little/big endian 32-bit floats
    inline float swapEndianFloat(float value) noexcept {
      swappableFloat<uint32_t,float> buffer;
      buffer.value = value;
      buffer.bytes = swapEndianInt32(buffer.bytes);
      return buffer.value;
    }
    /// @brief Convert little/big endian 64-bit double-precision floats
    inline double swapEndianDouble(double value) noexcept {
      swappableFloat<uint64_t,double> buffer;
      buffer.value = value;
      buffer.bytes = swapEndianInt64(buffer.bytes);
      return buffer.value;
    }

  }
}
