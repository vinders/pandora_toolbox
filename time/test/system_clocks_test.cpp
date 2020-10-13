#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <time/system_clocks.h>
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
  template <typename _ClockType>
  constexpr bool pandora::time::ClockAlias<_ClockType>::is_steady;
#endif

using namespace pandora::time;

class SystemClocksTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- clock timing --

template <typename _ClockType>
void _validateClockTimes() {
  EXPECT_TRUE(_ClockType::init());

  double periodCount = 4.0;
  double periodGuardband = 2.0;
  double nsPerPeriod = (_ClockType::clockPeriod())
                     * 1000000000.0 * static_cast<double>(_ClockType::period::num) / static_cast<double>(_ClockType::period::den);
  // very high precision clocks need more cycles to be compared with standard lower precision clocks
  if (nsPerPeriod < 7500000.0) {
    periodCount = 7500000.0 / nsPerPeriod;
    periodGuardband = periodCount*0.5;
  }

  // validate with standard clock measures
  std::chrono::nanoseconds timeToWait(static_cast<int64_t>(periodCount*nsPerPeriod));
  auto stdTimeBefore = std::chrono::high_resolution_clock::now();
  auto time1 = _ClockType::now();
  auto ticks1 = _ClockType::countTicks();

  while (stdTimeBefore + timeToWait > std::chrono::high_resolution_clock::now());

  auto time2 = _ClockType::now();
  auto ticks2 = _ClockType::countTicks();

  EXPECT_TRUE(time1 < time2);
  EXPECT_LE(time1 + std::chrono::nanoseconds(static_cast<int64_t>((periodCount - periodGuardband) * nsPerPeriod)), time2);
  EXPECT_GE(time1 + std::chrono::nanoseconds(static_cast<int64_t>((periodCount + periodGuardband) * nsPerPeriod)), time2);
  EXPECT_TRUE(ticks1 < ticks2);

  auto lowerBound = (nsPerPeriod < 1000000.0) ? (time1 - std::chrono::nanoseconds(4000000LL)) : (time1 - std::chrono::nanoseconds(8000000LL));
  auto upperBound = (nsPerPeriod < 1000000.0) ? (time2 + std::chrono::nanoseconds(5000000LL)) : (time2 + std::chrono::nanoseconds(18000000LL));
  EXPECT_GE(_ClockType::toTimePoint(ticks2), lowerBound);
  EXPECT_LE(_ClockType::toTimePoint(ticks2), upperBound);
  int64_t lowerDuration = (nsPerPeriod < 1000000.0) ? static_cast<int64_t>(periodCount*nsPerPeriod) - 5000000LL : static_cast<int64_t>((periodCount - periodGuardband)*nsPerPeriod) - 3000000LL;
  int64_t upperDuration = (nsPerPeriod < 1000000.0) ? static_cast<int64_t>(periodCount*nsPerPeriod) + 5000000LL : static_cast<int64_t>((periodCount + periodGuardband)*nsPerPeriod) + 3000000LL;
  EXPECT_GE(_ClockType::toDuration(ticks2 - ticks1).count(), lowerDuration);
  EXPECT_LE(_ClockType::toDuration(ticks2 - ticks1).count(), upperDuration);
  EXPECT_GE(_ClockType::interval(ticks1, ticks2).count(), lowerDuration);
  EXPECT_LE(_ClockType::interval(ticks1, ticks2).count(), upperDuration);
}

TEST_F(SystemClocksTest, highResolutionClock) {
  _validateClockTimes<HighResolutionClock>();
}

TEST_F(SystemClocksTest, highResolutionAuxClock) {
  _validateClockTimes<HighResolutionAuxClock>();
}

TEST_F(SystemClocksTest, steadyClock) {
  _validateClockTimes<SteadyClock>();
}


// -- clock specs --

TEST_F(SystemClocksTest, clockSupport) {
  EXPECT_TRUE(HighResolutionClock::isSupported());
  EXPECT_TRUE(HighResolutionAuxClock::isSupported());
  EXPECT_TRUE(SteadyClock::isSupported());

  EXPECT_TRUE(SteadyClock::is_steady);
  EXPECT_TRUE(HighResolutionClock::period::den >= SteadyClock::period::den);
  EXPECT_TRUE(HighResolutionAuxClock::period::den >= SteadyClock::period::den);
}

TEST_F(SystemClocksTest, clockPeriod) {
  EXPECT_TRUE(HighResolutionClock::init());
  EXPECT_TRUE(HighResolutionAuxClock::init());
  EXPECT_TRUE(SteadyClock::init());

  double hrPeriod = HighResolutionClock::clockPeriod();
  double hraPeriod = HighResolutionAuxClock::clockPeriod();
  double scPeriod = SteadyClock::clockPeriod();
  EXPECT_TRUE(hrPeriod > 0.0 && hrPeriod <= static_cast<double>(HighResolutionClock::period::den) / 60.0);
  EXPECT_TRUE(hraPeriod > 0.0 && hraPeriod <= static_cast<double>(HighResolutionAuxClock::period::den) / 40.0);
  EXPECT_TRUE(scPeriod > 0.0 && scPeriod <= static_cast<double>(SteadyClock::period::den) / 40.0);

  double hrFreq = HighResolutionClock::clockFrequency();
  double hraFreq = HighResolutionAuxClock::clockFrequency();
  double scFreq = SteadyClock::clockFrequency();
  EXPECT_TRUE(hrFreq > 0.0 && hrFreq <= static_cast<double>(HighResolutionClock::period::den));
  EXPECT_TRUE(hraFreq > 0.0 && hraFreq <= static_cast<double>(HighResolutionAuxClock::period::den));
  EXPECT_TRUE(scFreq > 0.0 && scFreq <= static_cast<double>(SteadyClock::period::den));

# ifdef _WINDOWS
    double expectedHrFreq = static_cast<double>(HighResolutionClock::period::den) / (hrPeriod * static_cast<double>(HighResolutionClock::period::num));
    double expectedHraFreq = static_cast<double>(HighResolutionAuxClock::period::den) / (hraPeriod * static_cast<double>(HighResolutionAuxClock::period::num));
    double expectedScFreq = static_cast<double>(SteadyClock::period::den) / (scPeriod * static_cast<double>(SteadyClock::period::num));
    EXPECT_TRUE(hrFreq >= expectedHrFreq*0.95 && hrFreq <= expectedHrFreq*1.05);
    EXPECT_TRUE(hraFreq >= expectedHraFreq*0.95 && hraFreq <= expectedHraFreq*1.05);
    EXPECT_TRUE(scFreq >= expectedScFreq*0.95 && scFreq <= expectedScFreq*1.05);
    // on Linux/Mac, some clocks use standard clocks -> period & frequency recalculated on every call -> too much variation for tests
# endif

# if defined(_DEBUG) || !defined(NDEBUG)
    printf("High-Res clock period: %lf ms\n"
           "Aux H-R. clock period: %lf ms\n"
           "Steady clock period:   %lf ms\n", 
           hrPeriod * 1000.0*static_cast<double>(HighResolutionClock::period::num)/static_cast<double>(HighResolutionClock::period::den), 
           hraPeriod * 1000.0*static_cast<double>(HighResolutionAuxClock::period::num)/static_cast<double>(HighResolutionAuxClock::period::den),
           scPeriod * 1000.0*static_cast<double>(SteadyClock::period::num)/static_cast<double>(SteadyClock::period::den));
# endif
}
