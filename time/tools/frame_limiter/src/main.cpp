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
void displayFrameRate(double computedRate, const TimerStats& stats, int64_t nbPeriodsBeforeRefresh) {
  printf("\r%2.4lf fps (%1.2lfx) - %8lld ns per period - frame %02llu:%02llu:%02llu:%02llu (%llu)", 
        stats.frequency, 
        stats.frequency/computedRate, 
        static_cast<long long>(stats.elapsedTime.count()/nbPeriodsBeforeRefresh),
        static_cast<unsigned long long>(stats.totalPeriods/(3600uLL*static_cast<uint64_t>(computedRate))),
        static_cast<unsigned long long>((stats.totalPeriods/(60uLL*static_cast<uint64_t>(computedRate))) % 60uLL),
        static_cast<unsigned long long>((stats.totalPeriods/static_cast<uint64_t>(computedRate)) % 60uLL),
        static_cast<unsigned long long>(stats.totalPeriods%static_cast<uint64_t>(computedRate)),
        static_cast<unsigned long long>(stats.totalPeriods));
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

  TimerStats lastStats = timer.getTotalStats();
  while (isRunning) {
    timer.waitPeriod();

    if ((timer.totalPeriodCount() % nbPeriodsBeforeRefresh) == 0uLL) {
      lastStats = timer.getTimeRangeStats(lastStats);
      displayFrameRate(computedRate, lastStats, nbPeriodsBeforeRefresh);
    }
  }
  return 0;
}
