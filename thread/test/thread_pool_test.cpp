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
#include <stdexcept>
#include <thread>
#include <chrono>
#include <functional>
#include <thread/thread_pool.h>

using namespace pandora::thread;

class ThreadPoolTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- helpers --

struct FakeType { int value = 0; };

static int totalValue = 0;
void _taskRunner(int& param) {
  totalValue += param;
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
}
void _taskRunner(FakeType& param) {
  totalValue += param.value;
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
}
void _taskRunner(std::shared_ptr<int>& param) {
  if (param != nullptr)
    totalValue += *param;
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
}
void _taskRunner(std::unique_ptr<int>& param) {
  if (param != nullptr)
    totalValue += *param;
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
}
void _exceptionRunner(int& val) {
  if (val == 42)
    throw val;
  totalValue += val;
  throw std::runtime_error("ok");
}

template <typename T>
bool _verifyAllThreadsBusy(const T& pool) {
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
  size_t freeThreads = 0xFFFFFFFFu;
  for (int retry = 0; retry < 255; ++retry) {
    if (freeThreads == 0) {
      break;
    }
    freeThreads = pool.freeThreads();
    std::this_thread::sleep_for(std::chrono::microseconds(1u));
  }
  return (freeThreads == 0);
}

template <typename T>
void _waitForJobsCompletion(const T& pool) {
  std::this_thread::sleep_for(std::chrono::milliseconds(5u));
  auto timeoutTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(2000u);
  while (std::chrono::steady_clock::now() < timeoutTime) {
    if (pool.busyThreads() > 0u) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1u));
    }
    else {
      std::this_thread::sleep_for(std::chrono::microseconds(1u));
      if (pool.busyThreads() == 0u) // second check, in case the previous one arrived right before a thread incremented its counter
        break;
    }
  }
}

// -- test procedures --

template <typename _Param>
void _validateSingleFunctionThreadPool(size_t poolSize, const _Param& param, int paramValue) {
  ThreadPool<_Param, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool(poolSize, &_taskRunner);
  EXPECT_EQ(poolSize, pool.size());
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));

  totalValue = 0;
  int nbJobs = 8 * static_cast<int>(pool.size() * pool.size());
  for (int i = 0; i < nbJobs; ++i)
    EXPECT_TRUE(pool.addJob(_Param(param)));
  EXPECT_TRUE(_verifyAllThreadsBusy(pool));
  auto cancelled = pool.cancelPendingJobs();
  EXPECT_LT(0u, cancelled);
  _waitForJobsCompletion(pool);

  EXPECT_GE(totalValue, paramValue * static_cast<int>(pool.size()));
  EXPECT_GE(paramValue * nbJobs, totalValue + paramValue * static_cast<int>(cancelled));
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
}

template <typename _Param>
void _validateSingleLambdaThreadPool(size_t poolSize, const _Param& param, int paramValue) {
  ThreadPool<_Param, ThreadRunnerMode::single, TaskRunnerType::lambda> pool(poolSize, [](_Param& param){ _taskRunner(param); });
  EXPECT_EQ(poolSize, pool.size());
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));

  totalValue = 0;
  int nbJobs = 8 * static_cast<int>(pool.size() * pool.size());
  for (int i = 0; i < nbJobs; ++i)
    EXPECT_TRUE(pool.addJob(_Param(param)));
  EXPECT_TRUE(_verifyAllThreadsBusy(pool));
  auto cancelled = pool.cancelPendingJobs();
  EXPECT_LT(0u, cancelled);
  _waitForJobsCompletion(pool);

  EXPECT_GE(totalValue, paramValue * static_cast<int>(pool.size()));
  EXPECT_GE(paramValue * nbJobs, totalValue + paramValue * static_cast<int>(cancelled));
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
}

template <typename _Param>
void _validatePerJobFunctionThreadPool(size_t poolSize, const _Param& param, int paramValue, 
                                       bool hasDefaultRunner = false, const _Param& defParam = _Param{}, int defParamValue = 0) {
  auto pool = (hasDefaultRunner)
            ? ThreadPool<_Param, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer>(poolSize, &_taskRunner)
            : ThreadPool<_Param, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer>(poolSize);
  EXPECT_EQ(poolSize, pool.size());
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));

  totalValue = 0;
  int nbJobs = 8 * static_cast<int>(pool.size() * pool.size());
  for (int i = 0; i < nbJobs; ++i)
    EXPECT_TRUE(pool.addJob(_Param(param), &_taskRunner));
  EXPECT_TRUE(_verifyAllThreadsBusy(pool));
  auto cancelled = pool.cancelPendingJobs();
  EXPECT_LT(0u, cancelled);
  _waitForJobsCompletion(pool);
  int totalValueWithCustomRunners = totalValue;

  int nbDefJobs = 8 * static_cast<int>(pool.size() * pool.size());
  for (int i = 0; i < nbJobs; ++i)
    EXPECT_TRUE(pool.addJob(defParam));
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
  auto cancelledDefRunner = pool.cancelPendingJobs();
  _waitForJobsCompletion(pool);

  if (hasDefaultRunner) {
    EXPECT_GE(totalValueWithCustomRunners, paramValue * static_cast<int>(pool.size()));
    EXPECT_GE(paramValue * nbJobs + defParamValue * nbDefJobs, 
              totalValue + paramValue * static_cast<int>(cancelled) + defParamValue * static_cast<int>(cancelledDefRunner));
  }
  else {
    EXPECT_EQ(totalValueWithCustomRunners, totalValue);
    EXPECT_GE(totalValue, paramValue * static_cast<int>(pool.size()));
    EXPECT_GE(paramValue * nbJobs, totalValue + paramValue * static_cast<int>(cancelled));
  }
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
}

