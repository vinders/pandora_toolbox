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
#include <memory>
#include <array>
#include <stdexcept>
#include <type_traits>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
# define __constexpr constexpr
#else
# define __if_constexpr if
# define __constexpr
#endif
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4100)
#endif

namespace pandora {
  namespace memory {
    /// @brief Memory allocation type for memory pool
    enum class MemoryPoolAllocation: uint32_t {
      automatic = 0,
      onStack = 1,
      onHeap = 2,
    };

#   define __P_IS_HEAP_POOL() (_Alloc==MemoryPoolAllocation::onHeap || (_Alloc==MemoryPoolAllocation::automatic && (_BytesCapacity + _GuardBandSize*2u) > 8000u))

    /// ---
    
    /// @class MemoryPool
    /// @brief Fixed-size pre-allocated memory pool, for efficient real-time memory management.
    template <size_t _BytesCapacity,               ///< Usable allocated pool size
              size_t _GuardBandSize = size_t{ 0 }, ///< Size of each security guard band (before/after allocated pool)
              MemoryPoolAllocation _Alloc = MemoryPoolAllocation::automatic, ///< Allocation type: automatic/onHeap recommended
              bool _DoSizeCheck = true>            ///< true: verify that commands aren't out of range (recommended) / false: skip verifications (do them externally)
    class MemoryPool final {
    public:
      using value_type = uint8_t;
      using size_type = size_t;
      using pool_type = typename std::conditional<__P_IS_HEAP_POOL(),
                                                  std::unique_ptr<value_type[]>,
                                                  std::array<value_type,(_BytesCapacity + _GuardBandSize*2u)> >::type;
      template <typename _DataType = value_type>
      using iterator = _DataType*;
      using Type = MemoryPool<_BytesCapacity,_GuardBandSize,_Alloc,_DoSizeCheck>;
      static_assert(_BytesCapacity > 0u, "MemoryPool: _BytesCapacity must be above 0.");

      
      MemoryPool() { 
        _allocate(this->_pool); 
        this->_first = &(this->_pool[_GuardBandSize]); 
        clear(); 
      }
      ~MemoryPool() = default;
      
      MemoryPool(const Type& rhs) = delete;
      MemoryPool(Type&& rhs) noexcept {
        __if_constexpr (__P_IS_HEAP_POOL()) {
          this->_pool = std::move(rhs._pool);
          this->_first = &(this->_pool[_GuardBandSize]); 
        }
        else {
          memcpy((void*)&(this->_pool[_GuardBandSize]), (void*)(rhs._first), _BytesCapacity);
          this->_first = &(this->_pool[_GuardBandSize]); 
        }
      }
      Type& operator=(const Type& rhs) = delete;
      Type& operator=(Type&& rhs) noexcept {
        __if_constexpr (__P_IS_HEAP_POOL()) {
          this->_pool = std::move(rhs._pool);
          this->_first = &(this->_pool[_GuardBandSize]); 
        }
        else {
          memcpy((void*)&(this->_pool[_GuardBandSize]), (void*)(rhs._first), _BytesCapacity);
          this->_first = &(this->_pool[_GuardBandSize]); 
        }
        return *this;
      }

      template<bool _HeapAlloc = __P_IS_HEAP_POOL(), typename std::enable_if<_HeapAlloc,int>::type = 0>
      inline void swap(Type& rhs) noexcept { std::swap(this->_pool, rhs._pool); std::swap(this->_first, rhs._first); }


      /// @brief Clone content of current instance
      /// @warning If objects are allocated (create/emplace), the cloned objects will be invalid.
      ///          Only use 'clone()' with POD types or for debug/trace purpose.
      inline Type clone() const { return Type{this->_pool}; }
      

      // -- pool metadata --

      static constexpr inline MemoryPoolAllocation allocationType() noexcept { ///< Actual allocation type
        return (__P_IS_HEAP_POOL()) ? MemoryPoolAllocation::onHeap : MemoryPoolAllocation::onStack;
      }
      
