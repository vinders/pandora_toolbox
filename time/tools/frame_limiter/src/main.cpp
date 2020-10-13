/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Small utility to test pandora.time.Timer as a frame limiter
*******************************************************************************/
#include <cstdio>
#include <time/rate_factory.h>
#include <time/timer.h>

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

// Main loop with frame limiter
int main() {
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
  return 0;
}
