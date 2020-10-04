#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <pattern/delegate.h>

using namespace pandora::pattern;

class DelegateTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- helpers --

static int g_handlerExecuted1 = 0;
int handlerCounter1(int val) { g_handlerExecuted1++; return val * 2; }
static int g_handlerExecuted2 = 0;
int handlerCounter2(int val) { g_handlerExecuted2++; return val * 3; }


// -- delegate class --

TEST_F(DelegateTest, accessors) {
  Delegate<std::mutex, DelegateMode::lambda, false, int, int> lambdaDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, false, int, int> pointerDelegate;
  Delegate<std::mutex, DelegateMode::lambda, true, int, int> lambdaDeferDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, true, int, int> pointerDeferDelegate;
  EXPECT_EQ(size_t{ 0 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 0 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 0 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 0 }, pointerDeferDelegate.size());
  EXPECT_TRUE(lambdaDelegate.empty());
  EXPECT_TRUE(pointerDelegate.empty());
  EXPECT_TRUE(lambdaDeferDelegate.empty());
  EXPECT_TRUE(pointerDeferDelegate.empty());

  auto contractLbd = lambdaDelegate.registerFunction([](int val) -> int { return val * 2; });
  auto contractPtr = pointerDelegate.registerFunction(&handlerCounter1);
  auto contractDefLbd = lambdaDeferDelegate.registerFunction([](int val) -> int { return val * 2; });
  auto contractDefPtr = pointerDeferDelegate.registerFunction(&handlerCounter1);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());
  EXPECT_FALSE(lambdaDelegate.empty());
  EXPECT_FALSE(pointerDelegate.empty());
  EXPECT_FALSE(lambdaDeferDelegate.empty());
  EXPECT_FALSE(pointerDeferDelegate.empty());

  Delegate<std::mutex, DelegateMode::lambda, false, int, int> lambdaDelegateMoved(std::move(lambdaDelegate));
  Delegate<std::mutex, DelegateMode::functionPointer, false, int, int> pointerDelegateMoved(std::move(pointerDelegate));
  Delegate<std::mutex, DelegateMode::lambda, true, int, int> lambdaDeferDelegateMoved(std::move(lambdaDeferDelegate));
  Delegate<std::mutex, DelegateMode::functionPointer, true, int, int> pointerDeferDelegateMoved(std::move(pointerDeferDelegate));
  EXPECT_EQ(size_t{ 1 }, lambdaDelegateMoved.size());
  EXPECT_EQ(size_t{ 1 }, pointerDelegateMoved.size());
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegateMoved.size());
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegateMoved.size());
  EXPECT_FALSE(lambdaDelegateMoved.empty());
  EXPECT_FALSE(pointerDelegateMoved.empty());
  EXPECT_FALSE(lambdaDeferDelegateMoved.empty());
  EXPECT_FALSE(pointerDeferDelegateMoved.empty());

  lambdaDelegate = std::move(lambdaDelegateMoved);
  pointerDelegate = std::move(pointerDelegateMoved);
  lambdaDeferDelegate = std::move(lambdaDeferDelegateMoved);
  pointerDeferDelegate = std::move(pointerDeferDelegateMoved);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());
  EXPECT_FALSE(lambdaDelegate.empty());
  EXPECT_FALSE(pointerDelegate.empty());
  EXPECT_FALSE(lambdaDeferDelegate.empty());
  EXPECT_FALSE(pointerDeferDelegate.empty());

  contractLbd.reset();
  EXPECT_EQ(size_t{ 0 }, lambdaDelegate.size());
  contractPtr.reset();
  EXPECT_EQ(size_t{ 0 }, pointerDelegate.size());
  contractDefLbd.reset();
  EXPECT_EQ(size_t{ 0 }, lambdaDeferDelegate.size());
  contractDefPtr.reset();
  EXPECT_EQ(size_t{ 0 }, pointerDeferDelegate.size());
}

