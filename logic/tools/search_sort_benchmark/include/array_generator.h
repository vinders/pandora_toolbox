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
Description : array generator for benchmark utility
*******************************************************************************/
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <cmath>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <random>
#include <logic/sort.h>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4702)
#endif

// type of array to generate
enum class CollectionId: uint32_t {
  continuous = 0,
  sameValue = 1,
  sameRepeats = 2,
  randomRepeats = 3,
  exponential = 4,
  logarithmic = 5,
  gaussianCurve = 6,
  extremes = 7
};
// order of generated array
enum class ArrayOrder : uint32_t {
  unordered = 0,
  asc = 1,
  desc = 2
};
// values of generated array
enum class ValueSign : uint32_t {
  positive = 0,
  negative = 1,
  both = 2
};

template <ArrayOrder _Order, ValueSign _Sign> void _generateContinuousArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateSameValueArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateSameRepeatsArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateRandomRepeatsArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateExponentialArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateLogarithmicArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateGaussianCurveArray(int*, size_t) noexcept;
template <ArrayOrder _Order, ValueSign _Sign> void _generateExtremesArray(int*, size_t) noexcept;
inline void _randomizeOrder(int*, size_t) noexcept;

// generate an ordered array, based on the requested type
template <ArrayOrder _Order = ArrayOrder::asc, ValueSign _Sign = ValueSign::positive>
inline void generateArray(CollectionId type, int* collection, size_t length) noexcept {
  assert(collection != nullptr && length > size_t{ 0 });
  switch (type) {
    case CollectionId::continuous: _generateContinuousArray<_Order,_Sign>(collection, length); break;
    case CollectionId::sameValue: _generateSameValueArray<_Order,_Sign>(collection, length); break;
    case CollectionId::sameRepeats: _generateSameRepeatsArray<_Order,_Sign>(collection, length); break;
    case CollectionId::randomRepeats: _generateRandomRepeatsArray<_Order, _Sign>(collection, length); break;
    case CollectionId::exponential: _generateExponentialArray<_Order,_Sign>(collection, length); break;
    case CollectionId::logarithmic: _generateLogarithmicArray<_Order,_Sign>(collection, length); break;
    case CollectionId::gaussianCurve: _generateGaussianCurveArray<_Order,_Sign>(collection, length); break;
    case CollectionId::extremes: _generateExtremesArray<_Order,_Sign>(collection, length); break;
    default: assert(false); break;
  }
}

// collection id to string serializer
inline std::string toString(CollectionId type) {
  switch (type) {
    case CollectionId::continuous:    return "Continuous";
    case CollectionId::sameValue:     return "Same value";
    case CollectionId::sameRepeats:   return "Regular repeat";
    case CollectionId::randomRepeats: return "Random repeats";
    case CollectionId::exponential:   return "Exponential";
    case CollectionId::logarithmic:   return "Logarithmic";
    case CollectionId::gaussianCurve: return "Gaussian curve";
    case CollectionId::extremes:      return "Extreme values";
    default: return "";
  }
}


// -- private ------------------------------------------------------------------

