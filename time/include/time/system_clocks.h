/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
System-specific clocks: 
 - windows:    QueryPerformanceClock, MultimediaTimerClock, TickCountClock
 - mac os:     MachSystemClock
 - unix/linux: MonotonicClock
Aliases for all systems: HighResolutionClock, HighResolutionAuxClock, SteadyClock

Note: before using one of these clock, <ClockType>::isSupported() should be verified.

Warning: High resolution clocks can have unexpected behaviors when they switch to different CPU cores.
         It is recommended to set a thread/CPU affinity to 1 core, to always read the clock from the same core.
         See <hardware/process_affinity.h>
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>
#if !defined(_WINDOWS) && (defined(__APPLE__) || defined(__ANDROID__) || defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION))
# include <unistd.h>
#endif

namespace pandora {
  namespace time {
    
#   if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINNT)
      /// @brief Very high-performance monotonic clock (microseconds resolution)
      /// @warning The timing can vary if the CPU context changes: 
      ///          -> on multi-core CPUs (or multi-CPU systems), set the timing thread CPU affinity to always run on the same core.
      class QueryPerformanceClock final {
      public:
        using rep = int64_t;
        using period = std::nano;
        using duration = std::chrono::nanoseconds;
        using time_point = std::chrono::time_point<QueryPerformanceClock>;

        static inline bool init() noexcept { return (clockFrequency() > 0.0); } ///< Initialize clock frequency (must be called before any measurement is done)
        
        static constexpr bool is_steady = true; ///< Verify if clock is monotonic
        static inline bool isSupported() noexcept { return init(); } ///< Check if clock works on current system
        /// @brief Get number of periods per second
        static inline double clockFrequency() noexcept { static double _clockFrequency = _getClockFrequency(); return _clockFrequency; }
        /// @brief Get number of ticks of a period
        static inline double clockPeriod() noexcept { return static_cast<double>(period::den) / (static_cast<double>(clockFrequency()) * static_cast<double>(period::num)); }
        
        static inline time_point now() noexcept { return toTimePoint<time_point>(countTicks()); } ///< Read current time
        static int64_t countTicks() noexcept; ///< Read current ticks
        
        /// @brief Compute std::chrono::duration of an interval of ticks
        static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
        template <typename _DurationType>
        static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept { 
          return toDuration<_DurationType>((currentTicks >= previousTicks) ? currentTicks - previousTicks : 0LL);
        }
        /// @brief Convert ticks to std::chrono::duration
        static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
        template <typename _DurationType>
        static inline _DurationType toDuration(int64_t ticks) noexcept {
          double value = (static_cast<double>(ticks) / clockFrequency()) * static_cast<double>(_DurationType::period::den / _DurationType::period::num);
          _DurationType duration(static_cast<typename _DurationType::rep>(value));
          return duration;
        }
        /// @brief Convert ticks to std::chrono::time_point
        static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
        template <typename _TimePointType>
        static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
          return _TimePointType( toDuration<typename _TimePointType::duration>(ticks) );
        }
        
      private:
        static double _getClockFrequency() noexcept;
      };
      using HighResolutionClock = QueryPerformanceClock;
      
      // ---
      
      /// @brief High-performance monotonic clock (resolution can be set down to 1 millisecond)
      /// @warning The timing can vary if the CPU context changes: 
      ///          -> on multi-core CPUs (or multi-CPU systems), set the timing thread CPU affinity to always run on the same core.
      class MultimediaTimerClock final {
      public:
        using rep = int64_t;
        using period = std::milli;
        using duration = std::chrono::milliseconds;
        using time_point = std::chrono::time_point<MultimediaTimerClock>;

        static inline bool init() noexcept { return true; } ///< Initialize clock frequency
        
        static constexpr bool is_steady = true; ///< Verify if clock is monotonic
        static constexpr inline bool isSupported() noexcept { return true; } ///< Check if clock works on current system
         /// @brief Get number of periods per second
        static inline double clockFrequency() noexcept { return (1000.0 / clockPeriod()); }
        /// @brief Get number of ticks of a period
        static inline double clockPeriod() noexcept { return static_cast<double>(MultimediaTimerClock::_clockPeriod.count()); }
       
        /// @brief Read the min/max periods that can be used when calling 'setPeriod'
        /// @returns Success retrieving values (or failure if period customization not supported)
        static bool getValidPeriodRange(std::chrono::milliseconds& outMin, std::chrono::milliseconds& outMax) noexcept;
        /// @brief Configure the clock with a period as close as possible to 'preferredPeriodMs'
        /// @warning Each call to 'setPeriod' should be associated with a call to 'cancelPeriod' when the timer isn't used anymore.
        ///          The value in 'cancelPeriod' should be the value returned by 'setPeriod'.
        ///          Using a very short period can result in poor performance due to context changes happening more often.
        /// @returns The period value accepted by the clock
        static std::chrono::milliseconds setPeriod(const std::chrono::milliseconds& preferredPeriod) noexcept;
        /// @brief Cancel a period previously configured, and restore system default value
        /// @warning 'actualCurrentPeriod' must have the value that was returned by the call to setPeriod. 
        static void cancelPeriod(const std::chrono::milliseconds& actualCurrentPeriod) noexcept;

