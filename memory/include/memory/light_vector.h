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

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>

#define __P_LTVEC_TYPE_CLASS(datatype)   typename std::enable_if<std::is_class<T>::value, datatype>::type
#define __P_LTVEC_TYPE_TRIVIAL(datatype) typename std::enable_if<!std::is_class<T>::value, datatype>::type

namespace pandora {
  namespace memory {
    /// @class LightVector
    /// @brief Simple vector container with dynamic allocation.
    ///        Useful to avoid the huge weight/overhead of std::vector when not needed.
    /// @remarks If the container size never changes (immutable/fixed-size), use DynamicArray instead.
    template <typename _DataType>
    class LightVector final {
    public:
      using Type = LightVector<_DataType>;

      inline LightVector() noexcept : _value(nullptr), _size(0), _allocSize(0) {}            ///< Create empty vector
      inline LightVector(size_t length) : _size(length), _allocSize(_getAllocSize(length)) { ///< Create vector of default constructed values
        if (this->_size > 0) {
          this->_value = new uint8_t[this->_allocSize*sizeof(_DataType)];
          _constructDefault((_DataType*)this->_value, this->_size);
        }
      }
      inline LightVector(const _DataType* values, size_t length) : _size(length), _allocSize(_getAllocSize(length)) { ///< Create initialized vector
        if (this->_size > 0) {
          this->_value = new uint8_t[this->_allocSize*sizeof(_DataType)];
          _constructCopyData((_DataType*)this->_value, values, length);
        }
      }
      inline ~LightVector() noexcept { clear(); }

      inline LightVector(const Type& rhs) : _size(rhs._size), _allocSize(_getAllocSize(rhs._size)) {
        if (this->_size > 0) {
          this->_value = new uint8_t[this->_allocSize*sizeof(_DataType)];
          _constructCopyData((_DataType*)this->_value, (const _DataType*)rhs._value, rhs._size);
        }
      }
      inline Type& operator=(const Type& rhs) { assign((const _DataType*)rhs._value, rhs._size); return *this; }
      
      inline LightVector(Type&& rhs) noexcept : _value(rhs._value), _size(rhs._size), _allocSize(rhs._allocSize) {
        rhs._value = nullptr; rhs._allocSize = rhs._size = 0;
      }
      inline Type& operator=(Type&& rhs) noexcept {
        clear();
        this->_value = rhs._value; this->_size = rhs._size; this->_allocSize = rhs._allocSize;
        rhs._value = nullptr; rhs._allocSize = rhs._size = 0;
        return *this;
      }
      
      
      // -- accessors --
      
      inline const _DataType* data() const noexcept { return (const _DataType*)this->_value; } ///< Get vector content (NULL if empty)
      inline _DataType* data() noexcept { return (_DataType*)this->_value; }                   ///< Get vector content (NULL if empty)
      
      constexpr inline size_t length() const noexcept { return this->_size; } ///< Get current size of the vector
      constexpr inline size_t size() const noexcept { return this->_size; }   ///< Get current size of the vector
      constexpr inline bool empty() const noexcept { return (this->_value == nullptr); } ///< Verify if the vector is empty
      
      inline const _DataType& operator[](size_t index) const {
        assert(index < this->_size);
        return ((const _DataType*)this->_value)[index];
      }
      inline _DataType& operator[](size_t index) {
        assert(index < this->_size);
        return ((_DataType*)this->_value)[index];
      }

      inline const _DataType& front() const noexcept { assert(this->_value != nullptr); return *(const _DataType*)this->_value; }
      inline const _DataType& back()  const noexcept { assert(this->_value != nullptr); return *((const _DataType*)this->_value + (intptr_t)(this->_size - 1u)); }
      inline _DataType& front() noexcept { assert(this->_value != nullptr); return *(_DataType*)this->_value; }
      inline _DataType& back()  noexcept { assert(this->_value != nullptr); return *((_DataType*)this->_value + (intptr_t)(this->_size - 1u)); }
      
