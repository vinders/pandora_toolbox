/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Types: Iterator, Iterable
Macros: 
  * Call one of these inside collection class (that implements previous/next) :
    _P_FORWARD_ITERATOR_MEMBERS, _P_BIDIRECT_ITERATOR_MEMBERS, _P_RANDOMACCESS_ITERATOR_MEMBERS,
  * Or call any of these inside collection class for custom behavior :
    _P_ITERATOR_MEMBERS, _P_REVERSE_ITERATOR_MEMBERS
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <iterator>
#include "./class_traits.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora {
  namespace pattern {
    /// @class Iterable
    /// @brief Iterable template - object that can be iterated through
    template <typename _ItemType, bool _IsReversible = true>
    class Iterable {};

    template <typename _ItemType>
    class Iterable<_ItemType, false> {
    public:
      virtual ~Iterable() {}

      /// @brief Get pointer to following element at position (current + 'offset')
      virtual _ItemType* next(_ItemType* current, uint32_t currentIndex, size_t offset) = 0;
      virtual const _ItemType* next(const _ItemType* current, uint32_t currentIndex, size_t offset) const = 0;
    };
    template <typename _ItemType>
    class Iterable<_ItemType, true> {
    public:
      virtual ~Iterable() {}

      /// @brief Get pointer to following element at position (current + 'offset')
      virtual _ItemType* next(_ItemType* current, uint32_t currentIndex, size_t offset) = 0;
      virtual const _ItemType* next(const _ItemType* current, uint32_t currentIndex, size_t offset) const = 0;
      /// @brief Get pointer to preceding element at position (current - 'offset')
      virtual _ItemType* previous(_ItemType* current, uint32_t currentIndex, size_t offset) = 0;
      virtual const _ItemType* previous(const _ItemType* current, uint32_t currentIndex, size_t offset) const = 0;
    };
    
    // ---
    
    /// @class Iterator
    /// @brief Iterator pattern - moves from the beginning to the end of a collection type
    /// @note  * _CollectionType can be: 
    ///          - either an object specializing or inheriting Iterable;
    ///          - or any object with the same method signatures (previous/next)
    ///        * _ItCategory must be: std:: [input/output/forward/bidirectional/random_access/...] _iterator_tag
    /// @warning If any element in the target collection is removed when iterators exist:
    ///          - all Iterator instances pointing to the removed element will be unsafe (any operation or dereferencing would cause a segfault or a misread);
    ///          - all Iterator instances may skip or repeat an element if moved forward/backward;
    ///          - comparisons between existing (old) and new Iterator instances will be invalidated.
    template <typename _CollectionType,
              typename _ValueType = typename _CollectionType::value_type,
              bool _IsConst = false,
              bool _IsReversed = false,
              typename _ItCategory = std::bidirectional_iterator_tag, 
              typename _DiffType = size_t>
    class Iterator {
    public:
      using iterator_category = _ItCategory;
      using value_type = typename std::conditional<_IsConst, const _ValueType, _ValueType>::type;
      using collection_type = typename std::conditional<_IsConst, const _CollectionType, _CollectionType>::type;
      using difference_type = _DiffType;
      using pointer = typename std::conditional<_IsConst, const _ValueType*, _ValueType*>::type;
      using const_pointer = const _ValueType*;
      using reference = typename std::conditional<_IsConst, const _ValueType&, _ValueType&>::type;
      using const_reference = const _ValueType&;
      using Type = Iterator<_CollectionType,_ValueType,_IsConst,_IsReversed,_ItCategory,_DiffType>;


      /// @brief Create an iterator at the beginning (or last item, if reversed) of a collection (or after the end, if argument omitted)
      explicit Iterator(collection_type& collection, value_type* first = nullptr) noexcept
        : _collection(&collection), _current(first), _index((first != nullptr) ? 0u : _endIndex()) {
        static_assert(!_IsReversed
          || std::is_base_of<std::bidirectional_iterator_tag, iterator_category>::value
          || is_bidirectional_iterator<Type>::value
          || is_random_access_iterator<Type>::value,
          "Iterator: reverse iterators must be bi-directional (_ItCategory: bidirectional_iterator_tag, random_access_iterator,...)");
      }
      /// @brief Create an iterator at any position of a collection (or after the end, if argument omitted)
      explicit Iterator(collection_type& collection, value_type* current, uint32_t index) noexcept
        : _collection(&collection), 
          _current((index < collection.size()) ? current : nullptr),
          _index((current != nullptr && index < collection.size()) ? index : _endIndex()) {}

      Iterator() noexcept : _collection(nullptr), _current(nullptr), _index(_endIndex()) {}
      Iterator(const Type&) = default;
      Iterator(Type&&) = default;
      Type& operator=(const Type&) = default;
      Type& operator=(Type&&) = default;
      ~Iterator() = default;
      inline Type base() const noexcept { return Type(*this); }

      Iterator(std::nullptr_t) noexcept : _collection(nullptr), _current(nullptr), _index(_endIndex()) {}
      Type& operator=(std::nullptr_t) noexcept { this->_collection = nullptr; this->_current = nullptr; this->_index = _endIndex(); }

      /// @brief Create reversed iterator from iterator and vice-versa
      explicit Iterator(const Iterator<_CollectionType,_ValueType,_IsConst,!_IsReversed,_ItCategory,_DiffType>& rev) noexcept
          : _collection(rev._collection), _current(rev._current), _index((rev._collection != nullptr) ? rev._reversedIndex() : _endIndex()) {}

      /// @brief Swap iterator with another instance
      void swap(Type& rhs) noexcept {
        Type copy(rhs);
        rhs = *this;
        *this = copy;
      }

      /// @brief Type cast to const iterator
      template <bool _Constness = _IsConst, typename std::enable_if<!_Constness, int>::type = 0>
      inline operator Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>() {
        return Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>(*(this->_collection), this->_current, this->_index);
      }
      
      
      // -- getters --
      
      inline bool hasValue() const noexcept { return (this->_current != nullptr); }
      inline reference data() noexcept             { assert(hasValue()); return *(this->_current); }
      inline const_reference data() const noexcept { assert(hasValue()); return *(this->_current); }
      inline reference operator*() noexcept             { assert(hasValue()); return *(this->_current); }
      inline const_reference operator*() const noexcept { assert(hasValue()); return *(this->_current); }
      inline pointer operator->() noexcept              { assert(hasValue()); return this->_current; }
      inline const_pointer operator->() const noexcept  { assert(hasValue()); return this->_current; }

      inline int32_t index() const noexcept {
        return (this->_index != _endIndex()) ? static_cast<int32_t>(this->_index) : static_cast<int32_t>((this->_collection != nullptr) ? this->_collection->size() : -1);
      }
      inline uint32_t rawIndex() const noexcept { return this->_index; }
      
      // -- comparisons --

      inline bool operator==(const Type& rhs) const noexcept { return (this->_current == rhs._current); }
      inline bool operator!=(const Type& rhs) const noexcept { return (this->_current != rhs._current); }
      inline bool operator<(const Type& rhs) const noexcept { return (this->_index < rhs._index); }
      inline bool operator<=(const Type& rhs) const noexcept { return (this->_current == rhs._current || this->_index < rhs._index); }
      inline bool operator>(const Type& rhs) const noexcept { return (this->_index > rhs._index); }
      inline bool operator>=(const Type& rhs) const noexcept { return (this->_current == rhs._current || this->_index > rhs._index); }

      inline bool operator==(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept { return (this->_current == rhs._current); }
      inline bool operator!=(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept { return (this->_current != rhs._current); }
      inline bool operator<(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept  { return (this->_index < rhs._index); }
      inline bool operator<=(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept { return (this->_current == rhs._current || this->_index < rhs._index); }
      inline bool operator>(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept  { return (this->_index > rhs._index); }
      inline bool operator>=(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept { return (this->_current == rhs._current || this->_index > rhs._index); }

      inline int32_t operator-(const Type& rhs) const noexcept { return (index() - rhs.index()); }
      inline int32_t operator-(const Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>& rhs) const noexcept { return (index() - rhs.index()); }
      
      // -- operations - next --
      
      /// @brief Pre-increment - moves to next position (previous if reversed)
      inline Type& operator++() { _moveForward<Type>(1u); return *this; }
      /// @brief Post-increment - moves to next position (previous if reversed) + return old value
      inline Type operator++(int) { Type copy = *this;  _moveForward<Type>(1u); return copy; }
      
      /// @brief Moves to position: current + offset
      inline Type& operator+=(size_t offset) { _moveForward<Type>(offset); return *this; }
      /// @brief Move a copy to position: current + offset
      inline Type operator+(size_t offset) { Type copy = *this; copy._moveForward<Type>(offset); return copy; }
      
      /// @brief Accesses the element located n positions away from the current element
      /// @warning Boundaries aren't verified -> may read out of range!
      inline reference operator[](size_t position) const noexcept { 
        Type copy = *this; 
        copy += position;
        return copy.data();
      }
      
      // -- operations - previous --
      
      /// @brief Pre-decrement - moves to previous position (next if reversed)
      inline Type& operator--() { _moveBack<Type>(1u); return *this; }
      /// @brief Post-decrement - moves to previous position (next if reversed) + return old value
      inline Type operator--(int) { Type copy = *this; _moveBack<Type>(1u); return copy; }
      
      /// @brief Moves to position: current - offset
      inline Type& operator-=(size_t offset) { _moveBack<Type>(offset); return *this; }
      /// @brief Move a copy to position: current - offset
      inline Type operator-(size_t offset) { Type copy = *this; copy._moveBack<Type>(offset); return copy; }

      
    private:
      static constexpr inline uint32_t _endIndex() noexcept { return 0xFFFFFFFFu; }
      inline uint32_t _reversedIndex() const noexcept { return (this->_index < this->_collection->size()) 
                                                             ? (static_cast<uint32_t>(this->_collection->size()) - this->_index - 1u) 
                                                             : _endIndex(); }

      template <typename T = Type, enable_if_iterator_reversible<T, int> = 0>
      inline void _moveForward(size_t offset) {
        if (hasValue()) {
          __if_constexpr (_IsReversed)
            this->_current = this->_collection->previous(this->_current, _reversedIndex(), offset);
          else
            this->_current = this->_collection->next(this->_current, this->_index, offset);
          this->_index = (this->_current != nullptr) ? this->_index + static_cast<uint32_t>(offset) : _endIndex();
        }
      }
      template <typename T = Type, enable_if_iterator_not_reversible<T, int> = 0>
      inline void _moveForward(size_t offset) {
        if (hasValue()) {
          this->_current = this->_collection->next(this->_current, this->_index, offset);
          this->_index = (this->_current != nullptr) ? this->_index + static_cast<uint32_t>(offset) : _endIndex();
        }
      }

      template <typename T = Type, enable_if_iterator_reversible<T,int> = 0>
      inline void _moveBack(size_t offset) {
        if (hasValue()) {
          __if_constexpr (_IsReversed)
            this->_current = this->_collection->next(this->_current, _reversedIndex(), offset);
          else
            this->_current = this->_collection->previous(this->_current, this->_index, offset);
          this->_index = (this->_current != nullptr) ? this->_index - static_cast<uint32_t>(offset) : _endIndex();
        }
      }
      template <typename T = Type, enable_if_iterator_not_reversible<T,int> = 0>
      inline void _moveBack(size_t offset) { assert(false); throw std::logic_error("Iterator._moveBack: cannot move backwards on forward-only iterators."); }

      friend class Iterator<_CollectionType,_ValueType,!_IsConst,_IsReversed,_ItCategory,_DiffType>;
      friend class Iterator<_CollectionType,_ValueType,_IsConst,!_IsReversed,_ItCategory,_DiffType>;

    private:
      collection_type* _collection = nullptr;
      value_type* _current = nullptr;
      uint32_t _index = _endIndex();
    };
    
  }
}
#undef __if_constexpr
        
// -- iterator definition macros (to use inside collection class) - general --

#define _P_ITERATOR_MEMBERS(collectionType, valueType, category, firstPtr) \
        using iterator = pandora::pattern::Iterator<collectionType, valueType, false, false, category, size_t>; \
        using const_iterator = pandora::pattern::Iterator<collectionType, valueType, true, false, category, size_t>; \
        inline iterator begin() noexcept { return iterator(*this, firstPtr); } \
        inline iterator end() noexcept   { return iterator(*this, nullptr); } \
        inline const_iterator begin() const noexcept  { return const_iterator(*this, firstPtr); } \
        inline const_iterator end() const noexcept    { return const_iterator(*this, nullptr); } \
        inline const_iterator cbegin() const noexcept { return const_iterator(*this, firstPtr); } \
        inline const_iterator cend() const noexcept   { return const_iterator(*this, nullptr); }
        
#define _P_REVERSE_ITERATOR_MEMBERS(collectionType, valueType, category, lastPtr) \
        using reverse_iterator = pandora::pattern::Iterator<collectionType, valueType, false, true, category, size_t>; \
        using const_reverse_iterator = pandora::pattern::Iterator<collectionType, valueType, true, true, category, size_t>; \
        inline reverse_iterator rbegin() noexcept { return reverse_iterator(*this, lastPtr); } \
        inline reverse_iterator rend() noexcept   { return reverse_iterator(*this, nullptr); } \
        inline const_reverse_iterator rbegin() const noexcept  { return const_reverse_iterator(*this, lastPtr); } \
        inline const_reverse_iterator rend() const noexcept    { return const_reverse_iterator(*this, nullptr); } \
        inline const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(*this, lastPtr); } \
        inline const_reverse_iterator crend() const noexcept   { return const_reverse_iterator(*this, nullptr); }

// -- iterator definition macros (to use inside collection class) - specialized --

#define _P_FORWARD_ITERATOR_MEMBERS(collectionType, valueType, firstPtr) \
        _P_ITERATOR_MEMBERS(collectionType, valueType, std::forward_iterator_tag, firstPtr)

#define _P_BIDIRECT_ITERATOR_MEMBERS(collectionType, valueType, firstPtr, lastPtr) \
        _P_ITERATOR_MEMBERS(collectionType, valueType, std::bidirectional_iterator_tag, firstPtr) \
        _P_REVERSE_ITERATOR_MEMBERS(collectionType, valueType, std::bidirectional_iterator_tag, lastPtr)

#define _P_RANDOMACCESS_ITERATOR_MEMBERS(collectionType, valueType, firstPtr, lastPtr) \
        _P_ITERATOR_MEMBERS(collectionType, valueType, std::random_access_iterator_tag, firstPtr) \
        _P_REVERSE_ITERATOR_MEMBERS(collectionType, valueType, std::random_access_iterator_tag, lastPtr)
