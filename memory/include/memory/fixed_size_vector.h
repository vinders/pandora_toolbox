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
#include <cstring>
#include <array>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>
#include <pattern/optional.h>
#include <pattern/iterator.h>
#include <pattern/class_traits.h>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 26495)
#endif

namespace pandora {
  namespace memory {
    /// @class FixedSizeVector
    /// @brief A container that behaves like a vector with a max length, without involving any dynamic allocation.
    ///        Useful for real-time apps or for any vector with a fixed max size. The vector is STL-compliant.
    template <typename _DataType,  // Data type for vector items.
              size_t _Capacity>    // Max number of items that may be contained in the fixed-size vector.
    class FixedSizeVector final : public pandora::pattern::ConstructorSelector<_DataType>, public pandora::pattern::AssignmentSelector<_DataType> {
    public:
      using value_type = _DataType;
      using item_type = typename std::conditional<std::is_class<_DataType>::value, pandora::pattern::Optional<_DataType>, _DataType>::type;
      using size_type = size_t;
      using reference = _DataType&;
      using const_reference = const _DataType&;
      using pointer = _DataType*;
      using const_pointer = const _DataType*;
      using Type = FixedSizeVector<_DataType,_Capacity>;
      static_assert((_Capacity > 0u), "FixedSizeVector: _Capacity can't be 0.");
      static_assert((_Capacity < 0x7FFFFFFFu), "FixedSizeVector: _Capacity can't exceed or equal 0x7FFFFFFF.");
      static_assert(noexcept(!std::is_copy_constructible<_DataType>::value
                          || std::is_nothrow_copy_constructible<_DataType>::value), "FixedSizeVector: _DataType copy constructor (if present) cannot throw");
      static_assert(noexcept(!std::is_move_constructible<_DataType>::value
                          || (std::is_nothrow_move_constructible<_DataType>::value
                            && std::is_nothrow_move_assignable<_DataType>::value) ), "FixedSizeVector: _DataType move constructor/operator (if present) cannot throw");

      /// @brief Create empty vector
      FixedSizeVector() = default;
      /// @brief Destroy vector and all child instances
      ~FixedSizeVector() noexcept { clear(); }

      FixedSizeVector(const Type& rhs) noexcept : pandora::pattern::ConstructorSelector<_DataType>(rhs)       { _assign<_Capacity,_DataType>(rhs); }
      FixedSizeVector(Type&& rhs) noexcept : pandora::pattern::ConstructorSelector<_DataType>(std::move(rhs)) { _assign<_DataType>(std::move(rhs)); }
      Type& operator=(const Type& rhs) noexcept { pandora::pattern::AssignmentSelector<_DataType>::operator=(rhs);
        clear(); 
        _assign<_Capacity,_DataType>(rhs); 
        return *this;
      }
      Type& operator=(Type&& rhs) noexcept { pandora::pattern::AssignmentSelector<_DataType>::operator=(std::move(rhs));
        clear();
        _assign<_DataType>(std::move(rhs)); 
        return *this; 
      }

      template <typename T = _DataType>
      inline void swap(typename std::enable_if<std::is_class<T>::value && !std::is_move_constructible<T>::value, Type&>::type rhs) noexcept {
        Type buffer(rhs);
        rhs = *this;
        *this = buffer;
      }
      template <typename T = _DataType>
      inline void swap(typename std::enable_if<!std::is_class<T>::value || std::is_move_constructible<T>::value, Type&>::type rhs) noexcept {
        Type buffer(std::move(rhs));
        rhs = std::move(*this);
        *this = std::move(buffer);
      }

      // -- special constructors --

