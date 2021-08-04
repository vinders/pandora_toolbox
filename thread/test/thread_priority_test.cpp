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
