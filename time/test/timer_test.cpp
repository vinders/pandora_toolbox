#include <gtest/gtest.h>
#include <chrono>
#include <time/timer.h>

using namespace pandora::time;

class TimerTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- mocks --

template <int64_t _Freq, bool _IsSupported = true>
class FakeClock final {
public:
  using period = std::nano;
  using time_point = std::chrono::time_point<std::chrono::nanoseconds>;

  static inline bool isSupported() noexcept { return _IsSupported; }
  static inline double clockFrequency() noexcept { return static_cast<double>(_Freq); }
  static inline double clockPeriod() noexcept { return static_cast<double>(period::den) / (static_cast<double>(clockFrequency()) * static_cast<double>(period::num)); }

  static inline time_point now() noexcept { return toTimePoint<time_point>(countTicks()); }
  static void setTicks(int64_t ticks) noexcept { _ticks = ticks; }
  static int64_t countTicks() noexcept { auto oldTicks = _ticks; _ticks++; return oldTicks; } // increment to avoid deadlocks

  static inline std::chrono::nanoseconds interval(int64_t previousTicks, int64_t currentTicks) noexcept { return interval<std::chrono::nanoseconds>(previousTicks, currentTicks); }
  template <typename _DurationType>
  static inline _DurationType interval(int64_t previousTicks, int64_t currentTicks) noexcept {
    return toDuration<_DurationType>((currentTicks >= previousTicks) ? currentTicks - previousTicks : 0LL);
  }
  static inline std::chrono::nanoseconds toDuration(int64_t ticks) noexcept { return toDuration<std::chrono::nanoseconds>(ticks); }
  template <typename _DurationType>
  static inline _DurationType toDuration(int64_t ticks) noexcept {
    double value = (static_cast<double>(ticks) / clockFrequency()) * static_cast<double>(_DurationType::period::den / _DurationType::period::num);
    _DurationType duration(static_cast<typename _DurationType::rep>(value));
    return duration;
  }
  static inline time_point toTimePoint(int64_t ticks) noexcept { return toTimePoint<time_point>(ticks); }
  template <typename _TimePointType>
  static inline _TimePointType toTimePoint(int64_t ticks) noexcept {
    return _TimePointType(toDuration<typename _TimePointType::duration>(ticks));
  }

private:
  static int64_t _ticks;
};

template <int64_t _Freq,bool _IsSupported>
int64_t FakeClock<_Freq, _IsSupported>::_ticks = 0LL;


// -- constructors / accessors --

