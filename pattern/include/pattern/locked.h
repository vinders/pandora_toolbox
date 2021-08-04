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

#include <mutex>
#include <memory>

namespace pandora {
  namespace pattern {
    /// @class Locked
    /// @brief Locked shared pointer of an object, for protected access
    template <typename _DataType, typename _LockType>
    class Locked final {
    public:
      /// @brief Create a pointer access that is locked until its destruction
      Locked(std::shared_ptr<_DataType> data, _LockType& lock) noexcept
        : _data(std::move(data)), _lock(lock) {}
      /// @brief Create a pointer access that is locked until its destruction - already locked
      Locked(std::shared_ptr<_DataType> data, std::unique_lock<_LockType>&& lock) noexcept
        : _data(std::move(data)), _lock(std::move(lock)) {}
      Locked(const Locked<_DataType, _LockType>&) = delete;
      Locked(Locked<_DataType, _LockType>&&) = default;
      Locked<_DataType, _LockType>& operator=(const Locked<_DataType, _LockType>&) = delete;
      Locked<_DataType, _LockType>& operator=(Locked<_DataType, _LockType>&&) = default;
      ~Locked() = default;

      inline const _DataType& value() const noexcept { assert(this->_data != nullptr); return *(this->_data); }
      inline _DataType& value() noexcept { assert(this->_data != nullptr); return *(this->_data); }

      inline _DataType& operator*() noexcept { assert(this->_data != nullptr); return *(this->_data); }
      inline const _DataType& operator*() const noexcept { assert(this->_data != nullptr); return *(this->_data); }
      inline _DataType* operator->() noexcept { assert(this->_data != nullptr); return this->_data.get(); }
      inline const _DataType* operator->() const noexcept { assert(this->_data != nullptr); return this->_data.get(); }

    private:
      std::shared_ptr<_DataType> _data;
      std::unique_lock<_LockType> _lock;
    };

    /// @class LockedRef
    /// @brief LockedRef reference of an object, for protected access
    /// @warning The lifetime of the referenced object is not guaranteed by the LockedRef.
    template <typename _DataType, typename _LockType>
    class LockedRef final {
    public:
      /// @brief Create a reference access that is locked until its destruction
      LockedRef(_DataType& data, _LockType& lock) noexcept
        : _lock(lock), _data(&data) {}
      /// @brief Create a reference access that is locked until its destruction - already locked
      LockedRef(_DataType& data, std::unique_lock<_LockType>&& lock) noexcept
        : _lock(std::move(lock)), _data(&data) {}
      LockedRef(const LockedRef<_DataType, _LockType>&) = delete;
      LockedRef(LockedRef<_DataType, _LockType>&&) = default;
      LockedRef<_DataType, _LockType>& operator=(const LockedRef<_DataType, _LockType>&) = delete;
      LockedRef<_DataType, _LockType>& operator=(LockedRef<_DataType, _LockType>&&) = default;
      ~LockedRef() = default;

      inline const _DataType& value() const noexcept { assert(this->_data != nullptr); return *(this->_data); }
      inline _DataType& value() noexcept { assert(this->_data != nullptr); return *(this->_data); }

      inline _DataType& operator*() noexcept { assert(this->_data != nullptr); return *(this->_data); }
      inline const _DataType& operator*() const noexcept { assert(this->_data != nullptr); return *(this->_data); }
      inline _DataType* operator->() noexcept { assert(this->_data != nullptr); return this->_data; }
      inline const _DataType* operator->() const noexcept { assert(this->_data != nullptr); return this->_data; }

    private:
      std::unique_lock<_LockType> _lock;
      _DataType* _data;
    };

  }
}