      static constexpr inline size_t size() noexcept { return _BytesCapacity; }     ///< Get full capacity of memory pool (bytes)
      static constexpr inline size_t capacity() noexcept { return _BytesCapacity; } ///< Get full capacity of memory pool (bytes)
      static constexpr inline size_t allocated() noexcept { return _BytesCapacity + _GuardBandSize*2u; } ///< Get full capacity of memory pool + guard bands (bytes)
      template <typename _DataType>
      static constexpr inline size_t length() noexcept { return _BytesCapacity/sizeof(_DataType); } ///< Get object capacity of memory pool


      // -- read data --

      inline value_type* get() noexcept { return this->_first; }             ///< Read raw pool data (bytes)
      inline const value_type* get() const noexcept { return this->_first; } ///< Read raw pool data (bytes)

      /// @brief Read raw data (bytes) at a custom position (byte index)
      inline value_type* get(size_t byteIndex) noexcept {
        return (_verifyIndex(byteIndex)) ? &(this->_first[byteIndex]) : nullptr; 
      }
      inline const value_type* get(size_t byteIndex) const noexcept {
        return (_verifyIndex(byteIndex)) ? &(this->_first[byteIndex]) : nullptr; 
      }
      inline value_type& operator[](size_t byteIndex) noexcept             { assert(byteIndex < _BytesCapacity); return *get(byteIndex); }
      inline const value_type& operator[](size_t byteIndex) const noexcept { assert(byteIndex < _BytesCapacity); return *get(byteIndex); } 

      /// @brief Read object data at a custom position (byte index)
      template <typename _DataType>
      inline _DataType* get(size_t byteIndex) noexcept {
        return (_verifyLength(byteIndex + sizeof(_DataType))) ? reinterpret_cast<_DataType*>(&(this->_first[byteIndex])) : nullptr; 
      }
      template <typename _DataType>
      inline const _DataType* get(size_t byteIndex) const noexcept {
        return (_verifyLength(byteIndex + sizeof(_DataType))) ? reinterpret_cast<const _DataType*>(&(this->_first[byteIndex])) : nullptr; 
      }

      /// @brief Read object data at a custom position (object index)
      template <typename _DataType>
      inline _DataType* at(size_t dataTypeIndex) noexcept {
        return (_verifyLength((dataTypeIndex+1u)*sizeof(_DataType))) ? reinterpret_cast<_DataType*>(get()) + dataTypeIndex  : nullptr;
      }
      template <typename _DataType>
      inline const _DataType* at(size_t dataTypeIndex) const noexcept {
        return (_verifyLength((dataTypeIndex+1u)*sizeof(_DataType))) ? reinterpret_cast<const _DataType*>(get()) + dataTypeIndex  : nullptr;
      }

      /// @brief Read object data at a custom position (object index) after a custom byte offset
      template <typename _DataType>
      inline _DataType* at(size_t byteOffset, size_t dataTypeIndexAfterOffset) noexcept {
        return (_verifyLength(byteOffset + (dataTypeIndexAfterOffset+1u)*sizeof(_DataType))) 
               ? reinterpret_cast<_DataType*>(&(this->_first[byteOffset])) + dataTypeIndexAfterOffset  : nullptr;
      }
      template <typename _DataType>
      inline const _DataType* at(size_t byteOffset, size_t dataTypeIndexAfterOffset) const noexcept {
        return (_verifyLength(byteOffset + (dataTypeIndexAfterOffset+1u)*sizeof(_DataType))) 
               ? reinterpret_cast<const _DataType*>(&(this->_first[byteOffset])) + dataTypeIndexAfterOffset  : nullptr;
      }


      // -- comparisons --