      inline _DataType* begin() noexcept { return (_DataType*)this->_value; }
      inline const _DataType* begin() const noexcept { return (const _DataType*)this->_value; }
      inline const _DataType* cbegin() const noexcept { return (const _DataType*)this->_value; }
      inline _DataType* end() noexcept { return (_DataType*)this->_value + (intptr_t)this->_size; }
      inline const _DataType* end() const noexcept { return (const _DataType*)this->_value + (intptr_t)this->_size; }
      inline const _DataType* cend() const noexcept { return (const _DataType*)this->_value + (intptr_t)this->_size; }
      
      // -- operators --
      
      void clear() noexcept { ///< Clear vector content (set to NULL)
        if (this->_value != nullptr) {
          _destroy((_DataType*)this->_value, this->_size);
          delete[] this->_value;
          this->_value = nullptr;
        }
        this->_allocSize = this->_size = 0;
      }
      
      void assign(const _DataType* value, size_t length) { ///< Assign vector data
        if (this->_allocSize < length) {
          uint8_t* extValue = new uint8_t[_getAllocSize(length)*sizeof(_DataType)];
          if (this->_value != nullptr) {
            _destroy((_DataType*)this->_value, this->_size);
            delete[] this->_value;
          }
          this->_value = extValue;
          this->_allocSize = _getAllocSize(length);
          _constructCopyData((_DataType*)this->_value, value, length);
        }
        else if (length > 0) {
          if (this->_size <= length) {
            _constructCopyData((_DataType*)this->_value + (intptr_t)this->_size, value + (intptr_t)this->_size, length - this->_size);
            _copyData((_DataType*)this->_value, value, this->_size);
          }
          else {
            _destroy((_DataType*)this->_value + (intptr_t)length, this->_size - length);
            _copyData((_DataType*)this->_value, value, length);
          }
        }
        else
          clear();
        this->_size = length;
      }

      void insert(uint32_t index, const _DataType& value) { ///< Insert new vector data at index
        assert(index <= this->_size);
        if (this->_size >= this->_allocSize) {
          size_t allocSize = (this->_allocSize > 0) ? (this->_allocSize << 1) : 4;
          uint8_t* extValue = new uint8_t[allocSize*sizeof(_DataType)];
          if (this->_value != nullptr) {
            _constructMoveData((_DataType*)extValue, (_DataType*)this->_value, index);
            if (index < this->_size) {
              _constructMoveData((_DataType*)extValue + (intptr_t)index + (intptr_t)1,
                                 (_DataType*)this->_value + (intptr_t)index, this->_size - (size_t)index);
              _constructCopyOne((_DataType*)extValue + (intptr_t)index, value);
            }
            else
              _constructCopyOne((_DataType*)extValue + (intptr_t)this->_size, value);
            _destroy((_DataType*)this->_value, this->_size);
            delete[] this->_value;
          }
          else
            _constructCopyOne((_DataType*)extValue, value);

          this->_value = extValue;
          this->_allocSize = allocSize;
        }
        else {
          if (index < this->_size) {
            _shiftRight((_DataType*)this->_value, (intptr_t)index, this->_size);
            *((_DataType*)_value + (intptr_t)index) = value;
          }
          else
            _constructCopyOne((_DataType*)this->_value + (intptr_t)this->_size, value);
        }
        ++_size;
      }
      void push_back(const _DataType& value) { ///< Append new vector data
        if (this->_size >= this->_allocSize) {
          size_t allocSize = (this->_allocSize > 0) ? (this->_allocSize << 1) : 4;
          uint8_t* extValue = new uint8_t[allocSize*sizeof(_DataType)];
          if (this->_value != nullptr) {
            _constructMoveData((_DataType*)extValue, (_DataType*)this->_value, this->_size);
            _destroy((_DataType*)this->_value, this->_size);
            delete[] this->_value;
          }
          this->_value = extValue;
          this->_allocSize = allocSize;
        }
        _constructCopyOne((_DataType*)this->_value + (intptr_t)this->_size, value);
        ++_size;
      }

