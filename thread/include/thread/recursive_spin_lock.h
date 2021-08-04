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
#include <thread>
#include <atomic>
#include <chrono>

namespace pandora {
  namespace thread {
    /// @class RecursiveSpinLock
    /// @brief Simple synchronization primitive, with recursive thread ownership, to protect data from being simultaneously accessed
    /// @description Simple synchronization primitive to protect data from being simultaneously accessed by multiple threads.
    ///              The spin-lock is very efficient for short critical sections, because it involves no system call.
    ///              It is however not recommended for longer critical sections, because it uses active polling.
    ///              The recursive version offers recursive thread ownership semantics.
    class RecursiveSpinLock final {
    public:
      /// @brief Create an new unlocked instance
      RecursiveSpinLock() noexcept {}
      RecursiveSpinLock(const RecursiveSpinLock&) = delete;
      RecursiveSpinLock(RecursiveSpinLock&&) = delete;
      RecursiveSpinLock& operator=(const RecursiveSpinLock&) = delete;
      RecursiveSpinLock& operator=(RecursiveSpinLock&&) = delete;
      ~RecursiveSpinLock() noexcept { reset(); }

      // -- simple lock management --

      /// @brief Get the thread-ID of the thread currently owning the lock
      inline std::thread::id owner() const noexcept { return this->_lockingThreadId; }

      /// @brief Wait until object unlocked, then lock it (or increase counter, if same thread)
      inline void lock() noexcept {
        std::thread::id currentThreadId(std::this_thread::get_id());

        if (this->_status.test_and_set(std::memory_order_acquire)) {
          // already locked + same thread -> increase lock count
          if (currentThreadId == owner()) { 
            ++(this->_lockCount);
            return;
          }
                    
          while (this->_status.test_and_set(std::memory_order_acquire))
            std::this_thread::yield();
        }
        this->_lockCount = 1;
        owner(currentThreadId);
      }

      /// @brief Only lock the object if there's no need to wait (already unlocked)
      /// @returns True on lock success, false if already locked by another owner
      inline bool tryLock() noexcept {
        if (!this->_status.test_and_set(std::memory_order_acquire)) { // was unlocked
          this->_lockCount = 1;
          owner(std::this_thread::get_id());
          return true;
        }
        else if (std::this_thread::get_id() == owner()) { // same thread -> increase lock count
          ++(this->_lockCount);
          return true;
        }
        return false;
      }
      inline bool try_lock() noexcept { return tryLock(); } // STL-compliant version

      /// @brief Unlock the object (or decrease counter, if multiple locks for current thread)
      inline bool unlock() noexcept {
        if (std::this_thread::get_id() != owner())
          return false; // different thread or already unlocked

        if (--(this->_lockCount) < 1)
          reset();
        return true;
      }

      // -- lock with timeout --

      /// @brief Only wait for a specific period for the object to be unlocked.
      /// @returns True on lock success, false if timeout
      template <typename _RepetitionType, typename _PeriodType>
      inline bool tryLock(const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept {
        return tryLockUntil(std::chrono::time_point<std::chrono::steady_clock>(std::chrono::steady_clock::now() + retryDuration));
      }
      template <typename _RepetitionType, typename _PeriodType>
      inline bool try_lock_for(const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept { return tryLock(retryDuration); } // STL-compliant version

      /// @brief Only wait until a specific time-point for the object to be unlocked.
      /// @returns True on lock success, false if timeout
      template <typename _ClockType, typename _DurationType>
      bool tryLockUntil(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept {
        std::thread::id currentThreadId(std::this_thread::get_id());

        if (this->_status.test_and_set(std::memory_order_acquire)) {
          if (currentThreadId == owner()) { // same thread -> increase lock count
            ++(this->_lockCount);
            return true;
          }
                      
          while (this->_status.test_and_set(std::memory_order_acquire)) {
            if (_ClockType::now() > timeoutTimePoint) // check for timeout
              return false;
                
            std::this_thread::yield();
          }
        }
        this->_lockCount = 1;
        owner(currentThreadId);
        return true;
      }
      template <typename _ClockType, typename _DurationType>
      inline bool try_lock_until(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept { return tryLockUntil(timeoutTimePoint); } // STL-compliant version
            
    private:
      // set locking thread ID
      inline void owner(const std::thread::id& threadId) noexcept { this->_lockingThreadId = threadId; }

      // brief    Force the object to be unlocked
      // warning  Use with caution: no thread verification
      inline void reset() noexcept {
        this->_lockCount = 0;
        owner(std::thread::id());
        this->_status.clear(std::memory_order_release);
      }

    private:
      uint32_t _lockCount{ 0 };         // number of locks called by the locking thread (only modified by lock owner)
      std::thread::id _lockingThreadId; // ID of thread that currently owns the lock (only modified by lock owner, and reading an int is atomic)
      std::atomic_flag _status = ATOMIC_FLAG_INIT;
    };
    
  }
}
