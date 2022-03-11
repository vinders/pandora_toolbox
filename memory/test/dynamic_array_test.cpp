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
#include <mutex>
#include <unordered_set>
#include <memory/dynamic_array.h>

using namespace pandora::memory;

class DynamicArrayTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {
    std::lock_guard<std::mutex> guard(registryLock);
    ctorCounter = dtorCounter = 0;
    registeredInstances.clear();
  }
  void TearDown() override {}

  // Verify if constructed/destructed object instances match the expected state
  void CheckObjectRegistrar(bool isEmpty, bool objectsHaveExisted) {
    std::lock_guard<std::mutex> guard(registryLock);
    if (isEmpty) {
      EXPECT_EQ(size_t{ 0 }, registeredInstances.size());
      EXPECT_EQ(ctorCounter, dtorCounter);
    }
    else {
      EXPECT_TRUE(registeredInstances.size() > 0);
    }
    if (objectsHaveExisted) {
      EXPECT_NE((int32_t)0, ctorCounter);
    }
    else {
      EXPECT_EQ((int32_t)0, ctorCounter);
    }
  }

public:
  static std::mutex registryLock;
  static std::unordered_set<void*> registeredInstances;
  static int32_t ctorCounter;
  static int32_t dtorCounter;
};
std::mutex DynamicArrayTest::registryLock;
std::unordered_set<void*> DynamicArrayTest::registeredInstances;
int32_t DynamicArrayTest::ctorCounter = 0;
int32_t DynamicArrayTest::dtorCounter = 0;


// -- Helper - Object with construction/destruction counters --

struct _DynamicArrayData final {
  _DynamicArrayData(int a_, int b_) : a(a_), b(b_), isUserConstructed(true) {}

  _DynamicArrayData() {
    std::lock_guard<std::mutex> guard(DynamicArrayTest::registryLock);
    DynamicArrayTest::registeredInstances.insert(this); ++DynamicArrayTest::ctorCounter;
  }
  _DynamicArrayData(const _DynamicArrayData& rhs) : a(rhs.a), b(rhs.b) {
    std::lock_guard<std::mutex> guard(DynamicArrayTest::registryLock);
    DynamicArrayTest::registeredInstances.insert(this); ++DynamicArrayTest::ctorCounter;
  }
  _DynamicArrayData(_DynamicArrayData&& rhs) noexcept : a(rhs.a), b(rhs.b) {
    std::lock_guard<std::mutex> guard(DynamicArrayTest::registryLock);
    DynamicArrayTest::registeredInstances.insert(this); ++DynamicArrayTest::ctorCounter;
  }
  ~_DynamicArrayData() noexcept {
    if (!isUserConstructed) {
      std::lock_guard<std::mutex> guard(DynamicArrayTest::registryLock);
      DynamicArrayTest::registeredInstances.erase(this); ++DynamicArrayTest::dtorCounter;
    }
  }
  _DynamicArrayData& operator=(const _DynamicArrayData& rhs) { a = rhs.a; b = rhs.b; return *this; }
  _DynamicArrayData& operator=(_DynamicArrayData&& rhs) noexcept { a = rhs.a; b = rhs.b; return *this; }

  int a = 0;
  int b = 1;
  bool isUserConstructed = false;
};


// -- DynamicArray --

TEST_F(DynamicArrayTest, arrayAccessorsCtorsInt) {
  DynamicArray<int> empty;
  const DynamicArray<int>* emptyConst = (const DynamicArray<int>*)&empty;
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_TRUE(emptyConst->data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_EQ(size_t{0}, empty.length());
  EXPECT_TRUE(empty.empty());

  DynamicArray<int> empty2(0);
  EXPECT_TRUE(empty2.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty2.size());
  EXPECT_TRUE(empty2.empty());
  DynamicArray<int> empty3(std::move(empty));
  EXPECT_TRUE(empty3.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty3.size());
  EXPECT_TRUE(empty3.empty());
  empty2 = std::move(empty3);
  EXPECT_TRUE(empty2.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty2.size());
  EXPECT_TRUE(empty2.empty());

  DynamicArray<int> prealloc(size_t{16u});
  const DynamicArray<int>* preallocConst = (const DynamicArray<int>*)&prealloc;
  EXPECT_TRUE(prealloc.data() != nullptr);
  EXPECT_EQ(size_t{16u}, prealloc.size());
  ASSERT_EQ(size_t{16u}, prealloc.length());
  EXPECT_FALSE(prealloc.empty());
  EXPECT_EQ(0, *prealloc.data());
  EXPECT_EQ(&prealloc[0], &prealloc.front());
  EXPECT_EQ(&prealloc[15], &prealloc.back());
  EXPECT_EQ(&(*preallocConst)[0], &preallocConst->front());
  EXPECT_EQ(&(*preallocConst)[15], &preallocConst->back());
  int nbIter = 0;
  for (auto& it : prealloc) {
    EXPECT_EQ(0, it);
    ++nbIter;
  }
  EXPECT_EQ(16, nbIter);
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(0, prealloc[i]);
    prealloc[i] = i;
  }
  EXPECT_EQ((int)0, preallocConst->front());
  EXPECT_EQ((int)15, preallocConst->back());
  EXPECT_EQ(size_t{16u}, prealloc.size());
}