      void erase(uint32_t index) noexcept { ///< Remove vector data at index
        if (index < this->_size) {
          _shiftLeft((_DataType*)this->_value, index, this->_size);
          --_size;
          if (this->_size == 0) {
            delete[] this->_value;
            this->_value = nullptr;
            this->_allocSize = 0;
          }
        }
      }
      void pop_back() noexcept { ///< Remove vector data at last index
        if (this->_size > 0) {
          --_size;
          _destroyOne((_DataType*)this->_value + (intptr_t)this->_size);
          if (this->_size == 0) {
            delete[] this->_value;
            this->_value = nullptr;
            this->_allocSize = 0;
          }
        }
      }

      // ---

    private:
      static constexpr inline size_t _getAllocSize(size_t length) noexcept { return ((length + 3) & ~(size_t)0x3); }

      // -- private - class item types --

      template <typename T = _DataType>
      static inline void _constructDefault(__P_LTVEC_TYPE_CLASS(_DataType*) lhs, size_t length) noexcept {
        for (const _DataType* lhsEnd = lhs + (intptr_t)length; lhs < lhsEnd; ++lhs)
          new(lhs) _DataType();
      }
      template <typename T = _DataType>
      static inline void _constructCopyOne(__P_LTVEC_TYPE_CLASS(_DataType*) lhs, const _DataType& rhs) noexcept {
        new(lhs) _DataType(rhs);
      }
      template <typename T = _DataType>
      static inline void _constructCopyData(__P_LTVEC_TYPE_CLASS(_DataType*) lhs, const _DataType* rhs, size_t length) noexcept {
        for (const _DataType* lhsEnd = lhs + (intptr_t)length; lhs < lhsEnd; ++lhs, ++rhs)
          new(lhs) _DataType(*rhs);
      }
      template <typename T = _DataType>
      static inline void _constructMoveData(typename std::enable_if<std::is_class<T>::value
                                            && std::is_move_constructible<T>::value, _DataType*>::type lhs, _DataType* rhs, size_t length) noexcept {
        for (const _DataType* lhsEnd = lhs + (intptr_t)length; lhs < lhsEnd; ++lhs, ++rhs)
          new(lhs) _DataType(*rhs);
      }

      template <typename T = _DataType>
      static inline void _copyData(__P_LTVEC_TYPE_CLASS(_DataType*) lhs, const _DataType* rhs, size_t length) noexcept {
        for (const _DataType* lhsEnd = lhs + (intptr_t)length; lhs < lhsEnd; ++lhs, ++rhs)
          *lhs = *rhs;
      }
      template <typename T = _DataType>
      static inline void _shiftLeft(__P_LTVEC_TYPE_CLASS(_DataType*) value, uint32_t erasedIndex, size_t totalSize) noexcept {
        _DataType* lhs = value + (intptr_t)erasedIndex;
        _DataType* rhs = value + (intptr_t)erasedIndex + (intptr_t)1u;
        for (const _DataType* endIt = value + (intptr_t)totalSize; rhs < endIt; ++lhs, ++rhs)
          *lhs = std::move(*rhs);
        (*lhs).~_DataType();
      }
      template <typename T = _DataType>
      static inline void _shiftRight(typename std::enable_if<std::is_class<T>::value
                                     && std::is_move_constructible<T>::value, _DataType*>::type value, uint32_t firstIndex, size_t oldTotalSize) noexcept {
        _DataType* lhs = value + (intptr_t)oldTotalSize;
        _DataType* rhs = value + (intptr_t)oldTotalSize - (intptr_t)1u;
        new(lhs) _DataType(std::move(*rhs));
        --lhs; --rhs;
        for (const _DataType* firstIndexPos = value + (intptr_t)firstIndex; rhs >= firstIndexPos; --lhs, --rhs)
          *lhs = std::move(*rhs);
      }
      template <typename T = _DataType>
      static inline void _shiftRight(typename std::enable_if<std::is_class<T>::value
                                     && !std::is_move_constructible<T>::value, _DataType*>::type value, uint32_t firstIndex, size_t oldTotalSize) noexcept {
        _DataType* lhs = value + (intptr_t)oldTotalSize;
        _DataType* rhs = value + (intptr_t)oldTotalSize - (intptr_t)1u;
        new(lhs) _DataType(*rhs);
        --lhs; --rhs;
        for (const _DataType* firstIndexPos = value + (intptr_t)firstIndex; rhs >= firstIndexPos; --lhs, --rhs)
          *lhs = *rhs;
      }

