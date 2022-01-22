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
#include <unordered_set>
#include <memory/light_vector.h>

using namespace pandora::memory;

class LightVectorTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {
    ctorCounter = dtorCounter = 0;
    registeredInstances.clear();
  }
  void TearDown() override {}

  // Verify if constructed/destructed object instances match the expected state
  void CheckObjectRegistrar(bool isEmpty, bool objectsHaveExisted) {
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
  static std::unordered_set<void*> registeredInstances;
  static int32_t ctorCounter;
  static int32_t dtorCounter;
};
std::unordered_set<void*> LightVectorTest::registeredInstances;
int32_t LightVectorTest::ctorCounter = 0;
int32_t LightVectorTest::dtorCounter = 0;


// -- Helper - Object with construction/destruction counters --

struct _LightVectorData final {
  _LightVectorData(int a_, int b_) : a(a_), b(b_), isUserConstructed(true) {}

  _LightVectorData() { LightVectorTest::registeredInstances.insert(this); ++LightVectorTest::ctorCounter; }
  _LightVectorData(const _LightVectorData& rhs) : a(rhs.a), b(rhs.b) {
    LightVectorTest::registeredInstances.insert(this); ++LightVectorTest::ctorCounter;
  }
  _LightVectorData(_LightVectorData&& rhs) noexcept : a(rhs.a), b(rhs.b) {
    LightVectorTest::registeredInstances.insert(this); ++LightVectorTest::ctorCounter;
  }
  ~_LightVectorData() noexcept {
    if (!isUserConstructed) {
      LightVectorTest::registeredInstances.erase(this); ++LightVectorTest::dtorCounter;
    }
  }
  _LightVectorData& operator=(const _LightVectorData& rhs) { a = rhs.a; b = rhs.b; return *this; }
  _LightVectorData& operator=(_LightVectorData&& rhs) noexcept { a = rhs.a; b = rhs.b; return *this; }

  int a = 0;
  int b = 1;
  bool isUserConstructed = false;
};


// -- LightVector --

