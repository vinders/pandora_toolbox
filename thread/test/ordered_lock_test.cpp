#include <gtest/gtest.h>
#include <thread>
#include <mutex>
#include <future>
#include <thread/ordered_lock.h>

using namespace pandora::thread;

class OrderedLockTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- lock operations --

TEST_F(OrderedLockTest, lockUnlockedInstance) {
  OrderedLock mutex;

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

  EXPECT_TRUE(mutex.tryLockUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(16)));
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();
  EXPECT_TRUE(mutex.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(16)));
  EXPECT_FALSE(mutex.tryLock());
  mutex.unlock();
}

TEST_F(OrderedLockTest, lockGuard) {
  OrderedLock mutex;
  { // scope
    std::lock_guard<OrderedLock> lock(mutex);
    EXPECT_FALSE(mutex.tryLock());
  } // end of scope

  EXPECT_TRUE(mutex.tryLock());
}

TEST_F(OrderedLockTest, tryLockFailure) {
  OrderedLock mutex;
  std::lock_guard<OrderedLock> lock(mutex);

  EXPECT_FALSE(mutex.tryLock());
  EXPECT_FALSE(mutex.try_lock());

  EXPECT_FALSE(mutex.tryLock(std::chrono::milliseconds(1)));
  EXPECT_FALSE(mutex.try_lock_for(std::chrono::microseconds(1)));

  auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(16);
  EXPECT_FALSE(mutex.tryLockUntil(timeout));
  EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout);

  timeout = std::chrono::steady_clock::now() + std::chrono::microseconds(16);
  EXPECT_FALSE(mutex.try_lock_until(timeout));
  EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout);
}


// -- concurrency test --

TEST_F(OrderedLockTest, multiThreadLock) {
  bool isAsyncReady = false;
  OrderedLock mutex;

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

// -- lock order --

TEST_F(OrderedLockTest, orderVerification) {
  bool isThread1Ready = false;
  bool isThread2Ready = false;
  bool isThread3Ready = false;
  std::vector<int> lockOrder;
  OrderedLock mutex;

  mutex.lock();

  auto promise1 = std::async(std::launch::async, [&mutex, &lockOrder , &isThread1Ready]() {
    isThread1Ready = true;

    bool isSuccess = mutex.tryLock(std::chrono::milliseconds(1000LL));
    lockOrder.push_back(1);
    EXPECT_TRUE(isSuccess);
    if (isSuccess)
      mutex.unlock();
  });

  auto promise2 = std::async(std::launch::async, [&mutex, &lockOrder, &isThread1Ready, &isThread2Ready]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isThread1Ready);
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    isThread2Ready = true;

    bool isSuccess = mutex.tryLock(std::chrono::milliseconds(1000LL));
    lockOrder.push_back(2);
    EXPECT_TRUE(isSuccess);
    if (isSuccess)
      mutex.unlock();
  });

  auto promise3 = std::async(std::launch::async, [&mutex, &lockOrder, &isThread2Ready, &isThread3Ready]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isThread2Ready);
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    isThread3Ready = true;

    bool isSuccess = mutex.tryLock(std::chrono::milliseconds(1000LL));
    lockOrder.push_back(3);
    EXPECT_TRUE(isSuccess);
    if (isSuccess)
      mutex.unlock();
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isThread1Ready || !isThread2Ready || !isThread3Ready);
  std::this_thread::sleep_for(std::chrono::milliseconds(2LL));
  mutex.unlock();

  EXPECT_TRUE(promise1.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promise2.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promise3.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(lockOrder.size() == 3 && lockOrder[0] == 1 && lockOrder[1] == 2 && lockOrder[2] == 3);
  EXPECT_TRUE(mutex.tryLock());
}

TEST_F(OrderedLockTest, orderVerificationReversed) {
  bool isThread1Ready = false;
  bool isThread2Ready = false;
  bool isThread3Ready = false;
  std::vector<int> lockOrder;
  OrderedLock mutex;

  mutex.lock();

  auto promise1 = std::async(std::launch::async, [&mutex, &lockOrder, &isThread1Ready, &isThread2Ready]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isThread2Ready);
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    isThread1Ready = true;

    bool isSuccess = mutex.tryLock(std::chrono::milliseconds(1000LL));
    lockOrder.push_back(1);
    EXPECT_TRUE(isSuccess);
    if (isSuccess)
      mutex.unlock();
  });

  auto promise2 = std::async(std::launch::async, [&mutex, &lockOrder, &isThread2Ready, &isThread3Ready]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isThread3Ready);
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    isThread2Ready = true;

    bool isSuccess = mutex.tryLock(std::chrono::milliseconds(1000LL));
    lockOrder.push_back(2);
    EXPECT_TRUE(isSuccess);
    if (isSuccess)
      mutex.unlock();
  });

  auto promise3 = std::async(std::launch::async, [&mutex, &lockOrder, &isThread3Ready]() {
    isThread3Ready = true;

    bool isSuccess = mutex.tryLock(std::chrono::milliseconds(1000LL));
    lockOrder.push_back(3);
    EXPECT_TRUE(isSuccess);
    if (isSuccess)
      mutex.unlock();
    });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isThread1Ready || !isThread2Ready);
  std::this_thread::sleep_for(std::chrono::milliseconds(2LL));
  mutex.unlock();

  EXPECT_TRUE(promise1.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promise2.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promise3.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(lockOrder.size() == 3 && lockOrder[0] == 3 && lockOrder[1] == 2 && lockOrder[2] == 1);
  EXPECT_TRUE(mutex.tryLock());
}