      template <typename T = _DataType>
      static inline void _destroyOne(__P_LTVEC_TYPE_CLASS(_DataType*) lhs) noexcept {
        (*lhs).~_DataType();
      }
      template <typename T = _DataType>
      static inline void _destroy(__P_LTVEC_TYPE_CLASS(_DataType*) lhs, size_t length) noexcept {
        for (const _DataType* lhsEnd = lhs + (intptr_t)length; lhs < lhsEnd; ++lhs)
          (*lhs).~_DataType();
      }

      // -- private -- trivial item types --

      template <typename T = _DataType>
      static inline void _constructDefault(__P_LTVEC_TYPE_TRIVIAL(_DataType*) lhs, size_t length) noexcept {
        memset(lhs, 0, length*sizeof(_DataType));
      }
      template <typename T = _DataType>
      static inline void _constructCopyOne(__P_LTVEC_TYPE_TRIVIAL(_DataType*) lhs, const _DataType& rhs) noexcept {
        *lhs = rhs;
      }
      template <typename T = _DataType>
      static inline void _constructCopyData(__P_LTVEC_TYPE_TRIVIAL(_DataType*) lhs, const _DataType* rhs, size_t length) noexcept {
        memcpy(lhs, rhs, length*sizeof(_DataType));
      }
      template <typename T = _DataType>
      static inline void _constructMoveData(typename std::enable_if<!std::is_class<T>::value
                                            || !std::is_move_constructible<T>::value, _DataType*>::type lhs, _DataType* rhs, size_t length) noexcept {
        _constructCopyData(lhs, rhs, length);
      }

      template <typename T = _DataType>
      static inline void _copyData(__P_LTVEC_TYPE_TRIVIAL(_DataType*) lhs, const _DataType* rhs, size_t length) noexcept {
        memcpy(lhs, rhs, length*sizeof(_DataType));
      }
      template <typename T = _DataType>
      static inline void _shiftLeft(__P_LTVEC_TYPE_TRIVIAL(_DataType*) value, uint32_t erasedIndex, size_t totalSize) noexcept {
        memmove(value + (intptr_t)erasedIndex, value + (intptr_t)erasedIndex + (intptr_t)1u,
                (totalSize - (size_t)erasedIndex - (size_t)1u)*sizeof(_DataType));
      }
      template <typename T = _DataType>
      static inline void _shiftRight(__P_LTVEC_TYPE_TRIVIAL(_DataType*) value, uint32_t firstIndex, size_t oldTotalSize) noexcept {
        memmove(value + (intptr_t)firstIndex + (intptr_t)1u, value + (intptr_t)firstIndex,
                (oldTotalSize - (size_t)firstIndex)*sizeof(_DataType));
      }

      template <typename T = _DataType>
      static inline void _destroyOne(__P_LTVEC_TYPE_TRIVIAL(_DataType*)) noexcept {}
      template <typename T = _DataType>
      static inline void _destroy(__P_LTVEC_TYPE_TRIVIAL(_DataType*), size_t) noexcept {}

    private:
      uint8_t* _value = nullptr;
      size_t _size = 0u;
      size_t _allocSize = 0u;
    };
  }
}
#undef __P_LTVEC_TYPE_CLASS
#undef __P_LTVEC_TYPE_TRIVIAL
