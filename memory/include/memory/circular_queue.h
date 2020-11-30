/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <memory>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <pattern/optional.h>
#include <pattern/iterator.h>
#include <pattern/class_traits.h>

namespace pandora {
  namespace memory {
    /// @class CircularQueue
    /// @brief Circular/wrap-around fixed-size queue collection (FIFO, stack allocation)
    template <typename _DataType, size_t _Capacity>
    class CircularQueue final : public pandora::pattern::ConstructorSelector<_DataType>, public pandora::pattern::AssignmentSelector<_DataType> {
    public:
      using value_type = _DataType;
      using item_type = typename std::conditional<std::is_class<_DataType>::value, pandora::pattern::Optional<_DataType>, _DataType>::type;
      using size_type = size_t;
      using reference = _DataType&;
      using const_reference = const _DataType&;
      using Type = CircularQueue<_DataType,_Capacity>;
      static_assert((_Capacity > 0u), "CircularQueue: _Capacity can't be 0.");
      static_assert((_Capacity <= 0x7FFFFFFFu), "CircularQueue: _Capacity can't exceed 0x7FFFFFFF.");
      static_assert(noexcept(!std::is_copy_constructible<_DataType>::value
                          || std::is_nothrow_copy_constructible<_DataType>::value), "CircularQueue: _DataType copy constructor (if present) cannot throw");
      static_assert(noexcept(!std::is_move_constructible<_DataType>::value
                          || (std::is_nothrow_move_constructible<_DataType>::value
                            && std::is_nothrow_move_assignable<_DataType>::value) ), "CircularQueue: _DataType move constructor/operator (if present) cannot throw");
      
      /// @brief Create empty queue
      CircularQueue() noexcept : _head(0), _tail(0) {}
      /// @brief Destroy queue and all child instances
      ~CircularQueue() = default;
      
