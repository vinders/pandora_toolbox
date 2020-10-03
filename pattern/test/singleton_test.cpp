#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <pattern/singleton.h>

using namespace pandora::pattern;

class SingletonTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};

struct GlobalObjectMock {
  int value = 0;
  std::chrono::steady_clock::time_point time{ std::chrono::nanoseconds(0LL) };
};


// -- singleton pattern --

TEST_F(SingletonTest, baseAccess) {
  Singleton<int>::instance() = 3;
  EXPECT_EQ(3, Singleton<int>::instance());

  EXPECT_EQ(0, Singleton<GlobalObjectMock>::instance().value);
  Singleton<GlobalObjectMock>::instance().value = 5;
  EXPECT_EQ(5, Singleton<GlobalObjectMock>::instance().value);
  EXPECT_EQ(5, Singleton<GlobalObjectMock>::instance().value); // value remains
}

TEST_F(SingletonTest, multiThread) {
  bool isAsyncReady = false;
  Singleton<int>::instance() = 3;
  EXPECT_EQ(3, Singleton<int>::instance());
  Singleton<GlobalObjectMock>::instance().value = 5;
  EXPECT_EQ(5, Singleton<GlobalObjectMock>::instance().value);

  auto promise = std::async(std::launch::async, [&isAsyncReady]() {
    EXPECT_EQ(3, Singleton<int>::instance());
    Singleton<int>::instance() = 32;
    EXPECT_EQ(32, Singleton<int>::instance());

    EXPECT_EQ(5, Singleton<GlobalObjectMock>::instance().value);
    Singleton<GlobalObjectMock>::instance().value = 52;
    EXPECT_EQ(52, Singleton<GlobalObjectMock>::instance().value);

    isAsyncReady = true;
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  promise.get();
  EXPECT_EQ(32, Singleton<int>::instance());
  EXPECT_EQ(52, Singleton<GlobalObjectMock>::instance().value);
}


// -- locked singleton pattern --

TEST_F(SingletonTest, lockedBaseAccess) {
  LockedSingleton<int>::instance().value() = 3;
  EXPECT_EQ(3, LockedSingleton<int>::instance().value());

  EXPECT_EQ(0, LockedSingleton<GlobalObjectMock>::instance().value().value);
  LockedSingleton<GlobalObjectMock>::instance().value().value = 5;
  EXPECT_EQ(5, LockedSingleton<GlobalObjectMock>::instance().value().value);
  EXPECT_EQ(5, LockedSingleton<GlobalObjectMock>::instance().value().value); // value remains
}

TEST_F(SingletonTest, lockedMultiThread) {
  bool isAsyncReady = false;
  LockedSingleton<int>::instance().value() = 3;
  EXPECT_EQ(3, LockedSingleton<int>::instance().value());
  LockedSingleton<GlobalObjectMock>::instance().value().value = 5;
  EXPECT_EQ(5, LockedSingleton<GlobalObjectMock>::instance().value().value);

  auto promise = std::async(std::launch::async, [&isAsyncReady]() {
    EXPECT_EQ(3, LockedSingleton<int>::instance().value());
    LockedSingleton<int>::instance().value() = 32;
    EXPECT_EQ(32, LockedSingleton<int>::instance().value());

    EXPECT_EQ(5, LockedSingleton<GlobalObjectMock>::instance().value().value);
    LockedSingleton<GlobalObjectMock>::instance().value().value = 52;
    EXPECT_EQ(52, LockedSingleton<GlobalObjectMock>::instance().value().value);

    isAsyncReady = true;
    });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  promise.get();
  EXPECT_EQ(32, LockedSingleton<int>::instance().value());
  EXPECT_EQ(52, LockedSingleton<GlobalObjectMock>::instance().value().value);
}

TEST_F(SingletonTest, concurrentAccess) {
  bool isAsyncReady = false;

  auto promise = std::async(std::launch::async, [&isAsyncReady]() {
    auto inst = LockedSingleton<GlobalObjectMock>::instance();
    isAsyncReady = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    inst.value().time = std::chrono::steady_clock::now();
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (std::chrono::steady_clock::now() == inst.value().time);
  });

  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady);
  auto inst = LockedSingleton<GlobalObjectMock>::instance();
  auto time = std::chrono::steady_clock::now();
  EXPECT_TRUE(time > inst.value().time);
  promise.get();
}
