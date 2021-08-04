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
  if (setCurrentProcessAffinity(newMask) && result) { // may fail on some systems, but should never crash
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