        static inline time_point now() noexcept { return toTimePoint<time_point>(countTicks()); } ///< Read current time
        static int64_t countTicks() noexcept; ///< Read current ticks
        
        /// @brief Compute std::chrono::duration of an interval of ticks
        static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
        template <typename _DurationType>
        static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept { 
          return toDuration<_DurationType>(currentTicks - previousTicks);
        }
        /// @brief Convert ticks to std::chrono::duration
        static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
        template <typename _DurationType>
        static inline _DurationType toDuration(int64_t ticks) noexcept {
          return std::chrono::duration_cast<_DurationType>( std::chrono::milliseconds(ticks) );
        }
        /// @brief Convert ticks to std::chrono::time_point
        static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
        template <typename _TimePointType>
        static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
          return _TimePointType( std::chrono::milliseconds(ticks) );
        }
        
      private:
        static constexpr inline std::chrono::milliseconds _defaultSystemPeriod() noexcept { return std::chrono::milliseconds(16); }
        static std::chrono::milliseconds _clockPeriod;
      };
      using HighResolutionAuxClock = MultimediaTimerClock;
      
      // ---
      
      /// @brief Simple monotonic clock, very fast to execute, very steady (low resolution)
      class TickCountClock final {
      public:
        using rep = int64_t;
        using period = std::milli;
        using duration = std::chrono::milliseconds;
        using time_point = std::chrono::time_point<TickCountClock>;

        static inline bool init() noexcept { return true; } ///< Initialize clock frequency
        
        static constexpr bool is_steady = true; ///< Verify if clock is monotonic
        static constexpr inline bool isSupported() noexcept { return true; } ///< Check if clock works on current system
        /// @brief Get number of periods per second
        static inline double clockFrequency() noexcept { return (1000.0 / _systemPeriodMilli()); }
        /// @brief Get number of ticks of a period
        static inline double clockPeriod() noexcept { return _systemPeriodMilli(); }
       
        static inline time_point now() noexcept { return toTimePoint<time_point>(countTicks()); } ///< Read current time
        static int64_t countTicks() noexcept; ///< Read current ticks
        
        /// @brief Compute std::chrono::duration of an interval of ticks
        static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
        template <typename _DurationType>
        static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept { 
          return toDuration<_DurationType>((currentTicks >= previousTicks) ? currentTicks - previousTicks : 0LL);
        }
        /// @brief Convert ticks to std::chrono::duration
        static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
        template <typename _DurationType>
        static inline _DurationType toDuration(int64_t ticks) noexcept {
          return std::chrono::duration_cast<_DurationType>( std::chrono::milliseconds(ticks) );
        }
        /// @brief Convert ticks to std::chrono::time_point
        static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
        template <typename _TimePointType>
        static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
          return _TimePointType( std::chrono::milliseconds(ticks) );
        }
       
      private:
        static constexpr inline double _systemPeriodMilli() { return 15.6; }
      };
      using SteadyClock = TickCountClock;
      
#   elif defined(__APPLE__) || defined(__ANDROID__) || defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
      /// @brief Generic template clock, to provide standard C++ clocks with the same interface as the other pandora::time clocks
      template <typename _ClockType>
      class ClockAlias final {
      public:
        using rep = typename _ClockType::rep;
        using period = typename _ClockType::period;
        using duration = typename _ClockType::duration;
        using time_point = typename _ClockType::time_point;

        static inline bool init() noexcept { return (clockPeriod() != 0.0); } ///< Initialize clock frequency
        static constexpr bool is_steady = _ClockType::is_steady; ///< Verify if clock is monotonic
        static inline bool isSupported() noexcept { return init(); } ///< Check if clock works on current system
        
        /// @brief Get number of periods per second
        static inline double clockFrequency() noexcept { return (static_cast<double>(period::den) / (clockPeriod() * static_cast<double>(period::num))); }
        /// @brief Get number of ticks of a period
        static inline double clockPeriod() noexcept { static double _clockPeriod = _getClockPeriod(); return _clockPeriod; }
        
        static inline time_point now() noexcept { return _ClockType::now(); } ///< Read current time
        static inline int64_t countTicks() noexcept { return static_cast<int64_t>(now().time_since_epoch().count()); } ///< Read current ticks
        
        /// @brief Compute std::chrono::duration of an interval of ticks
        static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
        template <typename _DurationType>
        static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept { 
          return toDuration<_DurationType>((currentTicks >= previousTicks) ? currentTicks - previousTicks : 0LL);
        }
        /// @brief Convert ticks to std::chrono::duration
        static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
        template <typename _DurationType>
        static inline _DurationType toDuration(int64_t ticks) noexcept {
          return std::chrono::duration_cast<_DurationType>( duration(static_cast<rep>(ticks)) );
        }
        /// @brief Convert ticks to std::chrono::time_point
        static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
        template <typename _TimePointType>
        static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
          return _TimePointType( std::chrono::nanoseconds(ticks) );
        }

      private:
        static inline double _getClockPeriod() noexcept {
          time_point startTime, endTime;
          double minPeriod = 15600000.0;
          for (int i = 3; i > 0; --i) {
            startTime = now();
            while ((endTime = now()) <= startTime);
            double measuredPeriod = static_cast<double>((endTime - startTime).count());
            if (measuredPeriod < minPeriod)
              minPeriod = measuredPeriod;
          }
          return minPeriod;
        }
      };