      /// @brief Compare entire pool with another pool
      inline int32_t compare(const Type& rhs) const noexcept {
        return memcmp((void*)get(), (void*)rhs.get(), size());
      }
      /// @brief Compare subset of pool (byte index) with same subset of another pool
      inline int32_t compare(const Type& rhs, size_t offset, size_t length) const noexcept {
        if (offset < size())
          return memcmp((void*)&(this->_first[offset]), (void*)&(rhs._first[offset]), (offset + length <= size()) ? length : size() - offset);
        return 0;
      }
      /// @brief Compare subset of pool (byte index) with an object value (or array of objects)
      template <typename _DataType>
      inline int32_t compare(size_t byteIndex, const _DataType* rhs, size_t sourceArraySize = 1u) const noexcept {
        if (rhs != nullptr) {
          sourceArraySize *= sizeof(_DataType);
          if (byteIndex + sourceArraySize <= size())
            return memcmp((void*)&(this->_pool[_GuardBandSize + byteIndex]), (void*)rhs, sourceArraySize);
          else {
            int32_t result = (byteIndex < size()) ? memcmp((void*)&(this->_pool[_GuardBandSize + byteIndex]), (void*)rhs, size() - byteIndex) : -1;
            return (result != 0) ? result : -1;
          }
        }
        return 1;
      }
      /// @brief Compare subset of pool (object index) with an object value (or array of objects)
      template <typename _DataType>
      inline int32_t compareAt(size_t dataTypeIndex, const _DataType* rhs, size_t sourceArraySize = 1u) const noexcept {
        return compare<_DataType>(dataTypeIndex*sizeof(_DataType), rhs, sourceArraySize);
      }
      /// @brief Compare subset of pool (byte offset + object index) with an object value (or array of objects)
      template <typename _DataType>
      inline int32_t compareAt(size_t byteOffset, size_t dataTypeIndexAfterOffset, const _DataType* rhs, size_t sourceArraySize = 1u) const noexcept {
        return compare<_DataType>(byteOffset + dataTypeIndexAfterOffset*sizeof(_DataType), rhs, sourceArraySize);
      }

      inline bool operator==(const Type& rhs) const noexcept { return (compare(rhs) == 0); }
      inline bool operator!=(const Type& rhs) const noexcept { return (compare(rhs) != 0); }
      inline bool operator< (const Type& rhs) const noexcept { return (compare(rhs) < 0); }
      inline bool operator<=(const Type& rhs) const noexcept { return (compare(rhs) <= 0); }
      inline bool operator> (const Type& rhs) const noexcept { return (compare(rhs) > 0); }
      inline bool operator>=(const Type& rhs) const noexcept { return (compare(rhs) >= 0); }


      // -- operations --
      
      /// @brief Clear entire pool
      inline void clear() noexcept {
        memset((void*)this->_first, 0, size());
      }
      /// @brief Clear a subset of the pool
      inline bool clear(size_t byteOffset, size_t byteLength) noexcept {
        if (_verifyLength(byteOffset + byteLength)) {
          memset((void*)&(this->_first[byteOffset]), 0, byteLength);
          return true;
        }
        return false;
      }
      /// @brief Fill entire pool with a value
      inline void fill(value_type value) noexcept {
        memset((void*)this->_first, value, size());
      }
      /// @brief Fill a subset of the pool with a value
      inline bool fill(value_type value, size_t byteOffset, size_t byteLength) noexcept {
        if (_verifyLength(byteOffset + byteLength)) {
          memset((void*)&(this->_first[byteOffset]), value, byteLength);
          return true;
        }
        return false;
      }
      
      /// @brief Write raw object data (or array) at a custom position (byte index, object count)
      template <typename _DataType>
      inline bool set(size_t byteIndex, const _DataType* sourceData, size_t sourceArraySize = 1u) noexcept {
        assert(sourceData != nullptr);
        sourceArraySize *= sizeof(_DataType);
        if (_verifyLength(byteIndex + sourceArraySize)) {
          memcpy((void*)&(this->_first[byteIndex]), (void*)sourceData, sourceArraySize);
          return true;
        }
        return false;
      }
      /// @brief Write raw object data (or array) at a custom position (object index, object count)
      template <typename _DataType>
      inline bool setAt(size_t dataTypeIndex, const _DataType* sourceData, size_t sourceArraySize = 1u) noexcept {
        return set<_DataType>(dataTypeIndex*sizeof(_DataType), sourceData, sourceArraySize);
      }
      /// @brief Write raw object data (or array) at a custom position (object index after byte offset, object count)
      template <typename _DataType>
      inline bool setAt(size_t byteOffset, size_t dataTypeIndexAfterOffset, const _DataType* sourceData, size_t sourceArraySize = 1u) noexcept {
        return set<_DataType>(byteOffset + dataTypeIndexAfterOffset*sizeof(_DataType), sourceData, sourceArraySize);
      }