TEST_F(DelegateTest, registerUnregister) {
  Delegate<std::mutex, DelegateMode::lambda, false, int, int> lambdaDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, false, int, int> pointerDelegate;
  Delegate<std::mutex, DelegateMode::lambda, true, int, int> lambdaDeferDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, true, int, int> pointerDeferDelegate;
  EXPECT_TRUE(lambdaDelegate.empty());
  EXPECT_TRUE(pointerDelegate.empty());
  EXPECT_TRUE(lambdaDeferDelegate.empty());
  EXPECT_TRUE(pointerDeferDelegate.empty());

  auto contractLbd = lambdaDelegate.registerFunction([](int val) -> int { return val * 2; });
  auto contractPtr = pointerDelegate.registerFunction(&handlerCounter1);
  auto contractDefLbd = lambdaDeferDelegate.registerFunction([](int val) -> int { return val * 2; });
  auto contractDefPtr = pointerDeferDelegate.registerFunction(&handlerCounter1);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());

  auto contractLbd2 = lambdaDelegate += [](int val) -> int { return val * 2; };
  auto contractPtr2 = pointerDelegate += &handlerCounter2;
  auto contractDefLbd2 = lambdaDeferDelegate += [](int val) -> int { return val * 2; };
  auto contractDefPtr2 = pointerDeferDelegate += &handlerCounter2;
  EXPECT_EQ(size_t{ 2 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 2 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDeferDelegate.size());

  lambdaDelegate.unregister(*contractLbd);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  pointerDelegate.unregister(*contractPtr);
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  lambdaDeferDelegate.unregister(*contractDefLbd);
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  pointerDeferDelegate.unregister(*contractDefPtr);
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());

  lambdaDelegate.unregister(*contractLbd); // already revoked
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());

  contractLbd = lambdaDelegate.registerFunction([](int val) -> int { return val * 2; });
  contractPtr = pointerDelegate.registerFunction(&handlerCounter1);
  contractDefLbd = lambdaDeferDelegate.registerFunction([](int val) -> int { return val * 2; });
  contractDefPtr = pointerDeferDelegate.registerFunction(&handlerCounter1);
  EXPECT_EQ(size_t{ 2 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 2 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDeferDelegate.size());

  lambdaDelegate -= *contractLbd2;
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  pointerDelegate -= *contractPtr2;
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  lambdaDeferDelegate -= *contractDefLbd2;
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  pointerDeferDelegate -= *contractDefPtr2;
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());

  contractLbd.reset();
  EXPECT_EQ(size_t{ 0 }, lambdaDelegate.size());
  contractPtr.reset();
  EXPECT_EQ(size_t{ 0 }, pointerDelegate.size());
  contractDefLbd.reset();
  EXPECT_EQ(size_t{ 0 }, lambdaDeferDelegate.size());
  contractDefPtr.reset();
  EXPECT_EQ(size_t{ 0 }, pointerDeferDelegate.size());
}

TEST_F(DelegateTest, registerClear) {
  Delegate<std::mutex, DelegateMode::lambda, false, int, int> lambdaDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, false, int, int> pointerDelegate;
  Delegate<std::mutex, DelegateMode::lambda, true, int, int> lambdaDeferDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, true, int, int> pointerDeferDelegate;
  EXPECT_TRUE(lambdaDelegate.empty());
  EXPECT_TRUE(pointerDelegate.empty());
  EXPECT_TRUE(lambdaDeferDelegate.empty());
  EXPECT_TRUE(pointerDeferDelegate.empty());

  auto contractLbd = lambdaDelegate.registerFunction([](int val) -> int { return val * 2; });
  auto contractPtr = pointerDelegate.registerFunction(&handlerCounter1);
  auto contractDefLbd = lambdaDeferDelegate.registerFunction([](int val) -> int { return val * 2; });
  auto contractDefPtr = pointerDeferDelegate.registerFunction(&handlerCounter1);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());

  auto contractLbd2 = lambdaDelegate += [](int val) -> int { return val * 2; };
  auto contractPtr2 = pointerDelegate += &handlerCounter2;
  auto contractDefLbd2 = lambdaDeferDelegate += [](int val) -> int { return val * 2; };
  auto contractDefPtr2 = pointerDeferDelegate += &handlerCounter2;
  EXPECT_EQ(size_t{ 2 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 2 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDeferDelegate.size());

  lambdaDelegate.clear();
  EXPECT_EQ(size_t{ 0 }, lambdaDelegate.size());
  pointerDelegate.clear();
  EXPECT_EQ(size_t{ 0 }, pointerDelegate.size());
  lambdaDeferDelegate.clear();
  EXPECT_EQ(size_t{ 0 }, lambdaDeferDelegate.size());
  pointerDeferDelegate.clear();
  EXPECT_EQ(size_t{ 0 }, pointerDeferDelegate.size());

  lambdaDelegate.unregister(*contractLbd); // already revoked
  EXPECT_EQ(size_t{ 0 }, lambdaDelegate.size());
}

