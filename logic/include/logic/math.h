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
------------------------------------------------------------------------
Description : general-purpose math algorithms
------------------------------------------------------------------------
Functions : nearEqual, floatEqual, doubleEqual
            isPow2
            gcd
*******************************************************************************/
#pragma once

#include <cstdint>
#include <cassert>
#include <type_traits>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __constexpr constexpr
#else
# define __constexpr
#endif

namespace pandora {
  namespace logic {
    // -- number analysis --

    /// @brief Check near-equality (with error margin)
    template <typename T>
    __constexpr inline bool nearEqual(T lhs, T rhs, T errorMargin) noexcept {
      assert(errorMargin >= static_cast<T>(0));
      return ((lhs >= rhs) ? (lhs - rhs <= errorMargin) : (rhs - lhs <= errorMargin));
    }
    /// @brief Check equality of floating-point values (epsilon error margin)
    constexpr inline bool floatEqual(float lhs, float rhs) noexcept {
      return (lhs >= rhs) ? ((lhs - rhs) < (lhs / 1000000.0f)) : ((rhs - lhs) < (rhs / 1000000.0f));
    }
    /// @brief Check equality of double precision values (epsilon error margin)
    constexpr inline bool doubleEqual(double lhs, double rhs) noexcept {
      return (lhs >= rhs) ? ((lhs - rhs) < (lhs / 100000000000000.0)) : ((rhs - lhs) < (rhs / 100000000000000.0));
    }

    /// @brief Check if an integer number is a power of 2
    template <typename T>
    __constexpr inline bool isPow2(const T val) noexcept {
      static_assert(std::is_integral<T>::value, "Integer template parameter required");
      return ((val & (val - 1)) == 0);
    }


    // -- calculations --

    /// @brief Calculate greatest common divider (Euclide)
    template <typename T>
    __constexpr inline T gcd(T lhs, T rhs) noexcept {
      if (lhs == 0)
        return rhs;

      while (rhs != 0) {
        T remainder = lhs % rhs;
        lhs = rhs;
        rhs = remainder;
      }
      return (lhs >= 0) ? lhs : -lhs;
    }

  }
}
#undef __constexpr
