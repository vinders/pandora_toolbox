/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
------------------------------------------------------------------------
Description : sorting algorithms
------------------------------------------------------------------------
Functions : bubbleSort, insertionSort, binaryInsertionSort
            heapSort, quickSort
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <utility>
#include "./sort_order.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora {
  namespace logic {
    template <typename _DataType> 
    using SortValue = typename std::conditional<std::is_class<_DataType>::value, const _DataType&, _DataType>::type;
    template <typename _ValueType, SortOrder _Order> inline int32_t _binarySearchInsertPosition(_ValueType*, int32_t, SortValue<_ValueType>) noexcept;
    template <typename _ValueType, SortOrder _Order> void _heapify(_ValueType*, int32_t, int32_t) noexcept;
    template <typename _ValueType, SortOrder _Order> int32_t _partitionFirstPivot(_ValueType*, int32_t, int32_t) noexcept;
    template <typename _ValueType, SortOrder _Order> int32_t _partitionLastPivot(_ValueType*, int32_t, int32_t) noexcept;
    template <typename _ValueType, SortOrder _Order> inline int32_t _partitionCentralPivot(_ValueType*, int32_t, int32_t) noexcept;


    // -- simple sorting - small arrays, or almost sorted arrays --

    /// @brief Simple sorting, repeatedly swapping adjacent elements that are unordered.
    ///        Very efficient for "almost sorted" arrays that just need a few swaps on short distances. Inefficient in other cases.
    ///        Complexity: worst case (reverse sorted): O(n*n)
    ///                    best case (already sorted): O(n)
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    void bubbleSort(_ValueType* collec, uint32_t n) noexcept {
      assert(collec != nullptr);
      int32_t cur;

      bool isUnordered = true;
      for (int32_t last = static_cast<int32_t>(n) - 1; isUnordered; --last) {
        isUnordered = false;
        for (cur = 0; cur < last; ++cur) {

          __if_constexpr (_Order == SortOrder::asc) {
            if (collec[cur] > collec[cur + 1]) { // unordered -> swap
              _ValueType buffer = std::move(collec[cur]);
              collec[cur] = std::move(collec[cur + 1]);
              collec[cur + 1] = std::move(buffer);
              isUnordered = true;
            }
          }
          else { // desc
            if (collec[cur] < collec[cur + 1]) { // unordered -> swap
              _ValueType buffer = std::move(collec[cur]);
              collec[cur] = std::move(collec[cur + 1]);
              collec[cur + 1] = std::move(buffer);
              isUnordered = true;
            }
          }
        }
      }
    }

    /// @brief Simple sorting, splitting the array in sorted/unsorted parts, and inserting each value at the correct position.
    ///        Very efficient for "almost sorted" small/medium arrays. Otherwise, relatively efficient for small arrays.
    ///        Complexity: worst case (reverse sorted): O(n*n)
    ///                    best case (already sorted): O(n)
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    void insertionSort(_ValueType* collec, uint32_t n) noexcept {
      assert(collec != nullptr);
      int32_t pos;

      for (int32_t last = 1; last < static_cast<int32_t>(n); ++last) {
        _ValueType key = std::move(collec[last]);

        // shift next elements to move key to appropriate location
        __if_constexpr (_Order == SortOrder::asc) {
          for (pos = last - 1; pos >= 0 && collec[pos] > key; --pos)
            collec[pos + 1] = std::move(collec[pos]);
        }
        else { // desc
          for (pos = last - 1; pos >= 0 && collec[pos] < key; --pos)
            collec[pos + 1] = std::move(collec[pos]);
        }
        collec[pos + 1] = std::move(key);
      }
    }

    /// @brief Simple sorting, splitting the array in sorted/unsorted parts, and inserting each value at the correct position.
    ///        Usually more efficient than normal insertion sort with medium/great arrays (too much overhead to gain performances with small arrays).
    ///        Very efficient for "almost sorted" medium/great arrays. Otherwise, relatively efficient for medium arrays.
    ///        Complexity: worst case (reverse sorted): O(n*n)
    ///                    best case (already sorted): O(n)
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    void binaryInsertionSort(_ValueType* collec, uint32_t n) noexcept {
      assert(collec != nullptr);
      int32_t pos, target;

      for (int32_t last = 1; last < static_cast<int32_t>(n); ++last) {
        // find target location to insert key
        pos = last - 1;
        target = _binarySearchInsertPosition<_ValueType,_Order>(collec, pos, collec[last]);
        if (target < last) {

          // shift next elements to move key to appropriate target location
          _ValueType key = std::move(collec[last]);
          do {
            collec[pos + 1] = std::move(collec[pos]);
            --pos;
          } 
          while (pos >= target);
          collec[pos + 1] = std::move(key);
        }
      }
    }


    // -- advanced sorting - medium/great arrays --

    /// @brief Sorting based on binary heap data structure, where the maximum is repeatedly placed at the end.
    ///        More efficient than simple sortings, except for very small arrays or already sorted arrays.
    ///        For unsorted arrays, quick sort (arrays) and merge sort (linked lists) are usually more efficient than heap sort.
    ///        Complexity: O(n*Log(n))
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc>
    void heapSort(_ValueType* collec, uint32_t n) noexcept {
      assert(collec != nullptr);

      // rearrange array -> build heap
      for (int32_t i = (static_cast<int32_t>(n) >> 1) - 1; i >= 0; --i)
        _heapify<_ValueType,_Order>(collec, static_cast<int32_t>(n), i);

      // extract elements from heap (one by one)
      for (int32_t i = static_cast<int32_t>(n) - 1; i > 0; --i) {
        _ValueType buffer = std::move(collec[0]); // move current root to the end
        collec[0] = std::move(collec[i]);
        collec[i] = std::move(buffer);

        _heapify<_ValueType,_Order>(collec, i, 0); // rearrange the reduced heap
      }
    }

    /// @brief Type of pivot to use for quick sort
    enum class SortPivotType : uint32_t {
      first = 0u,
      center = 1u,
      last = 2u
    };

    /// @brief Sorting that picks an item as pivot and partitions the given array around it (recursive).
    ///        Usually faster than heap sort (more efficient inner loop), but worst case is much slower (array already sorted or reverse sorted).
    ///        Choosing the type of pivot according to the type of data helps avoiding the worst case most of the time.
    ///        Quick sort works best for arrays/vectors. Prefer merge sort for huge data and external storage.
    ///        Complexity: worst case (already sorted or reverse sorted): O(n*n)
    ///                    average case & best case: O(n*Log(n))
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param first   First index to search in the collection (usually 0).
    /// @param last    Last index to search in the collection (usually SIZE - 1). Make sure that first <= last.
    /// @warning Only use for direct/random access collections: never use with linked lists.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc, SortPivotType _PivotType = SortPivotType::last>
    void quickSort(_ValueType* collec, int32_t first, int32_t last) noexcept {
      assert(collec != nullptr && first >= 0);

      if (first < last) {
        // set pivot element at correct position
        int32_t partitionIndex;
        __if_constexpr (_PivotType == SortPivotType::first)
          partitionIndex = _partitionFirstPivot<_ValueType,_Order>(collec, first, last);
        else __if_constexpr (_PivotType == SortPivotType::center)
          partitionIndex = _partitionCentralPivot<_ValueType,_Order>(collec, first, last);
        else
          partitionIndex = _partitionLastPivot<_ValueType,_Order>(collec, first, last);

        quickSort<_ValueType,_Order,_PivotType>(collec, first, partitionIndex - 1); // left side of pivot
        quickSort<_ValueType,_Order,_PivotType>(collec, partitionIndex + 1, last);  // right side of pivot
      }
    }
    /// @brief Sorting that picks an item as pivot and partitions the given array around it (recursive).
    ///        Usually faster than heap sort (more efficient inner loop), but worst case is much slower.
    ///        Choosing the type of pivot according to the type of data helps avoiding the worst case most of the time.
    ///        Quick sort works best for arrays/vectors. Prefer merge sort for huge data and external storage.
    ///        Complexity: worst case (already sorted or reverse sorted): O(n*n)
    ///                    average case & best case: O(n*Log(n))
    /// @param collec  A non-null collection at least as big as 'n'.
    /// @param n       Size of the collection.
    /// @warning Only use for direct/random access collections: never use with linked lists.
    template <typename _ValueType, SortOrder _Order = SortOrder::asc, SortPivotType _PivotType = SortPivotType::last>
    inline void quickSort(_ValueType* collec, uint32_t n) noexcept {
      quickSort<_ValueType,_Order,_PivotType>(collec, 0, static_cast<int32_t>(n) - 1);
    }
    

    // -- private --------------------------------------------------------------

    // binary search variation: find rightmost occurrence (or position after previous value, if no occurrence)
    template <typename _ValueType, SortOrder _Order>
    inline int32_t _binarySearchInsertPosition(_ValueType* collec, int32_t last, SortValue<_ValueType> key) noexcept {
      int32_t first = 0;
      int32_t mid;

      __if_constexpr (_Order == SortOrder::asc) {
        while (first < last) {
          mid = (first + last) >> 1;
          if (collec[mid] <= key)
            first = mid + 1u;
          else
            last = mid;
        }
        if (collec[first] < key)
          ++first;
      }
      else { // desc
        while (first < last) {
          mid = (first + last) >> 1;
          if (collec[mid] >= key)
            first = mid + 1u;
          else
            last = mid;
        }
        if (collec[first] > key)
          ++first;
      }
      return first;
    }

    // heapify max value of an sub-array (heap sort)
    template <typename _ValueType, SortOrder _Order>
    void _heapify(_ValueType* collec, int32_t n, int32_t nodeIndex) noexcept {
      __if_constexpr (_Order == SortOrder::asc) {
        int32_t pos = (nodeIndex << 1) + 1;
        while (pos < n) {
          if (pos + 1 < n && collec[pos] < collec[pos + 1])
            ++pos;
          if (collec[nodeIndex] < collec[pos]) {
            _ValueType buffer = std::move(collec[nodeIndex]);
            collec[nodeIndex] = std::move(collec[pos]);
            collec[pos] = std::move(buffer);

            nodeIndex = pos;
            pos = (nodeIndex << 1) + 1;
          }
          else
            break;
        }
      }
      else {
        int32_t pos = (nodeIndex << 1) + 1;
        while (pos < n) {
          if (pos + 1 < n && collec[pos] > collec[pos + 1])
            ++pos;
          if (collec[nodeIndex] > collec[pos]) {
            _ValueType buffer = std::move(collec[nodeIndex]);
            collec[nodeIndex] = std::move(collec[pos]);
            collec[pos] = std::move(buffer);

            nodeIndex = pos;
            pos = (nodeIndex << 1) + 1;
          }
          else
            break;
        }
      }
    }

    // take first element as pivot, move it at correct position, then sort other elements around it (quick sort)
    template <typename _ValueType, SortOrder _Order>
    int32_t _partitionFirstPivot(_ValueType* collec, int32_t first, int32_t last) noexcept {
      SortValue<_ValueType> pivot = collec[first];
      int32_t pivotTarget = first + 1;
      for (int32_t pos = first + 1; pos <= last; ++pos) {
        // current element should be before pivot -> swap with previous
        __if_constexpr (_Order == SortOrder::asc) {
          if (collec[pos] < pivot) { 
            if (pos != pivotTarget)
              std::swap(collec[pivotTarget], collec[pos]);
            ++pivotTarget;
          }
        }
        else {
          if (collec[pos] > pivot) { 
            if (pos != pivotTarget)
              std::swap(collec[pivotTarget], collec[pos]);
            ++pivotTarget;
          }
        }
      }
      // move pivot at target location (unless it already is)
      --pivotTarget;
      if (pivotTarget != first) {
        __if_constexpr (std::is_class<_ValueType>::value == true) {
          std::swap(collec[pivotTarget], collec[first]);
        }
        else {
          collec[first] = collec[pivotTarget];
          collec[pivotTarget] = pivot;
        }
      }
      return pivotTarget;
    }

    // take central element as pivot, move it at correct position, then sort other elements around it (quick sort)
    template <typename _ValueType, SortOrder _Order>
    inline int32_t _partitionCentralPivot(_ValueType* collec, int32_t first, int32_t last) noexcept {
      std::swap(collec[(first + last) >> 1], collec[last]);
      return _partitionLastPivot<_ValueType, _Order>(collec, first, last);
    }

    // take last element as pivot, move it at correct position, then sort other elements around it (quick sort)
    template <typename _ValueType, SortOrder _Order>
    int32_t _partitionLastPivot(_ValueType* collec, int32_t first, int32_t last) noexcept {
      SortValue<_ValueType> pivot = collec[last];
      int32_t pivotTarget = first;
      for (int32_t pos = first; pos < last; ++pos) {
        // current element should be before pivot -> swap with previous
        __if_constexpr (_Order == SortOrder::asc) {
          if (collec[pos] < pivot) { 
            if (pos != pivotTarget)
              std::swap(collec[pivotTarget], collec[pos]);
            ++pivotTarget;
          }
        }
        else {
          if (collec[pos] > pivot) { 
            if (pos != pivotTarget)
              std::swap(collec[pivotTarget], collec[pos]);
            ++pivotTarget;
          }
        }
      }
      // move pivot at target location (unless it already is)
      if (pivotTarget != last) {
        __if_constexpr (std::is_class<_ValueType>::value == true) {
          std::swap(collec[pivotTarget], collec[last]);
        }
        else {
          collec[last] = collec[pivotTarget];
          collec[pivotTarget] = pivot;
        }
      }
      return pivotTarget;
    }

  }
}
#undef __if_constexpr