template <typename _Param>
void _validatePerJobLambdaThreadPool(size_t poolSize, const _Param& param, int paramValue, 
                                     bool hasDefaultRunner = false, const _Param& defParam = _Param{}, int defParamValue = 0) {
  auto pool = (hasDefaultRunner)
            ? ThreadPool<_Param, ThreadRunnerMode::perJob, TaskRunnerType::lambda>(poolSize, [](_Param& param) { _taskRunner(param); })
            : ThreadPool<_Param, ThreadRunnerMode::perJob, TaskRunnerType::lambda>(poolSize);
  EXPECT_EQ(poolSize, pool.size());
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));

  totalValue = 0;
  int nbJobs = 8 * static_cast<int>(pool.size() * pool.size());
  for (int i = 0; i < nbJobs; ++i)
    EXPECT_TRUE(pool.addJob(_Param(param), [](_Param& param) { _taskRunner(param); }));
  EXPECT_TRUE(_verifyAllThreadsBusy(pool));
  auto cancelled = pool.cancelPendingJobs();
  EXPECT_LT(0u, cancelled);
  _waitForJobsCompletion(pool);
  int totalValueWithCustomRunners = totalValue;

  int nbDefJobs = 8 * static_cast<int>(pool.size() * pool.size());
  for (int i = 0; i < nbJobs; ++i)
    EXPECT_TRUE(pool.addJob(defParam));
  std::this_thread::sleep_for(std::chrono::milliseconds(1u));
  auto cancelledDefRunner = pool.cancelPendingJobs();
  _waitForJobsCompletion(pool);

  if (hasDefaultRunner) {
    EXPECT_GE(totalValueWithCustomRunners, paramValue * static_cast<int>(pool.size()));
    EXPECT_GE(paramValue * nbJobs + defParamValue * nbDefJobs, 
              totalValue + paramValue * static_cast<int>(cancelled) + defParamValue * static_cast<int>(cancelledDefRunner));
  }
  else {
    EXPECT_EQ(totalValueWithCustomRunners, totalValue);
    EXPECT_GE(totalValue, paramValue * static_cast<int>(pool.size()));
    EXPECT_GE(paramValue * nbJobs, totalValue + paramValue * static_cast<int>(cancelled));
  }
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(pool.size(), pool.freeThreads());
}


// -- special constructors --

TEST_F(ThreadPoolTest, emptyPoolCommonRunner) {
  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool;
  EXPECT_EQ(size_t{ 0u }, pool.size());
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(size_t{ 0u }, pool.freeThreads());
  EXPECT_FALSE(pool.addJob(5));
  EXPECT_EQ(0u, pool.cancelPendingJobs());

  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::lambda> pool2;
  EXPECT_EQ(size_t{ 0u }, pool2.size());
  EXPECT_EQ(size_t{ 0u }, pool2.busyThreads());
  EXPECT_EQ(size_t{ 0u }, pool2.freeThreads());
  EXPECT_FALSE(pool2.addJob(5));
  EXPECT_EQ(0u, pool2.cancelPendingJobs());
}

TEST_F(ThreadPoolTest, emptyPoolRunnerPerJob) {
  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> pool;
  EXPECT_EQ(size_t{ 0u }, pool.size());
  EXPECT_EQ(size_t{ 0u }, pool.busyThreads());
  EXPECT_EQ(size_t{ 0u }, pool.freeThreads());
  EXPECT_FALSE(pool.addJob(5, &_taskRunner));
  EXPECT_EQ(0u, pool.cancelPendingJobs());

  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::lambda> pool2;
  EXPECT_EQ(size_t{ 0u }, pool2.size());
  EXPECT_EQ(size_t{ 0u }, pool2.busyThreads());
  EXPECT_EQ(size_t{ 0u }, pool2.freeThreads());
  EXPECT_FALSE(pool2.addJob(5, [](int&) {}));
  EXPECT_EQ(0u, pool2.cancelPendingJobs());
}

