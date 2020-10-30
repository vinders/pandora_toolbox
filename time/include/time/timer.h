/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "./rate.h"
#include "./system_clocks.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora {
  namespace time {
    /// @brief Timer lateness management mode
    enum class DelayHandling: uint32_t {
      none = 0,      ///< always fixed periods
      compensate = 1 ///< when lateness occurs, adapt following period(s) to compensate
    };

    /// @brief Measured timer stats
    struct TimerStats {
      std::chrono::nanoseconds elapsedTime; ///< Time elapsed since time reference (last measures (if provided) or last reset)
      int64_t lastTicks = 0LL;    ///< Absolute tick count at the end of the last call to "waitPeriod"
      int64_t lastAuxTicks = 0LL; ///< Absolute tick count of auxiliary clock at the end of the last call to "waitPeriod"
      uint64_t totalPeriods = 0LL;///< Total number of periods (since last reset)
      uint64_t periodCount = 0LL; ///< Number of periods since time reference (last measures (if provided) or last reset)
      double frequency = 0.0;     ///< Estimated timer frequency since time reference (last measures (if provided) or last reset)
    };
    
    // ---
    
    /// @class Timer
    /// @brief Double clocked synchronization timer
    /// @warning - The isSupported() method should be verified for each clock type, before trying to use them with the timer.
    ///          - For a maximum stability of the clocks, the thread using the timer can set its CPU affinity (see <hardware/process_affinity.h>)
    template <typename _ClockType = HighResolutionClock,       // main clock used by timer
              typename _AuxClockType = HighResolutionAuxClock, // secondary clock used by timer when main clock has unexpected results
              DelayHandling _DelayAction = DelayHandling::compensate, // timer behaviour when lateness occurs
              bool _UseActivePolling = true> // true: real-time polling (100% CPU usage) / false: thread sleep between checks (less CPU intensive, but not real-time/predictable)
    class Timer final {
    public:
      using Type = Timer<_ClockType,_AuxClockType,_DelayAction,_UseActivePolling>;
    
      /// @brief Initialize timer frequency and first period
      /// @warning The frequency value must be above 0!
      /// @throws runtime_error if clock type not supported on current system.
      ///         invalid_argument if frequency is invalid.
      Timer(const Rate& targetFrequency) {
        if (!_ClockType::isSupported())
          throw std::runtime_error("Timer: chosen clock type is not supported on this system");
        if (!_AuxClockType::isSupported())
          throw std::runtime_error("Timer: chosen auxiliary clock type is not supported on this system");
        if (!reset<true,true>(targetFrequency))
          throw std::invalid_argument("Timer: invalid frequency argument");
      }
      /// @brief Initialize timer frequency and first period
      /// @warning The frequency value must be above 0!
      /// @throws runtime_error if clock type not supported on current system.
      ///         invalid_argument if frequency is invalid.
      Timer(double targetFrequency) {
        if (!_ClockType::isSupported())
          throw std::runtime_error("Timer: chosen clock type is not supported on this system");
        if (!_AuxClockType::isSupported())
          throw std::runtime_error("Timer: chosen auxiliary clock type is not supported on this system");
        if (!reset<true,true>(targetFrequency))
          throw std::invalid_argument("Timer: invalid frequency argument");
      }
      
      Timer(const Type&) = delete;
      Timer(Type&&) noexcept = default;
      Type& operator=(const Type&) = delete;
      Type& operator=(Type&&) noexcept = default;
      ~Timer() = default;
      
      // -- getters --
      
      inline double frequency() const noexcept { ///< Get configured timer frequency
        return (this->_period.count() != 0LL) ? (1000000000.0 / static_cast<double>(this->_period.count())) : 0.0;
      }
      inline const std::chrono::nanoseconds& periodDuration() const noexcept { return this->_period; } ///< Get configured timer period
      inline uint64_t totalPeriodCount() const noexcept { return this->_totalPeriods; } ///< Get total number of waited periods (since last reset)
      
      /// @brief Measure total timer stats since last reset (for general stats, on-screen display, ...)
      inline TimerStats getTotalStats() const noexcept {
        std::chrono::nanoseconds elapsedTime = _ClockType::template interval<std::chrono::nanoseconds>(this->_firstTicks, this->_lastTicks);
        double frequency = (elapsedTime.count() > 0LL) ? static_cast<double>(this->_totalPeriods) * 1000000000.0/static_cast<double>(elapsedTime.count()) : 0.0;
        return TimerStats{ elapsedTime, this->_lastTicks, this->_lastAuxTicks, this->_totalPeriods, this->_totalPeriods, frequency };
      }
      /// @brief Measure timer stats since previous measurements (to identify slowdowns, for on-screen display, ...)
      inline TimerStats getTimeRangeStats(const TimerStats& timeReference) const noexcept {
        std::chrono::nanoseconds elapsedTime = _ClockType::template interval<std::chrono::nanoseconds>(timeReference.lastTicks, this->_lastTicks);
        if (elapsedTime.count() <= 0LL)
          elapsedTime = _AuxClockType::template interval<std::chrono::nanoseconds>(timeReference.lastAuxTicks, this->_lastAuxTicks);

        uint64_t waitedPeriods = this->_totalPeriods - timeReference.totalPeriods;
        double frequency = (elapsedTime.count() > 0LL) ? static_cast<double>(waitedPeriods) * 1000000000.0 / static_cast<double>(elapsedTime.count()) : 0.0;
        return TimerStats{ elapsedTime, this->_lastTicks, this->_lastAuxTicks, this->_totalPeriods, waitedPeriods, frequency };
      }
      
      // -- setters --
      
      /// @brief Reset time reference of current period
      template <bool _ResetTime = true, bool _ResetStats = true>
      inline void reset() noexcept {
        this->_nextPeriod = this->_period;
        __if_constexpr (_ResetTime) {
          this->_lastTicks = _ClockType::countTicks();
          this->_lastAuxTicks = _AuxClockType::countTicks();
        }
        __if_constexpr (_ResetStats) {
          this->_totalPeriods = 0uLL;
          this->_firstTicks = this->_lastTicks;
        }
      }
      /// @brief Set timer frequency + reset time reference of current period
      /// @warning The frequency value must be above 0!
      template <bool _ResetTime = true, bool _ResetStats = true>
      inline bool reset(const Rate& targetFrequency) noexcept {
        if (targetFrequency.numerator() <= 0)
          return false;
        int64_t periodNano = static_cast<int64_t>(targetFrequency.denominator()) * 1000000000LL / static_cast<int64_t>(targetFrequency.numerator());

        this->_period = std::chrono::nanoseconds(periodNano);
        this->_timeout = (static_cast<int64_t>(_ClockType::clockFrequency()) >> 1) + periodNano;
        this->_auxTimeout = (static_cast<int64_t>(_AuxClockType::clockFrequency()) >> 1) + periodNano;
        
        reset<_ResetTime,_ResetStats>();
        return true;
      }
      /// @brief Set timer frequency + reset time reference of current period
      /// @warning The frequency value must be above 0!
      template <bool _ResetTime = true, bool _ResetStats = true>
      inline bool reset(double targetFrequency) noexcept {
        if (targetFrequency <= 0.0)
          return false;
        int64_t periodNano = static_cast<int64_t>(((double)1000000000.0 / targetFrequency) + 0.5);

        this->_period = std::chrono::nanoseconds(periodNano);
        this->_timeout = (static_cast<int64_t>(_ClockType::clockFrequency()) >> 1) + periodNano;
        this->_auxTimeout = (static_cast<int64_t>(_AuxClockType::clockFrequency()) >> 1) + periodNano;
        
        reset<_ResetTime,_ResetStats>();
        return true;
      }
      
      // -- operations --

      /// @brief Wait until current period (since last call) ends
      /// @returns Lateness (relative to expected period)
      std::chrono::nanoseconds waitPeriod() noexcept {
        int64_t currentTicks = 0LL;
        int64_t currentAuxTicks = 0LL;
        
        std::chrono::nanoseconds interval{ 0 };
        do {
          currentTicks = _ClockType::countTicks();
          currentAuxTicks = _AuxClockType::countTicks();
          
          if ((!_getInterval<_ClockType>(currentTicks, this->_lastTicks, this->_timeout, interval) || interval.count() < 0) // error (paused app?) or negative (context change?) -> use aux clock
          && (!_getInterval<_AuxClockType>(currentAuxTicks, this->_lastAuxTicks, this->_auxTimeout, interval) || interval.count() < 0)) { // error (paused app?) -> consider period finished
            interval = this->_period;
          }
          else __if_constexpr (_UseActivePolling == false) {
            if (interval < this->_nextPeriod - _minSleepTime())
              std::this_thread::sleep_for(this->_nextPeriod - interval - _minSleepTime());
          }
        } 
        while (interval < this->_nextPeriod);
        
        this->_lastTicks = currentTicks;
        this->_lastAuxTicks = currentAuxTicks;
        ++(this->_totalPeriods);
        return _processLateness(interval);
      }
      
        
    private:
      template <typename _Clock>
      static inline bool _getInterval(int64_t currentTicks, int64_t& lastTicks, int64_t timeout, std::chrono::nanoseconds& outInterval) noexcept {
        int64_t intervalTicks = currentTicks - lastTicks;
        if (intervalTicks < timeout) {
          if (intervalTicks >= 0LL)
            outInterval = _Clock::template toDuration<std::chrono::nanoseconds>(intervalTicks);
          else {
            lastTicks = currentTicks;
            outInterval = std::chrono::nanoseconds(-1);
          }
          return true;
        }
        return false;
      }
      
      inline std::chrono::nanoseconds _processLateness(const std::chrono::nanoseconds& interval) noexcept {
        std::chrono::nanoseconds lateness = interval - this->_nextPeriod;
        __if_constexpr (_DelayAction == DelayHandling::compensate) {
          if (lateness < this->_period)
            this->_nextPeriod = this->_period - lateness;
          else
            this->_nextPeriod = this->_period;
        }
        return lateness;
      }

      static constexpr inline std::chrono::nanoseconds _minSleepTime() noexcept { return std::chrono::nanoseconds(16000000); }
    
    private:
      // settings
      std::chrono::nanoseconds _period;
      int64_t _timeout = 1000000000LL;
      int64_t _auxTimeout = 1000000000LL;
      
      // timer status
      std::chrono::nanoseconds _nextPeriod;
      int64_t _lastTicks = 0LL;
      int64_t _lastAuxTicks = 0LL;

      // stats
      uint64_t _totalPeriods = 0uLL;
      int64_t _firstTicks = 0LL;
    };
    
  }
}
#undef __if_constexpr
