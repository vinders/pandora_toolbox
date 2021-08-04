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
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <chrono>

#if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINNT)
# ifndef NOMINMAX
#   define NOMINMAX 1
# endif
# include <Windows.h>
#elif defined(__APPLE__) || defined(__ANDROID__) || defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
# include <time.h>
# include <sys/time.h>
# include <unistd.h>
# if defined(__MACH__) && !defined(CLOCK_MONOTONIC)
#   include <mach/clock.h>
#   include <mach/mach.h>
# endif
#endif

#include "time/system_clocks.h"

using namespace pandora::time;


#if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINNT)
# if defined(_CPP_REVISION) && _CPP_REVISION == 14
    constexpr bool QueryPerformanceClock::is_steady;
    constexpr bool MultimediaTimerClock::is_steady;
    constexpr bool TickCountClock::is_steady;
# endif

  // -- QueryPerformanceClock --

  int64_t QueryPerformanceClock::countTicks() noexcept {
    assert(isSupported());
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return static_cast<int64_t>(count.QuadPart);
  }

  double QueryPerformanceClock::_getClockFrequency() noexcept {
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency))
      return static_cast<double>(frequency.QuadPart);
    return 0.0;
  }

  // -- MultimediaTimerClock --

  std::chrono::milliseconds MultimediaTimerClock::_clockPeriod = MultimediaTimerClock::_defaultSystemPeriod();

  // Time storage class for 32-bit clocks, to avoid time wraps
  class MultimediaTime {
  public:
    // Update total time + verify time wraps
    inline void updateTime(uint32_t currentCount) noexcept {
      if (currentCount >= this->lastCount) { // normal behaviour
        this->lastCount = currentCount;
      }
      else if (currentCount < 0x60000000u && this->lastCount > 0xA0000000u) { // max int32 reached -> time wrap
        this->totalWraps += 0xFFFFFFFFLL;
        this->lastCount = currentCount;
      }
      // else: small backwards variation -> ignore
    }
    // Get total time
    constexpr inline int64_t getTime() const noexcept { return (this->totalWraps + static_cast<int64_t>(this->lastCount)); }
    
    constexpr MultimediaTime() = default;
  private:
    int64_t totalWraps = 0LL;
    uint32_t lastCount = 0u;
  };

  bool MultimediaTimerClock::getValidPeriodRange(std::chrono::milliseconds& outMin, std::chrono::milliseconds& outMax) noexcept {
    TIMECAPS timerRes;
    if (timeGetDevCaps(&timerRes, sizeof(TIMECAPS)) == TIMERR_NOERROR) {
      outMin = (timerRes.wPeriodMin >= 1u && timerRes.wPeriodMin != 0xFFFFFFFFu) 
               ? std::chrono::milliseconds(timerRes.wPeriodMin) : std::chrono::milliseconds(1);
      outMax = (timerRes.wPeriodMax >= static_cast<UINT>(MultimediaTimerClock::_defaultSystemPeriod().count()) && timerRes.wPeriodMax != 0xFFFFFFFFu)
               ? std::chrono::milliseconds(timerRes.wPeriodMax) : MultimediaTimerClock::_defaultSystemPeriod();
      return true;
    }
    return false;
  }

  std::chrono::milliseconds MultimediaTimerClock::setPeriod(const std::chrono::milliseconds& preferredPeriod) noexcept {
    UINT usedPeriod = static_cast<UINT>(preferredPeriod.count());
    while (timeBeginPeriod(usedPeriod) != TIMERR_NOERROR && usedPeriod < static_cast<UINT>(_defaultSystemPeriod().count()))
      ++usedPeriod;
    MultimediaTimerClock::_clockPeriod = std::chrono::milliseconds(usedPeriod);
    return MultimediaTimerClock::_clockPeriod;
  }
  void MultimediaTimerClock::cancelPeriod(const std::chrono::milliseconds& actualCurrentPeriod) noexcept {
    timeEndPeriod((UINT)actualCurrentPeriod.count());
  }
          
  int64_t MultimediaTimerClock::countTicks() noexcept {
    static MultimediaTime timeValue;
    timeValue.updateTime(static_cast<uint32_t>(timeGetTime()));
    return timeValue.getTime();
  }

  // -- TickCountClock --

  int64_t TickCountClock::countTicks() noexcept { return static_cast<int64_t>(GetTickCount64()); }


#elif defined(__APPLE__) || defined(__ANDROID__) || defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
# if defined(__MACH__) && !defined(CLOCK_MONOTONIC)
#   if defined(_CPP_REVISION) && _CPP_REVISION == 14
      constexpr bool MachSystemClock::is_steady;
#   endif

    // -- MachSystemClock --

    double MachSystemClock::_getClockPeriod() noexcept {
      clock_res_t periodNano;
      return (clock_get_res(mach_host_self(), &periodNano) == 0)
             ? static_cast<double>(periodNano)
             : MachSystemClock::_defaultSystemPeriod();
    }

    int64_t MachSystemClock::countTicks() noexcept { 
      clock_serv_t clockServ;
      mach_timespec_t timeValue;

      host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clockServ);
      clock_get_time(clockServ, &timeValue);
      mach_port_deallocate(mach_task_self(), clockServ);
      return static_cast<int64_t>(timeValue.tv_sec) * 1000000000LL + static_cast<int64_t>(timeValue.tv_nsec);
    }

        
# elif defined(_POSIX_MONOTONIC_CLOCK) || defined(CLOCK_MONOTONIC)
#   if defined(_CPP_REVISION) && _CPP_REVISION == 14
      constexpr bool MonotonicClock::is_steady;
#   endif

    // -- MonotonicClock --

    double MonotonicClock::_getClockPeriod() noexcept {
      struct timespec period;
      return (clock_getres(CLOCK_MONOTONIC, &period) == 0)
             ? (static_cast<double>(period.tv_sec) * 1000000000.0 + static_cast<double>(period.tv_nsec))
             : MonotonicClock::_defaultSystemPeriod();
    }
                                  
    int64_t MonotonicClock::countTicks() noexcept { 
      struct timespec timeValue;
      clock_gettime(CLOCK_MONOTONIC, &timeValue);
      return static_cast<int64_t>(timeValue.tv_sec) * 1000000000LL + static_cast<int64_t>(timeValue.tv_nsec);
    }
# endif
#endif
