#include <gtest/gtest.h>
#include <memory/fixed_size_vector.h>
#include "./_fake_classes_helper.h"

using namespace pandora::memory;

class FixedSizeVectorTestData2 : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


#ifndef _P_CI_DISABLE_SLOW_TESTS

// -- iteration --

template <typename _DataType, size_t _Size>
void _iterateEmptyFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  const FixedSizeVector<_DataType, _Size>& cdata{ data };

  auto it = data.begin();
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  ++it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  --it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());

  auto cit = data.cbegin();
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  ++cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  --cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());

  auto crefit = cdata.begin();
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  --crefit;
  EXPECT_TRUE(crefit == data.end());
  EXPECT_FALSE(crefit != data.end());

  auto ccrefit = cdata.cbegin();
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  --ccrefit;
  EXPECT_TRUE(ccrefit == data.cend());
  EXPECT_FALSE(ccrefit != data.cend());
}
template <typename _DataType, size_t _Size>
void _iterateOneInFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  const FixedSizeVector<_DataType, _Size>& cdata{ data };
  data.emplace_back(42);

  auto it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  EXPECT_EQ(_DataType(42), *it);
  EXPECT_EQ(_DataType(42), it.data());
  EXPECT_EQ(0, it.index());
  ++it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());

  auto cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  EXPECT_EQ(_DataType(42), *cit);
  EXPECT_EQ(_DataType(42), cit.data());
  EXPECT_EQ(0, cit.index());
  ++cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());

  auto crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  EXPECT_EQ(_DataType(42), *crefit);
  EXPECT_EQ(_DataType(42), crefit.data());
  EXPECT_EQ(0, crefit.index());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());

  auto ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  EXPECT_EQ(_DataType(42), *ccrefit);
  EXPECT_EQ(_DataType(42), ccrefit.data());
  EXPECT_EQ(0, ccrefit.index());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  data.pop_back();

  it = data.begin();
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  ++it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  it++;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());

  cit = data.cbegin();
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  ++cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  cit++;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());

  crefit = cdata.begin();
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  crefit++;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());

  ccrefit = cdata.cbegin();
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  ccrefit++;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
}
template <typename _DataType, size_t _Size>
void _reverseIterateOneInFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  const FixedSizeVector<_DataType, _Size>& cdata{ data };
  data.emplace_back(42);

  auto it = data.rbegin();
  EXPECT_FALSE(it == data.rend());
  EXPECT_TRUE(it != data.rend());
  EXPECT_EQ(_DataType(42), *it);
  EXPECT_EQ(_DataType(42), it.data());
  EXPECT_EQ(0, it.index());
  ++it;
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());

  auto cit = data.crbegin();
  EXPECT_FALSE(cit == data.crend());
  EXPECT_TRUE(cit != data.crend());
  EXPECT_EQ(_DataType(42), *cit);
  EXPECT_EQ(_DataType(42), cit.data());
  EXPECT_EQ(0, cit.index());
  ++cit;
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());

  auto crefit = cdata.rbegin();
  EXPECT_FALSE(crefit == cdata.rend());
  EXPECT_TRUE(crefit != cdata.rend());
  EXPECT_EQ(_DataType(42), *crefit);
  EXPECT_EQ(_DataType(42), crefit.data());
  EXPECT_EQ(0, crefit.index());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());

  auto ccrefit = cdata.crbegin();
  EXPECT_FALSE(ccrefit == cdata.crend());
  EXPECT_TRUE(ccrefit != cdata.crend());
  EXPECT_EQ(_DataType(42), *ccrefit);
  EXPECT_EQ(_DataType(42), ccrefit.data());
  EXPECT_EQ(0, ccrefit.index());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());

  data.pop_back();

  it = data.rbegin();
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());
  ++it;
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());
  it++;
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());

  cit = data.crbegin();
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());
  ++cit;
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());
  cit++;
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());

  crefit = cdata.rbegin();
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());
  crefit++;
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());

  ccrefit = cdata.crbegin();
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());
  ccrefit++;
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());
}
template <typename _DataType, size_t _Size>
void _iterateMultipleInFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  const FixedSizeVector<_DataType, _Size>& cdata{ data };
  for (int i = 0; i < static_cast<int>(data.capacity()); ++i)
    data.emplace_back(i);

  auto it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  auto cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  auto crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  auto ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());

  auto it2 = it;
  ++it2;
  EXPECT_FALSE(it == it2);
  EXPECT_EQ(_DataType(1), *it2);
  --it2;
  EXPECT_TRUE(it == it2);
  EXPECT_EQ(_DataType(0), *it2);
  --it2;
  EXPECT_FALSE(it == it2);
  EXPECT_TRUE(it2 == data.end());
  auto cit2 = cit;
  ++cit2;
  EXPECT_FALSE(cit == cit2);
  EXPECT_EQ(_DataType(1), *cit2);
  --cit2;
  EXPECT_TRUE(cit == cit2);
  EXPECT_EQ(_DataType(0), *cit2);
  --cit2;
  EXPECT_FALSE(cit == cit2);
  EXPECT_TRUE(cit2 == data.cend());
  auto crefit2 = crefit;
  ++crefit2;
  EXPECT_FALSE(crefit == crefit2);
  EXPECT_EQ(_DataType(1), *crefit2);
  --crefit2;
  EXPECT_TRUE(crefit == crefit2);
  EXPECT_EQ(_DataType(0), *crefit2);
  --crefit2;
  EXPECT_FALSE(crefit == crefit2);
  EXPECT_TRUE(crefit2 == cdata.end());
  auto ccrefit2 = ccrefit;
  ++ccrefit2;
  EXPECT_FALSE(ccrefit == ccrefit2);
  EXPECT_EQ(_DataType(1), *ccrefit2);
  --ccrefit2;
  EXPECT_TRUE(ccrefit == ccrefit2);
  EXPECT_EQ(_DataType(0), *ccrefit2);
  --ccrefit2;
  EXPECT_FALSE(ccrefit == ccrefit2);
  EXPECT_TRUE(ccrefit2 == cdata.cend());

  it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  int index = 0;
  while (it != data.end()) {
    EXPECT_EQ(_DataType(index), *it);
    EXPECT_EQ(_DataType(index), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(index), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(index), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(index), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    ++it;
    ++cit;
    ++crefit;
    ++ccrefit;
    ++index;
  }
  EXPECT_EQ(index, static_cast<int>(data.size()));
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  data.pop_back();

  it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  index = 0;
  while (it != data.end()) {
    EXPECT_EQ(_DataType(index), *it);
    EXPECT_EQ(_DataType(index), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(index), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(index), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(index), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    it++;
    cit++;
    crefit++;
    ccrefit++;
    index++;
  }
  EXPECT_EQ(index, static_cast<int>(data.size()));
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  index = 0;
  while (it != data.end()) {
    EXPECT_EQ(_DataType(index), *it);
    EXPECT_EQ(_DataType(index), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(index), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(index), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(index), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    it += 4;
    cit += 4;
    crefit += 4;
    ccrefit += 4;
    index += 4;
  }
  EXPECT_TRUE(index / 4 == (static_cast<int>(data.size()) + 3) / 4);
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  data.clear();

  it = data.begin();
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  cit = data.cbegin();
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
}
template <typename _DataType, size_t _Size>
void _reverseIterateMultipleInFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  const FixedSizeVector<_DataType, _Size>& cdata{ data };
  for (int i = 0; i < static_cast<int>(data.capacity()); ++i)
    data.emplace_back(i);

  auto it = data.rbegin();
  EXPECT_FALSE(it == data.rend());
  EXPECT_TRUE(it != data.rend());
  auto cit = data.crbegin();
  EXPECT_FALSE(cit == data.crend());
  EXPECT_TRUE(cit != data.crend());
  auto crefit = cdata.rbegin();
  EXPECT_FALSE(crefit == cdata.rend());
  EXPECT_TRUE(crefit != cdata.rend());
  auto ccrefit = cdata.crbegin();
  EXPECT_FALSE(ccrefit == cdata.crend());
  EXPECT_TRUE(ccrefit != cdata.crend());

  auto it2 = it;
  ++it2;
  EXPECT_FALSE(it == it2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 2), *it2);
  --it2;
  EXPECT_TRUE(it == it2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), *it2);
  --it2;
  EXPECT_FALSE(it == it2);
  EXPECT_TRUE(it2 == data.rend());
  auto cit2 = cit;
  ++cit2;
  EXPECT_FALSE(cit == cit2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 2), *cit2);
  --cit2;
  EXPECT_TRUE(cit == cit2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), *cit2);
  --cit2;
  EXPECT_FALSE(cit == cit2);
  EXPECT_TRUE(cit2 == data.crend());
  auto crefit2 = crefit;
  ++crefit2;
  EXPECT_FALSE(crefit == crefit2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 2), *crefit2);
  --crefit2;
  EXPECT_TRUE(crefit == crefit2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), *crefit2);
  --crefit2;
  EXPECT_FALSE(crefit == crefit2);
  EXPECT_TRUE(crefit2 == cdata.rend());
  auto ccrefit2 = ccrefit;
  ++ccrefit2;
  EXPECT_FALSE(ccrefit == ccrefit2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 2), *ccrefit2);
  --ccrefit2;
  EXPECT_TRUE(ccrefit == ccrefit2);
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), *ccrefit2);
  --ccrefit2;
  EXPECT_FALSE(ccrefit == ccrefit2);
  EXPECT_TRUE(ccrefit2 == cdata.crend());

  it = data.rbegin();
  EXPECT_FALSE(it == data.rend());
  EXPECT_TRUE(it != data.rend());
  cit = data.crbegin();
  EXPECT_FALSE(cit == data.crend());
  EXPECT_TRUE(cit != data.crend());
  crefit = cdata.rbegin();
  EXPECT_FALSE(crefit == cdata.rend());
  EXPECT_TRUE(crefit != cdata.rend());
  ccrefit = cdata.crbegin();
  EXPECT_FALSE(ccrefit == cdata.crend());
  EXPECT_TRUE(ccrefit != cdata.crend());
  int index = 0;
  while (it != data.rend()) {
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 1), *it);
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 1), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 1), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 1), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 1), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    ++it;
    ++cit;
    ++crefit;
    ++ccrefit;
    ++index;
  }
  EXPECT_EQ(index, static_cast<int>(data.size()));
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());

  data.pop_back();

  it = data.rbegin();
  EXPECT_FALSE(it == data.rend());
  EXPECT_TRUE(it != data.rend());
  cit = data.crbegin();
  EXPECT_FALSE(cit == data.crend());
  EXPECT_TRUE(cit != data.crend());
  crefit = cdata.rbegin();
  EXPECT_FALSE(crefit == cdata.rend());
  EXPECT_TRUE(crefit != cdata.rend());
  ccrefit = cdata.crbegin();
  EXPECT_FALSE(ccrefit == cdata.crend());
  EXPECT_TRUE(ccrefit != cdata.crend());
  index = 0;
  while (it != data.rend()) {
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), *it);
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    it++;
    cit++;
    crefit++;
    ccrefit++;
    index++;
  }
  EXPECT_EQ(index, static_cast<int>(data.size()));
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());

  it = data.rbegin();
  EXPECT_FALSE(it == data.rend());
  EXPECT_TRUE(it != data.rend());
  cit = data.crbegin();
  EXPECT_FALSE(cit == data.crend());
  EXPECT_TRUE(cit != data.crend());
  crefit = cdata.rbegin();
  EXPECT_FALSE(crefit == cdata.rend());
  EXPECT_TRUE(crefit != cdata.rend());
  ccrefit = cdata.crbegin();
  EXPECT_FALSE(ccrefit == cdata.crend());
  EXPECT_TRUE(ccrefit != cdata.crend());
  index = 0;
  while (it != data.rend()) {
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), *it);
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(static_cast<int>(_Size) - index - 2), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    it += 4;
    cit += 4;
    crefit += 4;
    ccrefit += 4;
    index += 4;
  }
  EXPECT_TRUE(index / 4 == (static_cast<int>(data.size()) + 3) / 4);
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());

  data.clear();

  it = data.rbegin();
  EXPECT_TRUE(it == data.rend());
  EXPECT_FALSE(it != data.rend());
  cit = data.crbegin();
  EXPECT_TRUE(cit == data.crend());
  EXPECT_FALSE(cit != data.crend());
  crefit = cdata.rbegin();
  EXPECT_TRUE(crefit == cdata.rend());
  EXPECT_FALSE(crefit != cdata.rend());
  ccrefit = cdata.crbegin();
  EXPECT_TRUE(ccrefit == cdata.crend());
  EXPECT_FALSE(ccrefit != cdata.crend());
}

