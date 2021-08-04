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
#include <utility>
#include <mutex>
#include "./locked.h"

namespace pandora {
  namespace pattern {
    /// @class Singleton
    /// @brief Singleton pattern - unique access to global instance
    template <typename _DataType>
    class Singleton final {
    public:
      Singleton() = delete;

      /// @brief Access to unique instance
      static _DataType& instance() noexcept {
        static _DataType _instance;
        return _instance;
      }
    };
    
    /// @class LockedSingleton
    /// @brief Singleton pattern with lock - safe unique access to global instance
    template <typename _DataType, typename _LockType = std::mutex>
    class LockedSingleton final {
    public:
      LockedSingleton() = delete;
      
      /// @brief Locked access to unique instance
      static LockedRef<_DataType,_LockType> instance() noexcept {
        static std::pair<_DataType,_LockType> _instance;
        return LockedRef<_DataType,_LockType>(_instance.first, _instance.second);
      }
    };
    
  }
}
