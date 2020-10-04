#include <chrono>
#include <future>
#include <vector>
#include <gtest/gtest.h>
#include <pattern/observer.h>

using namespace pandora::pattern;

class ObserverTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- helpers --

class TestObserver : public Observer<int> {
public:
  TestObserver() = default;
  virtual ~TestObserver() = default;

  void onUpdate(int arg) override {
    while (isLocked())
      std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    _updates.emplace_back(arg);
  }

  inline const std::vector<int>& updates() const noexcept { return this->_updates; }
  inline bool isLocked() const noexcept { return this->_isLocked; }
  inline void setLocked(bool val) noexcept { this->_isLocked = val; }

private:
  std::vector<int> _updates;
  bool _isLocked = false;
};


// -- observable class --

TEST_F(ObserverTest, accessors) {
  Observable<std::mutex, false, int> obs;
  Observable<std::mutex, true, int> defObs;
  EXPECT_EQ(size_t{ 0 }, obs.size());
  EXPECT_EQ(size_t{ 0 }, defObs.size());
  EXPECT_TRUE(obs.empty());
  EXPECT_TRUE(defObs.empty());

  auto watcher = std::make_shared<TestObserver>();
  auto watcherDef = std::make_shared<TestObserver>();
  auto contract = obs.addObserver(watcher);
  auto contractDef = defObs.addObserver(watcherDef);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  EXPECT_EQ(size_t{ 1 }, defObs.size());
  EXPECT_FALSE(obs.empty());
  EXPECT_FALSE(defObs.empty());

  Observable<std::mutex, false, int> obsMoved(std::move(obs));
  Observable<std::mutex, true, int> defObsMoved(std::move(defObs));
  EXPECT_EQ(size_t{ 1 }, obsMoved.size());
  EXPECT_EQ(size_t{ 1 }, defObsMoved.size());
  EXPECT_FALSE(obsMoved.empty());
  EXPECT_FALSE(defObsMoved.empty());

  obs = std::move(obsMoved);
  defObs = std::move(defObsMoved);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  EXPECT_EQ(size_t{ 1 }, defObs.size());
  EXPECT_FALSE(obs.empty());
  EXPECT_FALSE(defObs.empty());

  contract.reset();
  EXPECT_EQ(size_t{ 0 }, obs.size());
  contractDef.reset();
  EXPECT_EQ(size_t{ 0 }, defObs.size());
}

TEST_F(ObserverTest, registerUnregister) {
  Observable<std::mutex, false, int> obs;
  Observable<std::mutex, true, int> defObs;
  EXPECT_TRUE(obs.empty());
  EXPECT_TRUE(defObs.empty());

  auto watcher = std::make_shared<TestObserver>();
  auto watcherDef = std::make_shared<TestObserver>();
  auto contract = obs.addObserver(watcher);
  auto contractDef = defObs.addObserver(watcherDef);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  auto watcher2 = std::make_shared<TestObserver>();
  auto watcherDef2 = std::make_shared<TestObserver>();
  auto contract2 = obs.addObserver(watcher2);
  auto contractDef2 = defObs.addObserver(watcherDef2);
  EXPECT_EQ(size_t{ 2 }, obs.size());
  EXPECT_EQ(size_t{ 2 }, defObs.size());

  obs.unregister(*contract);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  defObs.unregister(*contractDef);
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  obs.unregister(*contract); // already revoked
  EXPECT_EQ(size_t{ 1 }, obs.size());

  contract = obs.addObserver(watcher);
  contractDef = defObs.addObserver(watcherDef);
  EXPECT_EQ(size_t{ 2 }, obs.size());
  EXPECT_EQ(size_t{ 2 }, defObs.size());

  obs.unregister(*contract2);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  defObs.unregister(*contractDef2);
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  contract.reset();
  EXPECT_EQ(size_t{ 0 }, obs.size());
  contractDef.reset();
  EXPECT_EQ(size_t{ 0 }, defObs.size());
}

TEST_F(ObserverTest, registerClear) {
  Observable<std::mutex, false, int> obs;
  Observable<std::mutex, true, int> defObs;
  EXPECT_TRUE(obs.empty());
  EXPECT_TRUE(defObs.empty());

  auto watcher = std::make_shared<TestObserver>();
  auto watcherDef = std::make_shared<TestObserver>();
  auto contract = obs.addObserver(watcher);
  auto contractDef = defObs.addObserver(watcherDef);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  auto watcher2 = std::make_shared<TestObserver>();
  auto watcherDef2 = std::make_shared<TestObserver>();
  auto contract2 = obs.addObserver(watcher2);
  auto contractDef2 = defObs.addObserver(watcherDef2);
  EXPECT_EQ(size_t{ 2 }, obs.size());
  EXPECT_EQ(size_t{ 2 }, defObs.size());

  obs.clear();
  EXPECT_EQ(size_t{ 0 }, obs.size());
  defObs.clear();
  EXPECT_EQ(size_t{ 0 }, defObs.size());

  obs.unregister(*contract); // already revoked
  EXPECT_EQ(size_t{ 0 }, obs.size());
}

