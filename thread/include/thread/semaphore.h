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
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace pandora {
  namespace thread {
    /// @class Semaphore
    /// @brief Synchronization primitive (for threads) with a counter, for producer/consumer patterns.
    /// @details Semaphore for threads with an internal counter. Each call to wait() decrements it, and each call to notify() increments it.
    ///          When the counter equals zero, any call to wait() will wait until notify() is called by another thread.
    ///          Typically used for a producer/consumer pattern, or any other form of synchronization that requires a counter.
    ///          When multiple threads are waiting for notifications, the order in which they'll awake is not guaranteed.
    /// @warning To avoid deadlocks, the calls to wait() and notify() should never be in the same thread (unless notify() is called before wait()).
    class Semaphore final {
    public:
      /// @brief Create instance with an initial count of 0
      Semaphore() = default;
      /// @brief Create instance with a custom initial count
      Semaphore(uint32_t initialCount) noexcept : _count(initialCount) {}

      Semaphore(const Semaphore&) = delete;
      Semaphore(Semaphore&& rhs) = delete;
      Semaphore& operator=(const Semaphore&) = delete;
      Semaphore& operator=(Semaphore&& rhs) = delete;
      /// @brief Awake all waiters and destroy instance
      ~Semaphore() noexcept { 
        std::unique_lock<std::mutex> lock(this->_lock);
        while (this->_waiters) {
          this->_count += this->_waiters;
          lock.unlock();
          this->_condition.notify_all();
          std::this_thread::sleep_for(std::chrono::microseconds(1));
          lock.lock();
        }
      }

      // -- semaphore operations --

      /// @brief Wait until the counter has a value above 0, then consume 1 unit
      inline void wait() noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        ++(this->_waiters);
        while (this->_count == 0u)
          this->_condition.wait(lock);
        
        --(this->_count);
        --(this->_waiters);
      }
      /// @brief Wait for the counter to reach a specific count, then consume that count
      inline void wait(uint32_t reqCount) noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        this->_waiters += reqCount;
        while (this->_count < reqCount)
          this->_condition.wait(lock);
        
        this->_count -= reqCount;
        this->_waiters -= reqCount;
      }

      /// @brief Check if the counter has a value above 0, and consume 1 unit if it's the case
      /// @returns True on success, false if nothing to consume
      inline bool tryWait() noexcept {
        std::lock_guard<std::mutex> lock(this->_lock);
        if (this->_count > 0u) {
          --(this->_count);
          return true;
        }
        return false;
      }
      /// @brief Check if the counter has at least a specific value, and consume that count if it's the case
      /// @returns True on success, false if nothing to consume
      inline bool tryWait(uint32_t reqCount) noexcept {
        std::lock_guard<std::mutex> lock(this->_lock);
        if (this->_count >= reqCount) {
          this->_count -= reqCount;
          return true;
        }
        return false;
      }

      /// @brief Increase the counter of 1 unit, then awake one waiting thread
      inline void notify() noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        ++(this->_count);
        lock.unlock();

        this->_condition.notify_one();
      }
      /// @brief Increase the counter of multiple units, then awake that number of waiting threads
      inline void notify(uint32_t unitCount) noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        this->_count += unitCount;
        lock.unlock();

        for (unitCount = this->_count; unitCount; --unitCount)
          this->_condition.notify_one();
      }
      /// @brief Increase the counter to awake each waiting threads
      inline void notifyAll() noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        uint32_t unitCount = this->_waiters;
        this->_count += unitCount;
        lock.unlock();

        this->_condition.notify_all();
      }
      
      /// @brief Reset the counter to 0, and consume all units
      /// @returns Value that the counter held before being reset
      inline uint32_t reset() noexcept {
        std::lock_guard<std::mutex> lock(this->_lock);
        uint32_t unitCount = this->_count;
        this->_count = 0u;
        return unitCount;
      }

      // -- operations with timeout --

      /// @brief Only wait until the counter has a value above 0 for a specific period of time
      /// @returns True on success, false if timeout
      template <typename _RepetitionType, typename _PeriodType>
      inline bool tryWait(const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept {
        return tryWaitUntil(std::chrono::time_point<std::chrono::steady_clock>(std::chrono::steady_clock::now() + retryDuration));
      }
      /// @brief Only wait until the counter has reached a specific value for a specific period of time
      /// @returns True on success, false if timeout
      template <typename _RepetitionType, typename _PeriodType>
      inline bool tryWait(uint32_t reqCount, const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept {
        return tryWaitUntil(reqCount, std::chrono::time_point<std::chrono::steady_clock>(std::chrono::steady_clock::now() + retryDuration));
      }
      template <typename _RepetitionType, typename _PeriodType>
      inline bool try_wait_for(const std::chrono::duration<_RepetitionType, _PeriodType>& retryDuration) noexcept { return tryWait(retryDuration); } // STL-compliant version

      /// @brief Wait until the counter has a value above 0, or until a specific time-point
      /// @returns True on success, false if timeout
      template <typename _ClockType, typename _DurationType>
      inline bool tryWaitUntil(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept { return tryWaitUntil(1u, timeoutTimePoint); }
      /// @brief Wait until the counter has reached a specific value, or until a specific time-point
      /// @returns True on success, false if timeout
      template <typename _ClockType, typename _DurationType>
      bool tryWaitUntil(uint32_t reqCount, const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept {
        std::unique_lock<std::mutex> lock(this->_lock);
        this->_waiters += reqCount;
        while (this->_count < reqCount) {
          if (this->_condition.wait_until(lock, timeoutTimePoint) == std::cv_status::timeout) {
            this->_waiters -= reqCount;
            return false;
          }
        }
        this->_count -= reqCount;
        this->_waiters -= reqCount;
        return true;
      }
      template <typename _ClockType, typename _DurationType>
      inline bool try_wait_until(const std::chrono::time_point<_ClockType, _DurationType>& timeoutTimePoint) noexcept { return tryWaitUntil(timeoutTimePoint); } // STL-compliant version
            
    private:
      std::mutex _lock;
      std::condition_variable _condition;

      uint32_t _count{ 0u };
      uint32_t _waiters{ 0u };
    };
    
  }
}
