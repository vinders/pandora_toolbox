#include <gtest/gtest.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <atomic>
#include <hardware/process_affinity.h>

using namespace pandora::hardware;

class ProcessAffinityTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- process affinity --

TEST_F(ProcessAffinityTest, processAffinity) { // disabled for linux/unix/os-x CI tests (operations not allowed)
  int32_t origCoreMask = 0;
  bool result = getCurrentProcessAffinity(origCoreMask); // may fail on some systems, but should never crash
  if (!result)
    printf("getCurrentProcessAffinity is not supported or not allowed in current context...");

  int32_t newMask = 0x03;
  int32_t verifiedMask = 0;
  if (setCurrentProcessAffinity(newMask)) { // may fail on some systems, but should never crash
    EXPECT_TRUE(getCurrentProcessAffinity(verifiedMask));
    EXPECT_EQ(newMask, verifiedMask);
  }
  else
    printf("setCurrentProcessAffinity is not supported or not allowed in current context...");
  
  setCurrentProcessAffinity(result ? origCoreMask : 0); // restore initial affinity mask, to avoid affecting gtest process
}


// -- thread affinity --

#ifndef __ANDROID__
  void secondaryThread(std::atomic_bool* isRunning, std::atomic_bool* outIsThreadReady, bool* outResult) {
    if (outResult != nullptr)
      *outResult = setCurrentThreadAffinity(0x02);
    if (outIsThreadReady != nullptr)
      *outIsThreadReady = true;

    if (isRunning != nullptr) {
      auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(2000);
      while (*isRunning && std::chrono::high_resolution_clock::now() < timeout)
        std::this_thread::yield();
    }
  }
#endif

TEST_F(ProcessAffinityTest, threadAffinity) {
# ifndef __ANDROID__
    std::atomic_bool isRunning{ true };
    std::atomic_bool isThreadReady{ false };
    bool result = false;

    std::thread secondThread(secondaryThread, &isRunning, &isThreadReady, &result);

    auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(2000);
    while (!isThreadReady && std::chrono::high_resolution_clock::now() < timeout)
      std::this_thread::yield();
    EXPECT_TRUE(isThreadReady);

    EXPECT_TRUE(result);
#   ifndef __MINGW32__
      EXPECT_TRUE(setThreadAffinity(secondThread, 0x03));
#   else
    setThreadAffinity(secondThread, 0x03);
#   endif

    isRunning = false;
    std::this_thread::yield();
    secondThread.join();
# else
    printf("No thread affinity system available for Android...");
# endif
}
