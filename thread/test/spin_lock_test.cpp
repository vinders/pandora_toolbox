#include <gtest/gtest.h>
#include <thread>
#include <mutex>
#include <future>
#include <thread/spin_lock.h>

using namespace pandora::thread;

class SpinLockTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- lock operations --

TEST_F(SpinLockTest, lockUnlockedInstance) {
  SpinLock mutex;

  mutex.lock();
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();

  EXPECT_TRUE(mutex.tryLock());
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();
  EXPECT_TRUE(mutex.try_lock());
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();

  EXPECT_TRUE(mutex.tryLock(std::chrono::milliseconds(1)));
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();
  EXPECT_TRUE(mutex.try_lock_for(std::chrono::milliseconds(1)));
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();

  EXPECT_TRUE(mutex.tryLockUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();
  EXPECT_TRUE(mutex.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();
}

TEST_F(SpinLockTest, lockGuard) {
  SpinLock mutex; 
  { // scope
    std::lock_guard<SpinLock> lock(mutex);
    EXPECT_FALSE(mutex.tryLock());
  } // end of scope

  EXPECT_TRUE(mutex.tryLock());
}

TEST_F(SpinLockTest, tryLockFailure) {
  SpinLock mutex;
  std::lock_guard<SpinLock> lock(mutex);

  EXPECT_FALSE(mutex.tryLock());
  EXPECT_FALSE(mutex.try_lock());

  EXPECT_FALSE(mutex.tryLock(std::chrono::milliseconds(1)));
  EXPECT_FALSE(mutex.try_lock_for(std::chrono::microseconds(1)));

  auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(16LL);
  EXPECT_FALSE(mutex.tryLockUntil(timeout));
  EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout - std::chrono::milliseconds(2LL));//core switch error margin

  timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(16LL);
  EXPECT_FALSE(mutex.try_lock_until(timeout));
  EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout - std::chrono::milliseconds(2LL));//core switch error margin
}


// -- concurrency test --

TEST_F(SpinLockTest, multiThreadLock) {
  bool isAsyncReady = false;
  SpinLock mutex;

  // lock waiting for other thread with timeout duration
  mutex.lock();
  isAsyncReady = false;
  auto promise = std::async(std::launch::async, [&mutex, &isAsyncReady]() {
    bool isSuccess = mutex.tryLock();
    EXPECT_FALSE(isSuccess);
    isAsyncReady = true;

    if (!isSuccess) {
      EXPECT_TRUE(mutex.tryLock(std::chrono::milliseconds(2000LL)));
    }
    mutex.unlock();
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(2LL));
  mutex.unlock();
  promise.get();

  // lock waiting for other thread until timeout time-point
  EXPECT_TRUE(mutex.tryLock());
  isAsyncReady = false;
  auto promiseUntil = std::async(std::launch::async, [&mutex, &isAsyncReady]() {
    bool isSuccess = mutex.tryLock();
    EXPECT_FALSE(isSuccess);
    isAsyncReady = true;

    if (!isSuccess) {
      EXPECT_TRUE(mutex.tryLockUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(2000LL)));
    }
    mutex.unlock();
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(2LL));
  mutex.unlock();
  promiseUntil.get();

  // basic lock wait
  EXPECT_TRUE(mutex.tryLock());
  isAsyncReady = false;
  auto promiseBase = std::async(std::launch::async, [&mutex, &isAsyncReady]() {
    isAsyncReady = true;
    mutex.lock();
    mutex.unlock();
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  mutex.unlock();
  EXPECT_TRUE(promiseBase.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
}
