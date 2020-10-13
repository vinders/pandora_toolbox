#include <gtest/gtest.h>
#include <thread/thread_priority.h>

using namespace pandora::thread;

class ThreadPriorityTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- thread priority --

TEST_F(ThreadPriorityTest, setCurrent) {
# if defined(_WINDOWS) || defined(__P_USE_POSIX_PTHREAD)
    bool result = setCurrentThreadPriority(ThreadPriority::higher);
    EXPECT_TRUE(setCurrentThreadPriority(ThreadPriority::reset));
    printf("Result: %s", result?"true":"false");
# endif
}

void _testThreadRunner(bool* isRunning) {
  while (isRunning != nullptr && *isRunning)
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
}

TEST_F(ThreadPriorityTest, setOther) {
# if defined(_WINDOWS) || defined(__P_USE_POSIX_PTHREAD)
    bool isRunning = true;
    std::thread other(&_testThreadRunner, &isRunning);
    
    bool result = setThreadPriority(other, ThreadPriority::higher);
    printf("Result: %s", result?"true":"false");
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    
    isRunning = false;
    std::this_thread::yield();
    if (other.joinable())
      other.join();
# endif
}
