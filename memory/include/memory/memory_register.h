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
#include <type_traits>

namespace pandora {
  namespace memory {
    /// @class MemoryRegister
    /// @brief Memory register representation with multi-level access (bit - byte - word/16 - dword/32 - qword/64 - mask)
    template <typename _DataType = uint32_t>
    class MemoryRegister final {
    public:
      using value_type = _DataType;
      using size_type = size_t;
      using index_type = uint32_t;
      using Type = MemoryRegister<_DataType>;
      static_assert(std::is_integral<_DataType>::value, "MemoryRegister: integral _DataType required");

      MemoryRegister() : _value(0) {}
      MemoryRegister(_DataType value) : _value(value) {}
    
      MemoryRegister(const Type&) = default;
      MemoryRegister(Type&&) noexcept = default;
      Type& operator=(const Type&) = default;
      Type& operator=(Type&&) noexcept = default;
      ~MemoryRegister() = default;
      
      void swap(Type& rhs) noexcept {
        this->_value ^= rhs._value;
        rhs._value ^= this->_value;
        this->_value ^= rhs._value;
      }
      
      // -- accessors --
      
      constexpr inline _DataType value() const noexcept { return this->_value; } ///< Get raw bitmap value
      inline void value(_DataType value) noexcept { this->_value = value; }      ///< Set raw bitmap value
      inline void clear() noexcept { this->_value = 0; }                                       ///< Set all bits to 0
      inline void fill() noexcept  { _DataType emptyValue = 0; this->_value = (~emptyValue); } ///< Set all bits to 1
      
      constexpr inline size_t size() const noexcept { return sizeof(_DataType)*8u * sizeof(uint32_t)/4u; } ///< Get number of available bits
      constexpr inline bool empty() const noexcept  { return (this->_value == 0); }                           ///< Check if all bits equal 0
      inline bool full() const noexcept { _DataType emptyValue = 0; return (this->_value == static_cast<_DataType>(~emptyValue)); } ///< Check if all bits equal 1

      /// @brief Count number of bits set to 1
      constexpr inline size_t count() const noexcept {
        size_t counter = 0u;
        if (!empty()) {
          for (index_type bitIndex = 0; bitIndex < static_cast<index_type>(size()); ++bitIndex) {
            if (getBit(bitIndex))
              ++counter;
          }
        }
        return counter;
      }

      /// @brief Extract all bits set in a specific mask
      constexpr inline _DataType getBits(_DataType mask) const noexcept { return (this->_value & mask); }
      /// @brief Extract all bits set in a specific mask and shift them
      constexpr inline _DataType getBits(_DataType mask, uint32_t offsetAfterMask) const noexcept {
        return (getBits(mask) >> offsetAfterMask);
      }
      
      // -- bit-level --
      
      constexpr inline bool getBit(index_type bitIndex) const noexcept { ///< Read bit value
        return ((this->_value & (_DataType{1u} << bitIndex)) != 0);
      }
      inline void setBit(index_type bitIndex, bool value) noexcept { ///< Write bit value
          if (value)
              this->_value |= (_DataType{1u} << bitIndex);
          else
              this->_value &= ~(_DataType{1u} << bitIndex);
      }
      
      // -- byte-level (8) --
      
      constexpr inline uint8_t getByte8(index_type byteIndex) const noexcept { ///< Read byte value (8-bits)
        return static_cast<uint8_t>((this->_value >> (byteIndex<<3)) & 0xFFu);
      }
      inline void setByte8(index_type byteIndex, uint8_t value) noexcept { ///< Write byte value (8-bits)
        this->_value &= ~(static_cast<_DataType>(0xFFu) << (byteIndex<<3));
        this->_value |= (static_cast<_DataType>(value) << (byteIndex<<3));
      }
      
      // -- word-level (16) --
      
