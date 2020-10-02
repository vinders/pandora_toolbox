/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <chrono>

namespace pandora {
  namespace time {
    /// @class Stopwatch
    /// @brief High-resolution stopwatch
    /// @details A high-resolution stopwatch, that can be started and stopped to measure time durations.
    ///          Typically used for benchmarks and performance measures, but can also be used to control the state of a system.
    template <typename _ClockType = std::chrono::high_resolution_clock>
    class Stopwatch final {
    public:
      /// @brief Current state of a stopwatch
      enum class Status: uint32_t {
        idle = 0u,
        paused = 1u,
        running = 2u
      };
    
      /// @brief Create idle stopwatch
      Stopwatch() = default;
      
      Stopwatch(const Stopwatch&) = default;
      Stopwatch(Stopwatch&&) = default;
      inline Stopwatch& operator=(const Stopwatch&) = default;
      inline Stopwatch& operator=(Stopwatch&&) = default;
      ~Stopwatch() = default;

      // -- stopwatch operations --

      /// @brief Stop measurements and reset elapsed time to zero
      inline void reset() noexcept {
        this->_status = Status::idle;
        this->_currentNanosec = this->_startNanosec;
      }
      
      /// @brief Start/resume measuring elapsed time
      inline void start() noexcept {
        if (this->_status == Status::idle)
          this->_startNanosec = std::chrono::time_point_cast<std::chrono::nanoseconds>(_ClockType::now());
        else if (this->_status == Status::paused)
          this->_startNanosec = std::chrono::time_point_cast<std::chrono::nanoseconds>(_ClockType::now()) 
                                - (this->_currentNanosec - this->_startNanosec);
        this->_status = Status::running;
      }
      
      /// @brief Pause measurements - elapsed time between paused() and start() will not be counted
      inline void pause() noexcept {
        if (this->_status == Status::running)
          this->_currentNanosec = std::chrono::time_point_cast<std::chrono::nanoseconds>(_ClockType::now());
        this->_status = Status::paused;
      }
      
      /// @brief Stop measurements - calling start() will reset elapsed time
      inline void stop() noexcept {
        if (this->_status == Status::running)
          this->_currentNanosec = std::chrono::time_point_cast<std::chrono::nanoseconds>(_ClockType::now());
        this->_status = Status::idle;
      }

      // -- getters --
      
      constexpr inline Status status() const noexcept { return _status; }
      
      template <typename _DurationType>
      constexpr inline _DurationType elapsedTime() const noexcept {
        if (this->_status == Status::running)
          this->_currentNanosec = std::chrono::time_point_cast<std::chrono::nanoseconds>(_ClockType::now());
        return std::chrono::duration_cast<_DurationType>(this->_currentNanosec - this->_startNanosec); 
      }

    private:
      std::chrono::time_point<_ClockType, std::chrono::nanoseconds> _startNanosec;
      mutable std::chrono::time_point<_ClockType, std::chrono::nanoseconds> _currentNanosec;
      Status _status = Status::idle;
    };
    
  }
}
