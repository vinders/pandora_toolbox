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
#include <deque>
#include <condition_variable>

namespace pandora {
  namespace thread {
    /// @class OrderedLock
    /// @brief Synchronization primitive to protect data from being simultaneously accessed, with guaranteed FIFO order.
    /// @description Synchronization primitive to protect data from being simultaneously accessed by multiple threads.
    ///              Acts like a standard mutex, except that the order of lock is guaranteed (FIFO).
    class OrderedLock final {
      using InternalTicket = std::condition_variable*;
    public:
      /// @brief Create an new unlocked instance
      OrderedLock() = default;
      OrderedLock(const OrderedLock&) = delete;
      OrderedLock(OrderedLock&&) = delete;
      OrderedLock& operator=(const OrderedLock&) = delete;
      OrderedLock& operator=(OrderedLock&&) = delete;

      // -- lock management --

      /// @brief Wait until object unlocked, then lock it
      inline void lock() noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        if (!this->_isLocked) {
          this->_isLocked = true;
        }
        else {
          std::condition_variable condition;
          this->_orderedTickets.emplace_back(&condition);
          while (this->_notifiedTicket != &condition) // if awaken by other system signal, ignore it
            condition.wait(lock);
          this->_notifiedTicket = InternalTicket{ nullptr };
        }
      }

      /// @brief Only lock the object if there's no need to wait (already unlocked)
      /// @returns True on lock success, false if already locked by another owner
      inline bool tryLock() noexcept {
        std::lock_guard<std::mutex> lock(this->_lock);
        if (!this->_isLocked) {
          this->_isLocked = true;
          return true;
        }
        return false;
      }
      inline bool try_lock() noexcept { return tryLock(); } // STL-compliant version

      /// @brief Unlock the object
      /// @warning Should only be called after having called lock() (by the thread that currently owns the object).
      inline void unlock() noexcept {
        std::lock_guard<std::mutex> lock(this->_lock);
        if (this->_orderedTickets.empty()) {
          this->_isLocked = false;
        }
        else {
          this->_notifiedTicket = this->_orderedTickets.front();
          this->_orderedTickets.pop_front();
          this->_notifiedTicket->notify_one();
        }
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
        std::unique_lock<std::mutex> lock(this->_lock);
        if (!this->_isLocked) {
          this->_isLocked = true;
        }
        else {
          std::condition_variable condition;
          this->_orderedTickets.emplace_back(&condition);
          while (this->_notifiedTicket != &condition) { // if awaken by other system signal, ignore it
            if (condition.wait_until(lock, timeoutTimePoint) == std::cv_status::timeout && this->_notifiedTicket != &condition) {
              _cancelPendingTicket(&condition);
              return false;
            }
          }
          this->_notifiedTicket = InternalTicket{ nullptr };
        }
        return true;
      }
      template <typename _ClockType, typename _DurationType>
      inline bool try_lock_until(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept { return tryLockUntil(timeoutTimePoint); } // STL-compliant version
      
      // -- lock status --
      
      /// @brief Check whether the OrderedLock is locked or not
      inline bool isLocked() const noexcept { std::unique_lock<std::mutex> lock(this->_lock); return this->_isLocked; }
      /// @brief Get number of threads currently waiting for the lock
      inline size_t queueSize() const noexcept { std::unique_lock<std::mutex> lock(this->_lock); return this->_orderedTickets.size(); }
      
    private:
      // -- pending ticket management --
      
      void _cancelPendingTicket(InternalTicket ticket) noexcept {
        for (auto it = this->_orderedTickets.begin(); it != this->_orderedTickets.end(); ++it) {
          if (*it == ticket) {
            this->_orderedTickets.erase(it);
            break;
          }
        }
      }
      
    private:
      mutable std::mutex _lock;
      std::deque<InternalTicket> _orderedTickets;
      InternalTicket _notifiedTicket{ nullptr };
      bool _isLocked = false;
    };

  }
}