      constexpr inline uint16_t getWord16(index_type wordIndex) const noexcept { ///< Read word value (16-bits)
        return (static_cast<uint16_t>(this->_value >> (wordIndex<<4)) & 0xFFFFu);
      }
      inline void setWord16(index_type wordIndex, uint16_t value) noexcept { ///< Write word value (16-bits)
        this->_value &= ~(static_cast<_DataType>(0xFFFFu) << (wordIndex<<4));
        this->_value |= (static_cast<_DataType>(value) << (wordIndex<<4));
      }
      
      // -- dword-level (32) --
      
      constexpr inline uint32_t getDword32(index_type dwordIndex) const noexcept { ///< Read dword value (32-bits)
        return (static_cast<uint32_t>(this->_value >> (dwordIndex<<5)) & 0xFFFFFFFFu);
      }
      inline void setDword32(index_type dwordIndex, uint32_t value) noexcept { ///< Write dword value (32-bits)
        this->_value &= ~(static_cast<_DataType>(0xFFFFFFFFu) << (dwordIndex<<5));
        this->_value |= (static_cast<_DataType>(value) << (dwordIndex<<5));
      }
      
      // -- qword-level (64) --
      
      constexpr inline uint64_t getQword64(index_type qwordIndex) const noexcept { ///< Read qword value (64-bits)
        return (static_cast<uint64_t>(this->_value >> (qwordIndex<<6)) & 0xFFFFFFFFFFFFFFFFu);
      }
      inline void setQword64(index_type qwordIndex, uint64_t value) noexcept { ///< Write qword value (64-bits)
        this->_value &= ~(static_cast<_DataType>(0xFFFFFFFFFFFFFFFFu) << (qwordIndex<<6));
        this->_value |= (static_cast<_DataType>(value) << (qwordIndex<<6));
      }
      
      
      // -- bitwise operators --
      
      Type operator&(const Type& rhs) const noexcept { return Type(this->_value & rhs._value); }
      Type operator|(const Type& rhs) const noexcept { return Type(this->_value | rhs._value); }
      Type operator^(const Type& rhs) const noexcept { return Type(this->_value ^ rhs._value); }
      Type& operator&=(const Type& rhs) noexcept { this->_value &= rhs._value; return *this; }
      Type& operator|=(const Type& rhs) noexcept { this->_value |= rhs._value; return *this; }
      Type& operator^=(const Type& rhs) noexcept { this->_value ^= rhs._value; return *this; }
      
      Type operator&(_DataType rhs) const noexcept { return Type(this->_value & rhs); }
      Type operator|(_DataType rhs) const noexcept { return Type(this->_value | rhs); }
      Type operator^(_DataType rhs) const noexcept { return Type(this->_value ^ rhs); }
      Type& operator&=(_DataType rhs) noexcept { this->_value &= rhs; return *this; }
      Type& operator|=(_DataType rhs) noexcept { this->_value |= rhs; return *this; }
      Type& operator^=(_DataType rhs) noexcept { this->_value ^= rhs; return *this; }
      
      Type operator<<(uint32_t offset) const noexcept { return Type(this->_value << offset); }
      Type operator>>(uint32_t offset) const noexcept { return Type(this->_value >> offset); }
      Type& operator<<=(uint32_t offset) noexcept { this->_value <<= offset; return *this; }
      Type& operator>>=(uint32_t offset) noexcept { this->_value >>= offset; return *this; }
      
      Type operator~() const noexcept { return Type(~(this->_value)); }
      operator bool() const noexcept { return (this->_value != 0); }
      
      // -- comparisons --
      
      bool operator==(const Type& rhs) const noexcept { return (this->_value == rhs._value); }
      bool operator!=(const Type& rhs) const noexcept { return (this->_value != rhs._value); }
      bool operator<(const Type& rhs) const noexcept { return (this->_value < rhs._value); }
      bool operator<=(const Type& rhs) const noexcept { return (this->_value <= rhs._value); }
      bool operator>(const Type& rhs) const noexcept { return (this->_value > rhs._value); }
      bool operator>=(const Type& rhs) const noexcept { return (this->_value >= rhs._value); }

    private:
      _DataType _value;
    };
    
  }
}
