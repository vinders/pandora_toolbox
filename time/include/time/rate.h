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
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace time {
    /// @class Rate
    /// @brief Rational representation of a frequency (video sampling, audio sampling, time management, ...)
    class Rate final {
    public:
      constexpr Rate() noexcept : _numerator(1), _denominator(1u) {}
      constexpr Rate(int32_t numerator, uint32_t denominator) noexcept
        : _numerator(numerator), _denominator(denominator) { assert(denominator != 0u); }
      Rate(const Rate&) = default;
      Rate(Rate&&) noexcept = default;
      Rate& operator=(const Rate&) = default;
      Rate& operator=(Rate&&) noexcept = default;
      ~Rate() = default;
      
      // -- getters / setters --
      
      constexpr inline int32_t numerator() const noexcept { return this->_numerator; }
      inline void numerator(int32_t numerator) noexcept { this->_numerator = numerator; }
      constexpr inline uint32_t denominator() const noexcept { return this->_denominator; }
      inline void denominator(uint32_t denominator) noexcept { assert(denominator != 0u); this->_denominator = denominator; }
      
      // -- comparisons --
      
      bool operator==(const Rate& rhs) const noexcept { return (this->_numerator == rhs._numerator && this->_denominator == rhs._denominator); }
      bool operator!=(const Rate& rhs) const noexcept { return !(this->operator==(rhs)); }

      // -- formatters --

      std::string toString(char separator = ':') const { return std::to_string(this->_numerator) + separator + std::to_string(this->_denominator); }
      
      constexpr double compute() const noexcept { return (static_cast<double>(this->_numerator) / static_cast<double>(this->_denominator)); }
      
      constexpr double toPercent() const noexcept { return (compute() * 100.0); }
      
      constexpr int32_t toFloorPercent() const noexcept {
        double floorValue = (this->_numerator >= 0) ? toPercent() + 0.000000001 : toPercent() - 0.000000001;
        return static_cast<int32_t>(floorValue);
      }
      constexpr int32_t toRoundPercent() const noexcept {
        double roundValue = (this->_numerator >= 0) ? toPercent() + 0.500000001 : toPercent() - 0.500000001;
        return static_cast<int32_t>(roundValue);
      }
      
    private:
      int32_t _numerator = 1;
      uint32_t _denominator = 1u;
    };

  }
}
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