template <ArrayOrder _Order, ValueSign _Sign>
void _generateContinuousArray(int* collection, size_t length) noexcept {
  int startValue = (_Sign == ValueSign::positive) ? 0 : ((_Sign == ValueSign::negative) ? -static_cast<int>(length) : -static_cast<int>(length >> 1));

  int* it = collection;
  __if_constexpr (_Order == ArrayOrder::desc) {
    for (int i = static_cast<int>(length) - 1; i >= 0; --i, ++it)
      *it = startValue + i;
  }
  else {
    for (int i = 0; i < static_cast<int>(length); ++i, ++it)
      *it = startValue + i;
    __if_constexpr  (_Order == ArrayOrder::unordered)
      _randomizeOrder(collection, length);
  }
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateSameValueArray(int* collection, size_t length) noexcept {
  int value = (_Sign == ValueSign::positive) ? 42 : ((_Sign == ValueSign::negative) ? -42 : 0);
  for (int i = 0; i < static_cast<int>(length); ++i, ++collection)
    *collection = value;
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateSameRepeatsArray(int* collection, size_t length) noexcept {
  int repeats = static_cast<int>(sqrt(static_cast<int>(length)));
  if (repeats < 3)
    repeats = 3;
  int value = (_Sign == ValueSign::positive) ? 0 : ((_Sign == ValueSign::negative) ? -static_cast<int>(length)/repeats : -static_cast<int>(length >> 1)/repeats);

  __if_constexpr (_Order == ArrayOrder::desc) {
    int* it = collection + (length - 1);
    for (int i = 0; i < static_cast<int>(length); ++i, --it) {
      *it = value;
      if (i != 0 && ((i + 1) % repeats) == 0)
        ++value;
    }
  }
  else {
    int* it = collection;
    for (int i = 0; i < static_cast<int>(length); ++i, ++it) {
      *it = value;
      if (i != 0 && ((i + 1) % repeats) == 0)
        ++value;
    }
    __if_constexpr  (_Order == ArrayOrder::unordered)
      _randomizeOrder(collection, length);
  }
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateRandomRepeatsArray(int* collection, size_t length) noexcept {
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> randomizer(0, static_cast<int>(length - length/4u));

  int* it = collection;
  for (int i = 0; i < static_cast<int>(length); ++i, ++it) {
    __if_constexpr (_Sign == ValueSign::positive)
      *it = randomizer(generator);
    else __if_constexpr(_Sign == ValueSign::negative)
      *it = -randomizer(generator);
    else
      *it = randomizer(generator) - (static_cast<int>(length - length / 4u) >> 1);
  }

  __if_constexpr (_Order == ArrayOrder::asc)
    pandora::logic::heapSort<int,pandora::logic::SortOrder::asc>(collection, static_cast<uint32_t>(length));
  else __if_constexpr (_Order == ArrayOrder::desc)
    pandora::logic::heapSort<int, pandora::logic::SortOrder::desc>(collection, static_cast<uint32_t>(length));
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateExponentialArray(int* collection, size_t length) noexcept {
  __if_constexpr (_Sign == ValueSign::both) {
    __if_constexpr (_Order != ArrayOrder::desc) {
      _generateExponentialArray<_Order,ValueSign::negative>(collection, (length >> 1));
      _generateExponentialArray<_Order,ValueSign::positive>(collection + (length >> 1), length - (length >> 1));
    }
    else {
      _generateExponentialArray<_Order,ValueSign::positive>(collection, (length >> 1));
      _generateExponentialArray<_Order,ValueSign::negative>(collection + (length >> 1), length - (length >> 1));
    }
    return;
  }

  double stepMultiplier = (length <= size_t{ 225 }) ? (2.75 - 1.1 * sqrt(log10(length))) : (1.1 - sqrt(length)/360.0);
  if (stepMultiplier < 1.01)
    stepMultiplier = 1.01;

  double value = (_Sign == ValueSign::positive) ? 0.2 : -0.2;
  if (length < size_t{ 20u })
    value *= (20.0 / length);

  __if_constexpr ((_Order == ArrayOrder::desc && _Sign != ValueSign::negative) || (_Order == ArrayOrder::asc && _Sign == ValueSign::negative)) {
    int* it = collection + (length - 1);
    for (int i = 0; i < static_cast<int>(length); ++i, --it) {
      *it = static_cast<int>(value);
      value *= stepMultiplier;
    }
  }
  else {
    int* it = collection;
    for (int i = 0; i < static_cast<int>(length); ++i, ++it) {
      *it = static_cast<int>(value);
      value *= stepMultiplier;
    }
    __if_constexpr (_Order == ArrayOrder::unordered)
      _randomizeOrder(collection, length);
  }
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateLogarithmicArray(int* collection, size_t length) noexcept {
  int rootCount = static_cast<int>(log2(length));
  double stepDivider = 10.5; 
  for (int i = 1; i < rootCount; ++i)
    stepDivider = sqrt(stepDivider);

  double value = (_Sign != ValueSign::both) ? 0.0 : ((length >= size_t{ 4 }) ? -300.0 : -100.0);
  double step = (_Sign != ValueSign::negative) ? 100.0 : -100.0;

  __if_constexpr ((_Order == ArrayOrder::desc && _Sign != ValueSign::negative) || (_Order == ArrayOrder::asc && _Sign == ValueSign::negative)) {
    int* it = collection + (length - 1);
    for (int i = 0; i < static_cast<int>(length); ++i, --it) {
      *it = static_cast<int>(value);
      value += step;
      step /= stepDivider;
    }
  }
  else {
    int* it = collection;
    for (int i = 0; i < static_cast<int>(length); ++i, ++it) {
      *it = static_cast<int>(value);
      value += step;
      step /= stepDivider;
    }
    __if_constexpr  (_Order == ArrayOrder::unordered)
      _randomizeOrder(collection, length);
  }
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateGaussianCurveArray(int* collection, size_t length) noexcept {
  int pivot1 = static_cast<int>(0.375 * static_cast<double>(length));
  int pivot2 = static_cast<int>(0.425 * static_cast<double>(length));
  double stepMultiplier = (length <= size_t{ 225 }) ? (2.9 - 1.1 * sqrt(log10(length))) : (1.1 - sqrt(length)/360.0);
  if (stepMultiplier < 1.015)
    stepMultiplier = 1.015;
  double step = (_Sign != ValueSign::negative) ? 1.2 : -1.2;
  for (int i = static_cast<int>(length)/10; i >= 0; --i)
    step /= stepMultiplier;

  int* leftIt = collection;
  int* rightIt = collection + (length - 1);
  int value = 0;
  for (int i = 0; i < static_cast<int>(length) >> 1; ++i, ++leftIt, --rightIt) {
    *leftIt = *rightIt = value;
    step = (i <= pivot1) ? step * stepMultiplier : ((i <= pivot2) ? step : step/(3.0*stepMultiplier));
    value += static_cast<int>(step);
  }
  int maxValue = collection[(length >> 1) - 1u];
  if (length & 0x1) {
    maxValue += static_cast<int>(maxValue * 0.05);
    *leftIt = (_Sign != ValueSign::both) ? maxValue : 0;
    ++leftIt;
    --rightIt;
  }

  for (int i = 0; i < static_cast<int>(length) >> 1; ++i) {
    __if_constexpr ((_Order != ArrayOrder::desc && _Sign != ValueSign::negative) || (_Order == ArrayOrder::desc && _Sign == ValueSign::negative)) {
      __if_constexpr (_Sign != ValueSign::both)
        *leftIt = 2*maxValue - *leftIt;
      else {
        *leftIt = maxValue - *leftIt;
        *rightIt -= maxValue;
        --rightIt;
      }
      ++leftIt;
    }
    else {
      __if_constexpr (_Sign != ValueSign::both)
        *rightIt = 2*maxValue - *rightIt;
      else {
        *leftIt -= maxValue;
        *rightIt = maxValue - *rightIt;
        ++leftIt;
      }
      --rightIt;
    }
  }

  __if_constexpr  (_Order == ArrayOrder::unordered)
    _randomizeOrder(collection, length);
}

template <ArrayOrder _Order, ValueSign _Sign>
void _generateExtremesArray(int* collection, size_t length) noexcept {
  srand(static_cast<unsigned int>(time(nullptr)));
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> randomizer(0, static_cast<int>(length - length / 4u));

  int* it = collection;
  for (int i = 0; i < static_cast<int>(length); ++i, ++it) {
    __if_constexpr(_Sign == ValueSign::positive)
      * it = randomizer(generator);
    else __if_constexpr(_Sign == ValueSign::negative)
    * it = -randomizer(generator);
    else
    *it = randomizer(generator) - (static_cast<int>(length - length / 4u) >> 1);
  }

  int step = static_cast<int>(sqrt(length));
  if (step <= 1)
    step = 2;

  it = collection;
  for (int i = 1; i < static_cast<int>(length); i += step, it += step)
      *it *= (100 + rand() % 1000);

  __if_constexpr(_Order == ArrayOrder::asc)
    pandora::logic::heapSort<int, pandora::logic::SortOrder::asc>(collection, static_cast<uint32_t>(length));
  else __if_constexpr(_Order == ArrayOrder::desc)
    pandora::logic::heapSort<int, pandora::logic::SortOrder::desc>(collection, static_cast<uint32_t>(length));
}

inline void _randomizeOrder(int* collection, size_t length) noexcept {
  std::mt19937 generator(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
  std::shuffle(&collection[0], &collection[length - 1u], generator);
}

#undef __if_constexpr
#ifdef _MSC_VER
# pragma warning(pop)
#endif
