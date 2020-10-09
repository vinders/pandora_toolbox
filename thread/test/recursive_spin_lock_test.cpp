#include <gtest/gtest.h>
#include <thread>
#include <mutex>
#include <future>
#include <thread/recursive_spin_lock.h>

using namespace pandora::thread;

class RecursiveSpinLockTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- lock operations --

TEST_F(RecursiveSpinLockTest, lockUnlockedInstance) {
  RecursiveSpinLock mutex;
  std::thread::id threadId = std::this_thread::get_id();
  EXPECT_EQ(std::thread::id(), mutex.owner());

  mutex.lock();
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());

  EXPECT_TRUE(mutex.tryLock());
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());
  EXPECT_TRUE(mutex.try_lock());
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());

  EXPECT_TRUE(mutex.tryLock(std::chrono::milliseconds(1)));
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());
  EXPECT_TRUE(mutex.try_lock_for(std::chrono::milliseconds(1)));
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());

  EXPECT_TRUE(mutex.tryLockUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());
  EXPECT_TRUE(mutex.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_EQ(threadId, mutex.owner());
  mutex.unlock();
  EXPECT_EQ(std::thread::id(), mutex.owner());
}

TEST_F(RecursiveSpinLockTest, lockGuard) {
  RecursiveSpinLock mutex;
  { // scope
    std::lock_guard<RecursiveSpinLock> lock(mutex);
    EXPECT_EQ(std::this_thread::get_id(), mutex.owner());
  } // end of scope

  EXPECT_EQ(std::thread::id(), mutex.owner());
}

TEST_F(RecursiveSpinLockTest, reLock) {
  RecursiveSpinLock mutex;
  std::thread::id threadId = std::this_thread::get_id();
  EXPECT_EQ(std::thread::id(), mutex.owner());
  EXPECT_FALSE(mutex.unlock());

  mutex.lock();
  EXPECT_EQ(threadId, mutex.owner());
  mutex.lock();
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(std::thread::id(), mutex.owner());

  mutex.lock();
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.tryLock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.try_lock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(std::thread::id(), mutex.owner());

  mutex.lock();
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.tryLock(std::chrono::milliseconds(1000LL)));
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.try_lock_for(std::chrono::milliseconds(1000LL)));
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(std::thread::id(), mutex.owner());

  auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000LL);
  mutex.lock();
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.tryLockUntil(timeout));
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.try_lock_until(timeout));
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(threadId, mutex.owner());
  EXPECT_TRUE(mutex.unlock());
  EXPECT_EQ(std::thread::id(), mutex.owner());
}

TEST_F(RecursiveSpinLockTest, tryLockThreadFailure) {
  RecursiveSpinLock mutex;
  std::thread::id firstThreadId = std::this_thread::get_id();
  {
    std::lock_guard<RecursiveSpinLock> lock(mutex);

    auto promise = std::async(std::launch::async, [&mutex, firstThreadId]() {
      EXPECT_FALSE(mutex.tryLock());
      EXPECT_FALSE(mutex.try_lock());
      EXPECT_EQ(firstThreadId, mutex.owner());

      EXPECT_FALSE(mutex.tryLock(std::chrono::milliseconds(1LL)));
      EXPECT_FALSE(mutex.try_lock_for(std::chrono::microseconds(1LL)));

      auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(16LL);
      EXPECT_FALSE(mutex.tryLockUntil(timeout));
      EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout - std::chrono::milliseconds(2LL));//core switch error margin

      timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(16LL);
      EXPECT_FALSE(mutex.try_lock_until(timeout));
      EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout - std::chrono::milliseconds(2LL));//core switch error margin
    });
    EXPECT_TRUE(promise.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  }
}


// -- concurrency test --

TEST_F(RecursiveSpinLockTest, multiThreadLock) {
  bool isAsyncReady = false;
  RecursiveSpinLock mutex;
  std::thread::id firstThreadId = std::this_thread::get_id();

  // lock waiting for other thread with timeout duration
  mutex.lock();
  mutex.lock(); // double lock
  EXPECT_EQ(firstThreadId, mutex.owner());
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

  mutex.unlock(); // first unlock -> one lock remaining
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