TEST_F(DynamicArrayTest, arrayAccessorsCtorsObject) {
  {//scope
    DynamicArray<_DynamicArrayData> empty;
    const DynamicArray<_DynamicArrayData>* emptyConst = (const DynamicArray<_DynamicArrayData>*)&empty;
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_TRUE(emptyConst->data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_EQ(size_t{0}, empty.length());
    EXPECT_TRUE(empty.empty());

    DynamicArray<_DynamicArrayData> empty2(0);
    EXPECT_TRUE(empty2.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty2.size());
    EXPECT_TRUE(empty2.empty());
    DynamicArray<_DynamicArrayData> empty3(std::move(empty));
    EXPECT_TRUE(empty3.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty3.size());
    EXPECT_TRUE(empty3.empty());
    empty2 = std::move(empty3);
    EXPECT_TRUE(empty2.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty2.size());
    EXPECT_TRUE(empty2.empty());
    CheckObjectRegistrar(true, false);
  
    DynamicArray<_DynamicArrayData> prealloc(size_t{16u});
    EXPECT_TRUE(prealloc.data() != nullptr);
    EXPECT_EQ(size_t{16u}, prealloc.size());
    ASSERT_EQ(size_t{16u}, prealloc.length());
    EXPECT_FALSE(prealloc.empty());
    EXPECT_EQ(0, prealloc.data()->a);
    EXPECT_EQ(1, prealloc.data()->b);
    EXPECT_EQ(&prealloc[0], &prealloc.front());
    EXPECT_EQ(&prealloc[15], &prealloc.back());
    for (int i = 0; i < 16; ++i) {
      EXPECT_EQ(0, prealloc[i].a);
      EXPECT_EQ(1, prealloc[i].b);
    }
    int nbIter = 0;
    for (auto& it : prealloc) {
      EXPECT_EQ(0, it.a);
      EXPECT_EQ(1, it.b);
      ++nbIter;
    }
    EXPECT_EQ(16, nbIter);

    prealloc[5] = _DynamicArrayData{ 5, 6 };
    for (int i = 0; i < 16; ++i) {
      if (i == 5) {
        EXPECT_EQ(5, prealloc[i].a);
        EXPECT_EQ(6, prealloc[i].b);
      }
      else {
        EXPECT_EQ(0, prealloc[i].a);
        EXPECT_EQ(1, prealloc[i].b);
      }
    }
    EXPECT_EQ(size_t{16u}, prealloc.size());
    CheckObjectRegistrar(false, true);
  }
  CheckObjectRegistrar(true, true);
}

TEST_F(DynamicArrayTest, arrayAssignClearInt) {
  DynamicArray<int> empty;
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_TRUE(empty.empty());
  empty.clear();
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_TRUE(empty.empty());

  int data1 = 5;
  int data2[3] = {0,1,3};
  int data3[6] = {7,6,5,4,3,2};

  empty = DynamicArray<int>(&data1, size_t{ 1u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{1u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data1, *empty.data());
  EXPECT_EQ(data1, empty[0]);
  empty = DynamicArray<int>(data2, size_t{ 3u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{3u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data2[0], *empty.data());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(data2[i], empty[i]);
  }
  empty = DynamicArray<int>(data3, size_t{ 6u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{6u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data3[0], *empty.data());
  for (int i = 0; i < 6; ++i) {
    EXPECT_EQ(data3[i], empty[i]);
  }
  empty = DynamicArray<int>(&data1, size_t{ 1u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{1u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data1, *empty.data());
  EXPECT_EQ(data1, empty[0]);
  empty.clear();
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_TRUE(empty.empty());
  empty = DynamicArray<int>(&data1, size_t{ 1u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{1u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data1, *empty.data());
  EXPECT_EQ(data1, empty[0]);
  empty = DynamicArray<int>(nullptr, 0);
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_TRUE(empty.empty());

  DynamicArray<int> val1(16);
  EXPECT_TRUE(val1.data() != nullptr);
  EXPECT_EQ(size_t{16u}, val1.size());
  EXPECT_FALSE(val1.empty());

  val1 = DynamicArray<int>(&data1, size_t{ 1u });
  EXPECT_TRUE(val1.data() != nullptr);
  ASSERT_EQ(size_t{1u}, val1.size());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(data1, *val1.data());
  EXPECT_EQ(data1, val1[0]);
  val1 = DynamicArray<int>(data2, size_t{ 3u });
  EXPECT_TRUE(val1.data() != nullptr);
  ASSERT_EQ(size_t{3u}, val1.size());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(data2[0], *val1.data());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(data2[i], val1[i]);
  }
  val1.clear();
  EXPECT_TRUE(val1.data() == nullptr);
  EXPECT_EQ(size_t{0}, val1.size());
  EXPECT_TRUE(val1.empty());
  val1 = DynamicArray<int>(&data1, size_t{ 1u });
  EXPECT_TRUE(val1.data() != nullptr);
  ASSERT_EQ(size_t{1u}, val1.size());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(data1, *val1.data());
  EXPECT_EQ(data1, val1[0]);
}

TEST_F(DynamicArrayTest, arrayAssignClearObject) {
  {//scope
    DynamicArray<_DynamicArrayData> empty;
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    empty.clear();
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    CheckObjectRegistrar(true, false);

    _DynamicArrayData data1{ 5,6 };
    _DynamicArrayData data2[3] = {_DynamicArrayData{ 0,1 }, _DynamicArrayData{ 1,2 }, _DynamicArrayData{ 3,4 }};
    _DynamicArrayData data3[6] = {_DynamicArrayData{ 7,8 }, _DynamicArrayData{ 6,7 }, _DynamicArrayData{ 5,6 },
                                 _DynamicArrayData{ 4,5 }, _DynamicArrayData{ 3,4 }, _DynamicArrayData{ 2,3 }};

    empty = DynamicArray<_DynamicArrayData>(&data1, size_t{ 1u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{1u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data1.a, empty.data()->a);
    EXPECT_EQ(data1.b, empty.data()->b);
    EXPECT_EQ(data1.a, empty[0].a);
    EXPECT_EQ(data1.b, empty[0].b);
    CheckObjectRegistrar(false, true);
    empty = DynamicArray<_DynamicArrayData>(data2, size_t{ 3u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{3u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data2[0].a, empty.data()->a);
    EXPECT_EQ(data2[0].b, empty.data()->b);
    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(data2[i].a, empty[i].a);
      EXPECT_EQ(data2[i].b, empty[i].b);
    }
    empty = DynamicArray<_DynamicArrayData>(data3, size_t{ 6u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{6u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data3[0].a, empty.data()->a);
    EXPECT_EQ(data3[0].b, empty.data()->b);
    for (int i = 0; i < 6; ++i) {
      EXPECT_EQ(data3[i].a, empty[i].a);
      EXPECT_EQ(data3[i].b, empty[i].b);
    }
    empty = DynamicArray<_DynamicArrayData>(&data1, size_t{ 1u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{1u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data1.a, empty.data()->a);
    EXPECT_EQ(data1.b, empty.data()->b);
    EXPECT_EQ(data1.a, empty[0].a);
    EXPECT_EQ(data1.b, empty[0].b);
    empty.clear();
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    empty = DynamicArray<_DynamicArrayData>(&data1, size_t{ 1u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{1u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data1.a, empty.data()->a);
    EXPECT_EQ(data1.b, empty.data()->b);
    EXPECT_EQ(data1.a, empty[0].a);
    EXPECT_EQ(data1.b, empty[0].b);
    empty = DynamicArray<_DynamicArrayData>(nullptr, 0);
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    CheckObjectRegistrar(true, true);

    DynamicArray<_DynamicArrayData> val1(16);
    EXPECT_TRUE(val1.data() != nullptr);
    EXPECT_EQ(size_t{16u}, val1.size());
    EXPECT_FALSE(val1.empty());

    val1 = DynamicArray<_DynamicArrayData>(&data1, size_t{ 1u });
    EXPECT_TRUE(val1.data() != nullptr);
    ASSERT_EQ(size_t{1u}, val1.size());
    EXPECT_FALSE(val1.empty());
    EXPECT_EQ(data1.a, val1.data()->a);
    EXPECT_EQ(data1.b, val1.data()->b);
    EXPECT_EQ(data1.a, val1[0].a);
    EXPECT_EQ(data1.b, val1[0].b);
    val1 = DynamicArray<_DynamicArrayData>(data2, size_t{ 3u });
    EXPECT_TRUE(val1.data() != nullptr);
    ASSERT_EQ(size_t{3u}, val1.size());
    EXPECT_FALSE(val1.empty());
    EXPECT_EQ(data2[0].a, val1.data()->a);
    EXPECT_EQ(data2[0].b, val1.data()->b);
    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(data2[i].a, val1[i].a);
      EXPECT_EQ(data2[i].b, val1[i].b);
    }
    val1.clear();
    EXPECT_TRUE(val1.data() == nullptr);
    EXPECT_EQ(size_t{0}, val1.size());
    EXPECT_TRUE(val1.empty());
    val1 = DynamicArray<_DynamicArrayData>(&data1, size_t{ 1u });
    EXPECT_TRUE(val1.data() != nullptr);
    ASSERT_EQ(size_t{1u}, val1.size());
    EXPECT_FALSE(val1.empty());
    EXPECT_EQ(data1.a, val1.data()->a);
    EXPECT_EQ(data1.b, val1.data()->b);
    EXPECT_EQ(data1.a, val1[0].a);
    EXPECT_EQ(data1.b, val1[0].b);
  }
  CheckObjectRegistrar(true, true);
}