TEST_F(DelegateTest, registerRun) {
  Delegate<std::mutex, DelegateMode::lambda, false, int, int> lambdaDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, false, int, int> pointerDelegate;
  Delegate<std::mutex, DelegateMode::lambda, true, int, int> lambdaDeferDelegate;
  Delegate<std::mutex, DelegateMode::functionPointer, true, int, int> pointerDeferDelegate;
  EXPECT_TRUE(lambdaDelegate.empty());
  EXPECT_TRUE(pointerDelegate.empty());
  EXPECT_TRUE(lambdaDeferDelegate.empty());
  EXPECT_TRUE(pointerDeferDelegate.empty());

  int contractLbdExecuted = 0;
  int contractDefLbdExecuted = 0;
  g_handlerExecuted1 = 0;
  auto contractLbd = lambdaDelegate.registerFunction([&contractLbdExecuted](int val) -> int { contractLbdExecuted++; return val * 2; });
  auto contractPtr = pointerDelegate.registerFunction(&handlerCounter1);
  auto contractDefLbd = lambdaDeferDelegate.registerFunction([&contractDefLbdExecuted](int val) -> int { contractDefLbdExecuted++;  return val * 2; });
  auto contractDefPtr = pointerDeferDelegate.registerFunction(&handlerCounter1);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());

  int contractLbdExecuted2 = 0;
  int contractDefLbdExecuted2 = 0;
  g_handlerExecuted2 = 0;
  auto contractLbd2 = lambdaDelegate += [&contractLbdExecuted2](int val) -> int { contractLbdExecuted2++;  return val * 3; };
  auto contractPtr2 = pointerDelegate += &handlerCounter2;
  auto contractDefLbd2 = lambdaDeferDelegate += [&contractDefLbdExecuted2](int val) -> int { contractDefLbdExecuted2++;  return val * 3; };
  auto contractDefPtr2 = pointerDeferDelegate += &handlerCounter2;
  EXPECT_EQ(size_t{ 2 }, lambdaDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDelegate.size());
  EXPECT_EQ(size_t{ 2 }, lambdaDeferDelegate.size());
  EXPECT_EQ(size_t{ 2 }, pointerDeferDelegate.size());

  lambdaDelegate.run(42);
  pointerDelegate.run(42);
  lambdaDeferDelegate.run(42);
  pointerDeferDelegate.run(42);
  EXPECT_EQ(1, contractLbdExecuted);
  EXPECT_EQ(1, contractDefLbdExecuted);
  EXPECT_EQ(2, g_handlerExecuted1);
  EXPECT_EQ(1, contractLbdExecuted2);
  EXPECT_EQ(1, contractDefLbdExecuted2);
  EXPECT_EQ(2, g_handlerExecuted2);

  lambdaDelegate(42);
  pointerDelegate(42);
  lambdaDeferDelegate(42);
  pointerDeferDelegate(42);
  EXPECT_EQ(2, contractLbdExecuted);
  EXPECT_EQ(2, contractDefLbdExecuted);
  EXPECT_EQ(4, g_handlerExecuted1);
  EXPECT_EQ(2, contractLbdExecuted2);
  EXPECT_EQ(2, contractDefLbdExecuted2);
  EXPECT_EQ(4, g_handlerExecuted2);

  std::vector<int> resLbd = lambdaDelegate.runResults(42);
  std::vector<int> resPtr = pointerDelegate.runResults(42);
  std::vector<int> resDefLbd = lambdaDeferDelegate.runResults(42);
  std::vector<int> resDefPtr = pointerDeferDelegate.runResults(42);
  EXPECT_EQ(3, contractLbdExecuted);
  EXPECT_EQ(3, contractDefLbdExecuted);
  EXPECT_EQ(6, g_handlerExecuted1);
  EXPECT_EQ(3, contractLbdExecuted2);
  EXPECT_EQ(3, contractDefLbdExecuted2);
  EXPECT_EQ(6, g_handlerExecuted2);
  ASSERT_EQ(size_t{ 2u }, resLbd.size());
  ASSERT_EQ(size_t{ 2u }, resPtr.size());
  ASSERT_EQ(size_t{ 2u }, resDefLbd.size());
  ASSERT_EQ(size_t{ 2u }, resDefPtr.size());
  int firstResult = (resLbd[0] == 84) ? 84 : 126;
  int lastResult = (resLbd[0] == 84) ? 126 : 84;
  EXPECT_EQ(firstResult, resLbd[0]);
  EXPECT_EQ(firstResult, resPtr[0]);
  EXPECT_EQ(firstResult, resDefLbd[0]);
  EXPECT_EQ(firstResult, resDefPtr[0]);
  EXPECT_EQ(lastResult, resLbd[1]);
  EXPECT_EQ(lastResult, resPtr[1]);
  EXPECT_EQ(lastResult, resDefLbd[1]);
  EXPECT_EQ(lastResult, resDefPtr[1]);

  lambdaDelegate.unregister(*contractLbd);
  EXPECT_EQ(size_t{ 1 }, lambdaDelegate.size());
  pointerDelegate.unregister(*contractPtr);
  EXPECT_EQ(size_t{ 1 }, pointerDelegate.size());
  lambdaDeferDelegate.unregister(*contractDefLbd);
  EXPECT_EQ(size_t{ 1 }, lambdaDeferDelegate.size());
  pointerDeferDelegate.unregister(*contractDefPtr);
  EXPECT_EQ(size_t{ 1 }, pointerDeferDelegate.size());

  resLbd = lambdaDelegate.runResults(42);
  resPtr = pointerDelegate.runResults(42);
  resDefLbd = lambdaDeferDelegate.runResults(42);
  resDefPtr = pointerDeferDelegate.runResults(42);
  EXPECT_EQ(3, contractLbdExecuted);
  EXPECT_EQ(3, contractDefLbdExecuted);
  EXPECT_EQ(6, g_handlerExecuted1);
  EXPECT_EQ(4, contractLbdExecuted2);
  EXPECT_EQ(4, contractDefLbdExecuted2);
  EXPECT_EQ(8, g_handlerExecuted2);
  ASSERT_EQ(size_t{ 1u }, resLbd.size());
  ASSERT_EQ(size_t{ 1u }, resPtr.size());
  ASSERT_EQ(size_t{ 1u }, resDefLbd.size());
  ASSERT_EQ(size_t{ 1u }, resDefPtr.size());
  EXPECT_EQ(126, resLbd[0]);
  EXPECT_EQ(126, resPtr[0]);
  EXPECT_EQ(126, resDefLbd[0]);
  EXPECT_EQ(126, resDefPtr[0]);
}