TEST_F(ObserverTest, registerRun) {
  Observable<std::mutex, false, int> obs;
  Observable<std::mutex, true, int> defObs;
  EXPECT_TRUE(obs.empty());
  EXPECT_TRUE(defObs.empty());

  auto watcher = std::make_shared<TestObserver>();
  auto watcherDef = std::make_shared<TestObserver>();
  auto contract = obs.addObserver(watcher);
  auto contractDef = defObs.addObserver(watcherDef);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  auto watcher2 = std::make_shared<TestObserver>();
  auto watcherDef2 = std::make_shared<TestObserver>();
  auto contract2 = obs.addObserver(watcher2);
  auto contractDef2 = defObs.addObserver(watcherDef2);
  EXPECT_EQ(size_t{ 2 }, obs.size());
  EXPECT_EQ(size_t{ 2 }, defObs.size());

  obs.notify(42);
  EXPECT_EQ(size_t{ 1 }, watcher->updates().size());
  EXPECT_EQ(size_t{ 1 }, watcher2->updates().size());
  defObs.notify(42);
  EXPECT_EQ(size_t{ 1 }, watcherDef->updates().size());
  EXPECT_EQ(size_t{ 1 }, watcherDef2->updates().size());

  obs.notify(64);
  ASSERT_EQ(size_t{ 2 }, watcher->updates().size());
  ASSERT_EQ(size_t{ 2 }, watcher2->updates().size());
  EXPECT_EQ(42, watcher->updates()[0]);
  EXPECT_EQ(42, watcher2->updates()[0]);
  EXPECT_EQ(64, watcher->updates()[1]);
  EXPECT_EQ(64, watcher2->updates()[1]);
  defObs.notify(64);
  ASSERT_EQ(size_t{ 2 }, watcherDef->updates().size());
  ASSERT_EQ(size_t{ 2 }, watcherDef2->updates().size());
  EXPECT_EQ(42, watcherDef->updates()[0]);
  EXPECT_EQ(42, watcherDef2->updates()[0]);
  EXPECT_EQ(64, watcherDef->updates()[1]);
  EXPECT_EQ(64, watcherDef2->updates()[1]);

  obs.unregister(*contract);
  EXPECT_EQ(size_t{ 1 }, obs.size());
  defObs.unregister(*contractDef);
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  obs.notify(22);
  ASSERT_EQ(size_t{ 2 }, watcher->updates().size());
  ASSERT_EQ(size_t{ 3 }, watcher2->updates().size());
  EXPECT_EQ(42, watcher->updates()[0]);
  EXPECT_EQ(42, watcher2->updates()[0]);
  EXPECT_EQ(64, watcher->updates()[1]);
  EXPECT_EQ(64, watcher2->updates()[1]);
  EXPECT_EQ(22, watcher2->updates()[2]);
  defObs.notify(22);
  ASSERT_EQ(size_t{ 2 }, watcherDef->updates().size());
  ASSERT_EQ(size_t{ 3 }, watcherDef2->updates().size());
  EXPECT_EQ(42, watcherDef->updates()[0]);
  EXPECT_EQ(42, watcherDef2->updates()[0]);
  EXPECT_EQ(64, watcherDef->updates()[1]);
  EXPECT_EQ(64, watcherDef2->updates()[1]);
  EXPECT_EQ(22, watcherDef2->updates()[2]);
}

// -- concurrency tests --

TEST_F(ObserverTest, multiThreadDirect) {
  bool isAsyncReady = false;
  bool isRunComplete = false;
  Observable<std::mutex, false, int> obs;
  auto watcher = std::make_shared<TestObserver>();
  auto contract = obs.addObserver(watcher);
  EXPECT_EQ(size_t{ 1 }, obs.size());

  auto promiseRun = std::async(std::launch::async, [&isAsyncReady, &isRunComplete, &watcher, &obs]() {
    watcher->setLocked(true);
    isAsyncReady = true;
    obs.notify(42);
    isRunComplete = true;
  });
  auto promiseAdd = std::async(std::launch::async, [&isAsyncReady, &isRunComplete, &obs]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isAsyncReady);
    std::this_thread::sleep_for(std::chrono::milliseconds(2LL));

    auto contractAdded = obs.addObserver(std::make_shared<TestObserver>());
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isRunComplete);
  });

  auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(2000LL);
  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady && std::chrono::high_resolution_clock::now() < timeout);
  EXPECT_TRUE(isAsyncReady);
  isAsyncReady = true;
  std::this_thread::sleep_for(std::chrono::milliseconds(10LL));
  watcher->setLocked(false);
  EXPECT_TRUE(promiseRun.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_TRUE(promiseAdd.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
}

TEST_F(ObserverTest, multiThreadDeferred) {
  bool isAsyncReady = false;
  bool isRunComplete = false;
  Observable<std::mutex, true, int> defObs;
  auto watcher = std::make_shared<TestObserver>();
  auto contract = defObs.addObserver(watcher);
  EXPECT_EQ(size_t{ 1 }, defObs.size());

  auto promiseRun = std::async(std::launch::async, [&isAsyncReady, &isRunComplete, &watcher, &defObs]() {
    watcher->setLocked(true);
    isAsyncReady = true;
    defObs.notify(42);
    isRunComplete = true;
    });
  auto promiseAdd = std::async(std::launch::async, [&isAsyncReady, &isRunComplete, &watcher, &defObs]() {
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isAsyncReady);
    std::this_thread::sleep_for(std::chrono::milliseconds(2LL));

    auto contractAdded = defObs.addObserver(std::make_shared<TestObserver>());
    watcher->setLocked(false);
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    } while (!isRunComplete);

    defObs.notify(42);
    EXPECT_EQ(size_t{ 2 }, defObs.size());
  });

  auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(2000LL);
  do { std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
  } while (!isAsyncReady && std::chrono::high_resolution_clock::now() < timeout);
  EXPECT_TRUE(isAsyncReady);
  isAsyncReady = true;
  EXPECT_TRUE(promiseRun.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
  EXPECT_FALSE(watcher->isLocked());
  watcher->setLocked(false);
  EXPECT_TRUE(promiseAdd.wait_for(std::chrono::milliseconds(2000LL)) != std::future_status::timeout);
}
