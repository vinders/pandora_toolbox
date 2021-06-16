#include <gtest/gtest.h>
#include <vector>
#include <memory/fixed_size_vector.h>
#include "./_fake_classes_helper.h"

using namespace pandora::memory;

class FixedSizeVectorTestOp2 : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


#ifndef _P_CI_DISABLE_SLOW_TESTS

// -- erase operations --

template <typename _DataType, size_t _Size>
void _popLastFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_FALSE(data.pop_back());

  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  data.pop_back();
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(22);
  data.emplace_back(42);
  ASSERT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.back());
  EXPECT_TRUE(data.pop_back());
  ASSERT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(22), data.back());
  EXPECT_TRUE(data.pop_back());
  EXPECT_EQ(size_t{ 0 }, data.size());

  for (int i = 0; i < static_cast<int>(_Size); ++i) {
    data.emplace_back(i);
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
  }

  for (int i = static_cast<int>(_Size); i > 0; --i) {
    EXPECT_TRUE(data.pop_back());
    ASSERT_EQ(static_cast<size_t>(i - 1), data.size());
    if (i > 1) {
      EXPECT_EQ(_DataType(i - 2), data.back());
    }
  }
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_FALSE(data.pop_back());
}
template <typename _DataType, size_t _Size>
void _eraseFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_EQ(size_t{ 0 }, data.size());

  EXPECT_FALSE(data.erase(size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_FALSE(data.erase(_Size * 2u));
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_FALSE(data.erase(_Size * 2u));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_FALSE(data.erase(size_t{ 1u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_TRUE(data.erase(size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(22);
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_FALSE(data.erase(_Size * 2u));
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_FALSE(data.erase(size_t{ 3u }));
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  EXPECT_TRUE(data.erase(size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.emplace_back(52);
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_TRUE(data.erase(size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(52), data.back());
  EXPECT_TRUE(data.erase(size_t{ 1u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(32), data.back());
  data.clear();

  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_EQ(_Size, data.size());
  EXPECT_FALSE(data.erase(_Size));
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_TRUE(data.erase(size_t{ 2u }));
  EXPECT_EQ(_Size - size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.at(1));
  EXPECT_EQ(_DataType(3), data.at(2));
  EXPECT_EQ(_DataType(4), data.at(3));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_TRUE(data.erase(size_t{ 0 }));
  EXPECT_EQ(_Size - size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
}
template <typename _DataType, size_t _Size>
void _eraseCountFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_EQ(size_t{ 0 }, data.size());

  EXPECT_EQ(size_t{ 0 }, data.erase(size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_EQ(size_t{ 0 }, data.erase(_Size * 2u, size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(size_t{ 0 }, data.erase(_Size * 2u, size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(size_t{ 0 }, data.erase(size_t{ 1u }, size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(size_t{ 0 }, data.erase(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, data.size());
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(22);
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(size_t{ 0 }, data.erase(_Size * 2u, size_t{ 2u }));
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(size_t{ 0 }, data.erase(size_t{ 3u }, size_t{ 2u }));
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.emplace_back(52);
  EXPECT_EQ(size_t{ 2u }, data.erase(size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(52), data.front());
  data.emplace_back(62);
  data.emplace_back(72);
  EXPECT_EQ(size_t{ 3u }, data.erase(size_t{ 0 }, size_t{ 3u }));
  EXPECT_EQ(size_t{ 0u }, data.size());
  data.emplace_back(22);
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 1u }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.emplace_back(62);
  EXPECT_EQ(size_t{ 2u }, data.erase(size_t{ 1u }, size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 2u }, data.erase(size_t{ 1u }, size_t{ 3u }));
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 2u }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(32), data.back());

  data.clear();
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(size_t{ 0 }, data.erase(_Size, size_t{ 2u }));
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_EQ(size_t{ 0 }, data.erase(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 2u }, size_t{ 1u }));
  EXPECT_EQ(_Size - size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.at(1));
  EXPECT_EQ(_DataType(3), data.at(2));
  EXPECT_EQ(_DataType(4), data.at(3));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_EQ(size_t{ 2u }, data.erase(size_t{ 2u }, size_t{ 2u }));
  EXPECT_EQ(_Size - size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.at(1));
  EXPECT_EQ(_DataType(5), data.at(2));
  EXPECT_EQ(_DataType(6), data.at(3));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_EQ(size_t{ 1u }, data.erase(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(_Size - size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_EQ(size_t{ 1u }, data.erase(data.size() - 1u, size_t{ 1u }));
  EXPECT_EQ(_Size - size_t{ 5u }, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 2), data.back());
  EXPECT_EQ(size_t{ 2u }, data.erase(data.size() - 2u, size_t{ 3u }));
  EXPECT_EQ(_Size - size_t{ 7u }, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 4), data.back());

  data.clear();
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_EQ(_Size, data.erase(size_t{ 0u }, data.size()));
  EXPECT_EQ(size_t{ 0u }, data.size());

  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_EQ(_Size - 2u, data.erase(size_t{ 2u }, data.size() - 2u));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.back());
}
template <typename _DataType, size_t _Size>
void _eraseIteratorFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_EQ(size_t{ 0 }, data.size());

  auto it = data.erase(data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it = data.erase(data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  it = data.erase(data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  it = data.erase(data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(22);
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 3u }, data.size());
  it = data.erase(data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it = data.erase(data.begin());
  EXPECT_TRUE(it == data.begin());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(32), *it);
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.emplace_back(52);
  EXPECT_EQ(size_t{ 3u }, data.size());
  it = data.erase(data.begin() + 1);
  EXPECT_TRUE(it == data.begin() + 1);
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(52), *it);
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(52), data.back());
  it = data.erase(data.begin() + 1);
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(32), data.back());
  data.clear();

  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_EQ(_Size, data.size());
  it = data.erase(data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  it = data.erase(data.begin() + 2);
  EXPECT_TRUE(it == data.begin() + 2);
  EXPECT_EQ(_Size - size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(3), *it);
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.at(1));
  EXPECT_EQ(_DataType(3), data.at(2));
  EXPECT_EQ(_DataType(4), data.at(3));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  it = data.erase(data.begin());
  EXPECT_TRUE(it == data.begin());
  EXPECT_EQ(_Size - size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(1), *it);
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
}
template <typename _DataType, size_t _Size>
void _eraseIterationFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_EQ(size_t{ 0 }, data.size());

  auto it = data.erase(data.begin(), data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it = data.erase(data.begin(), data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it = data.erase(data.end(), data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  it = data.erase(data.end(), data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  it = data.erase(data.end(), data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  it = data.erase(data.begin(), data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  it = data.erase(data.begin(), data.end());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(22);
  data.emplace_back(32);
  data.emplace_back(42);
  EXPECT_EQ(size_t{ 3u }, data.size());
  it = data.erase(data.begin(), data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it = data.erase(data.end(), data.begin());
  EXPECT_TRUE(it == data.end());
  ASSERT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it = data.erase(data.begin() + 1, data.begin());
  EXPECT_TRUE(it == data.end());
  ASSERT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it = data.erase(data.begin(), data.begin() + 1);
  EXPECT_TRUE(it == data.begin());
  ASSERT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(32), *it);
  EXPECT_EQ(_DataType(32), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.emplace_back(52);
  it = data.erase(data.begin(), data.begin() + 2);
  EXPECT_TRUE(it == data.begin());
  ASSERT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(52), *it);
  EXPECT_EQ(_DataType(52), data.front());
  EXPECT_EQ(_DataType(52), data.back());
  data.emplace_back(62);
  data.emplace_back(72);
  EXPECT_EQ(size_t{ 3u }, data.size());
  it = data.erase(data.begin() + 1, data.end());
  EXPECT_TRUE(it == data.end());
  ASSERT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(52), data.front());
  EXPECT_EQ(_DataType(52), data.back());
  data.clear();

  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_EQ(_Size, data.size());
  it = data.erase(data.begin(), data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  it = data.erase(data.end(), data.begin());
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  it = data.erase(data.begin() + 2, data.begin() + 1);
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  it = data.erase(data.begin() + 2, data.begin() + 4);
  EXPECT_TRUE(it == data.begin() + 2);
  ASSERT_EQ(_Size - size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(4), *it);
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.at(1));
  EXPECT_EQ(_DataType(4), data.at(2));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  it = data.erase(data.begin() + 2, data.end());
  EXPECT_TRUE(it == data.end());
  ASSERT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.back());
}

TEST_F(FixedSizeVectorTestOp2, popLast) {
  _popLastFixedVector<int, 2>();
  _popLastFixedVector<CopyObject, 2>();
  _popLastFixedVector<MoveObject, 2>();
  _popLastFixedVector<CopyMoveObject, 2>();
  _popLastFixedVector<int, 20>();
  _popLastFixedVector<CopyObject, 20>();
  _popLastFixedVector<MoveObject, 20>();
  _popLastFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp2, eraseAt) {
  _eraseFixedVector<int, 20>();
  _eraseFixedVector<CopyObject, 20>();
  _eraseFixedVector<MoveObject, 20>();
  _eraseFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp2, eraseCountAt) {
  _eraseCountFixedVector<int, 20>();
  _eraseCountFixedVector<CopyObject, 20>();
  _eraseCountFixedVector<MoveObject, 20>();
  _eraseCountFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp2, eraseAtIterator) {
  _eraseIteratorFixedVector<int, 20>();
  _eraseIteratorFixedVector<CopyObject, 20>();
  _eraseIteratorFixedVector<MoveObject, 20>();
  _eraseIteratorFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp2, eraseIteration) {
  _eraseIterationFixedVector<int, 20>();
  _eraseIterationFixedVector<CopyObject, 20>();
  _eraseIterationFixedVector<MoveObject, 20>();
  _eraseIterationFixedVector<CopyMoveObject, 20>();

  // out of range end iterator
  FixedSizeVector<int, size_t{ 5u } > data;
  for (int i = 0; i < 5; ++i)
    data.emplace_back(i);
  auto invalidBeginIt = data.begin();
  auto invalidEndIt = data.begin() + 3;
  data.resize(size_t{ 2u }, 0);
  auto it = data.erase(invalidBeginIt, invalidEndIt);
  EXPECT_TRUE(it == data.end());
  EXPECT_EQ(size_t{ 0u }, data.size());
}

#endif
