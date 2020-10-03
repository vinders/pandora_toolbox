#include <gtest/gtest.h>
#include <mutex>
#include <future>
#include <pattern/locked.h>

using namespace pandora::pattern;

class LockedTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};

struct ObjectMock {
  int x = 0;
  int y = 0;
  ObjectMock() = default;
  ObjectMock(int x) : x(x), y(x) {}
  ObjectMock(int x, int y) : x(x), y(y) {}
  bool operator==(const ObjectMock& rhs) const noexcept { return (x == rhs.x && y == rhs.y); }
  bool operator!=(const ObjectMock& rhs) const noexcept { return !(this->operator==(rhs)); }
};


// -- locked shared object --

TEST_F(LockedTest, lockedAccessors) {
  auto dummy = std::make_shared<ObjectMock>(5, 6);
  std::mutex lock;

  Locked<ObjectMock, std::mutex> locked(dummy, lock);
  EXPECT_TRUE(locked.value().x == 5);
  EXPECT_TRUE(locked.value().y == 6);
  EXPECT_TRUE(*locked == ObjectMock(5, 6));
  EXPECT_TRUE(locked->x == 5);
  EXPECT_TRUE(locked->y == 6);

  const Locked<ObjectMock, std::mutex>& lockedConst = locked;
  EXPECT_TRUE(lockedConst.value().x == 5);
  EXPECT_TRUE(lockedConst.value().y == 6);
  EXPECT_TRUE(*lockedConst == ObjectMock(5, 6));
  EXPECT_FALSE(*lockedConst != ObjectMock(5, 6));
  EXPECT_TRUE(lockedConst->x == 5);
  EXPECT_TRUE(lockedConst->y == 6);

  dummy = nullptr; // shared locked -> kept alive

  Locked<ObjectMock, std::mutex> lockedMoved(std::move(locked));
  EXPECT_TRUE(lockedMoved->x == 5);
  locked = std::move(lockedMoved);
  EXPECT_TRUE(locked->x == 5);
}

TEST_F(LockedTest, providedLock) {
  auto dummy = std::make_shared<ObjectMock>(5, 6);
  std::mutex lock;

  std::unique_lock<std::mutex> guard(lock);
  Locked<ObjectMock, std::mutex> locked(dummy, std::move(guard));
  EXPECT_TRUE(locked.value().x == 5);
  EXPECT_TRUE(locked.value().y == 6);
  EXPECT_TRUE(*locked == ObjectMock(5, 6));
  EXPECT_TRUE(locked->x == 5);
  EXPECT_TRUE(locked->y == 6);
}

TEST_F(LockedTest, concurrentAccess) {
  bool isAsyncReady = false;
  auto dummy = std::make_shared<ObjectMock>(5, 6);
  std::mutex lock;

  auto promise = std::async(std::launch::async, [&isAsyncReady, &dummy, &lock]() {
    Locked<ObjectMock, std::mutex> locked(dummy, lock);
    isAsyncReady = true;
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (dummy->x == 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(3LL));
    locked.value().x = 7;
  });

  do { std::this_thread::sleep_for(std::chrono::microseconds(1LL));
  } while (!isAsyncReady);
  dummy->x = 6;
  Locked<ObjectMock, std::mutex> locked(dummy, lock);
  EXPECT_TRUE(locked->x == 7);
  promise.get();
}

// -- locked object reference --

TEST_F(LockedTest, lockedRefAccessors) {
  ObjectMock dummy(5, 6);
  std::mutex lock;

  LockedRef<ObjectMock, std::mutex> locked(dummy, lock);
  EXPECT_TRUE(locked.value().x == 5);
  EXPECT_TRUE(locked.value().y == 6);
  EXPECT_TRUE(*locked == ObjectMock(5, 6));
  EXPECT_TRUE(locked->x == 5);
  EXPECT_TRUE(locked->y == 6);

  const LockedRef<ObjectMock, std::mutex>& lockedConst = locked;
  EXPECT_TRUE(lockedConst.value().x == 5);
  EXPECT_TRUE(lockedConst.value().y == 6);
  EXPECT_TRUE(*lockedConst == ObjectMock(5, 6));
  EXPECT_TRUE(lockedConst->x == 5);
  EXPECT_TRUE(lockedConst->y == 6);

  LockedRef<ObjectMock, std::mutex> lockedMoved(std::move(locked));
  EXPECT_TRUE(lockedMoved->x == 5);
  locked = std::move(lockedMoved);
  EXPECT_TRUE(locked->x == 5);
}

TEST_F(LockedTest, providedRefLock) {
  ObjectMock dummy(5, 6);
  std::mutex lock;

  std::unique_lock<std::mutex> guard(lock);
  LockedRef<ObjectMock, std::mutex> locked(dummy, std::move(guard));
  EXPECT_TRUE(locked.value().x == 5);
  EXPECT_TRUE(locked.value().y == 6);
  EXPECT_TRUE(*locked == ObjectMock(5, 6));
  EXPECT_TRUE(locked->x == 5);
  EXPECT_TRUE(locked->y == 6);
}

TEST_F(LockedTest, concurrentRefAccess) {
  bool isAsyncReady = false;
  ObjectMock dummy(5, 6);
  std::mutex lock;

  auto promise = std::async(std::launch::async, [&isAsyncReady, &dummy, &lock]() {
    LockedRef<ObjectMock, std::mutex> locked(dummy, lock);
    isAsyncReady = true;
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (dummy.x == 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(3LL));
    locked.value().x = 7;
    });

  do { std::this_thread::sleep_for(std::chrono::microseconds(1LL));
  } while (!isAsyncReady);
  dummy.x = 6;
  LockedRef<ObjectMock, std::mutex> locked(dummy, lock);
  EXPECT_TRUE(locked->x == 7);
  promise.get();
}