TEST_F(LightVectorTest, vectorAccessorsCtorsInt) {
  LightVector<int> empty;
  const LightVector<int>* emptyConst = (const LightVector<int>*)&empty;
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_TRUE(emptyConst->data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_EQ(size_t{0}, empty.length());
  EXPECT_TRUE(empty.empty());
  for (auto& it : empty) {
    EXPECT_FALSE(true);
    ASSERT_TRUE(&it != nullptr);
  }
  for (auto& it : *emptyConst) {
    EXPECT_FALSE(true);
    ASSERT_TRUE(&it != nullptr);
  }

  LightVector<int> empty2(0);
  EXPECT_TRUE(empty2.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty2.size());
  EXPECT_TRUE(empty2.empty());
  LightVector<int> empty3(nullptr, 0);
  EXPECT_TRUE(empty3.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty3.size());
  EXPECT_TRUE(empty3.empty());
  LightVector<int> empty4(empty);
  EXPECT_TRUE(empty4.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty4.size());
  EXPECT_TRUE(empty4.empty());
  empty3 = empty4;
  EXPECT_TRUE(empty3.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty3.size());
  EXPECT_TRUE(empty3.empty());

  LightVector<int> prealloc(size_t{16u});
  const LightVector<int>* preallocConst = (const LightVector<int>*)&prealloc;
  EXPECT_TRUE(prealloc.data() != nullptr);
  EXPECT_EQ(size_t{16u}, prealloc.size());
  ASSERT_EQ(size_t{16u}, prealloc.length());
  EXPECT_FALSE(prealloc.empty());
  EXPECT_EQ(0, *prealloc.data());
  EXPECT_EQ(&prealloc[0], &prealloc.front());
  EXPECT_EQ(&prealloc[15], &prealloc.back());
  EXPECT_EQ(&(*preallocConst)[0], &preallocConst->front());
  EXPECT_EQ(&(*preallocConst)[15], &preallocConst->back());
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(0, prealloc[i]);
  }
  int nbIter = 0;
  for (auto& it : prealloc) {
    EXPECT_EQ(0, it);
    ++nbIter;
  }
  EXPECT_EQ(16, nbIter);
  prealloc[5] = 5;
  for (int i = 0; i < 16; ++i) {
    if (i == 5) {
      EXPECT_EQ(5, prealloc[i]);
    }
    else {
      EXPECT_EQ(0, prealloc[i]);
    }
  }
  EXPECT_EQ(size_t{16u}, prealloc.size());

  int data1 = 5;
  LightVector<int> val1(&data1, size_t{1u});
  EXPECT_TRUE(val1.data() != nullptr);
  EXPECT_EQ(size_t{1u}, val1.size());
  ASSERT_EQ(size_t{1u}, val1.length());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(data1, *val1.data());
  EXPECT_EQ(&val1[0], &val1.front());
  EXPECT_EQ(&val1[0], &val1.back());
  EXPECT_EQ(data1, val1[0]);
  nbIter = 0;
  for (auto& it : val1) {
    EXPECT_EQ(data1, it);
    ++nbIter;
  }
  EXPECT_EQ(1, nbIter);
  val1[0] = 4;
  EXPECT_EQ(4, val1[0]);
  EXPECT_EQ(size_t{1u}, val1.size());

  int data2[3] = {0,1,3};
  LightVector<int> val2(data2, size_t{3});
  EXPECT_TRUE(val2.data() != nullptr);
  EXPECT_EQ(size_t{3u}, val2.size());
  ASSERT_EQ(size_t{3u}, val2.length());
  EXPECT_FALSE(val2.empty());
  EXPECT_EQ(data2[0], *val2.data());
  EXPECT_EQ(&val2[0], &val2.front());
  EXPECT_EQ(&val2[2], &val2.back());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(data2[i], val2[i]);
  }
  nbIter = 0;
  for (auto& it : val2) {
    EXPECT_EQ(data2[nbIter], it);
    ++nbIter;
  }
  EXPECT_EQ(3, nbIter);
  val2[0] = 4;
  EXPECT_EQ(4, val2[0]);
  EXPECT_EQ(size_t{3u}, val2.size());
  val2[0] = data2[0];
  EXPECT_EQ(data2[0], val2[0]);

  LightVector<int> copied(prealloc);
  EXPECT_TRUE(prealloc.data() != nullptr);
  EXPECT_TRUE(copied.data() != nullptr);
  EXPECT_EQ(prealloc.size(), copied.size());
  ASSERT_EQ(size_t{16u}, copied.length());
  EXPECT_FALSE(copied.empty());
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(prealloc[i], copied[i]);
  }
  nbIter = 0;
  for (auto& it : copied) {
    EXPECT_TRUE(&it != nullptr);
    ++nbIter;
  }
  EXPECT_EQ(16, nbIter);

  val1 = copied;
  EXPECT_TRUE(copied.data() != nullptr);
  EXPECT_TRUE(val1.data() != nullptr);
  EXPECT_EQ(copied.size(), val1.size());
  ASSERT_EQ(size_t{16u}, val1.length());
  EXPECT_FALSE(val1.empty());
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(copied[i], val1[i]);
  }

  LightVector<int> moved(std::move(val2));
  EXPECT_TRUE(val2.data() == nullptr);
  EXPECT_TRUE(moved.data() != nullptr);
  EXPECT_EQ(size_t{0}, val2.size());
  EXPECT_EQ(size_t{3u}, moved.size());
  ASSERT_EQ(size_t{3u}, moved.length());
  EXPECT_FALSE(moved.empty());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(data2[i], moved[i]);
  }
  nbIter = 0;
  for (auto& it : moved) {
    EXPECT_TRUE(&it != nullptr);
    ++nbIter;
  }
  EXPECT_EQ(3, nbIter);

  val2 = std::move(moved);
  EXPECT_TRUE(moved.data() == nullptr);
  EXPECT_TRUE(val2.data() != nullptr);
  EXPECT_EQ(size_t{0}, moved.size());
  EXPECT_EQ(size_t{3u}, val2.size());
  ASSERT_EQ(size_t{3u}, val2.length());
  EXPECT_FALSE(val2.empty());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(data2[i], val2[i]);
  }
}

