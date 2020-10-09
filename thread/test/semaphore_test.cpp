#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <future>
#include <thread/semaphore.h>

using namespace pandora::thread;

class SemaphoreTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- semaphore operations --

TEST_F(SemaphoreTest, waitNotifyOne) {
  Semaphore sema;

  EXPECT_FALSE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait(1u));
  EXPECT_FALSE(sema.tryWait(2u));

  sema.notify();
  ASSERT_TRUE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait());
  sema.notify();
  sema.wait();
  EXPECT_FALSE(sema.tryWait());

  sema.notify();
  ASSERT_TRUE(sema.tryWait(1u));
  EXPECT_FALSE(sema.tryWait());
  sema.notify();
  sema.wait(1u);
  EXPECT_FALSE(sema.tryWait());

  sema.notify(1u);
  ASSERT_TRUE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait());
  sema.notify(1u);
  sema.wait();
  EXPECT_FALSE(sema.tryWait());
}

TEST_F(SemaphoreTest, waitNotifyCount) {
  Semaphore sema;

  sema.notify(2u);
  EXPECT_TRUE(sema.tryWait());
  ASSERT_TRUE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  sema.wait();
  EXPECT_TRUE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  sema.wait();
  sema.wait();
  EXPECT_FALSE(sema.tryWait());

  sema.notify(2u);
  ASSERT_TRUE(sema.tryWait(2u));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  sema.wait(2u);
  EXPECT_FALSE(sema.tryWait());

  sema.notify(2u);
  EXPECT_FALSE(sema.tryWait(3u));
  EXPECT_TRUE(sema.tryWait(2u));
  EXPECT_FALSE(sema.tryWait());

  sema.notify(3u);
  ASSERT_TRUE(sema.tryWait(3u));
  EXPECT_FALSE(sema.tryWait(2u));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(3u);
  sema.wait(3u);
  EXPECT_FALSE(sema.tryWait(2u));
  EXPECT_FALSE(sema.tryWait());

  sema.notify(3u);
  EXPECT_TRUE(sema.tryWait(2u));
  ASSERT_TRUE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait());
  sema.notify(3u);
  sema.wait(2u);
  sema.wait();
  EXPECT_FALSE(sema.tryWait());

  sema.notify(3u);
  EXPECT_TRUE(sema.tryWait(1u));
  ASSERT_TRUE(sema.tryWait(2u));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(3u);
  sema.wait(1u);
  sema.wait(2u);
  EXPECT_FALSE(sema.tryWait());
}

TEST_F(SemaphoreTest, initCtorWait) {
  Semaphore sema1(1u);
  EXPECT_FALSE(sema1.tryWait(2u));
  EXPECT_TRUE(sema1.tryWait());
  EXPECT_FALSE(sema1.tryWait());

  Semaphore sema2(2u);
  EXPECT_TRUE(sema2.tryWait());
  EXPECT_TRUE(sema2.tryWait());
  EXPECT_FALSE(sema2.tryWait());
  Semaphore sema3(2u);
  EXPECT_TRUE(sema3.tryWait(2u));
  EXPECT_FALSE(sema3.tryWait());
}

TEST_F(SemaphoreTest, reset) {
  Semaphore sema;
  EXPECT_EQ(0u, sema.reset());
  EXPECT_FALSE(sema.tryWait());

  Semaphore sema1(1u);
  EXPECT_EQ(1u, sema1.reset());
  EXPECT_FALSE(sema1.tryWait());
  EXPECT_EQ(0u, sema1.reset());

  Semaphore sema2(2u);
  EXPECT_TRUE(sema2.tryWait());
  EXPECT_EQ(1u, sema2.reset());
  EXPECT_FALSE(sema2.tryWait());
  EXPECT_EQ(0u, sema2.reset());
  Semaphore sema3(2u);
  EXPECT_EQ(2u, sema3.reset());
  EXPECT_FALSE(sema3.tryWait());
  EXPECT_EQ(0u, sema3.reset());
}