TEST_F(ThreadPoolTest, movedPoolCommonRunner) {
  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool1;
  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool2(4u, &_taskRunner);
  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool3(5u, &_taskRunner);

  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> poolMoved1(std::move(pool1));
  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> poolMoved2(std::move(pool2));
  pool1 = std::move(pool3);
  EXPECT_EQ(0u, poolMoved1.size());
  EXPECT_EQ(4u, poolMoved2.size());
  EXPECT_EQ(5u, pool1.size());
}

TEST_F(ThreadPoolTest, movedPoolRunnerPerJob) {
  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> pool1;
  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> pool2(4u);
  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> pool3(5u);

  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> poolMoved1(std::move(pool1));
  ThreadPool<int, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> poolMoved2(std::move(pool2));
  pool1 = std::move(pool3);
  EXPECT_EQ(0u, poolMoved1.size());
  EXPECT_EQ(4u, poolMoved2.size());
  EXPECT_EQ(5u, pool1.size());
}


// -- thread pool with common runner --

TEST_F(ThreadPoolTest, commonRunner) {
  // base type
  int param1 = 2;
  _validateSingleFunctionThreadPool(4u, param1, param1);
  _validateSingleLambdaThreadPool(4u, param1, param1);
  // standard type
  FakeType param2{ 2 };
  _validateSingleFunctionThreadPool(5u, param2, param2.value);
  _validateSingleLambdaThreadPool(5u, param2, param2.value);
  // pointer type
  std::shared_ptr<int> param3 = std::make_shared<int>(3);
  _validateSingleFunctionThreadPool(3u, param3, *param3);
  _validateSingleLambdaThreadPool(3u, param3, *param3);
  // move-only type
  ThreadPool<std::unique_ptr<int>, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool(4u, &_taskRunner); 
  EXPECT_TRUE(pool.addJob(std::make_unique<int>(2)));
}


// -- thread pool with different runner for each job --

TEST_F(ThreadPoolTest, runnerPerJob) {
  // base type
  int param1 = 2;
  _validatePerJobFunctionThreadPool(4u, param1, param1);
  _validatePerJobLambdaThreadPool(4u, param1, param1);
  // standard type
  FakeType param2{ 2 };
  _validatePerJobFunctionThreadPool(5u, param2, param2.value);
  _validatePerJobLambdaThreadPool(5u, param2, param2.value);
  // pointer type
  std::shared_ptr<int> param3 = std::make_shared<int>(3);
  _validatePerJobFunctionThreadPool(3u, param3, *param3);
  _validatePerJobLambdaThreadPool(3u, param3, *param3);
  // move-only type
  ThreadPool<std::unique_ptr<int>, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> pool(4u);
  EXPECT_TRUE(pool.addJob(std::make_unique<int>(2), &_taskRunner));
}


// -- thread pool with different runner for each job and default runner --

TEST_F(ThreadPoolTest, runnerPerJobWithDefault) {
  // base type
  int param1 = 2;
  int param1Def = 1;
  _validatePerJobFunctionThreadPool(4u, param1, param1, true, param1Def, param1Def);
  _validatePerJobLambdaThreadPool(4u, param1, param1, true, param1Def, param1Def);
  // standard type
  FakeType param2{ 2 };
  FakeType param2Def{ 1 };
  _validatePerJobFunctionThreadPool(5u, param2, param2.value, true, param2Def, param2Def.value);
  _validatePerJobLambdaThreadPool(5u, param2, param2.value, true, param2Def, param2Def.value);
  // pointer type
  std::shared_ptr<int> param3 = std::make_shared<int>(3);
  std::shared_ptr<int> param3Def = std::make_shared<int>(1);
  _validatePerJobFunctionThreadPool(3u, param3, *param3, true, param3Def, *param3Def);
  _validatePerJobLambdaThreadPool(3u, param3, *param3, true, param3Def, *param3Def);
  // move-only type
  ThreadPool<std::unique_ptr<int>, ThreadRunnerMode::perJob, TaskRunnerType::functionPointer> pool(4u, &_taskRunner);
  EXPECT_TRUE(pool.addJob(std::make_unique<int>(2), &_taskRunner));
}

// -- exception management --
TEST_F(ThreadPoolTest, runnerWithException) {
  int startVal = totalValue;
  ThreadPool<int, ThreadRunnerMode::single, TaskRunnerType::functionPointer> pool(1u, &_exceptionRunner);

  EXPECT_TRUE(pool.addJob(2));
  _waitForJobsCompletion(pool);
  EXPECT_EQ(startVal + 2, totalValue);

  EXPECT_TRUE(pool.addJob(42));
  _waitForJobsCompletion(pool);
  EXPECT_EQ(startVal + 2, totalValue);
}