// -- concurrency tests --

TEST_F(DelegateTest, multiThreadDirect) {
  bool isUnlocked = false;
  bool isAsyncReady = false;
  bool isRunComplete = false;
  Delegate<std::mutex, DelegateMode::lambda, false, int, int> lbdDelegate;
  auto contract = lbdDelegate.registerFunction([&isUnlocked, &isAsyncReady, &isRunComplete](int val) -> int { 
    isAsyncReady = true; 
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isUnlocked);
    isRunComplete = true;
    return val; 
  });
  EXPECT_EQ(size_t{ 1 }, lbdDelegate.size());

  auto promiseRun = std::async(std::launch::async, [&lbdDelegate]() {
    lbdDelegate(42);
  });
  auto promiseAdd = std::async(std::launch::async, [&isAsyncReady, &lbdDelegate, &isRunComplete]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isAsyncReady);

    auto contractAdded = lbdDelegate += [](int val) { return val; };
    EXPECT_TRUE(isRunComplete);
  });

  auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(2000LL);
  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady && std::chrono::high_resolution_clock::now() < timeout);
  EXPECT_TRUE(isAsyncReady);
  isAsyncReady = true;
  std::this_thread::sleep_for(std::chrono::milliseconds(2LL));

  isUnlocked = true;

  EXPECT_TRUE(promiseRun.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promiseAdd.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
}

TEST_F(DelegateTest, multiThreadDeferred) {
  bool isUnlocked = false;
  bool isAsyncReady = false;
  bool isRunComplete = false;
  Delegate<std::mutex, DelegateMode::lambda, true, int, int> lbdDelegate;
  auto contract = lbdDelegate.registerFunction([&isUnlocked, &isAsyncReady, &isRunComplete](int val) -> int {
    isAsyncReady = true;
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isUnlocked);
    isRunComplete = true;
    return val;
  });
  EXPECT_EQ(size_t{ 1 }, lbdDelegate.size());

  auto promiseRun = std::async(std::launch::async, [&lbdDelegate]() {
    lbdDelegate(42);
  });
  auto promiseAdd = std::async(std::launch::async, [&isAsyncReady, &lbdDelegate, &isUnlocked, &isRunComplete, &contract]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isAsyncReady);

    auto contractAdded = lbdDelegate += [](int val) { return val * 2; }; // new contract -> pending
    contract->revoke(); // previous contract removal -> pending
    isUnlocked = true;
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isRunComplete);

    std::vector<int> results = lbdDelegate.runResults(42);
    ASSERT_EQ(size_t{ 1 }, results.size());
    EXPECT_EQ(84, results[0]);
  });

  auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(2000LL);
  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady && std::chrono::high_resolution_clock::now() < timeout);
  EXPECT_TRUE(isAsyncReady);
  isAsyncReady = true;
  EXPECT_TRUE(promiseRun.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);

  timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1000LL);
  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isUnlocked && std::chrono::high_resolution_clock::now() < timeout);
  EXPECT_TRUE(isUnlocked);
  isUnlocked = true;
  EXPECT_TRUE(promiseAdd.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
}