TEST_F(LightVectorTest, vectorAccessorsCtorsObject) {
  {//scope
    LightVector<_LightVectorData> empty;
    const LightVector<_LightVectorData>* emptyConst = (const LightVector<_LightVectorData>*)&empty;
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_TRUE(emptyConst->data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_EQ(size_t{0}, empty.length());
    EXPECT_TRUE(empty.empty());
    for (auto& it : empty) {
      EXPECT_FALSE(true);
      ASSERT_TRUE(&it != nullptr);
    }
    for (auto& it : *emptyConst) {
      EXPECT_FALSE(true);
      ASSERT_TRUE(&it != nullptr);
    }

    LightVector<_LightVectorData> empty2(0);
    EXPECT_TRUE(empty2.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty2.size());
    EXPECT_TRUE(empty2.empty());
    LightVector<_LightVectorData> empty3(nullptr, 0);
    EXPECT_TRUE(empty3.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty3.size());
    EXPECT_TRUE(empty3.empty());
    LightVector<_LightVectorData> empty4(empty);
    EXPECT_TRUE(empty4.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty4.size());
    EXPECT_TRUE(empty4.empty());
    empty3 = empty4;
    EXPECT_TRUE(empty3.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty3.size());
    EXPECT_TRUE(empty3.empty());
    CheckObjectRegistrar(true, false);
  
    LightVector<_LightVectorData> prealloc(size_t{16u});
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
    prealloc[5] = _LightVectorData{ 5, 6 };
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
  
    _LightVectorData data1{ 5, 6 };
    LightVector<_LightVectorData> val1(&data1, size_t{1u});
    EXPECT_TRUE(val1.data() != nullptr);
    EXPECT_EQ(size_t{1u}, val1.size());
    ASSERT_EQ(size_t{1u}, val1.length());
    EXPECT_FALSE(val1.empty());
    EXPECT_EQ(data1.a, val1.data()->a);
    EXPECT_EQ(data1.b, val1.data()->b);
    EXPECT_EQ(&val1[0], &val1.front());
    EXPECT_EQ(&val1[0], &val1.back());
    EXPECT_EQ(data1.a, val1[0].a);
    EXPECT_EQ(data1.b, val1[0].b);
    nbIter = 0;
    for (auto& it : val1) {
      EXPECT_EQ(data1.a, it.a);
      EXPECT_EQ(data1.b, it.b);
      ++nbIter;
    }
    EXPECT_EQ(1, nbIter);
    val1[0] = _LightVectorData{ 4, 2 };
    val1[0].b = 3;
    EXPECT_EQ(4, val1[0].a);
    EXPECT_EQ(3, val1[0].b);
    EXPECT_EQ(size_t{1u}, val1.size());
  
    _LightVectorData data2[3] = {_LightVectorData{}, _LightVectorData{ 1,2 }, _LightVectorData{ 3,4 }};
    LightVector<_LightVectorData> val2(data2, size_t{3});
    EXPECT_TRUE(val2.data() != nullptr);
    EXPECT_EQ(size_t{3u}, val2.size());
    ASSERT_EQ(size_t{3u}, val2.length());
    EXPECT_FALSE(val2.empty());
    EXPECT_EQ(data2[0].a, val2.data()->a);
    EXPECT_EQ(data2[0].b, val2.data()->b);
    EXPECT_EQ(&val2[0], &val2.front());
    EXPECT_EQ(&val2[2], &val2.back());
    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(data2[i].a, val2[i].a);
      EXPECT_EQ(data2[i].b, val2[i].b);
    }
    nbIter = 0;
    for (auto& it : val2) {
      EXPECT_EQ(data2[nbIter].a, it.a);
      EXPECT_EQ(data2[nbIter].b, it.b);
      ++nbIter;
    }
    EXPECT_EQ(3, nbIter);
    val2[0] = _LightVectorData{ 4, 3 };
    EXPECT_EQ(4, val2[0].a);
    EXPECT_EQ(3, val2[0].b);
    EXPECT_EQ(size_t{3u}, val2.size());
    val2[0] = data2[0];
    EXPECT_EQ(data2[0].a, val2[0].a);
    EXPECT_EQ(data2[0].b, val2[0].b);

    LightVector<_LightVectorData> copied(prealloc);
    EXPECT_TRUE(prealloc.data() != nullptr);
    EXPECT_TRUE(copied.data() != nullptr);
    EXPECT_EQ(prealloc.size(), copied.size());
    ASSERT_EQ(size_t{16u}, copied.length());
    EXPECT_FALSE(copied.empty());
    for (int i = 0; i < 16; ++i) {
      EXPECT_EQ(prealloc[i].a, copied[i].a);
      EXPECT_EQ(prealloc[i].b, copied[i].b);
    }
    nbIter = 0;
    for (auto& it : copied) {
      EXPECT_TRUE(&it != nullptr);
      ++nbIter;
    }
    EXPECT_EQ(16, nbIter);

    val1 = copied;
    EXPECT_TRUE(copied.data() != nullptr);
    EXPECT_TRUE(val1.data() != nullptr);
    EXPECT_EQ(copied.size(), val1.size());
    ASSERT_EQ(size_t{16u}, val1.length());
    EXPECT_FALSE(val1.empty());
    for (int i = 0; i < 16; ++i) {
      EXPECT_EQ(copied[i].a, val1[i].a);
      EXPECT_EQ(copied[i].b, val1[i].b);
    }
  
    LightVector<_LightVectorData> moved(std::move(val2));
    EXPECT_TRUE(val2.data() == nullptr);
    EXPECT_TRUE(moved.data() != nullptr);
    EXPECT_EQ(size_t{0}, val2.size());
    EXPECT_EQ(size_t{3u}, moved.size());
    ASSERT_EQ(size_t{3u}, moved.length());
    EXPECT_FALSE(moved.empty());
    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(data2[i].a, moved[i].a);
      EXPECT_EQ(data2[i].b, moved[i].b);
    }
    nbIter = 0;
    for (auto& it : moved) {
      EXPECT_TRUE(&it != nullptr);
      ++nbIter;
    }
    EXPECT_EQ(3, nbIter);

    val2 = std::move(moved);
    EXPECT_TRUE(moved.data() == nullptr);
    EXPECT_TRUE(val2.data() != nullptr);
    EXPECT_EQ(size_t{0}, moved.size());
    EXPECT_EQ(size_t{3u}, val2.size());
    ASSERT_EQ(size_t{3u}, val2.length());
    EXPECT_FALSE(val2.empty());
    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(data2[i].a, val2[i].a);
      EXPECT_EQ(data2[i].b, val2[i].b);
    }
  }
  CheckObjectRegistrar(true, true);
}

