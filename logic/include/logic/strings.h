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
Description : string manipulation utilities
------------------------------------------------------------------------
Functions : stringLength, findNoTrimIndex, rfindAfterNoTrimIndex
            stringAssign
            pad/padSelf, lpad/lpadSelf, rpad/rpadSelf
            trim/trimSelf, ltrim/ltrimSelf, rtrim/rtrimSelf
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cassert>
#include <cstring>
#include <string>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora {
  namespace logic {
    // -- string analysis --

    /// @brief Get length of any string type
    template <typename _CharType>
    inline size_t stringLength(const _CharType* value) noexcept {
      assert(value != nullptr);
      const _CharType* it = value;
      while (*it != static_cast<_CharType>(0))
        ++it;
      return (it - value);
    }

    /// @brief Find first non-trimmable character index in string
    template <typename _CharType>
    inline size_t findNoTrimIndex(const _CharType* value) noexcept {
      assert(value != nullptr);
      const _CharType* it = value;
      while (*it <= 0x20 && *it > 0)
        ++it;
      return (it - value);
    }
    /// @brief Find position after last non-trimmable character index in string
    /// @warning The property 'valueLength' must not exceed the size of 'value'.
    template <typename _CharType>
    inline size_t rfindAfterNoTrimIndex(const _CharType* value, size_t valueLength) noexcept {
      assert(value != nullptr);
      if (valueLength > size_t{ 0 }) {
        const _CharType* it = value + valueLength - 1;
        while (it >= value && static_cast<uint32_t>(*it) <= 0x20)
          --it;
        ++it; // index after last -> +1
        return (it - value);
      }
      return size_t{ 0 };
    }


    // -- string initialization --

    /// @brief Fill a string with a character (repeated)
    template <typename _CharType, bool _AddNullEnding = true>
    inline void stringAssign(_CharType* out, size_t count, _CharType val) noexcept {
      assert(out != nullptr);

      __if_constexpr (std::is_same<_CharType, char>::value == true) {
        if (count > size_t{ 0 })
          memset((void*)out, (int)val, count*sizeof(_CharType));
        __if_constexpr(_AddNullEnding)
          out[count] = static_cast<_CharType>(0);
      }
      else {
        while (count > size_t{ 0 }) {
          *out = val;
          ++out;
          --count;
        }
        __if_constexpr(_AddNullEnding)
          *out = static_cast<_CharType>(0);
      }
    }


    // -- padding --

    /// @brief If size < minSize, add padding characters before/after value (centered)
    template <typename _CharType>
    inline std::basic_string<_CharType> pad(const _CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);
      if (valueLength < minSize) {
        std::basic_string<_CharType> padded;
        padded.reserve(minSize);

        size_t padding = minSize - valueLength;
        if (padding > size_t{ 1 })
          padded.assign(padding >> 1, _PadChar);
        padded.append(value, valueLength);
        padded.append(padding - (padding >> 1), _PadChar);
        return padded;
      }
      return std::basic_string<_CharType>(value, valueLength);
    }
    /// @brief If size < minSize, add padding characters before/after value (centered) in pre-allocated output buffer
    /// @warning The 'out' buffer size must be at least (minSize + 1) !
    template <typename _CharType>
    inline void pad(_CharType* out, const _CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr && out != nullptr && value != out);
      size_t valueLength = stringLength(value);

      if (valueLength < minSize) {
        size_t padding = minSize - valueLength;
        if (padding > size_t{ 1 }) {
          stringAssign<_CharType, false>(out, padding >> 1, _PadChar);
          out += (padding >> 1);
        }
        if (valueLength > size_t{ 0 })
          memcpy((void*)out, (void*)value, valueLength*sizeof(_CharType));
        stringAssign<_CharType, true>(out + valueLength, padding - (padding >> 1), _PadChar);
      }
      else {
        if (valueLength > size_t{ 0 })
          memcpy((void*)out, (void*)value, valueLength*sizeof(_CharType));
        *(out + valueLength) = static_cast<_CharType>(0);
      }
    }
    /// @brief If size < minSize, add padding characters before/after value (centered) in same buffer
    /// @warning The 'value' buffer size must be at least (minSize + 1) !
    template <typename _CharType>
    inline void padSelf(_CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      if (valueLength < minSize) {
        size_t padding = minSize - valueLength;
        if (padding > size_t{ 1 }) {
          if (valueLength > size_t{ 0 }) {
            size_t leftPadding = padding >> 1;
            memmove((void*)(value + leftPadding), (void*)value, valueLength*sizeof(_CharType));
            stringAssign<_CharType, false>(value, leftPadding, _PadChar);
            stringAssign<_CharType, true>(value + valueLength + leftPadding, padding - leftPadding, _PadChar);
          }
          else
            stringAssign<_CharType, true>(value, padding, _PadChar);
        }
        else {
          *(value + valueLength) = _PadChar;
          *(value + valueLength + 1) = static_cast<_CharType>(0);
        }
      }
    }

    /// @brief If size < minSize, add padding at the beginning of the string
    template <typename _CharType>
    inline std::basic_string<_CharType> lpad(const _CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);
      if (valueLength < minSize) {
        std::basic_string<_CharType> padded;
        padded.reserve(minSize);

        padded.assign(minSize - valueLength, _PadChar);
        padded.append(value, valueLength);
        return padded;
      }
      return std::basic_string<_CharType>(value, valueLength);
    }
    /// @brief If size < minSize, add padding at the beginning of the string in pre-allocated output buffer
    /// @warning The 'out' buffer size must be at least (minSize + 1) !
    template <typename _CharType>
    inline void lpad(_CharType* out, const _CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr && out != nullptr && value != out);
      size_t valueLength = stringLength(value);

      if (valueLength < minSize) {
        stringAssign<_CharType, false>(out, minSize - valueLength, _PadChar);
        out += (minSize - valueLength);
      }
      if (valueLength > size_t{ 0 })
        memcpy((void*)out, (void*)value, valueLength*sizeof(_CharType));
      *(out + valueLength) = static_cast<_CharType>(0);
    }
    /// @brief If size < minSize, add padding at the beginning of the string in same buffer
    /// @warning The 'value' buffer size must be at least (minSize + 1) !
    template <typename _CharType>
    inline void lpadSelf(_CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      if (valueLength < minSize) {
        size_t padding = minSize - valueLength;
        if (valueLength > size_t{ 0 })
          memmove((void*)(value + padding), (void*)value, valueLength*sizeof(_CharType));
        stringAssign<_CharType, false>(value, padding, _PadChar);
        *(value + padding + valueLength) = static_cast<_CharType>(0);
      }
    }

    /// @brief If size < minSize, add padding at the end of the string
    template <typename _CharType>
    inline std::basic_string<_CharType> rpad(const _CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);
      if (valueLength < minSize) {
        std::basic_string<_CharType> padded(value, valueLength);
        padded.append(minSize - valueLength, _PadChar);
        return padded;
      }
      return std::basic_string<_CharType>(value, valueLength);
    }
    /// @brief If size < minSize, add padding at the end of the string in pre-allocated output buffer
    /// @warning The 'out' buffer size must be at least (minSize + 1) !
    template <typename _CharType>
    inline void rpad(_CharType* out, const _CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr && out != nullptr && value != out);
      size_t valueLength = stringLength(value);

      if (valueLength > size_t{ 0 }) {
        memcpy((void*)out, (void*)value, valueLength*sizeof(_CharType));
        out += valueLength;
      }
      if (valueLength < minSize)
        stringAssign<_CharType, true>(out, minSize - valueLength, _PadChar);
      else
        *out = static_cast<_CharType>(0);
    }
    /// @brief If size < minSize, add padding at the end of the string in same buffer
    /// @warning The 'value' buffer size must be at least (minSize + 1) !
    template <typename _CharType>
    inline void rpadSelf(_CharType* value, size_t minSize, _CharType _PadChar = static_cast<_CharType>(' ')) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      if (valueLength < minSize)
        stringAssign<_CharType, true>(value + valueLength, minSize - valueLength, _PadChar);
    }


    // -- trim --

    /// @brief Remove all spaces/tabs/controls chars at the beginning/end of a string
    template <typename _CharType>
    inline std::basic_string<_CharType> trim(const _CharType* value) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      size_t firstIndex = findNoTrimIndex(value);
      size_t afterLastIndex = rfindAfterNoTrimIndex(value, valueLength);
      return (firstIndex < afterLastIndex) ? std::basic_string<_CharType>(value + firstIndex, afterLastIndex - firstIndex) : std::basic_string<_CharType>{};
    }
    /// @brief Remove all spaces/tabs/controls chars at the beginning/end in pre-allocated output buffer
    template <typename _CharType>
    inline void trim(_CharType* out, const _CharType* value) noexcept {
      assert(value != nullptr && out != nullptr && value != out);
      size_t valueLength = stringLength(value);

      size_t firstIndex = findNoTrimIndex(value);
      size_t afterLastIndex = rfindAfterNoTrimIndex(value, valueLength);
      if (firstIndex < afterLastIndex) {
        memcpy((void*)out, (void*)(value + firstIndex), (afterLastIndex - firstIndex)*sizeof(_CharType));
        *(out + (afterLastIndex - firstIndex)) = static_cast<_CharType>(0);
      }
      else
        *out = static_cast<_CharType>(0);
    }
    /// @brief Remove all spaces/tabs/controls chars at the beginning/end of string in same buffer
    template <typename _CharType>
    inline void trimSelf(_CharType* value) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      size_t firstIndex = findNoTrimIndex(value);
      size_t afterLastIndex = rfindAfterNoTrimIndex(value, valueLength);
      if (firstIndex < afterLastIndex) {
        if (firstIndex > size_t{ 0 })
          memmove((void*)value, (void*)(value + firstIndex), (afterLastIndex - firstIndex)*sizeof(_CharType));
        *(value + (afterLastIndex - firstIndex)) = static_cast<_CharType>(0);
      }
      else
        *value = static_cast<_CharType>(0);
    }

    /// @brief Remove all spaces/tabs/controls chars at the beginning of a string
    template <typename _CharType>
    inline std::basic_string<_CharType> ltrim(const _CharType* value) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      size_t firstIndex = findNoTrimIndex(value);
      return std::basic_string<_CharType>(value + firstIndex, valueLength - firstIndex);
    }
    /// @brief Remove all spaces/tabs/controls chars at the beginning in pre-allocated output buffer
    template <typename _CharType>
    inline void ltrim(_CharType* out, const _CharType* value) noexcept {
      assert(value != nullptr && out != nullptr && value != out);
      size_t valueLength = stringLength(value);

      size_t firstIndex = findNoTrimIndex(value);
      if (firstIndex < valueLength) {
        memcpy((void*)out, (void*)(value + firstIndex), (valueLength - firstIndex)*sizeof(_CharType));
        *(out + (valueLength - firstIndex)) = static_cast<_CharType>(0);
      }
      else
        *out = static_cast<_CharType>(0);
    }
    /// @brief Remove all spaces/tabs/controls chars at the beginning of string in same buffer
    template <typename _CharType>
    inline void ltrimSelf(_CharType* value) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      size_t firstIndex = findNoTrimIndex(value);
      if (firstIndex < valueLength) {
        if (firstIndex > size_t{ 0 }) {
          memmove((void*)value, (void*)(value + firstIndex), (valueLength - firstIndex)*sizeof(_CharType));
          *(value + (valueLength - firstIndex)) = static_cast<_CharType>(0);
        }
      }
      else
        *value = static_cast<_CharType>(0);
    }

    /// @brief Remove all spaces/tabs/controls chars at the end of a string
    template <typename _CharType>
    inline std::basic_string<_CharType> rtrim(const _CharType* value) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      size_t afterLastIndex = rfindAfterNoTrimIndex(value, valueLength);
      return std::basic_string<_CharType>(value, afterLastIndex);
    }
    /// @brief Remove all spaces/tabs/controls chars at the end in pre-allocated output buffer
    template <typename _CharType>
    inline void rtrim(_CharType* out, const _CharType* value) noexcept {
      assert(value != nullptr && out != nullptr && value != out);
      size_t valueLength = stringLength(value);

      size_t afterLastIndex = rfindAfterNoTrimIndex(value, valueLength);
      if (afterLastIndex > size_t{ 0 })
        memcpy((void*)out, (void*)value, afterLastIndex*sizeof(_CharType));
      *(out + afterLastIndex) = static_cast<_CharType>(0);
    }
    /// @brief Remove all spaces/tabs/controls chars at the end of string in same buffer
    template <typename _CharType>
    inline void rtrimSelf(_CharType* value) noexcept {
      assert(value != nullptr);
      size_t valueLength = stringLength(value);

      size_t afterLastIndex = rfindAfterNoTrimIndex(value, valueLength);
      if (afterLastIndex != valueLength) {
        if (afterLastIndex > size_t{ 0 })
          memmove((void*)value, (void*)value, afterLastIndex*sizeof(_CharType));
        *(value + afterLastIndex) = static_cast<_CharType>(0);
      }
    }

  }
}
#undef __if_constexpr