      CircularQueue(const Type& rhs) noexcept : pandora::pattern::ConstructorSelector<_DataType>(rhs), _queue(), _head(0), _tail(0) { _copyEachItem<_DataType>(rhs); }
      CircularQueue(Type&& rhs) noexcept : pandora::pattern::ConstructorSelector<_DataType>(std::move(rhs)), _queue(), _head(0), _tail(0) { _moveEachItem<_DataType>(rhs); }
      Type& operator=(const Type& rhs) noexcept { pandora::pattern::AssignmentSelector<_DataType>::operator=(rhs);
        clear(); 
        _copyEachItem<_DataType>(rhs); 
        return *this; 
      }
      Type& operator=(Type&& rhs) noexcept { pandora::pattern::AssignmentSelector<_DataType>::operator=(std::move(rhs));
        clear(); 
        _moveEachItem<_DataType>(rhs); 
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
      
      // -- getters --
      
      /// @brief Get number of items currently stored in queue ([0; _Capacity])
      constexpr inline size_t size() const noexcept { 
        return (this->_tail <= this->_head) 
               ? static_cast<size_t>(this->_head) - static_cast<size_t>(this->_tail)
               : ( (this->_head == -1) ? capacity() : (_Capacity - static_cast<size_t>(this->_tail) + static_cast<size_t>(this->_head)) );
      }
      constexpr inline size_t max_size() const noexcept { return _Capacity; } ///< Get maximum capacity of the queue
      constexpr inline size_t capacity() const noexcept { return _Capacity; } ///< Get maximum capacity of the queue
      constexpr inline bool empty() const noexcept { return (this->_head == this->_tail); } ///< Check if any item is present in the queue
      constexpr inline bool full() const noexcept  { return (this->_head == -1); } ///< Check if the queue is full
      
      /// @brief Get first item in the queue (check not empty() before!)
      inline _DataType& front() noexcept { assert(!empty()); return _at(this->_tail); }
      /// @brief Get first item in the queue (check not empty() before!)
      inline const _DataType& front() const noexcept { assert(!empty()); return _at(this->_tail); }
      
      /// @brief Get most recent item in the queue (check not empty() before!)
      inline _DataType& back() noexcept { assert(!empty()); return _at(_lastIndex()); }
      /// @brief Get most recent item in the queue (check not empty() before!)
      inline const _DataType& back() const noexcept { assert(!empty()); return _at(_lastIndex()); }
      
      // -- operations --
      
      /// @brief Remove all items from the queue
      inline void clear() noexcept {
        this->_queue = std::array<item_type, _Capacity>{};
        this->_head = this->_tail = 0u;
      }
      
      /// @brief Insert an item at the end of the queue (copy)
      template <typename T = _DataType>
      inline bool push(pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept {
        if (!full()) {
          Type::_copyIn(this->_queue[this->_head], value);
          this->_head = _nextIndex(this->_head);
          if (this->_head == this->_tail)
            this->_head = -1;
          return true;
        }
        return false;
      }
      template <typename T = _DataType>
      inline bool push_back(pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept { return push(value); }
      
      /// @brief Insert an item at the end of the queue (move)
      template <typename T = _DataType>
      inline bool push(pandora::pattern::enable_if_move_constructible<T, T&&> value) noexcept {
        if (!full()) {
          Type::_moveIn(this->_queue[this->_head], std::move(value));
          this->_head = _nextIndex(this->_head);
          if (this->_head == this->_tail)
            this->_head = -1;
          return true;
        }
        return false;
      }
      template <typename T = _DataType>
      inline bool push_back(pandora::pattern::enable_if_move_constructible<T, T&&> value) noexcept { return push(std::move(value)); }
      
      /// @brief Create an item at the end of the queue
      template <typename ... _Args>
      bool emplace(_Args&&... args) { 
        if (!full()) {
          Type::_emplaceIn(this->_queue[this->_head], std::forward<_Args>(args)...);
          this->_head = _nextIndex(this->_head);
          if (this->_head == this->_tail)
            this->_head = -1;
          return true;
        }
        return false;
      }
      template <typename ... _Args>
      bool emplace_back(_Args&&... args) { return emplace(std::forward<_Args>(args)...); }
      
      /// @brief Remove first item from the queue
      bool pop() noexcept {
        if (!empty()) {
          _clearAt(this->_tail);
          if (full())
            this->_head = this->_tail;
          this->_tail = _nextIndex(this->_tail);
          return true;
        }
        return false;
      }
      inline bool pop_front() noexcept { return pop(); }
      
      
      // -- iterators --

      _P_FORWARD_ITERATOR_MEMBERS(Type, _DataType, _first())
      _DataType* next(_DataType* current, uint32_t currentIndex, size_t offset = 1u) noexcept { return _next(current, currentIndex, offset); }
      const _DataType* next(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) const noexcept { return _next(current, currentIndex, offset); }
      
    private:
      static constexpr inline int32_t _previousIndex(int32_t index) noexcept {
        return (index > 0) ? (index - 1) : (static_cast<int32_t>(_Capacity) - 1);
      }
      static constexpr inline int32_t _nextIndex(int32_t index) noexcept {
        return ((index + 1) % static_cast<uint32_t>(_Capacity));
      }
      constexpr inline int32_t _lastIndex() const noexcept {
        return (this->_head > -1) ? _previousIndex(this->_head) : _previousIndex(this->_tail);
      }

      static inline _DataType& _getValue(pandora::pattern::Optional<_DataType>& embedder) noexcept { return embedder.value(); }
      static inline _DataType& _getValue(_DataType& value) noexcept { return value; }
      static inline const _DataType& _getConstValue(const pandora::pattern::Optional<_DataType>& embedder) noexcept { return embedder.value(); }
      static inline const _DataType& _getConstValue(const _DataType& value) noexcept { return value; }
      inline _DataType& _at(int32_t index) noexcept { return Type::_getValue(this->_queue[index]); }
      inline const _DataType& _at(int32_t index) const noexcept { return Type::_getConstValue(this->_queue[index]); }


      // -- private - operations --

      template <typename T, typename _EnabledType = void>
      using enable_if_copyable_class = typename std::enable_if<std::is_class<T>::value && std::is_copy_constructible<T>::value, _EnabledType>::type;
      template <typename T, typename _EnabledType = void>
      using enable_if_movable_class = typename std::enable_if<std::is_class<T>::value && std::is_move_constructible<T>::value, _EnabledType>::type;
      template <typename T, typename _EnabledType = void>
      using enable_if_base_type = typename std::enable_if<!std::is_class<T>::value, _EnabledType>::type;

      template <typename T = _DataType>
      void _copyEachItem(enable_if_copyable_class<T, const CircularQueue<T,_Capacity>&> rhs) noexcept {
        if (rhs._tail <= rhs._head) {
          for (int32_t i = rhs._tail; i < rhs._head; ++i)
            this->_queue[i].assign(rhs._queue[i].value());
        }
        else {
          for (int32_t i = rhs._tail; i < static_cast<int32_t>(_Capacity); ++i)
            this->_queue[i].assign(rhs._queue[i].value());
          int32_t endIndex = (rhs._head == -1) ? rhs._tail : rhs._head;
          for (int32_t i = 0; i < endIndex; ++i)
            this->_queue[i].assign(rhs._queue[i].value());
        }
        this->_head = rhs._head;
        this->_tail = rhs._tail;
      }
      template <typename T = _DataType>
      void _copyEachItem(enable_if_base_type<T, const CircularQueue<T,_Capacity>&> rhs) noexcept {
        memcpy((void*)&(this->_queue[0]), (void*)&(rhs._queue[0]), sizeof(_DataType)*_Capacity);
        this->_head = rhs._head;
        this->_tail = rhs._tail;
      }

      template <typename T = _DataType>
      void _moveEachItem(enable_if_movable_class<T, CircularQueue<T,_Capacity>&> rhs) noexcept {
        if (rhs._tail <= rhs._head) {
          for (int32_t i = rhs._tail; i < rhs._head; ++i)
            this->_queue[i].assign(std::move(rhs._queue[i].value()));
        }
        else {
          for (int32_t i = rhs._tail; i < static_cast<int32_t>(_Capacity); ++i)
            this->_queue[i].assign(std::move(rhs._queue[i].value()));
          int32_t endIndex = (rhs._head == -1) ? rhs._tail : rhs._head;
          for (int32_t i = 0; i < endIndex; ++i)
            this->_queue[i].assign(std::move(rhs._queue[i].value()));
        }
        this->_head = rhs._head;
        this->_tail = rhs._tail;
        rhs._head = rhs._tail = 0;
      }
      template <typename T = _DataType>
      void _moveEachItem(enable_if_base_type<T, CircularQueue<T,_Capacity>&> rhs) noexcept {
        memcpy((void*)&(this->_queue[0]), (void*)&(rhs._queue[0]), sizeof(_DataType) * _Capacity);
        this->_head = rhs._head;
        this->_tail = rhs._tail;
        rhs._head = rhs._tail = 0;
      }

      template <typename T = _DataType>
      static inline void _copyIn(pandora::pattern::Optional<_DataType>& destination, pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept {
        destination.assign(value);
      }
      template <typename T = _DataType>
      static inline void _copyIn(_DataType& destination, pandora::pattern::enable_if_copy_constructible<T, const T&> value) noexcept { destination = value; }

      template <typename T = _DataType>
      static inline void _moveIn(pandora::pattern::Optional<_DataType>& destination, pandora::pattern::enable_if_move_constructible<T, T&&> value) noexcept {
        destination.assign(std::move(value));
      }
      template <typename T = _DataType>
      static inline void _moveIn(_DataType& destination, pandora::pattern::enable_if_move_constructible<T, T&&> value) noexcept { destination = std::move(value); }

      template <typename ... _Args>
      static inline void _emplaceIn(pandora::pattern::Optional<_DataType>& destination, _Args&&... args) { destination.emplace(std::forward<_Args>(args)...); }
      template <typename ... _Args>
      static inline void _emplaceIn(_DataType& destination, _Args&&... args) noexcept { destination = _DataType(std::forward<_Args>(args)...); }

      template <typename T = _DataType>
      inline void _clearAt(typename std::enable_if<std::is_class<T>::value, size_t>::type index) noexcept { this->_queue[index].reset(); }
      template <typename T = _DataType>
      inline void _clearAt(typename std::enable_if<!std::is_class<T>::value, size_t>::type index) noexcept { this->_queue[index] = (T)0; }

      // -- private - iterators --

      inline _DataType* _first() noexcept { return (!empty()) ? &front() : nullptr; }
      inline const _DataType* _first() const noexcept { return (!empty()) ? &front() : nullptr; }

      _DataType* _next(_DataType* current, uint32_t currentIndex, size_t offset = 1u) noexcept {
        int32_t nextIndex = static_cast<int32_t>(currentIndex) + static_cast<int32_t>(offset); // iteration index
        if (current != nullptr && nextIndex < static_cast<int32_t>(size())) {
          return &(_at( ((nextIndex + this->_tail) % static_cast<int32_t>(_Capacity)) )); // to array position index
        }
        return nullptr;
      }
      const _DataType* _next(const _DataType* current, uint32_t currentIndex, size_t offset = 1u) const noexcept {
        int32_t nextIndex = static_cast<int32_t>(currentIndex) + static_cast<int32_t>(offset); // iteration index
        if (current != nullptr && nextIndex < static_cast<int32_t>(size())) {
          return &(_at( ((nextIndex + this->_tail) % static_cast<int32_t>(_Capacity)) )); // to array position index
        }
        return nullptr;
      }
      
    private:
      std::array<item_type, _Capacity> _queue{};
      int32_t _head{ 0 };
      int32_t _tail{ 0 };
    };
    
  }
}