TEST_F(LightVectorTest, vectorAssignClearInt) {
  LightVector<int> empty;
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

  empty.assign(&data1, size_t{ 1u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{1u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data1, *empty.data());
  EXPECT_EQ(data1, empty[0]);
  empty.assign(data2, size_t{ 3u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{3u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data2[0], *empty.data());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(data2[i], empty[i]);
  }
  empty.assign(data3, size_t{ 6u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{6u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data3[0], *empty.data());
  for (int i = 0; i < 6; ++i) {
    EXPECT_EQ(data3[i], empty[i]);
  }
  empty.assign(&data1, size_t{ 1u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{1u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data1, *empty.data());
  EXPECT_EQ(data1, empty[0]);
  empty.clear();
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_TRUE(empty.empty());
  empty.assign(&data1, size_t{ 1u });
  EXPECT_TRUE(empty.data() != nullptr);
  ASSERT_EQ(size_t{1u}, empty.size());
  EXPECT_FALSE(empty.empty());
  EXPECT_EQ(data1, *empty.data());
  EXPECT_EQ(data1, empty[0]);
  empty.assign(nullptr, 0);
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_TRUE(empty.empty());

  LightVector<int> val1(16);
  EXPECT_TRUE(val1.data() != nullptr);
  EXPECT_EQ(size_t{16u}, val1.size());
  EXPECT_FALSE(val1.empty());

  val1.assign(&data1, size_t{ 1u });
  EXPECT_TRUE(val1.data() != nullptr);
  ASSERT_EQ(size_t{1u}, val1.size());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(data1, *val1.data());
  EXPECT_EQ(data1, val1[0]);
  val1.assign(data2, size_t{ 3u });
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
  val1.assign(&data1, size_t{ 1u });
  EXPECT_TRUE(val1.data() != nullptr);
  ASSERT_EQ(size_t{1u}, val1.size());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(data1, *val1.data());
  EXPECT_EQ(data1, val1[0]);
}

TEST_F(LightVectorTest, vectorAssignClearObject) {
  {//scope
    LightVector<_LightVectorData> empty;
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    empty.clear();
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    CheckObjectRegistrar(true, false);

    _LightVectorData data1{ 5,6 };
    _LightVectorData data2[3] = {_LightVectorData{ 0,1 }, _LightVectorData{ 1,2 }, _LightVectorData{ 3,4 }};
    _LightVectorData data3[6] = {_LightVectorData{ 7,8 }, _LightVectorData{ 6,7 }, _LightVectorData{ 5,6 },
                                 _LightVectorData{ 4,5 }, _LightVectorData{ 3,4 }, _LightVectorData{ 2,3 }};

    empty.assign(&data1, size_t{ 1u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{1u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data1.a, empty.data()->a);
    EXPECT_EQ(data1.b, empty.data()->b);
    EXPECT_EQ(data1.a, empty[0].a);
    EXPECT_EQ(data1.b, empty[0].b);
    CheckObjectRegistrar(false, true);
    empty.assign(data2, size_t{ 3u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{3u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data2[0].a, empty.data()->a);
    EXPECT_EQ(data2[0].b, empty.data()->b);
    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(data2[i].a, empty[i].a);
      EXPECT_EQ(data2[i].b, empty[i].b);
    }
    empty.assign(data3, size_t{ 6u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{6u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data3[0].a, empty.data()->a);
    EXPECT_EQ(data3[0].b, empty.data()->b);
    for (int i = 0; i < 6; ++i) {
      EXPECT_EQ(data3[i].a, empty[i].a);
      EXPECT_EQ(data3[i].b, empty[i].b);
    }
    empty.assign(&data1, size_t{ 1u });
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
    empty.assign(&data1, size_t{ 1u });
    EXPECT_TRUE(empty.data() != nullptr);
    ASSERT_EQ(size_t{1u}, empty.size());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(data1.a, empty.data()->a);
    EXPECT_EQ(data1.b, empty.data()->b);
    EXPECT_EQ(data1.a, empty[0].a);
    EXPECT_EQ(data1.b, empty[0].b);
    empty.assign(nullptr, 0);
    EXPECT_TRUE(empty.data() == nullptr);
    EXPECT_EQ(size_t{0}, empty.size());
    EXPECT_TRUE(empty.empty());
    CheckObjectRegistrar(true, true);

    LightVector<_LightVectorData> val1(16);
    EXPECT_TRUE(val1.data() != nullptr);
    EXPECT_EQ(size_t{16u}, val1.size());
    EXPECT_FALSE(val1.empty());

    val1.assign(&data1, size_t{ 1u });
    EXPECT_TRUE(val1.data() != nullptr);
    ASSERT_EQ(size_t{1u}, val1.size());
    EXPECT_FALSE(val1.empty());
    EXPECT_EQ(data1.a, val1.data()->a);
    EXPECT_EQ(data1.b, val1.data()->b);
    EXPECT_EQ(data1.a, val1[0].a);
    EXPECT_EQ(data1.b, val1[0].b);
    val1.assign(data2, size_t{ 3u });
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
    val1.assign(&data1, size_t{ 1u });
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

TEST_F(LightVectorTest, vectorInsertInt) {
  LightVector<int> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  int item0 = 5; // empty vector
  data.insert(0, item0);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(item0, data[0]);

  int item1 = 2; // end of vector
  data.insert(1, item1);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(item0, data[0]);
  EXPECT_EQ(item1, data[1]);

  int item2 = 3; // before first
  data.insert(0, item2);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(item2, data[0]);
  EXPECT_EQ(item0, data[1]);
  EXPECT_EQ(item1, data[2]);

  int item3 = 7; // after first
  data.insert(1, item3);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(item2, data[0]);
  EXPECT_EQ(item3, data[1]);
  EXPECT_EQ(item0, data[2]);
  EXPECT_EQ(item1, data[3]);

  for (int i = 4; i < 16; ++i) { // realloc
    int itemI = i;
    uint32_t index = (uint32_t)data.size() / 2u;
    auto oldAtIndex = data[index];
    data.insert(index, itemI);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(item2, data[0]);
    EXPECT_EQ(itemI, data[index]);
    EXPECT_EQ(oldAtIndex, data[index + 1u]);
    EXPECT_EQ(item1, data.back());
  }

  int itemN = 42;
  data.insert(0, itemN);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 17u }, data.size());
  EXPECT_EQ(itemN, data[0]);
  EXPECT_EQ(item2, data[1]);
  EXPECT_EQ(item3, data[2]);
  EXPECT_EQ(item1, data.back());

  data.clear();
  data.insert(0, itemN);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(itemN, data[0]);

  LightVector<int> data2(size_t{ 16u });
  EXPECT_FALSE(data2.empty());
  EXPECT_EQ(size_t{ 16u }, data2.size());

  data2.insert(16, itemN);
  EXPECT_FALSE(data2.empty());
  EXPECT_EQ(size_t{ 17u }, data2.size());
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(0, data2[i]);
  }
  EXPECT_EQ(itemN, data2[16]);
  EXPECT_EQ(itemN, data2.back());
}

TEST_F(LightVectorTest, vectorInsertObject) {
  {//scope
    LightVector<_LightVectorData> data;
    EXPECT_TRUE(data.empty());
    EXPECT_EQ(size_t{ 0 }, data.size());
    CheckObjectRegistrar(true, false);

    _LightVectorData item0{ 5,4 }; // empty vector
    data.insert(0, item0);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_EQ(item0.a, data[0].a);
    EXPECT_EQ(item0.b, data[0].b);
    CheckObjectRegistrar(false, true);

    _LightVectorData item1{ 2,1 }; // end of vector
    data.insert(1, item1);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 2u }, data.size());
    EXPECT_EQ(item0.a, data[0].a);
    EXPECT_EQ(item0.b, data[0].b);
    EXPECT_EQ(item1.a, data[1].a);
    EXPECT_EQ(item1.b, data[1].b);

    _LightVectorData item2{ 3,2 }; // before first
    data.insert(0, item2);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 3u }, data.size());
    EXPECT_EQ(item2.a, data[0].a);
    EXPECT_EQ(item2.b, data[0].b);
    EXPECT_EQ(item0.a, data[1].a);
    EXPECT_EQ(item0.b, data[1].b);
    EXPECT_EQ(item1.a, data[2].a);
    EXPECT_EQ(item1.b, data[2].b);

    _LightVectorData item3{ 7,8 }; // after first
    data.insert(1, item3);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 4u }, data.size());
    EXPECT_EQ(item2.a, data[0].a);
    EXPECT_EQ(item2.b, data[0].b);
    EXPECT_EQ(item3.a, data[1].a);
    EXPECT_EQ(item3.b, data[1].b);
    EXPECT_EQ(item0.a, data[2].a);
    EXPECT_EQ(item0.b, data[2].b);
    EXPECT_EQ(item1.a, data[3].a);
    EXPECT_EQ(item1.b, data[3].b);

    for (int i = 4; i < 16; ++i) { // realloc
      _LightVectorData itemI{ i, 16-i };
      uint32_t index = (uint32_t)data.size() / 2u;
      auto oldAtIndex = data[index];
      data.insert(index, itemI);
      EXPECT_FALSE(data.empty());
      EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
      EXPECT_EQ(item2.a, data[0].a);
      EXPECT_EQ(item2.b, data[0].b);
      EXPECT_EQ(itemI.a, data[index].a);
      EXPECT_EQ(itemI.b, data[index].b);
      EXPECT_EQ(oldAtIndex.a, data[index + 1u].a);
      EXPECT_EQ(oldAtIndex.b, data[index + 1u].b);
      EXPECT_EQ(item1.a, data.back().a);
      EXPECT_EQ(item1.b, data.back().b);
    }

    _LightVectorData itemN{ 42,0 };
    data.insert(0, itemN);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 17u }, data.size());
    EXPECT_EQ(itemN.a, data[0].a);
    EXPECT_EQ(itemN.b, data[0].b);
    EXPECT_EQ(item2.a, data[1].a);
    EXPECT_EQ(item2.b, data[1].b);
    EXPECT_EQ(item3.a, data[2].a);
    EXPECT_EQ(item3.b, data[2].b);
    EXPECT_EQ(item1.a, data.back().a);
    EXPECT_EQ(item1.b, data.back().b);

    data.clear();
    data.insert(0, itemN);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_EQ(itemN.a, data[0].a);
    EXPECT_EQ(itemN.b, data[0].b);

    LightVector<_LightVectorData> data2(size_t{ 16u });
    EXPECT_FALSE(data2.empty());
    EXPECT_EQ(size_t{ 16u }, data2.size());

    _LightVectorData emptyVal;
    data2.insert(16, itemN);
    EXPECT_FALSE(data2.empty());
    EXPECT_EQ(size_t{ 17u }, data2.size());
    for (int i = 0; i < 16; ++i) {
      EXPECT_EQ(emptyVal.a, data2[i].a);
      EXPECT_EQ(emptyVal.b, data2[i].b);
    }
    EXPECT_EQ(itemN.a, data2[16].a);
    EXPECT_EQ(itemN.b, data2[16].b);
    EXPECT_EQ(itemN.a, data2.back().a);
    EXPECT_EQ(itemN.b, data2.back().b);
  }
  CheckObjectRegistrar(true, true);
}

TEST_F(LightVectorTest, vectorPushInt) {
  LightVector<int> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  int item0 = 5; // empty vector
  data.push_back(item0);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(item0, data[0]);

  int item1 = 2; // end of vector
  data.push_back(item1);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(item0, data[0]);
  EXPECT_EQ(item1, data[1]);

  for (int i = 2; i < 16; ++i) { // realloc
    int itemI = i;
    data.push_back(itemI);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(item0, data[0]);
    EXPECT_EQ(item1, data[1]);
    EXPECT_EQ(itemI, data[i]);
    EXPECT_EQ(itemI, data.back());
  }

  int itemN = 42;
  data.push_back(itemN);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 17u }, data.size());
  EXPECT_EQ(item0, data[0]);
  EXPECT_EQ(item1, data[1]);
  EXPECT_EQ(itemN, data[16]);
  EXPECT_EQ(itemN, data.back());

  data.clear();
  data.push_back(itemN);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(itemN, data[0]);
}

TEST_F(LightVectorTest, vectorPushObject) {
  {//scope
    LightVector<_LightVectorData> data;
    EXPECT_TRUE(data.empty());
    EXPECT_EQ(size_t{ 0 }, data.size());
    CheckObjectRegistrar(true, false);

    _LightVectorData item0{ 5,4 }; // empty vector
    data.push_back(item0);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_EQ(item0.a, data[0].a);
    EXPECT_EQ(item0.b, data[0].b);
    CheckObjectRegistrar(false, true);

    _LightVectorData item1{ 2,1 }; // end of vector
    data.push_back(item1);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 2u }, data.size());
    EXPECT_EQ(item0.a, data[0].a);
    EXPECT_EQ(item0.b, data[0].b);
    EXPECT_EQ(item1.a, data[1].a);
    EXPECT_EQ(item1.b, data[1].b);

    for (int i = 2; i < 16; ++i) { // realloc
      _LightVectorData itemI{ i, 16-i };
      data.push_back(itemI);
      EXPECT_FALSE(data.empty());
      EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
      EXPECT_EQ(item0.a, data[0].a);
      EXPECT_EQ(item0.b, data[0].b);
      EXPECT_EQ(item1.a, data[1].a);
      EXPECT_EQ(item1.b, data[1].b);
      EXPECT_EQ(itemI.a, data[i].a);
      EXPECT_EQ(itemI.b, data[i].b);
      EXPECT_EQ(itemI.a, data.back().a);
      EXPECT_EQ(itemI.b, data.back().b);
    }

    _LightVectorData itemN{ 42,0 };
    data.push_back(itemN);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 17u }, data.size());
    EXPECT_EQ(item0.a, data[0].a);
    EXPECT_EQ(item0.b, data[0].b);
    EXPECT_EQ(item1.a, data[1].a);
    EXPECT_EQ(item1.b, data[1].b);
    EXPECT_EQ(itemN.a, data[16].a);
    EXPECT_EQ(itemN.b, data[16].b);
    EXPECT_EQ(itemN.a, data.back().a);
    EXPECT_EQ(itemN.b, data.back().b);

    data.clear();
    CheckObjectRegistrar(true, true);
    data.push_back(itemN);
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_EQ(itemN.a, data[0].a);
    EXPECT_EQ(itemN.b, data[0].b);
    CheckObjectRegistrar(false, true);
  }
  CheckObjectRegistrar(true, true);
}