TEST_F(TimerTest, rateCtorAccessors) {
  // unsupported timers
  try { Timer<FakeClock<2000LL,false>, FakeClock<1000LL,true> > invalid(Rate(50, 1)); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  try { Timer<FakeClock<2000LL,true>, FakeClock<1000LL,false> > invalid(Rate(50, 1)); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  try { Timer<FakeClock<2000LL,false>, FakeClock<1000LL,false> > invalid(Rate(50, 1)); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  // invalid rates
  try { Timer<FakeClock<2000LL,true>, FakeClock<1000LL,true> > invalid(Rate(0, 1)); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  try { Timer<FakeClock<2000LL,true>, FakeClock<1000LL,true> > invalid(Rate(-50, 1)); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }

  // valid timer (init + move)
  FakeClock<2000LL, true>::setTicks(154LL);
  FakeClock<1000LL, true>::setTicks(154LL);
  Timer<FakeClock<2000LL,true>, FakeClock<1000LL,true> > validTimer(Rate(50, 1));
  EXPECT_TRUE(validTimer.frequency() >= 49.999999999);
  EXPECT_TRUE(validTimer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, validTimer.periodDuration().count());
  EXPECT_EQ(0uLL, validTimer.totalPeriodCount());

  TimerStats stats = validTimer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(154LL, stats.lastTicks);
  EXPECT_EQ(154LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);

  auto movedTimer = std::move(validTimer);
  EXPECT_TRUE(movedTimer.frequency() >= 49.999999999);
  EXPECT_TRUE(movedTimer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, movedTimer.periodDuration().count());
  EXPECT_EQ(0uLL, movedTimer.totalPeriodCount());

  validTimer = std::move(movedTimer);
  EXPECT_TRUE(validTimer.frequency() >= 49.999999999);
  EXPECT_TRUE(validTimer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, validTimer.periodDuration().count());
  EXPECT_EQ(0uLL, validTimer.totalPeriodCount());

  stats = validTimer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(154LL, stats.lastTicks);
  EXPECT_EQ(154LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);
}

TEST_F(TimerTest, freqCtorAccessors) {
  // unsupported timers
  try { Timer<FakeClock<2000LL,false>, FakeClock<1000LL,true> > invalid(50.0); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  try { Timer<FakeClock<2000LL,true>, FakeClock<1000LL,false> > invalid(50.0); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  try { Timer<FakeClock<2000LL,false>, FakeClock<1000LL,false> > invalid(50.0); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  // invalid rates
  try { Timer<FakeClock<2000LL,true>, FakeClock<1000LL,true> > invalid(0.0); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }
  try { Timer<FakeClock<2000LL,true>, FakeClock<1000LL,true> > invalid(-50.0); EXPECT_TRUE(false); }
  catch (...) { EXPECT_TRUE(true); }

  // valid timer (init + move)
  FakeClock<2000LL, true>::setTicks(154LL);
  FakeClock<1000LL, true>::setTicks(154LL);
  Timer<FakeClock<2000LL,true>, FakeClock<1000LL,true> > validTimer(50.0);
  EXPECT_TRUE(validTimer.frequency() >= 49.999999999);
  EXPECT_TRUE(validTimer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, validTimer.periodDuration().count());
  EXPECT_EQ(0uLL, validTimer.totalPeriodCount());

  TimerStats stats = validTimer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(154LL, stats.lastTicks);
  EXPECT_EQ(154LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);

  auto movedTimer = std::move(validTimer);
  EXPECT_TRUE(movedTimer.frequency() >= 49.999999999);
  EXPECT_TRUE(movedTimer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, movedTimer.periodDuration().count());
  EXPECT_EQ(0uLL, movedTimer.totalPeriodCount());

  validTimer = std::move(movedTimer);
  EXPECT_TRUE(validTimer.frequency() >= 49.999999999);
  EXPECT_TRUE(validTimer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, validTimer.periodDuration().count());
  EXPECT_EQ(0uLL, validTimer.totalPeriodCount());

  stats = validTimer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(154LL, stats.lastTicks);
  EXPECT_EQ(154LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);
}


// -- timer operations --

TEST_F(TimerTest, resetTimer) {
  FakeClock<2000LL, true>::setTicks(154LL);
  FakeClock<1000LL, true>::setTicks(154LL);
  Timer<FakeClock<2000LL, true>, FakeClock<1000LL, true> > timer(Rate(50, 1));

  timer.reset<true,true>(); // countTicks -> 154 + 1
  EXPECT_TRUE(timer.frequency() >= 49.999999999);
  EXPECT_TRUE(timer.frequency() <= 50.000000001);
  EXPECT_EQ(1000000000LL / 50LL, timer.periodDuration().count());
  EXPECT_EQ(0uLL, timer.totalPeriodCount());

  TimerStats stats = timer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(155LL, stats.lastTicks);
  EXPECT_EQ(155LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);

  FakeClock<2000LL, true>::setTicks(156LL);
  FakeClock<1000LL, true>::setTicks(156LL);
  timer.reset<true,true>(Rate(80, 1));
  EXPECT_TRUE(timer.frequency() >= 79.999999999);
  EXPECT_TRUE(timer.frequency() <= 80.000000001);
  EXPECT_EQ(1000000000LL / 80LL, timer.periodDuration().count());
  EXPECT_EQ(0uLL, timer.totalPeriodCount());

  stats = timer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(156LL, stats.lastTicks);
  EXPECT_EQ(156LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);

  FakeClock<2000LL, true>::setTicks(157LL);
  FakeClock<1000LL, true>::setTicks(157LL);
  timer.reset<true,true>(80.0);
  EXPECT_TRUE(timer.frequency() >= 79.999999999);
  EXPECT_TRUE(timer.frequency() <= 80.000000001);
  EXPECT_EQ(1000000000LL / 80LL, timer.periodDuration().count());
  EXPECT_EQ(0uLL, timer.totalPeriodCount());

  stats = timer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(157LL, stats.lastTicks);
  EXPECT_EQ(157LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);

  FakeClock<2000LL, true>::setTicks(158LL);
  FakeClock<1000LL, true>::setTicks(158LL);
  timer.reset<false,true>(90.0);
  EXPECT_TRUE(timer.frequency() >= 89.999999999);
  EXPECT_TRUE(timer.frequency() <= 90.001);
  EXPECT_EQ(1000000000LL / 90LL, timer.periodDuration().count());
  EXPECT_EQ(0uLL, timer.totalPeriodCount());

  stats = timer.getTotalStats();
  EXPECT_EQ(0LL, stats.elapsedTime.count());
  EXPECT_EQ(0.0, stats.frequency);
  EXPECT_EQ(157LL, stats.lastTicks);
  EXPECT_EQ(157LL, stats.lastAuxTicks);
  EXPECT_EQ(0uLL, stats.periodCount);
  EXPECT_EQ(0uLL, stats.totalPeriods);
}

TEST_F(TimerTest, waitPeriodOnTime) {
  FakeClock<2000LL, true>::setTicks(500LL);
  FakeClock<1000LL, true>::setTicks(500LL);
  Timer<FakeClock<2000LL, true>, FakeClock<1000LL, true> > timer(Rate(50, 1));

  FakeClock<2000LL, true>::setTicks(540LL);
  FakeClock<1000LL, true>::setTicks(520LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  TimerStats stats = timer.getTotalStats();
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(540LL, stats.lastTicks);
  EXPECT_EQ(520LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(1uLL, stats.totalPeriods);

  FakeClock<2000LL, true>::setTicks(580LL);
  FakeClock<1000LL, true>::setTicks(540LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(580LL, stats.lastTicks);
  EXPECT_EQ(540LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(2uLL, stats.totalPeriods);
}

TEST_F(TimerTest, waitPeriodLate) {
  FakeClock<2000LL, true>::setTicks(500LL);
  FakeClock<1000LL, true>::setTicks(500LL);
  Timer<FakeClock<2000LL, true>, FakeClock<1000LL, true> > timer(Rate(50, 1));

  // late
  FakeClock<2000LL, true>::setTicks(560LL);
  FakeClock<1000LL, true>::setTicks(530LL);
  EXPECT_LT(0LL, timer.waitPeriod().count());

  TimerStats stats = timer.getTotalStats();
  EXPECT_LT(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency < 50.0);
  EXPECT_EQ(560LL, stats.lastTicks);
  EXPECT_EQ(530LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(1uLL, stats.totalPeriods);

  // late
  FakeClock<2000LL, true>::setTicks(620LL);
  FakeClock<1000LL, true>::setTicks(560LL);
  EXPECT_LT(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_LT(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency < 50.0);
  EXPECT_EQ(620LL, stats.lastTicks);
  EXPECT_EQ(560LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(2uLL, stats.totalPeriods);

  // valid period after it
  FakeClock<2000LL, true>::setTicks(660LL);
  FakeClock<1000LL, true>::setTicks(580LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(660LL, stats.lastTicks);
  EXPECT_EQ(580LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(3uLL, stats.totalPeriods);
}

TEST_F(TimerTest, waitPeriodLaterThanWholePeriod) {
  FakeClock<2000LL, true>::setTicks(500LL);
  FakeClock<1000LL, true>::setTicks(500LL);
  Timer<FakeClock<2000LL, true>, FakeClock<1000LL, true> > timer(Rate(50, 1));

  // very late
  FakeClock<2000LL, true>::setTicks(860LL);
  FakeClock<1000LL, true>::setTicks(830LL);
  EXPECT_LT(0LL, timer.waitPeriod().count());

  TimerStats stats = timer.getTotalStats();
  EXPECT_LT(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency < 50.0);
  EXPECT_EQ(860LL, stats.lastTicks);
  EXPECT_EQ(830LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(1uLL, stats.totalPeriods);

  // valid period after it
  FakeClock<2000LL, true>::setTicks(900LL);
  FakeClock<1000LL, true>::setTicks(850LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(900LL, stats.lastTicks);
  EXPECT_EQ(850LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(2uLL, stats.totalPeriods);

  // total still late
  stats = timer.getTotalStats();
  EXPECT_LT(40000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency < 50.0);
  EXPECT_EQ(900LL, stats.lastTicks);
  EXPECT_EQ(850LL, stats.lastAuxTicks);
  EXPECT_EQ(2uLL, stats.periodCount);
  EXPECT_EQ(2uLL, stats.totalPeriods);
}

TEST_F(TimerTest, waitPeriodInvalidMainClock) {
  FakeClock<2000LL, true>::setTicks(500LL);
  FakeClock<1000LL, true>::setTicks(500LL);
  Timer<FakeClock<2000LL, true>, FakeClock<1000LL, true> > timer(Rate(50, 1));
  TimerStats stats = timer.getTotalStats();

  // invalid main
  FakeClock<2000LL, true>::setTicks(480LL);
  FakeClock<1000LL, true>::setTicks(520LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(480LL, stats.lastTicks);
  EXPECT_EQ(520LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(1uLL, stats.totalPeriods);

  // invalid main
  FakeClock<2000LL, true>::setTicks(460LL);
  FakeClock<1000LL, true>::setTicks(540LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(460LL, stats.lastTicks);
  EXPECT_EQ(540LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(2uLL, stats.totalPeriods);

  // valid period after it
  FakeClock<2000LL, true>::setTicks(500LL);
  FakeClock<1000LL, true>::setTicks(560LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(500LL, stats.lastTicks);
  EXPECT_EQ(560LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(3uLL, stats.totalPeriods);
}

TEST_F(TimerTest, waitPeriodAllInvalid) {
  FakeClock<2000LL, true>::setTicks(500LL);
  FakeClock<1000LL, true>::setTicks(500LL);
  Timer<FakeClock<2000LL, true>, FakeClock<1000LL, true> > timer(Rate(50, 1));
  TimerStats stats = timer.getTotalStats();

  // invalid main + aux
  FakeClock<2000LL, true>::setTicks(480LL);
  FakeClock<1000LL, true>::setTicks(480LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(1uLL, stats.totalPeriods);

  // valid period after it
  FakeClock<2000LL, true>::setTicks(520LL);
  FakeClock<1000LL, true>::setTicks(500LL);
  EXPECT_EQ(0LL, timer.waitPeriod().count());

  stats = timer.getTimeRangeStats(stats);
  EXPECT_EQ(20000000LL, stats.elapsedTime.count());
  EXPECT_TRUE(stats.frequency >= 49.999999999);
  EXPECT_TRUE(stats.frequency <= 50.000000001);
  EXPECT_EQ(520LL, stats.lastTicks);
  EXPECT_EQ(500LL, stats.lastAuxTicks);
  EXPECT_EQ(1uLL, stats.periodCount);
  EXPECT_EQ(2uLL, stats.totalPeriods);
}

TEST_F(TimerTest, waitPeriodTimingCheck) {
  auto timeBefore = std::chrono::high_resolution_clock::now();

  Timer<HighResolutionClock,HighResolutionAuxClock> timer(Rate(50, 1));
  auto lateness = timer.waitPeriod() + timer.waitPeriod();

  auto timeAfter = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE((timeAfter - timeBefore).count() >= 30000000LL);
  EXPECT_TRUE((timeAfter - timeBefore).count() <= 45000000LL + lateness.count());
}
