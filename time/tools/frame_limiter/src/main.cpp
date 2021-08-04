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
--------------------------------------------------------------------------------
Small utility to test pandora.time.Timer as a frame limiter
*******************************************************************************/
#include <cstdio>
#include <time/rate_factory.h>
#include <time/timer.h>
#if defined(__ANDROID__)
# include <system/api/android_app.h>
#endif

using namespace pandora::time;

// Display current timer stats
void displayFrameRate(double computedRate, const TimerStats& stats, int64_t nbPeriodsBeforeRefresh, int64_t worst) {
  printf("\r%2.4lf fps (%1.2lfx) - %8lld ns per period - frame %02llu:%02llu:%02llu:%02llu (%llu) - worst: %8lld ns", 
        stats.frequency, 
        stats.frequency/computedRate, 
        static_cast<long long>(stats.elapsedTime.count()/nbPeriodsBeforeRefresh),
        static_cast<unsigned long long>(stats.totalPeriods/(3600uLL*static_cast<uint64_t>(computedRate))),
        static_cast<unsigned long long>((stats.totalPeriods/(60uLL*static_cast<uint64_t>(computedRate))) % 60uLL),
        static_cast<unsigned long long>((stats.totalPeriods/static_cast<uint64_t>(computedRate)) % 60uLL),
        static_cast<unsigned long long>(stats.totalPeriods%static_cast<uint64_t>(computedRate)),
        static_cast<unsigned long long>(stats.totalPeriods),
        static_cast<unsigned long long>(worst));
}

// Frame limiter loop
void limitFrameRate() {
  bool isRunning = true;
  Rate expectedFrameRate = RateFactory::fromSmpteRate(SmpteRate::ntsc_60pi);
  double computedRate = expectedFrameRate.compute();
  int64_t nbPeriodsBeforeRefresh = 8uLL;

  Timer<HighResolutionClock, HighResolutionAuxClock, DelayHandling::compensate, false> timer(expectedFrameRate);
  printf("Configuration: %lf fps - timer frequency: %lf - expected period: %lld ns\n\n", 
         computedRate, timer.frequency(), static_cast<long long>(timer.periodDuration().count()));

  int64_t worstLateness = 0LL;
  TimerStats lastStats = timer.getTotalStats();
  while (isRunning) {
    int64_t lateness = timer.waitPeriod().count();
    if (lateness > worstLateness)
      worstLateness = lateness;

    if ((timer.totalPeriodCount() % nbPeriodsBeforeRefresh) == 0uLL) {
      lastStats = timer.getTimeRangeStats(lastStats);
      displayFrameRate(computedRate, lastStats, nbPeriodsBeforeRefresh, static_cast<int64_t>(timer.periodDuration().count()) + worstLateness);
    }
  }
}

// ---

#if defined(__ANDROID__)
  void android_main(struct android_app* state) {
    pandora::system::AndroidApp::instance().init(state);
    limitFrameRate();
  }
#else
  int main() {
    limitFrameRate();
    return 0;
  }
#endif