TEST_F(LightVectorTest, vectorEraseInt) {
  LightVector<int> data;
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());

  data.erase(size_t{ 0 });
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());
  data.erase(size_t{ 2u });
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());

  data.push_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_FALSE(data.empty());
  data.erase(size_t{ 1u });
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(42, data.front());
  EXPECT_EQ(42, data.front());
  data.erase(size_t{ 0 });
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());

  data.push_back(22);
  data.push_back(32);
  data.push_back(42);
  data.push_back(52);
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_FALSE(data.empty());
  data.erase(size_t{ 4u });
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_FALSE(data.empty());
  data.erase(size_t{ 3u });
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(22, data.front());
  EXPECT_EQ(42, data.back());
  data.erase(size_t{ 0 });
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(32, data.front());
  EXPECT_EQ(42, data.back());
  data.push_back(52);
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_FALSE(data.empty());
  data.erase(size_t{ 1u });
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(32, data.front());
  EXPECT_EQ(52, data.back());
  data.erase(size_t{ 1u });
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(32, data.front());
  EXPECT_EQ(32, data.back());
  data.clear();
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());
}

TEST_F(LightVectorTest, vectorEraseObject) {
  {//scope
    LightVector<_LightVectorData> data;
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());

    data.erase(size_t{ 0 });
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    data.erase(size_t{ 2u });
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    CheckObjectRegistrar(true, false);

    _LightVectorData item22{ 2,2 };
    _LightVectorData item32{ 3,2 };
    _LightVectorData item42{ 4,2 };
    _LightVectorData item52{ 5,2 };
    data.push_back(item42);
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_FALSE(data.empty());
    data.erase(size_t{ 1u });
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item42.a, data.front().a);
    EXPECT_EQ(item42.b, data.front().b);
    data.erase(size_t{ 0 });
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    CheckObjectRegistrar(true, true);

    data.push_back(item22);
    data.push_back(item32);
    data.push_back(item42);
    data.push_back(item52);
    EXPECT_EQ(size_t{ 4u }, data.size());
    EXPECT_FALSE(data.empty());
    data.erase(size_t{ 4u });
    EXPECT_EQ(size_t{ 4u }, data.size());
    EXPECT_FALSE(data.empty());
    data.erase(size_t{ 3u });
    EXPECT_EQ(size_t{ 3u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item22.a, data.front().a);
    EXPECT_EQ(item42.a, data.back().a);
    data.erase(size_t{ 0 });
    EXPECT_EQ(size_t{ 2u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item32.a, data.front().a);
    EXPECT_EQ(item42.a, data.back().a);
    data.push_back(item52);
    EXPECT_EQ(size_t{ 3u }, data.size());
    EXPECT_FALSE(data.empty());
    data.erase(size_t{ 1u });
    EXPECT_EQ(size_t{ 2u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item32.a, data.front().a);
    EXPECT_EQ(item52.a, data.back().a);
    data.erase(size_t{ 1u });
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item32.a, data.front().a);
    EXPECT_EQ(item32.a, data.back().a);
    data.clear();
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    CheckObjectRegistrar(true, true);
  }
  CheckObjectRegistrar(true, true);
}

