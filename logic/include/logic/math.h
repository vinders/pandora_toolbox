/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