TEST_F(FixedSizeVectorTestData2, iterateEmpty) {
  _iterateEmptyFixedVector<int, 1>();
  _iterateEmptyFixedVector<int, 2>();
  _iterateEmptyFixedVector<int, 20>();

  _iterateEmptyFixedVector<CopyObject, 1>();
  _iterateEmptyFixedVector<CopyObject, 2>();
  _iterateEmptyFixedVector<CopyObject, 20>();

  _iterateEmptyFixedVector<MoveObject, 1>();
  _iterateEmptyFixedVector<MoveObject, 2>();
  _iterateEmptyFixedVector<MoveObject, 20>();

  _iterateEmptyFixedVector<CopyMoveObject, 1>();
  _iterateEmptyFixedVector<CopyMoveObject, 2>();
  _iterateEmptyFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData2, iterateOne) {
  _iterateOneInFixedVector<int, 1>();
  _iterateOneInFixedVector<int, 2>();
  _iterateOneInFixedVector<int, 20>();

  _iterateOneInFixedVector<CopyObject, 1>();
  _iterateOneInFixedVector<CopyObject, 2>();
  _iterateOneInFixedVector<CopyObject, 20>();

  _iterateOneInFixedVector<MoveObject, 1>();
  _iterateOneInFixedVector<MoveObject, 2>();
  _iterateOneInFixedVector<MoveObject, 20>();

  _iterateOneInFixedVector<CopyMoveObject, 1>();
  _iterateOneInFixedVector<CopyMoveObject, 2>();
  _iterateOneInFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData2, reverseIterateOne) {
  _reverseIterateOneInFixedVector<int, 1>();
  _reverseIterateOneInFixedVector<int, 2>();
  _reverseIterateOneInFixedVector<int, 20>();

  _reverseIterateOneInFixedVector<CopyObject, 1>();
  _reverseIterateOneInFixedVector<CopyObject, 2>();
  _reverseIterateOneInFixedVector<CopyObject, 20>();

  _reverseIterateOneInFixedVector<MoveObject, 1>();
  _reverseIterateOneInFixedVector<MoveObject, 2>();
  _reverseIterateOneInFixedVector<MoveObject, 20>();

  _reverseIterateOneInFixedVector<CopyMoveObject, 1>();
  _reverseIterateOneInFixedVector<CopyMoveObject, 2>();
  _reverseIterateOneInFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData2, iterateMultiple) {
  _iterateMultipleInFixedVector<int, 2>();
  _iterateMultipleInFixedVector<int, 20>();

  _iterateMultipleInFixedVector<CopyObject, 2>();
  _iterateMultipleInFixedVector<CopyObject, 20>();

  _iterateMultipleInFixedVector<MoveObject, 2>();
  _iterateMultipleInFixedVector<MoveObject, 20>();

  _iterateMultipleInFixedVector<CopyMoveObject, 2>();
  _iterateMultipleInFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData2, reverseIterateMultiple) {
  _reverseIterateMultipleInFixedVector<int, 2>();
  _reverseIterateMultipleInFixedVector<int, 20>();

  _reverseIterateMultipleInFixedVector<CopyObject, 2>();
  _reverseIterateMultipleInFixedVector<CopyObject, 20>();

  _reverseIterateMultipleInFixedVector<MoveObject, 2>();
  _reverseIterateMultipleInFixedVector<MoveObject, 20>();

  _reverseIterateMultipleInFixedVector<CopyMoveObject, 2>();
  _reverseIterateMultipleInFixedVector<CopyMoveObject, 20>();
}

#endif