#     if defined(__MACH__) && !defined(CLOCK_MONOTONIC)
        /// @brief High-performance monotonic clock (resolution about 1 millisecond or less)
        class MachSystemClock final {
        public:
          using rep = int64_t;
          using period = std::nano;
          using duration = std::chrono::nanoseconds;
          using time_point = std::chrono::time_point<MachSystemClock>;

          static inline bool init() noexcept { return (clockPeriod() != 0.0); } ///< Initialize clock frequency
          
          static constexpr bool is_steady = true; ///< Verify if clock is monotonic
          static inline bool isSupported() noexcept { return init(); } ///< Check if clock works on current system
          /// @brief Get number of periods per second
          static inline double clockFrequency() noexcept { return (1000000000.0 / clockPeriod()); }
          /// @brief Get number of ticks of a period
          static double clockPeriod() noexcept { static double _clockPeriod = _getClockPeriod(); return _clockPeriod; }
          
          static inline time_point now() noexcept { return toTimePoint<time_point>(countTicks()); } ///< Read current time
          static int64_t countTicks() noexcept; ///< Read current ticks
          
          /// @brief Compute std::chrono::duration of an interval of ticks
          static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
          template <typename _DurationType>
          static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept { 
            return toDuration<_DurationType>((currentTicks >= previousTicks) ? currentTicks - previousTicks : 0LL);
          }
          /// @brief Convert ticks to std::chrono::duration
          static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
          template <typename _DurationType>
          static inline _DurationType toDuration(int64_t ticks) noexcept {
            return std::chrono::duration_cast<_DurationType>( std::chrono::nanoseconds(ticks) );
          }
          /// @brief Convert ticks to std::chrono::time_point
          static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
          template <typename _TimePointType>
          static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
            return _TimePointType( std::chrono::nanoseconds(ticks) );
          }

        private:
          static double _getClockPeriod() noexcept;
          static constexpr inline double _defaultSystemPeriod() noexcept { return 1000000.0; }
        };
        using HighResolutionClock = MachSystemClock;
        using HighResolutionAuxClock = ClockAlias<std::chrono::high_resolution_clock>;
        
#     elif defined(_POSIX_MONOTONIC_CLOCK) || defined(CLOCK_MONOTONIC)
        /// @brief High-performance monotonic clock (resolution about 1 millisecond or less)
        class MonotonicClock final {
        public:
          using rep = int64_t;
          using period = std::nano;
          using duration = std::chrono::nanoseconds;
          using time_point = std::chrono::time_point<MonotonicClock>;

          static inline bool init() noexcept { return (clockPeriod() != 0.0); } ///< Initialize clock frequency

          static constexpr bool is_steady = true; ///< Verify if clock is monotonic
          static inline bool isSupported() noexcept { return init(); } ///< Check if clock works on current system
          /// @brief Get number of periods per second
          static inline double clockFrequency() noexcept { return (1000000000.0 / clockPeriod()); }
          /// @brief Get number of ticks of a period
          static double clockPeriod() noexcept { static double _clockPeriod = _getClockPeriod(); return _clockPeriod; }
          
          static inline time_point now() noexcept { return toTimePoint<time_point>(countTicks()); } ///< Read current time
          static int64_t countTicks() noexcept; ///< Read current ticks
          
          /// @brief Compute std::chrono::duration of an interval of ticks
          static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
          template <typename _DurationType>
          static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept { 
            return toDuration<_DurationType>((currentTicks >= previousTicks) ? currentTicks - previousTicks : 0LL);
          }
          /// @brief Convert ticks to std::chrono::duration
          static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
          template <typename _DurationType>
          static inline _DurationType toDuration(int64_t ticks) noexcept {
            return std::chrono::duration_cast<_DurationType>( std::chrono::nanoseconds(ticks) );
          }
          /// @brief Convert ticks to std::chrono::time_point
          static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
          template <typename _TimePointType>
          static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
            return _TimePointType( std::chrono::nanoseconds(ticks) );
          }

        private:
          static double _getClockPeriod() noexcept;
          static constexpr inline double _defaultSystemPeriod() noexcept { return 1000000.0; }
        };
        using HighResolutionClock = MonotonicClock;
        using HighResolutionAuxClock = ClockAlias<std::chrono::high_resolution_clock>;

#     else
        using HighResolutionClock = ClockAlias<std::chrono::high_resolution_clock>;
        using HighResolutionAuxClock = ClockAlias<std::chrono::system_clock>;
#     endif
      
      using SteadyClock = ClockAlias<std::chrono::steady_clock>;
#   endif
  }
}
