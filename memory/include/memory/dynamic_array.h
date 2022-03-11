/*******************************************************************************
MIT License
Copyright (c) 2022 Romain Vinders

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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace pandora {
  namespace memory {
    /// @class DynamicArray
    /// @brief RAII dynamically-allocated array container with size defined at runtime (as a constructor param).
    ///        Useful to avoid the huge weight/overhead of std::vector (or even the reduced cost of LightVector)
    ///        when the container size never changes.
    /// @remarks If the container needs to be resized (entries appended/inserted/erased), use LightVector instead.
    template <typename _DataType>
    struct DynamicArray final {
      constexpr inline DynamicArray() noexcept : _value(nullptr) {} ///< Create empty array
      inline DynamicArray(size_t length) ///< Create fixed-size array of default constructed values
        : _value(length ? new _DataType[length]() : nullptr), _length(length) {}
      inline DynamicArray(const _DataType* values, size_t length) ///< Create initialized array
        : _value(length ? new _DataType[length] : nullptr), _length(length) {
        _constructCopyData<_DataType>(_value, values, _length);
      }
      inline ~DynamicArray() noexcept {
        if (_value != nullptr)
          delete[] _value;
      }

      inline DynamicArray(const DynamicArray& rhs)
        : _value(rhs._length ? new _DataType[rhs._length] : nullptr), _length(rhs._length) {
        _constructCopyData<_DataType>(_value, rhs._value, _length);
      }
      inline DynamicArray& operator=(const DynamicArray& rhs) {
        clear();
        this->_value = rhs._length ? new _DataType[rhs._length] : nullptr;
        this->_length = rhs._length;
        _constructCopyData<_DataType>(_value, rhs._value, _length);
      }
      inline DynamicArray(DynamicArray&& rhs) noexcept : _value(rhs._value), _length(rhs._length) {
        rhs._value = nullptr;
        rhs._length = 0;
      }
      inline DynamicArray& operator=(DynamicArray&& rhs) noexcept { 
        if (_value != nullptr)
          delete[] _value;
        _value = rhs._value;
        _length = rhs._length;
        rhs._value = nullptr;
        rhs._length = 0;
        return *this;
      }
      
      // -- accessors --
      
      inline const _DataType* data() const noexcept { return this->_value; } ///< Get array content (NULL if empty)
      inline _DataType* data() noexcept { return this->_value; }             ///< Get array content (NULL if empty)
      
      constexpr inline size_t length() const noexcept { return this->_length; } ///< Get fixed array size
      constexpr inline size_t size() const noexcept { return this->_length; }   ///< Get fixed array size
      constexpr inline bool empty() const noexcept { return (this->_length == 0); } ///< Verify if the array is empty
      
      inline const _DataType& operator[](size_t index) const noexcept {
        assert(index < _length);
        return this->_value[index];
      }
      inline _DataType& operator[](size_t index) noexcept {
        assert(index < _length);
        return this->_value[index];
      }

      inline const _DataType& front() const noexcept { assert(_value != nullptr); return *_value; }
      inline const _DataType& back()  const noexcept { assert(_value != nullptr); return *(_value + (intptr_t)_length - 1u); }
      inline _DataType& front() noexcept { assert(_value != nullptr); return *_value; }
      inline _DataType& back()  noexcept { assert(_value != nullptr); return *(_value + (intptr_t)_length - 1u); }
      
      inline _DataType* begin() noexcept { return this->_value; }
      inline const _DataType* begin() const noexcept { return this->_value; }
      inline const _DataType* cbegin() const noexcept { return this->_value; }
      inline _DataType* end() noexcept { return this->_value + (intptr_t)this->_length; }
      inline const _DataType* end() const noexcept { return this->_value + (intptr_t)this->_length; }
      inline const _DataType* cend() const noexcept { return this->_value + (intptr_t)this->_length; }
      
      
      // -- operators --

      void clear() noexcept { ///< Clear array (set size 0)
        if (_value != nullptr)
          delete[] _value;
        _value = nullptr;
        _length = 0;
      }

    private:
      template <typename T = _DataType>
      static inline void _constructCopyData(typename std::enable_if<std::is_class<T>::value, _DataType*>::type lhs,
                                            const _DataType* rhs, size_t length) noexcept {
        for (const _DataType* lhsEnd = lhs + (intptr_t)length; lhs < lhsEnd; ++lhs, ++rhs)
          *lhs = _DataType(*rhs);
      }
      template <typename T = _DataType>
      static inline void _constructCopyData(typename std::enable_if<!std::is_class<T>::value, _DataType*>::type lhs,
                                            const _DataType* rhs, size_t length) noexcept {
        memcpy(lhs, rhs, length*sizeof(_DataType));
      }

    private:
      _DataType* _value = nullptr;
      size_t _length = 0;
    };
  }
}
