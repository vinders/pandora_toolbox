/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <thread>
#include <atomic>
#include <chrono>

namespace pandora {
  namespace thread {
    /// @class SpinLock
    /// @brief Simple synchronization primitive to protect data from being simultaneously accessed.
    /// @description Simple synchronization primitive to protect data from being simultaneously accessed by multiple threads.
    ///              The spin-lock is very efficient for short critical sections, because it involves no system call.
    ///              It is however not recommended for longer critical sections, because it uses active polling.
    class SpinLock final {
    public:
      /// @brief Create an new unlocked instance
      SpinLock() noexcept {}
      SpinLock(const SpinLock&) = delete;
      SpinLock(SpinLock&&) = delete;
      SpinLock& operator=(const SpinLock&) = delete;
      SpinLock& operator=(SpinLock&&) = delete;

      // -- simple lock management --

      /// @brief Wait until object unlocked, then lock it
      inline void lock() noexcept {
        while (this->_status.test_and_set(std::memory_order_acquire))
          std::this_thread::yield();
      }

      /// @brief Only lock the object if there's no need to wait (already unlocked)
      /// @returns True on lock success, false if already locked by another owner
      inline bool tryLock() noexcept {
        return !(this->_status.test_and_set(std::memory_order_acquire)); 
      }
      inline bool try_lock() noexcept { return tryLock(); } // STL-compliant version

      /// @brief Unlock the object
      /// @warning Should only be called after having called lock() (by the thread that currently owns the object).
      inline void unlock() noexcept {
        this->_status.clear(std::memory_order_release);
      }

      // -- lock with timeout --

      /// @brief Only wait for a specific period for the object to be unlocked.
      /// @returns True on lock success, false if timeout
      template <typename _RepetitionType, typename _PeriodType>
      inline bool tryLock(const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept {
        if (this->_status.test_and_set(std::memory_order_acquire)) // if already locked
          return tryLockUntil(std::chrono::time_point<std::chrono::steady_clock>(std::chrono::steady_clock::now() + retryDuration));
        return true;
      }
      template <typename _RepetitionType, typename _PeriodType>
      inline bool try_lock_for(const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept { return tryLock(retryDuration); } // STL-compliant version

      /// @brief Only wait until a specific time-point for the object to be unlocked.
      /// @returns True on lock success, false if timeout
      template <typename _ClockType, typename _DurationType>
      inline bool tryLockUntil(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept {
        while (this->_status.test_and_set(std::memory_order_acquire)) { // retry while still locked
          if (_ClockType::now() > timeoutTimePoint) // check for timeout
            return false;

          std::this_thread::yield();
        }
        return true;
      }
      template <typename _ClockType, typename _DurationType>
      inline bool try_lock_until(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept { return tryLockUntil(timeoutTimePoint); } // STL-compliant version
            
    private:
      std::atomic_flag _status = ATOMIC_FLAG_INIT;
    };
    
  }
}
