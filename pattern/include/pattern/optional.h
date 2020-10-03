/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <type_traits>
#include "./class_traits.h"
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
  namespace pattern {
    /// @class Optional
    /// @brief Pointer-like buffer optionally containing an object with in-place instance creation (no dynamic allocation)
    template <typename _DataType>
    class Optional final : public ConstructorSelector<_DataType>, public AssignmentSelector<_DataType> {
    public:
      static_assert((sizeof(_DataType) > 0u), "Optional: _DataType can't be empty/zero-sized.");
      static_assert(noexcept(!std::is_copy_constructible<_DataType>::value
                          || std::is_nothrow_copy_constructible<_DataType>::value), "Optional: _DataType copy constructor (if present) cannot throw");
      static_assert(noexcept(!std::is_move_constructible<_DataType>::value
                          || (std::is_nothrow_move_constructible<_DataType>::value
                            && std::is_nothrow_move_assignable<_DataType>::value) ), "Optional: _DataType move constructor/operator (if present) cannot throw");

      /// @brief Create empty buffer
      Optional() noexcept : _hasValue(false) {}
      Optional(std::nullptr_t) noexcept : _hasValue(false) {} ///< Create empty buffer
      /// @brief Destroy buffer and optional instance
      ~Optional() noexcept { reset(); }

      /// @brief Create buffer containing an instance (copy)
      template <typename T = _DataType>
      Optional(enable_if_copy_constructible<T, const T&> value) noexcept { _assign<true, _DataType>(value); }
      /// @brief Create buffer containing an instance (move)
      template <typename T = _DataType>
      Optional(enable_if_move_constructible<T, T&&> value) noexcept { _assign<true, _DataType>(std::move(value)); }

      Optional(const Optional<_DataType>& rhs) noexcept : ConstructorSelector<_DataType>(rhs) { _copy<true>(rhs); }
      Optional(Optional<_DataType>&& rhs) noexcept { _move<true>(std::move(rhs)); }
      Optional<_DataType>& operator=(const Optional<_DataType>& rhs) noexcept { AssignmentSelector<_DataType>::operator=(rhs); _copy<false>(rhs); return *this; }
      Optional<_DataType>& operator=(Optional<_DataType>&& rhs) noexcept { _move<false>(std::move(rhs)); return *this; }

      void swap(Optional<_DataType>& rhs) noexcept {
        Optional<_DataType> buffer = std::move(*this);
        *this = std::move(rhs);
        rhs = std::move(buffer);
      }
      
      // -- getters --
      
      inline _DataType* get() noexcept { return (hasValue()) ? _get() : nullptr; } ///< Get buffer address (if instance exists)
      inline const _DataType* get() const noexcept { return (hasValue()) ? _get() : nullptr; } ///< Get buffer address (if instance exists)
      
      inline bool hasValue() const noexcept { return this->_hasValue; } ///< Verify if an instance is currently in the buffer
      inline operator bool() const noexcept { return this->_hasValue; }

      inline _DataType& value() noexcept { assert(hasValue()); return *_get(); } ///< Get current instance value (warning: always verify hasValue() first!)
      inline const _DataType& value() const noexcept { assert(hasValue()); return *_get(); } ///< Get current instance value (warning: always verify hasValue() first!)
      
      inline _DataType& operator*() noexcept { return value(); }
      inline const _DataType& operator*() const noexcept  { return value(); }
      inline _DataType* operator->() noexcept { assert(hasValue()); return _get(); }
      inline const _DataType* operator->() const noexcept { assert(hasValue()); return _get(); }
      
      // -- comparisons --
      
      template <typename T = _DataType>
      inline bool operator==(enable_if_operator_equals<T, const Optional<T>& > rhs) const noexcept { 
        return (this->_hasValue == rhs._hasValue && (!this->_hasValue || this->value() == rhs.value())); 
      }
      template <typename T = _DataType>
      inline bool operator!=(enable_if_operator_equals<T, const Optional<T>& > rhs) const noexcept { return !(this->operator==(rhs)); }

      inline bool operator==(std::nullptr_t) const noexcept { return !(this->_hasValue); }
      inline bool operator!=(std::nullptr_t) const noexcept { return (this->_hasValue); }

      template <typename T = _DataType>
      inline bool operator<(enable_if_operator_less<T, const Optional<T>& > rhs) const noexcept {
        return (this->_hasValue) ? (rhs._hasValue && this->value() < rhs.value()) : rhs._hasValue;
      }
      template <typename T = _DataType>
      inline bool operator<=(enable_if_operator_less_eq<T, const Optional<T>& > rhs) const noexcept {
        return (!this->_hasValue || (rhs._hasValue && this->value() <= rhs.value()));
      }
      template <typename T = _DataType>
      inline bool operator>(enable_if_operator_greater<T, const Optional<T>& > rhs) const noexcept {
        return (this->_hasValue && (!rhs._hasValue || this->value() > rhs.value()));
      }
      template <typename T = _DataType>
      inline bool operator>=(enable_if_operator_greater_eq<T, const Optional<T>& > rhs) const noexcept {
        return (this->_hasValue) ? (!rhs._hasValue || this->value() >= rhs.value()) : !rhs._hasValue;
      }
      
      // -- operations --
      
      /// @brief Destroy current instance (if any)
      inline void reset() noexcept {
        if (this->_hasValue) {
          value().~_DataType();
          _hasValue = false;
        }
      }
      inline Optional<_DataType>& operator=(std::nullptr_t) noexcept { reset(); return *this; }

      /// @brief Create an instance (copy)
      template <typename T = _DataType>
      inline void assign(enable_if_copy_constructible<T, const T&> value) noexcept { _assign<false,T>(value); }
      /// @brief Create an instance (move)
      template <typename T = _DataType>
      inline void assign(enable_if_move_constructible<T, T&&> value) noexcept { _assign<false,T>(std::move(value)); }
      /// @brief Create an instance (emplace)
      template <typename ... _Args>
      inline void emplace(_Args&&... args) { _emplace<false, _Args...>(std::forward<_Args>(args)...); }

      template <typename T = _DataType>
      inline Optional<_DataType>& operator=(enable_if_copy_constructible<T, const T&> value) noexcept { _assign<false,T>(value); return *this; }
      template <typename T = _DataType>
      inline Optional<_DataType>& operator=(enable_if_move_constructible<T, T&&> value) noexcept { _assign<false,T>(std::move(value)); return *this; }
      
    private:
      inline _DataType* _get() noexcept { return reinterpret_cast<_DataType*>(this->_value); }
      inline const _DataType* _get() const noexcept { return reinterpret_cast<const _DataType*>(this->_value); }

      template <bool _IsConstruction, typename T>
      inline void _copy(const Optional<T>& rhs) noexcept {
        if (rhs._hasValue)
          _assign<_IsConstruction,T>(rhs.value());
        else
          this->_hasValue = false;
      }
      template <bool _IsConstruction, typename T = _DataType>
      inline void _move(typename std::enable_if<std::is_class<T>::value && std::is_move_constructible<T>::value, Optional<T>&&>::type rhs) noexcept {
        if (rhs._hasValue) {
          _assign<_IsConstruction,T>(std::move(rhs.value()));
          rhs.reset();
        }
        else
          this->_hasValue = false;
      }
      template <bool _IsConstruction, typename T = _DataType>
      inline void _move(typename std::enable_if<!std::is_class<T>::value || !std::is_move_constructible<T>::value, const Optional<T>&>::type rhs) noexcept {
        if (rhs._hasValue) {
          _assign<_IsConstruction, T>(rhs.value());
        }
        else
          this->_hasValue = false;
      }

      template <bool _IsConstruction, typename T = _DataType>
      inline void _assign(enable_if_copy_constructible<T, const T&> value) noexcept {
        __if_constexpr(!_IsConstruction)
          reset();
        new(_get()) _DataType(value);
        this->_hasValue = true;
      }
      template <bool _IsConstruction, typename T = _DataType>
      inline void _assign(enable_if_move_constructible<T, T&&> value) noexcept {
        __if_constexpr(!_IsConstruction)
          reset();
        new(_get()) _DataType(std::move(value));
        this->_hasValue = true;
      }
      template <bool _IsConstruction, typename ... _Args>
      inline void _emplace(_Args&&... args) {
        __if_constexpr(!_IsConstruction)
          reset();
        new(_get()) _DataType(std::forward<_Args>(args)...);
        this->_hasValue = true;
      }
      
    private:
      bool _hasValue = false;
      uint8_t _value[sizeof(_DataType)];
    };
    
  }
}
#undef __if_constexpr
#ifdef _MSC_VER
# pragma warning(pop)
#endif
