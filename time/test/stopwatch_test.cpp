#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <time/stopwatch.h>

using namespace pandora::time;

class StopwatchTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- constructors/accessors --

TEST_F(StopwatchTest, defaultCtor) {
  Stopwatch<> sw;
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());
  
  sw.reset();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());
}

// -- stopwatch operations --

TEST_F(StopwatchTest, startStop) {
  Stopwatch<> sw;
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());

  sw.start();
  EXPECT_TRUE(sw.elapsedTime<std::chrono::nanoseconds>() < std::chrono::nanoseconds{ 5000000 });
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  sw.stop();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_TRUE(sw.elapsedTime<std::chrono::nanoseconds>() >= std::chrono::nanoseconds{ 10000000 });

  sw.start();
  auto before = std::chrono::high_resolution_clock::now();
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  EXPECT_TRUE(sw.elapsedTime<std::chrono::nanoseconds>() < std::chrono::nanoseconds{ 5000000 });
  while (std::chrono::high_resolution_clock::now() == before)
    std::this_thread::yield();
  sw.stop();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_TRUE(sw.elapsedTime<std::chrono::nanoseconds>() > std::chrono::nanoseconds{ 0 });
}

TEST_F(StopwatchTest, startPause) {
  Stopwatch<> sw;
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());

  sw.start();
  EXPECT_TRUE(sw.elapsedTime<std::chrono::nanoseconds>() < std::chrono::nanoseconds{ 5000000 });
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  sw.pause();
  EXPECT_EQ(Stopwatch<>::Status::paused, sw.status());
  auto firstPauseTime = sw.elapsedTime<std::chrono::nanoseconds>();
  EXPECT_TRUE(firstPauseTime >= std::chrono::nanoseconds{ 10000000 });

  sw.start();
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  EXPECT_TRUE(sw.elapsedTime<std::chrono::nanoseconds>() >= std::chrono::nanoseconds{ 5000000 });
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  sw.pause();
  EXPECT_EQ(Stopwatch<>::Status::paused, sw.status());
  auto secondPauseTime = sw.elapsedTime<std::chrono::nanoseconds>();
  EXPECT_TRUE(secondPauseTime > firstPauseTime);
  sw.stop();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(secondPauseTime, sw.elapsedTime<std::chrono::nanoseconds>());
}

TEST_F(StopwatchTest, reset) {
  Stopwatch<> sw;
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());
  
  sw.start();
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  sw.reset();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());

  sw.start();
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  sw.stop();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  sw.reset();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());

  sw.start();
  EXPECT_EQ(Stopwatch<>::Status::running, sw.status());
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  sw.pause();
  EXPECT_EQ(Stopwatch<>::Status::paused, sw.status());
  sw.reset();
  EXPECT_EQ(Stopwatch<>::Status::idle, sw.status());
  EXPECT_EQ(std::chrono::nanoseconds{ 0 }, sw.elapsedTime<std::chrono::nanoseconds>());
}
