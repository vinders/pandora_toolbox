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
Description : interval search algorithms for ordered collections
------------------------------------------------------------------------
Functions : binarySearch, jumpSearch, exponentialSearch, interpolationSearch
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <type_traits>
#include "./sort_order.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 26451)
#endif

namespace pandora {
  namespace logic {
    constexpr inline uint32_t indexNotFound() noexcept { return static_cast<uint32_t>(-1); } ///< Constant indicating that no result was found

    template <typename _DataType>
    using SearchValue = typename std::conditional<std::is_class<_DataType>::value, const _DataType&, _DataType>::type;

    // ---
    
    /// @brief Search a sorted collection, by repeatedly dividing the search interval. Much faster than a linear search.
    ///        Complexity: O(Log(n))
    /// @param collec  A non-null collection at least as big as 'lastIndex' + 1.
    /// @param first   First index to search in the collection (usually 0).
    /// @param last    Last index to search in the collection (usually SIZE - 1). Make sure that first <= last.
    /// @param target  Value to search.
    /// @returns Location of first occurrence; or indexNotFound() if no occurrence.
    /// @warning - Only use for direct/random access collections: never use with linked lists (very slow).
    ///          - The collection must be sorted.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    uint32_t binarySearch(const _ValueType* collec, uint32_t first, uint32_t last, SearchValue<_ValueType> target) noexcept {
      assert(collec != nullptr && first <= last);
      uint32_t mid;

