/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