      /// @brief Write raw object data (or array) at iterator position (iterator type length)
      template <typename _DataType>
      inline bool setIn(iterator<_DataType> pos, const _DataType* sourceData, size_t sourceArraySize = 1u) noexcept {
        assert(sourceData != nullptr && pos != nullptr);
        sourceArraySize *= sizeof(_DataType);
        if (_verifyIterator((iterator<value_type>)pos, sourceArraySize)) {
          memcpy((void*)pos, (void*)sourceData, sourceArraySize);
          return true;
        }
        return false;
      }
      
      // -- unmanaged object operations (unsafe!) --
      
      /// @brief Create an instance - copied (byte offset)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType>
      inline bool create(size_t byteIndex, const _DataType& value) { return _emplace(get<_DataType>(byteIndex), value); }
      /// @brief Create an instance - copied (object index)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType>
      inline bool createAt(size_t dataTypeIndex, const _DataType& value) { return _emplace(at<_DataType>(dataTypeIndex), value); }
      /// @brief Create an instance - copied (object index after byte offset)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType>
      inline bool createAt(size_t byteOffset, size_t dataTypeIndexAfterOffset, const _DataType& value) {
        return _emplace(at<_DataType>(byteOffset, dataTypeIndexAfterOffset), value);
      }

      /// @brief Create an instance - moved (byte offset)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType>
      inline bool create(size_t byteIndex, _DataType&& value) noexcept { return _emplace(get<_DataType>(byteIndex), std::move(value)); }
      /// @brief Create an instance - moved (object index)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType>
      inline bool createAt(size_t dataTypeIndex, _DataType&& value) noexcept { return _emplace(at<_DataType>(dataTypeIndex), std::move(value)); }
      /// @brief Create an instance - moved (object index after byte offset)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType>
      inline bool createAt(size_t byteOffset, size_t dataTypeIndexAfterOffset, _DataType&& value) noexcept {
        return _emplace(at<_DataType>(byteOffset, dataTypeIndexAfterOffset), std::move(value));
      }

      /// @brief Create an instance in place (byte offset)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType, typename ... _Args>
      bool emplace(size_t byteIndex, _Args&&... args) { return _emplace(get<_DataType>(byteIndex), std::forward<_Args>(args)...); }
      /// @brief Create an instance in place (object index)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType, typename ... _Args>
      bool emplaceAt(size_t dataTypeIndex, _Args&&... args) { return _emplace(at<_DataType>(dataTypeIndex), std::forward<_Args>(args)...); }
      /// @brief Create an instance in place (object index after byte offset)
      /// @warning Will not be destroyed automatically!
      template <typename _DataType, typename ... _Args>
      bool emplaceAt(size_t byteOffset, size_t dataTypeIndexAfterOffset, _Args&&... args) {
        return _emplace(at<_DataType>(byteOffset, dataTypeIndexAfterOffset), std::forward<_Args>(args)...);
      }

      /// @brief Destroy current instance (if any) (byte offset)
      /// @warning Will not verify if the instance exists!
      template <typename _DataType>
      inline bool destroy(size_t byteIndex) noexcept { return _destroy(get<_DataType>(byteIndex)); }
      /// @brief Destroy current instance (if any) (object index)
      /// @warning Will not verify if the instance exists!
      template <typename _DataType>
      inline bool destroyAt(size_t dataTypeIndex) noexcept { return _destroy(at<_DataType>(dataTypeIndex)); }
      /// @brief Destroy current instance (if any) (object index after byte offset)
      /// @warning Will not verify if the instance exists!
      template <typename _DataType>
      inline bool destroyAt(size_t byteOffset, size_t dataTypeIndexAfterOffset) noexcept {
        return _destroy(at<_DataType>(byteOffset, dataTypeIndexAfterOffset));
      }


      // -- iteration --
      