TEST_F(SemaphoreTest, waitForDuration) {
  Semaphore sema;

  sema.notify();
  EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify();
  EXPECT_TRUE(sema.try_wait_for(std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify();
  EXPECT_TRUE(sema.tryWait(1u, std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  EXPECT_TRUE(sema.tryWait(2u, std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  EXPECT_TRUE(sema.tryWait(1u, std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWait(1u, std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());

  EXPECT_FALSE(sema.tryWait(std::chrono::milliseconds(1)));

  sema.notify(2u);
  EXPECT_FALSE(sema.tryWait(3u, std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWait(1u, std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWait());
}

TEST_F(SemaphoreTest, waitUntilTimeout) {
  Semaphore sema;

  sema.notify();
  EXPECT_TRUE(sema.tryWaitUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify();
  EXPECT_TRUE(sema.try_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify();
  EXPECT_TRUE(sema.tryWaitUntil(1u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  EXPECT_TRUE(sema.tryWaitUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWaitUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  EXPECT_TRUE(sema.tryWaitUntil(2u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());
  sema.notify(2u);
  EXPECT_TRUE(sema.tryWaitUntil(1u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWaitUntil(1u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_FALSE(sema.tryWait());

  auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(16LL);
  EXPECT_FALSE(sema.tryWaitUntil(timeout));
  EXPECT_TRUE(std::chrono::steady_clock::now() >= timeout - std::chrono::milliseconds(2LL));//core switch error margin

  sema.notify(2u);
  EXPECT_FALSE(sema.tryWaitUntil(3u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWaitUntil(1u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)));
  EXPECT_TRUE(sema.tryWait());
}


// -- concurrency tests --

TEST_F(SemaphoreTest, multiThreadWaitOne) {
  bool isAsyncReady = false;
  bool isAsyncSuccess = false;
  Semaphore sema;

  // other thread waiting first
  isAsyncReady = false;
  isAsyncSuccess = false;
  auto promiseWait = std::async(std::launch::async, [&sema, &isAsyncReady, &isAsyncSuccess]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady = true;
    sema.wait();
    isAsyncSuccess = true;
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema.notify();
  ASSERT_TRUE(promiseWait.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(isAsyncSuccess);

  // other thread waiting first for a duration
  isAsyncReady = false;
  isAsyncSuccess = false;
  auto promiseWaitFor = std::async(std::launch::async, [&sema, &isAsyncReady, &isAsyncSuccess]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady = true;
    EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(1000LL)));
    isAsyncSuccess = true;
    EXPECT_FALSE(sema.tryWait());
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema.notify();
  EXPECT_TRUE(promiseWaitFor.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(isAsyncSuccess);

  // other thread waiting first until timeout
  isAsyncReady = false;
  isAsyncSuccess = false;
  auto promiseWaitUntil = std::async(std::launch::async, [&sema, &isAsyncReady, &isAsyncSuccess]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady = true;
    EXPECT_TRUE(sema.tryWaitUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000LL)));
    isAsyncSuccess = true;
    EXPECT_FALSE(sema.tryWait());
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema.notify();
  EXPECT_TRUE(promiseWaitUntil.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(isAsyncSuccess);
}

TEST_F(SemaphoreTest, multiThreadWaitCount) {
  bool isAsyncReady = false;
  bool isAsyncSuccess = false;
  Semaphore sema;

  // other thread waiting first
  isAsyncReady = false;
  isAsyncSuccess = false;
  auto promiseWait = std::async(std::launch::async, [&sema, &isAsyncReady, &isAsyncSuccess]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady = true;
    sema.wait(2u);
    isAsyncSuccess = true;
    EXPECT_TRUE(sema.tryWait());
    EXPECT_FALSE(sema.tryWait());
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema.notify(3u);
  ASSERT_TRUE(promiseWait.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(isAsyncSuccess);

  // other thread waiting first for a duration
  isAsyncReady = false;
  isAsyncSuccess = false;
  auto promiseWaitFor = std::async(std::launch::async, [&sema, &isAsyncReady, &isAsyncSuccess]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady = true;
    EXPECT_TRUE(sema.tryWait(2u, std::chrono::milliseconds(1000LL)));
    isAsyncSuccess = true;
    EXPECT_TRUE(sema.tryWait());
    EXPECT_FALSE(sema.tryWait());
    });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema.notify(3u);
  EXPECT_TRUE(promiseWaitFor.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(isAsyncSuccess);

  // other thread waiting first until timeout
  isAsyncReady = false;
  isAsyncSuccess = false;
  auto promiseWaitUntil = std::async(std::launch::async, [&sema, &isAsyncReady, &isAsyncSuccess]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady = true;
    EXPECT_TRUE(sema.tryWaitUntil(2u, std::chrono::steady_clock::now() + std::chrono::milliseconds(1000LL)));
    isAsyncSuccess = true;
    EXPECT_TRUE(sema.tryWait());
    EXPECT_FALSE(sema.tryWait());
    });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema.notify(3u);
  EXPECT_TRUE(promiseWaitUntil.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(isAsyncSuccess);
}

TEST_F(SemaphoreTest, crossedWait) {
  bool isAsyncReady = false;
  Semaphore sema1; 
  Semaphore sema2;

  isAsyncReady = false;
  auto promiseWait = std::async(std::launch::async, [&sema1, &sema2, &isAsyncReady]() {
    EXPECT_FALSE(sema1.tryWait());
    isAsyncReady = true;
    EXPECT_TRUE(sema1.tryWaitUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000LL)));
    EXPECT_FALSE(sema1.tryWait());

    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    sema2.notify();
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  sema1.notify();
  EXPECT_TRUE(sema2.tryWaitUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000LL)));
  EXPECT_FALSE(sema2.tryWait());

  EXPECT_TRUE(promiseWait.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
}

TEST_F(SemaphoreTest, notifyAll) {
  Semaphore sema;

  bool isAsyncReady1 = false;
  auto promiseWaitFor = std::async(std::launch::async, [&sema, &isAsyncReady1]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady1 = true;
    EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(2000LL)));
  });
  bool isAsyncReady2 = false;
  auto promiseWaitUntil = std::async(std::launch::async, [&sema, &isAsyncReady2]() {
    EXPECT_FALSE(sema.tryWait());
    isAsyncReady2 = true;
    EXPECT_TRUE(sema.tryWaitUntil(2u, std::chrono::steady_clock::now() + std::chrono::milliseconds(2000LL)));
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady1 || !isAsyncReady2);
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));

  sema.notifyAll();
  EXPECT_TRUE(promiseWaitFor.wait_for(std::chrono::milliseconds(2500LL)) != std::future_status::timeout);
  EXPECT_TRUE(promiseWaitUntil.wait_for(std::chrono::milliseconds(2500LL)) != std::future_status::timeout);
}

TEST_F(SemaphoreTest, destroyWhileThreadWaiting) {
  std::unique_ptr<Semaphore> sema = std::make_unique<Semaphore>();

  bool isAsyncReady = false;
  auto promiseWaitFor = std::async(std::launch::async, [&sema, &isAsyncReady]() {
    EXPECT_FALSE(sema->tryWait());
    isAsyncReady = true;
    EXPECT_TRUE(sema->tryWait(std::chrono::milliseconds(2000LL)));
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  std::this_thread::sleep_for(std::chrono::milliseconds(2LL));

  sema = nullptr;
  EXPECT_TRUE(promiseWaitFor.wait_for(std::chrono::milliseconds(2500LL)) != std::future_status::timeout);
}

TEST_F(SemaphoreTest, orderVerification) {
  bool isThread1Ready = false;
  bool isThread2Ready = false;
  bool isThread3Ready = false;
  Semaphore sema;

  auto promise1 = std::async(std::launch::async, [&sema, &isThread1Ready]() {
    isThread1Ready = true;

    EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(2000LL)));
    sema.notify();
  });

  auto promise2 = std::async(std::launch::async, [&sema, &isThread1Ready, &isThread2Ready]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isThread1Ready);
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    isThread2Ready = true;

    EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(2000LL)));
    sema.notify();
  });

  auto promise3 = std::async(std::launch::async, [&sema, &isThread2Ready, &isThread3Ready]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isThread2Ready);
    std::this_thread::sleep_for(std::chrono::milliseconds(2LL));
    isThread3Ready = true;

    EXPECT_TRUE(sema.tryWait(std::chrono::milliseconds(2000LL)));
    sema.notify();
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isThread1Ready || !isThread2Ready || !isThread3Ready);
  std::this_thread::sleep_for(std::chrono::milliseconds(5LL));

  sema.notify();

  EXPECT_TRUE(promise1.wait_for(std::chrono::milliseconds(8000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promise2.wait_for(std::chrono::milliseconds(8000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promise3.wait_for(std::chrono::milliseconds(8000LL)) != std::future_status::timeout);
  EXPECT_TRUE(sema.tryWait());
  EXPECT_FALSE(sema.tryWait());

  sema.notifyAll();
  std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
}