      /// @brief Create a vector initialized with a value repeated N times (N = 'count')
      /// @warning If (count > _Capacity): will use _Capacity as max size
      template <typename T = _DataType>
      explicit FixedSizeVector(size_t count, pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept { assign<_DataType>(count, value); }
      /// @brief Create a vector initialized with a range of iterators ('end' is excluded)
      /// @warning If (size > _Capacity): will use _Capacity as max size
      template <typename _Iterator, typename T = _DataType, pandora::pattern::enable_if_copy_constructible<T,int> = 0>
      FixedSizeVector(pandora::pattern::enable_if_iterator<_Iterator, _Iterator> first, _Iterator end) noexcept { assign<_Iterator,_DataType>(first, end); }

      /// @brief Create a vector with an initializer list
      /// @warning If (init.size() > _Capacity): will use _Capacity as max size
      template <typename T = _DataType, pandora::pattern::enable_if_copy_constructible<T, int> = 0>
      FixedSizeVector(std::initializer_list<_DataType> init) noexcept {
        auto endIt = _fixEndIterator(init.begin(), init.end(), init.size(), _Capacity);
        for (auto it = init.begin(); it != endIt; ++it)
          _emplace<false>(*it);
      }


      // -- getters --

      inline const item_type* data() const noexcept { return this->_value.data(); } ///< Get pointer to raw vector data

      constexpr inline size_t size() const noexcept { return this->_size; }          ///< Get current size of the vector
      constexpr static inline size_t max_size() noexcept { return _Capacity; } ///< Get fixed max capacity of the vector
      constexpr static inline size_t capacity() noexcept { return _Capacity; } ///< Get fixed max capacity of the vector
      constexpr inline bool empty() const noexcept { return (this->_size == 0u); }   ///< Check if the vector is empty
      constexpr inline bool full() const noexcept  { return (this->_size == _Capacity); } ///< Check if the max capacity of the vector has been reached

      inline _DataType& operator[](size_t index) noexcept { assert(index < this->_size); return _at(index); }             ///< Read value at a custom index (not verified!)
      inline const _DataType& operator[](size_t index) const noexcept { assert(index < this->_size); return _at(index); } ///< Read value at a custom index (not verified!) - const
      inline _DataType& at(size_t index) { ///< Read value at a custom index, with index verification (throws)
        if (index < this->_size)
          return _at(index);
        throw std::out_of_range("FixedSizeVector.at: invalid index (out of range).");
      }
      inline const _DataType& at(size_t index) const { ///< Read value at a custom index, with index verification - const (throws)
        if (index < this->_size)
          return _at(index);
        throw std::out_of_range("FixedSizeVector.at: invalid index (out of range).");
      }

      inline _DataType& front() noexcept { assert(!empty()); return _at(0); }             ///< Read first value of the vector (not verified -> check empty() !)
      inline const _DataType& front() const noexcept { assert(!empty()); return _at(0); } ///< Read first value of the vector (not verified -> check empty() !) - const
      inline _DataType& back() noexcept { assert(!empty()); return _at(this->_size - 1u); }             ///< Read last value of the vector (not verified -> check empty() !)
      inline const _DataType& back() const noexcept { assert(!empty()); return _at(this->_size - 1u); } ///< Read last value of the vector (not verified -> check empty() !)

      _P_RANDOMACCESS_ITERATOR_MEMBERS(Type, _DataType, _first(), _last());

      // -- comparisons --

      template <typename T = _DataType>
      inline bool operator==(pandora::pattern::enable_if_operator_equals<T, const Type&> rhs) const noexcept {
        if (this->_size == rhs._size) {
          if (this->_size > size_t{ 0u }) {
            const item_type* it = &(this->_value[this->_size - 1u]);
            const item_type* rhsIt = &(rhs._value[rhs._size - 1u]);
            while (it >= &(this->_value[0])) {
              if (_getValue(*it) != _getValue(*rhsIt))
                return false;
              --it;
              --rhsIt;
            }
          }
          return true;
        }
        return false;
      }
      template <typename T = _DataType>
      inline bool operator!=(pandora::pattern::enable_if_operator_equals<T, const Type&> rhs) const noexcept { 
        return !(this->operator==(rhs)); 
      }


      // -- change size --

      /// @brief Remove all vector items
      inline void clear() noexcept {
        while (_removeLast<true>());
      }
      /// @brief Change size of vector - add default values to reach new size or remove last items
      /// @warning If (length > _Capacity): will use _Capacity as max size
      template <typename T = _DataType>
      inline void resize(size_t length, pandora::pattern::enable_if_copy_constructible<T, const T&> defaultValue) noexcept {
        _fixMaxLength(length, _Capacity);
        while (length < this->_size)
          _removeLast<false>();
        while (length > this->_size)
          _emplace<false>(defaultValue);
      }

      // -- assignment --

      /// @brief Clear vector and assign a value repeated N times (N = 'count')
      /// @warning If (count > _Capacity): will use _Capacity as max size
      /// @returns New size of vector
      template <typename T = _DataType>
      inline size_t assign(size_t count, pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept {
        _fixMaxLength(count, _Capacity);
        clear();
        while (count > 0u) {
          _emplace<false>(value);
          --count;
        }
        return this->_size;
      }

      /// @brief Clear vector and assign a range of iterators ('end' is excluded)
      /// @warning If (size > _Capacity): will use _Capacity as max size
      /// @returns New size of vector
      template <typename _Iterator, typename T = _DataType, pandora::pattern::enable_if_copy_constructible<T,int> = 0>
      inline size_t assign(pandora::pattern::enable_if_iterator<_Iterator, _Iterator> first, _Iterator end) noexcept {
        end = _fixEndIterator(first, end, std::distance(first, end), _Capacity);
        clear();
        while (first < end) {
          _emplace<false>(*first);
          ++first;
        }
        return this->_size;
      }

      // -- append --

      /// @brief Add a value at the end of the vector
      /// @warning If vector already full: returns false
      template <typename T = _DataType>
      inline bool push_back(pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept { return _emplace<true>(value); }
      template <typename T = _DataType>
      inline bool push_back(pandora::pattern::enable_if_move_constructible<T, _DataType&&> value) noexcept { return _emplace<true>(std::move(value)); }
      /// @brief Create a value in place at the end of the vector
      /// @warning If vector already full: returns false
      template<class... Args>
      inline bool emplace_back(Args&&... args) { return _emplace<true>(std::forward<Args>(args)...); }

      // -- insert --

      /// @brief Insert value at the position identified by an iterator
      /// @warning If vector already full: returns end()
      template <typename T = _DataType>
      inline iterator insert(const_iterator position, pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept {
        const uint32_t posIndex = (position.rawIndex() <= static_cast<uint32_t>(this->_size)) ? position.rawIndex() : static_cast<uint32_t>(this->_size);
        if (_emplaceInsert<true,true>(posIndex, value))
          return iterator(*this, &(_at(posIndex)), posIndex);
        return this->end();
      }
      template <typename T = _DataType>
      inline iterator insert(const_iterator position, pandora::pattern::enable_if_move_constructible<T, T&&> value) noexcept {
        const uint32_t posIndex = (position.rawIndex() <= static_cast<uint32_t>(this->_size)) ? position.rawIndex() : static_cast<uint32_t>(this->_size);
        if (_emplaceInsert<true,true>(posIndex, std::move(value)))
          return iterator(*this, &(_at(posIndex)), posIndex);
        return this->end();
      }
      /// @brief Create value in place at the position identified by an iterator
      /// @warning If vector already full: returns end()
      template <typename... Args>
      inline iterator emplace(const_iterator position, Args&&... args) {
        const uint32_t posIndex = (position.rawIndex() <= static_cast<uint32_t>(this->_size)) ? position.rawIndex() : static_cast<uint32_t>(this->_size);
        if (_emplaceInsert<true,true>(posIndex, std::forward<Args>(args)...))
          return iterator(*this, &(_at(posIndex)), posIndex);
        return this->end();
      }

      /// @brief Insert value repeated N times (N = 'count') at the position identified by an iterator
      /// @warning If vector already full: returns end()
      template <typename T = _DataType>
      inline iterator insert(const_iterator position, size_t count, pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept {
        _fixMaxLength(count, _Capacity - this->_size);
        const uint32_t posIndex = (position.rawIndex() <= static_cast<uint32_t>(this->_size)) ? position.rawIndex() : static_cast<uint32_t>(this->_size);

        if (count > 0) {
          _rshiftItems(posIndex, count);
          for (uint32_t index = posIndex; count > 0u; ++index, --count) {
            _emplaceInsert<false,false>(index, value);
          }
          return iterator(*this, &(_at(posIndex)), posIndex);
        }
        return this->end();
      }
      /// @brief Insert a range of iterators at the position identified by an iterator ('end' is excluded)
      /// @warning If vector already full: returns end()
      template <typename _Iterator, typename T = _DataType, pandora::pattern::enable_if_copy_constructible<T,int> = 0>
      inline iterator insert(const_iterator position, pandora::pattern::enable_if_iterator<_Iterator, _Iterator> first, _Iterator end) noexcept {
        size_t count = std::distance(first, end);
        end = _fixEndIteratorAndLength<_Iterator>(first, end, count, _Capacity - this->_size);
        const uint32_t posIndex = (position.rawIndex() <= static_cast<uint32_t>(this->_size)) ? position.rawIndex() : static_cast<uint32_t>(this->_size);

        if (first < end) {
          _rshiftItems(posIndex, count);
          for (uint32_t index = posIndex; first < end; ++index, ++first) {
            _emplaceInsert<false,false>(index, *first);
          }
          return iterator(*this, &(_at(posIndex)), posIndex);
        }
        return this->end();
      }

      /// @brief Insert initializer list of values at the position identified by an iterator
      /// @warning If vector already full: returns end()
      template <typename T = _DataType>
      inline iterator insert(const_iterator position, pandora::pattern::enable_if_copy_constructible<T, std::initializer_list<T> > init) noexcept {
        size_t count = init.size();
        auto endIt = _fixEndIteratorAndLength(init.begin(), init.end(), count, _Capacity - this->_size);
        const uint32_t posIndex = (position.rawIndex() <= static_cast<uint32_t>(this->_size)) ? position.rawIndex() : static_cast<uint32_t>(this->_size);

        if (count > 0) {
          _rshiftItems(posIndex, count);
          uint32_t index = posIndex;
          for (auto it = init.begin(); it != endIt; ++it, ++index) {
            _emplaceInsert<false,false>(index, *it);
          }
          return iterator(*this, &(_at(posIndex)), posIndex);
        }
        return this->end();
      }

      // -- erase --

      /// @brief Remove last vector item
      inline bool pop_back() noexcept { return _removeLast<true>(); }

      /// @brief Remove vector item at the index position
      template <typename T = _DataType>
      inline pandora::pattern::enable_if_copy_or_move_constructible<T, bool> erase(size_t index) noexcept {
        return _remove<true,true>(index);
      }
      /// @brief Remove N vector items (N = 'count') from the index position
      template <typename T = _DataType>
      inline pandora::pattern::enable_if_copy_or_move_constructible<T, size_t> erase(size_t index, size_t count) noexcept {
        if (index < this->_size) {
          size_t endIndex = index + count;
          if (endIndex > this->_size)
            endIndex = this->_size;

          for (size_t cur = index; cur < endIndex; ++cur)
            _remove<false,false>(cur);
          _lshiftItems(endIndex, static_cast<size_t>(endIndex - index));
          this->_size -= (endIndex - index);
          return (endIndex - index);
        }
        return size_t{ 0 };
      }

      /// @brief Remove vector item at the position identified by an iterator
      template <typename T = _DataType>
      inline pandora::pattern::enable_if_copy_or_move_constructible<T, iterator> erase(const_iterator position) noexcept {
        if (_remove<true,true>(position.rawIndex()))
          return iterator(*this, (position.rawIndex() < this->_size) ? &(_at(position.rawIndex())) : nullptr, position.rawIndex());
        return end();
      }
      /// @brief Remove vector items located in a range of iterators ('end' is excluded)
      template <typename T = _DataType>
      pandora::pattern::enable_if_copy_or_move_constructible<T, iterator> erase(const_iterator first, const_iterator end) noexcept {
        if (first < end && first.rawIndex() < this->_size) {
          uint32_t endIndex = first.rawIndex() + static_cast<uint32_t>(end - first);
          if (endIndex > static_cast<uint32_t>(this->_size))
            endIndex = static_cast<uint32_t>(this->_size);

          for (uint32_t cur = first.rawIndex(); cur < endIndex;  ++cur)
            _remove<false,false>(cur);
          _lshiftItems(endIndex, static_cast<size_t>(endIndex - first.rawIndex()));
          this->_size -= (endIndex - first.rawIndex());
          return iterator(*this, (first.rawIndex() < static_cast<uint32_t>(this->_size)) ? &(_at(first.rawIndex())) : nullptr, first.rawIndex());
        }
        return this->end();
      }

      // -- iterators --

      _DataType* next(_DataType* current, uint32_t currentIndex, size_t offset = 1u) noexcept { return _next(current, currentIndex, offset); }
      const _DataType* next(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) const noexcept { return _next(current, currentIndex, offset); }
      _DataType* previous(_DataType* current, uint32_t currentIndex, size_t offset = 1u) noexcept { return _previous(current, currentIndex, offset); }
      const _DataType* previous(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) const noexcept { return _previous(current, currentIndex, offset); }


    private:
      // -- private - conditional getters --

      inline _DataType& _at(size_t index) noexcept { return Type::_getValue(this->_value[index]); }
      inline const _DataType& _at(size_t index) const noexcept { return Type::_getValue(this->_value[index]); }

      static inline _DataType& _getValue(pandora::pattern::Optional<_DataType>& location) noexcept { return location.value(); }
      static inline const _DataType& _getValue(const pandora::pattern::Optional<_DataType>& location) noexcept { return location.value(); }
      static inline _DataType& _getValue(_DataType& location) noexcept { return location; }
      static inline const _DataType& _getValue(const _DataType& location) noexcept { return location; }

      // -- private - conditional setters --

      template <typename T = _DataType, typename... Args>
      inline void _emplaceAt(typename std::enable_if<std::is_class<T>::value, size_t>::type index, Args&&... args) {
        this->_value[index].emplace(std::forward<Args>(args)...);
      }
      template <typename T = _DataType, typename... Args>
      inline void _emplaceAt(typename std::enable_if<!std::is_class<T>::value, size_t>::type index, Args&&... args) noexcept {
        this->_value[index] = T(std::forward<Args>(args)...);
      }
      
      template <typename T = _DataType, typename std::enable_if<std::is_class<T>::value && std::is_move_constructible<T>::value,int>::type = 0>
      inline void _moveItem(size_t fromIndex, size_t toIndex) noexcept { this->_value[toIndex] = std::move(this->_value[fromIndex].value()); }
      template <typename T = _DataType, typename std::enable_if<!std::is_class<T>::value || !std::is_move_constructible<T>::value,int>::type = 0>
      inline void _moveItem(size_t fromIndex, size_t toIndex) noexcept { this->_value[toIndex] = this->_value[fromIndex]; }

      template <typename T = _DataType, typename std::enable_if<std::is_class<T>::value, int>::type = 0>
      inline void _clearAt(size_t index) noexcept { this->_value[index].reset(); }
      template <typename T = _DataType, typename std::enable_if<!std::is_class<T>::value, int>::type = 0>
      inline void _clearAt(size_t index) noexcept { this->_value[index] = (T)0; }

      // -- private - operations --

      inline void _lshiftItems(size_t firstIndex, size_t offset) noexcept {
        while (firstIndex < this->_size) {
          _moveItem(firstIndex, firstIndex - offset); // _size must not be modified here !
          ++firstIndex;
        }
      }
      inline void _rshiftItems(size_t firstIndex, size_t offset) noexcept {
        size_t afterLastIndex = this->_size;
        while (afterLastIndex > firstIndex) {
          _moveItem(afterLastIndex - 1u, afterLastIndex + offset - 1u); // _size must not be modified here !
          --afterLastIndex;
        }
      }

      template <size_t _RhsCapacity, typename T = _DataType>
      inline void _assign(pandora::pattern::enable_if_copy_constructible<T, const FixedSizeVector<T,_RhsCapacity>&> rhs) noexcept {
        for (size_t index = 0u; index < rhs.size(); ++index)
          this->_value[index] = rhs._value[index];
        this->_size = rhs.size();
      }
      template <typename T = _DataType>
      inline void _assign(pandora::pattern::enable_if_move_constructible<T, Type&&> rhs) noexcept {
        for (size_t index = 0u; index < rhs._size; ++index)
          this->_value[index] = std::move(rhs._value[index]);
        this->_size = rhs._size;
        rhs._size = 0;
      }

      template<bool _DoSizeCheck, typename... Args>
      inline bool _emplace(Args&&... args) {
        __if_constexpr (_DoSizeCheck) {
          if (this->_size >= _Capacity)
            return false;
        }
        _emplaceAt<_DataType>(this->_size, std::forward<Args>(args)...);
        ++(this->_size);
        return true;
      }

      template <bool _DoSizeCheck, bool _DoShiftNext, typename... Args>
      inline bool _emplaceInsert(size_t index, Args&&... args) {
        __if_constexpr (_DoSizeCheck) {
          if (this->_size >= _Capacity)
            return false;
        }
        __if_constexpr (_DoShiftNext)
          _rshiftItems(index, 1u);
        _emplaceAt<_DataType>(index, std::forward<Args>(args)...);
        ++(this->_size);
        return true;
      }

      template <bool _DoSizeCheck>
      inline bool _removeLast() noexcept {
        __if_constexpr (_DoSizeCheck) {
          if (this->_size == 0u)
            return false;
        }
        --(this->_size);
        _clearAt<_DataType>(this->_size);
        return true;
      }

      template <bool _DoIndexCheck, bool _DoShiftNext, typename T = _DataType>
      inline bool _remove(pandora::pattern::enable_if_copy_or_move_constructible<T, size_t> index) noexcept {
        __if_constexpr (_DoIndexCheck) {
          if (index >= this->_size)
            return false;
        }
        _clearAt<_DataType>(index);
        __if_constexpr (_DoShiftNext) {
          _lshiftItems(index + 1u, 1u);
          --(this->_size);
        }
        return true;
      }

      // -- private - length verifications --

      static inline void _fixMaxLength(size_t& length, size_t maxLength) noexcept {
        if (length > maxLength)
          length = maxLength;
      }
      template <typename _Iterator>
      static inline _Iterator _fixEndIterator(_Iterator first, _Iterator end, size_t expectedSize, size_t availableSize) noexcept {
        return (expectedSize <= availableSize || first >= end) ? end : first + availableSize;
      }
      template <typename _Iterator>
      static inline _Iterator _fixEndIteratorAndLength(_Iterator first, _Iterator end, size_t& expectedSize, size_t availableSize) noexcept {
        if (expectedSize <= availableSize || first >= end)
          return end;
        else {
          expectedSize = availableSize;
          return first + availableSize;
        }
      }

      // -- private - iterators --

      inline _DataType* _first() noexcept { return (!empty()) ? &front() : nullptr; }
      const inline _DataType* _first() const noexcept { return (!empty()) ? &front() : nullptr; }
      inline _DataType* _last() noexcept { return (!empty()) ? &back() : nullptr; }
      const inline _DataType* _last() const noexcept { return (!empty()) ? &back() : nullptr; }

      _DataType* _next(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) noexcept {
        if (current != nullptr && currentIndex + offset < this->_size)
          return &(_at(currentIndex + offset));
        return nullptr;
      }
      const _DataType* _next(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) const noexcept {
        if (current != nullptr && currentIndex + offset < this->_size)
          return &(_at(currentIndex + offset));
        return nullptr;
      }
      _DataType* _previous(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) noexcept {
        if (current != nullptr && currentIndex >= offset && currentIndex - offset < this->_size)
          return &(_at(currentIndex - offset));
        return nullptr;
      }
      const _DataType* _previous(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) const noexcept {
        if (current != nullptr && currentIndex >= offset && currentIndex - offset < this->_size)
          return &(_at(currentIndex - offset));
        return nullptr;
      }

    private:
      std::array<item_type, _Capacity> _value{};
      size_t _size{ 0u };
    };

  }
}
#undef __if_constexpr
#ifdef _MSC_VER
# pragma warning(pop)
#endif