      template <typename _DataType = value_type>
      inline iterator<_DataType> begin() noexcept { return reinterpret_cast<_DataType*>(this->_first); }             ///< Iterator at beginning of pool
      template <typename _DataType = value_type>
      inline const iterator<_DataType> begin() const noexcept { return reinterpret_cast<_DataType*>(this->_first); } ///< Iterator at beginning of pool (const)
      template <typename _DataType = value_type>
      inline const iterator<_DataType> cbegin() const noexcept { return reinterpret_cast<_DataType*>(this->_first); } ///< Iterator at beginning of pool (const)

      template <typename _DataType = value_type>
      inline iterator<_DataType> end() noexcept { return reinterpret_cast<_DataType*>(this->_first + _BytesCapacity); }             ///< Iterator after end of pool
      template <typename _DataType = value_type>
      inline const iterator<_DataType> end() const noexcept { return reinterpret_cast<_DataType*>(this->_first + _BytesCapacity); } ///< Iterator after end of pool (const)
      template <typename _DataType = value_type>
      inline const iterator<_DataType> cend() const noexcept { return reinterpret_cast<_DataType*>(this->_first + _BytesCapacity); } ///< Iterator after end of pool (const)

      template <typename _DataType = value_type>
      inline iterator<_DataType> first() noexcept { return reinterpret_cast<_DataType*>(this->_first); }             ///< Get first element
      template <typename _DataType = value_type>
      inline const iterator<_DataType> first() const noexcept { return reinterpret_cast<_DataType*>(this->_first); } ///< Get first element (const)

      template <typename _DataType = value_type>
      inline iterator<_DataType> last() noexcept { return reinterpret_cast<_DataType*>(this->_first + (_BytesCapacity - sizeof(_DataType))); }             ///< Get last element
      template <typename _DataType = value_type>
      inline const iterator<_DataType> last() const noexcept { return reinterpret_cast<_DataType*>(this->_first + (_BytesCapacity - sizeof(_DataType))); } ///< Get last element (const)

      
    private:
      MemoryPool(const pool_type& sourcePool) {
        _allocate(this->_pool); 
        this->_first = &(this->_pool[_GuardBandSize]);
        memcpy((void*)(this->_first), (void*)(&sourcePool[_GuardBandSize]), size()); 
      }

      static inline void _allocate(const std::array<value_type,(_BytesCapacity + _GuardBandSize*2u)>&) noexcept {} // stack: no additional allocation
      static inline void _allocate(std::unique_ptr<value_type[]>& pool) { // heap: dynamic allocation
        pool = nullptr; // in case of alloc exception
        pool = std::make_unique<value_type[]>(_BytesCapacity + _GuardBandSize*2u);
      }

      // -- size check --

      __constexpr inline bool _verifyIndex(size_t byteIndex) const noexcept {
        __if_constexpr (_DoSizeCheck)
          return (byteIndex < size());
        else
          return true;
      }
      __constexpr inline bool _verifyLength(size_t byteLength) const noexcept {
        __if_constexpr (_DoSizeCheck)
          return (byteLength <= size());
        else
          return true;
      }
      __constexpr inline bool _verifyIterator(iterator<value_type> pos, size_t byteLength) const noexcept {
        __if_constexpr (_DoSizeCheck)
          return (pos >= this->_first && pos <= this->_first + _BytesCapacity - byteLength);
        else
          return true;
      }

      // -- sub-objects --

      template <typename _DataType, typename ... _Args>
      inline bool _emplace(_DataType* destination, _Args&&... args) {
        if (destination != nullptr) {
          new(destination) _DataType(std::forward<_Args>(args)...);
          return true;
        }
        return false;
      }
      template <typename _DataType>
      inline bool _destroy(_DataType* destination) noexcept {
        if (destination != nullptr) {
          (*destination).~_DataType();
          return true;
        }
        return false;
      }

    private:
      pool_type _pool;
      value_type* _first = nullptr;
    };
    
  }
}
#undef __P_IS_HEAP_POOL
#undef __if_constexpr
#undef __constexpr
#ifdef _MSC_VER
# pragma warning(pop)
#endif