TEST_F(LightVectorTest, vectorPopInt) {
  LightVector<int> data;
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());
  data.pop_back();
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());

  data.push_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_FALSE(data.empty());
  data.pop_back();
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());

  data.push_back(22);
  data.push_back(42);
  ASSERT_EQ(size_t{ 2u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(42, data.back());
  data.pop_back();
  ASSERT_EQ(size_t{ 1u }, data.size());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(22, data.back());
  data.pop_back();
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_TRUE(data.empty());
}

TEST_F(LightVectorTest, vectorPopObject) {
  {//scope
    LightVector<_LightVectorData> data;
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    data.pop_back();
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    CheckObjectRegistrar(true, false);

    _LightVectorData item22{ 2,2 };
    _LightVectorData item42{ 4,2 };
    data.push_back(item42);
    EXPECT_EQ(size_t{ 1u }, data.size());
    EXPECT_FALSE(data.empty());
    data.pop_back();
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());

    data.push_back(item22);
    data.push_back(item42);
    ASSERT_EQ(size_t{ 2u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item42.a, data.back().a);
    data.pop_back();
    ASSERT_EQ(size_t{ 1u }, data.size());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(item22.a, data.back().a);
    data.pop_back();
    EXPECT_EQ(size_t{ 0 }, data.size());
    EXPECT_TRUE(data.empty());
    CheckObjectRegistrar(true, true);
  }
  CheckObjectRegistrar(true, true);
}