      // divide -> compare -> repeat
      __if_constexpr (_Order == SortOrder::asc) {
        while (first < last) {
          mid = (first + last) >> 1;
          if (collec[mid] < target)
            first = mid + 1u;
          else
            last = mid;
        }
      }
      else {
        while (first < last) {
          mid = (first + last) >> 1;
          if (collec[mid] > target)
            first = mid + 1u;
          else
            last = mid;
        }
      }
      return (collec[first] == target) ? first : indexNotFound();
    }
    /// @brief Search a sorted collection, by repeatedly dividing the search interval. Much faster than a linear search.
    ///        Complexity: O(Log(n))
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    /// @param target  Value to search.
    /// @returns Location of first occurrence; or indexNotFound() if no occurrence.
    /// @warning - Only use for direct/random access collections: never use with linked lists (very slow).
    ///          - The collection must be sorted.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    inline uint32_t binarySearch(const _ValueType* collec, uint32_t n, SearchValue<_ValueType> target) noexcept {
      return (n > 0) ? binarySearch<_ValueType,_Order>(collec, 0u, n - 1u, target) : indexNotFound();
    }

    /// @brief Search a sorted collection, by jumping ahead by fixed linear steps to skip some elements (ideal step size: sqrt(n)).
    ///        Slower than binary search for common cases, but works better for the worst situations of binary search (target close to extreme or shorter/greater than min/max...).
    ///        Complexity: O(sqrt(n))
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    /// @param target  Value to search.
    /// @returns Location of first occurrence; or indexNotFound() if no occurrence.
    /// @warning The collection must be sorted.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    uint32_t jumpSearch(const _ValueType* collec, uint32_t n, SearchValue<_ValueType> target) noexcept {
      assert(collec != nullptr);
      if (n == 0)
        return indexNotFound();
      const uint32_t stepSize = static_cast<uint32_t>(sqrt(n));

      // find block containing target (repeat jump)
      uint32_t first = 0;
      uint32_t last = stepSize;
      __if_constexpr(_Order == SortOrder::asc) {
        while (first < n && collec[(last <= n) ? last - 1u : n - 1u] < target) {
          first = last;
          last += stepSize;
        }
      }
      else {
        while (first < n && collec[(last <= n) ? last - 1u : n - 1u] > target) {
          first = last;
          last += stepSize;
        }
      }
      if (first >= n)
        return indexNotFound();
      if (last > n)
        last = n;

      // linear search in block [first ; last[.
      __if_constexpr(_Order == SortOrder::asc) {
        while (first < last && collec[first] < target)
          ++first;
      }
      else {
        while (first < last && collec[first] > target)
          ++first;
      }
      return (collec[first] == target) ? first : indexNotFound();
    }

    /// @brief Search a sorted collection, by exponentially increasing the range of search, then doing a binary search when the range is appropriate.
    ///        More efficient than binary search when the target is closer to the first element.
    ///        Very useful for unbounded searches (infinite array size)
    ///        Complexity: O(Log(n))
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    /// @param target  Value to search.
    /// @returns Location of first occurrence; or indexNotFound() if no occurrence.
    /// @warning - Only use for direct/random access collections: never use with linked lists (very slow).
    ///          - The collection must be sorted.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    uint32_t exponentialSearch(const _ValueType* collec, uint32_t n, SearchValue<_ValueType> target) noexcept {
      assert(collec != nullptr);
      if (n == 0)
        return indexNotFound();

      if (collec[0] == target) // check index 0 first (it can't be doubled)
        return 0;

      // find range of search containing target (repeat doubling -> exponential increase)
      uint32_t i = 1u;
      __if_constexpr (_Order == SortOrder::asc) {
        while (i < n && collec[i] < target)
          i <<= 1;
      }
      else {
        while (i < n && collec[i] > target)
          i <<= 1;
      }
      return (i < n) ? binarySearch<_ValueType,_Order>(collec, (i >> 1) + 1u, i, target) // binary search in the last range only
                     : binarySearch<_ValueType, _Order>(collec, i >> 1, n - 1u, target);
    }

    /// @brief Search a sorted collection, by checking locations according to the value of the target being searched.
    ///        Much more efficient than binary search when the values are uniformly distributed. Much slower otherwise.
    ///        Complexity: best case (elements uniformly distributed): O(Log(Log(n)))
    ///                    worst case (exponential distribution): O(n)
    /// @param collec  A non-null collection at least as big as 'lastIndex' + 1.
    /// @param n       Size of the collection.
    /// @param target  Value to search.
    /// @returns Location of first occurrence; or indexNotFound() if no occurrence.
    /// @warning - Only use for direct/random access collections: never use with linked lists (very slow).
    ///          - The collection must be sorted.
    ///          - The value type must be usable in calculations (operator-() + cast to double)
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    uint32_t interpolationSearch(const _ValueType* collec, uint32_t n, SearchValue<_ValueType> target) noexcept {
      assert(collec != nullptr);
      if (n == 0)
        return indexNotFound();

      int32_t first = 0;
      int32_t last = static_cast<int32_t>(n) - 1;
      __if_constexpr(_Order == SortOrder::asc) {
        while (collec[first] < collec[last] && collec[first] <= target) { // (first < last && collec[first] != collec[last]) -> collec[first] < collec[last]
          // searching position for uniform distribution
          int32_t pos = first + static_cast<int32_t>(((double)(last - first) * (double)(target - collec[first]) / (double)(collec[last] - collec[first])) );
          if (pos > last)
            pos = last;

          if (collec[pos] < target)
            first = (pos < last) ? pos + 1 : last;
          else if (collec[pos] > target)
            last = pos - 1;
          else  // found -> search first occurrence
            return binarySearch<_ValueType, _Order>(collec, first, pos, target);
        }
      }
      else {
        while (collec[first] > collec[last] && collec[first] >= target) { // (first < last && collec[first] != collec[last]) -> collec[first] > collec[last]
          // searching position for uniform distribution
          int32_t pos = first + static_cast<int32_t>(((double)(last - first) * (double)(collec[first] - target) / (double)(collec[first] - collec[last])) );
          if (pos > last)
            pos = last;

          if (collec[pos] > target)
            first = (pos < last) ? pos + 1 : last;
          else if (collec[pos] < target)
            last = pos - 1;
          else // found -> search first occurrence
            return binarySearch<_ValueType, _Order>(collec, first, pos, target);
        }
      }
      return (collec[first] == target) ? first : indexNotFound();
    }

  }
}
#undef __if_constexpr
#ifdef _MSC_VER
# pragma warning(pop)
#endif
