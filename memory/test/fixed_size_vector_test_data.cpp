#include <gtest/gtest.h>
#include <memory/fixed_size_vector.h>
#include "./_fake_classes_helper.h"

using namespace pandora::memory;

class FixedSizeVectorTestData : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


#ifndef _P_CI_DISABLE_SLOW_TESTS

// -- accessors/compare --

template <typename _DataType, size_t _Size>
void _testAccessorsFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  const FixedSizeVector<_DataType, _Size>& cdata{ data };
  EXPECT_EQ(size_t{ 0u }, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_THROW(data.at(0), std::out_of_range);
  EXPECT_THROW(cdata.at(0), std::out_of_range);

  data.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(0));
  EXPECT_EQ(_DataType(42), data[0]);
  EXPECT_NE(nullptr, data.data());
  EXPECT_EQ(_DataType(42), data.back());
  EXPECT_THROW(data.at(1u), std::out_of_range);
  EXPECT_EQ(_DataType(42), cdata.front());
  EXPECT_EQ(_DataType(42), cdata.at(0));
  EXPECT_EQ(_DataType(42), cdata[0]);
  EXPECT_NE(nullptr, cdata.data());
  EXPECT_EQ(_DataType(42), cdata.back());
  EXPECT_THROW(cdata.at(1u), std::out_of_range);

  for (int i = 1; i < static_cast<int>(_Size); ++i)
    data.emplace_back(21);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(0));
  EXPECT_EQ(_DataType(42), data[0]);
  EXPECT_NE(nullptr, data.data());
  EXPECT_EQ(_DataType(21), data.back());
  EXPECT_EQ(_DataType(21), data.at(1u));
  EXPECT_EQ(_DataType(21), data[1u]);
  EXPECT_EQ(_DataType(21), data.at(data.size() - 1u));
  EXPECT_EQ(_DataType(21), data[data.size() - 1u]);
  EXPECT_THROW(data.at(data.size()), std::out_of_range);

  for (int i = 1; i < static_cast<int>(_Size); ++i)
    data.pop_back();
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(0));
  EXPECT_EQ(_DataType(42), data[0]);
  EXPECT_NE(nullptr, data.data());
  EXPECT_EQ(_DataType(42), data.back());
  EXPECT_THROW(data.at(1u), std::out_of_range);

  data.emplace_back(64);
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE((_Size == 2u) ? data.full() : !data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(0));
  EXPECT_EQ(_DataType(42), data[0]);
  EXPECT_NE(nullptr, data.data());
  EXPECT_EQ(_DataType(64), data.back());
  EXPECT_EQ(_DataType(64), data.at(1u));
  EXPECT_EQ(_DataType(64), data[1u]);
  EXPECT_THROW(data.at(data.size()), std::out_of_range);

  data.pop_back();
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(0));
  EXPECT_EQ(_DataType(42), data[0]);
  EXPECT_NE(nullptr, data.data());
  EXPECT_EQ(_DataType(42), data.back());
  EXPECT_THROW(data.at(1u), std::out_of_range);

  data.pop_back();
  EXPECT_EQ(size_t{ 0u }, data.size());
  EXPECT_EQ(_Size, data.capacity());
  EXPECT_EQ(_Size, data.max_size());
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_THROW(data.at(0), std::out_of_range);
}
template <typename _DataType, size_t _Size>
void _compareFixedVectors() {
  FixedSizeVector<_DataType, _Size> data;
  FixedSizeVector<_DataType, _Size> data2;
  EXPECT_TRUE(data == data);
  EXPECT_TRUE(data2 == data2);
  EXPECT_TRUE(data == data2);
  EXPECT_FALSE(data != data);
  EXPECT_FALSE(data2 != data2);
  EXPECT_FALSE(data != data2);

  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(i);
  EXPECT_FALSE(data == data2);
  EXPECT_TRUE(data != data2);

  data2.emplace_back(0);
  EXPECT_FALSE(data == data2);
  EXPECT_TRUE(data != data2);
  for (int i = 1; i < static_cast<int>(_Size); ++i)
    data2.emplace_back(i);
  EXPECT_TRUE(data == data2);
  EXPECT_FALSE(data != data2);

  data.clear();
  EXPECT_FALSE(data == data2);
  EXPECT_TRUE(data != data2);
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    data.emplace_back(static_cast<int>(_Size) - i);
  EXPECT_FALSE(data == data2);
  EXPECT_TRUE(data != data2);

  data.clear();
  data2.clear();
  EXPECT_TRUE(data == data2);
  EXPECT_FALSE(data != data2);
}

TEST_F(FixedSizeVectorTestData, accessors) {
  _testAccessorsFixedVector<int, 2>();
  _testAccessorsFixedVector<CopyObject, 2>();
  _testAccessorsFixedVector<MoveObject, 2>();
  _testAccessorsFixedVector<CopyMoveObject, 2>();
  _testAccessorsFixedVector<int, 20>();
  _testAccessorsFixedVector<CopyObject, 20>();
  _testAccessorsFixedVector<MoveObject, 20>();
  _testAccessorsFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData, compare) {
  _compareFixedVectors<int, 2>();
  _compareFixedVectors<CopyObject, 2>();
  _compareFixedVectors<MoveObject, 2>();
  _compareFixedVectors<CopyMoveObject, 2>();
  _compareFixedVectors<int, 20>();
  _compareFixedVectors<CopyObject, 20>();
  _compareFixedVectors<MoveObject, 20>();
  _compareFixedVectors<CopyMoveObject, 20>();
}


// -- constructors/assignments --

template <typename _DataType, size_t _Size>
void _copyEmptyFixedVector() {
  FixedSizeVector<_DataType, _Size> baseType;

  FixedSizeVector<_DataType, _Size> baseTypeCopy(baseType);
  EXPECT_EQ(baseType.size(), baseTypeCopy.size());
  EXPECT_TRUE(baseTypeCopy.empty());
  EXPECT_FALSE(baseTypeCopy.full());
  EXPECT_THROW(baseTypeCopy.at(0), std::out_of_range);

  baseType = baseTypeCopy;
  EXPECT_EQ(baseTypeCopy.size(), baseType.size());
  EXPECT_TRUE(baseType.empty());
  EXPECT_FALSE(baseType.full());
  EXPECT_THROW(baseType.at(0), std::out_of_range);
}
template <typename _DataType, size_t _Size>
void _copyFixedVector() {
  FixedSizeVector<_DataType, _Size> baseType;
  baseType.emplace_back(5);
  baseType.emplace_back(7);

  FixedSizeVector<_DataType, _Size> baseTypeCopy(baseType);
  EXPECT_EQ(baseType.size(), baseTypeCopy.size());
  EXPECT_EQ(baseType.front(), baseTypeCopy.front());
  EXPECT_EQ(baseType.back(), baseTypeCopy.back());

  baseType.clear();
  EXPECT_EQ(size_t{ 0 }, baseType.size());
  baseType = baseTypeCopy;
  EXPECT_EQ(baseTypeCopy.size(), baseType.size());
  EXPECT_EQ(baseTypeCopy.front(), baseType.front());
  EXPECT_EQ(baseTypeCopy.back(), baseType.back());
}
template <typename _DataType, size_t _Size>
void _copyFullFixedVector() {
  FixedSizeVector<_DataType, _Size> baseType;
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    baseType.emplace_back(i);
  baseType.pop_back();
  baseType.emplace_back(static_cast<int>(_Size));

  FixedSizeVector<_DataType, _Size> baseTypeCopy(baseType);
  EXPECT_EQ(baseType.size(), baseTypeCopy.size());
  EXPECT_EQ(baseType.front(), baseTypeCopy.front());
  EXPECT_EQ(baseType.back(), baseTypeCopy.back());
  for (size_t i = 0; i < _Size; ++i) {
    EXPECT_EQ(baseType[i], baseTypeCopy[i]);
  }
  EXPECT_THROW(baseTypeCopy.at(baseTypeCopy.size()), std::out_of_range);

  baseType.clear();
  EXPECT_TRUE(baseType.empty());
  baseType = baseTypeCopy;
  EXPECT_EQ(baseTypeCopy.size(), baseType.size());
  EXPECT_EQ(baseTypeCopy.front(), baseType.front());
  EXPECT_EQ(baseTypeCopy.back(), baseType.back());
  for (size_t i = 0; i < _Size; ++i) {
    EXPECT_EQ(baseTypeCopy[i], baseType[i]);
  }
  EXPECT_THROW(baseType.at(baseType.size()), std::out_of_range);
}
template <typename _DataType, size_t _Size>
void _initListCtorFixedVector() {
  FixedSizeVector<_DataType, _Size> data = { _DataType(5), _DataType(7), _DataType(42) };
  EXPECT_EQ((_Size >= 3) ? size_t{ 3u } : _Size, data.size());
  EXPECT_EQ(_DataType(5), data[0]);
  EXPECT_EQ(_DataType(7), data[1]);
  if (data.size() >= 3) {
    EXPECT_EQ(_DataType(42), data[2]);
  }
}

template <typename _DataType, size_t _Size>
void _moveEmptyFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_TRUE(data.empty());

  FixedSizeVector<_DataType, _Size> movedData(std::move(data));
  EXPECT_TRUE(movedData.empty());
  EXPECT_TRUE(movedData.empty());

  data.clear();
  data = std::move(movedData);
  EXPECT_TRUE(data.empty());
  EXPECT_TRUE(data.empty());
}
template <typename _DataType, size_t _Size>
void _moveFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  data.push_back(_DataType(5));
  data.push_back(_DataType(7));
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(5), data.front());
  EXPECT_EQ(_DataType(7), data.back());

  FixedSizeVector<_DataType, _Size> movedData(std::move(data));
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 2u }, movedData.size());
  EXPECT_EQ(_DataType(5), movedData.front());
  EXPECT_EQ(_DataType(7), movedData.back());

  data = std::move(movedData);
  EXPECT_TRUE(movedData.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(5), data.front());
  EXPECT_EQ(_DataType(7), data.back());
}
template <typename _DataType, size_t _Size>
void _moveFullFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  for (int i = 1; i <= static_cast<int>(_Size); ++i)
    data.push_back(_DataType(i));
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size)), data.back());

  FixedSizeVector<_DataType, _Size> movedData(std::move(data));
  EXPECT_FALSE(movedData.empty());
  EXPECT_TRUE(movedData.full());
  EXPECT_EQ(_Size, movedData.size());
  EXPECT_EQ(_DataType(1), movedData.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size)), movedData.back());

  data = std::move(movedData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size)), data.back());
}

TEST_F(FixedSizeVectorTestData, copyEmpty) {
  _copyEmptyFixedVector<int, 2>();
  _copyEmptyFixedVector<CopyObject, 2>();
  _copyEmptyFixedVector<CopyMoveObject, 2>();
  _copyEmptyFixedVector<int, 20>();
  _copyEmptyFixedVector<CopyObject, 20>();
  _copyEmptyFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData, copyMultiple) {
  _copyFixedVector<int, 2>();
  _copyFixedVector<CopyObject, 2>();
  _copyFixedVector<CopyMoveObject, 2>();
  _copyFixedVector<int, 20>();
  _copyFixedVector<CopyObject, 20>();
  _copyFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData, copyFull) {
  _copyFullFixedVector<int, 2>();
  _copyFullFixedVector<CopyObject, 2>();
  _copyFullFixedVector<CopyMoveObject, 2>();
  _copyFullFixedVector<int, 20>();
  _copyFullFixedVector<CopyObject, 20>();
  _copyFullFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData, initializerListCtor) {
  _initListCtorFixedVector<int, 2>();
  _initListCtorFixedVector<CopyMoveObject, 2>();
  _initListCtorFixedVector<int, 20>();
  _initListCtorFixedVector<CopyMoveObject, 20>();
# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
    _initListCtorFixedVector<CopyObject, 2>();
    _initListCtorFixedVector<CopyObject, 20>();
# endif
}

TEST_F(FixedSizeVectorTestData, moveEmpty) {
  _moveEmptyFixedVector<int, 2>();
  _moveEmptyFixedVector<MoveObject, 2>();
  _moveEmptyFixedVector<CopyMoveObject, 2>();
  _moveEmptyFixedVector<int, 20>();
  _moveEmptyFixedVector<MoveObject, 20>();
  _moveEmptyFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData, moveMultiple) {
  _moveFixedVector<int, 2>();
  _moveFixedVector<MoveObject, 2>();
  _moveFixedVector<CopyMoveObject, 2>();
  _moveFixedVector<int, 20>();
  _moveFixedVector<MoveObject, 20>();
  _moveFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestData, moveFull) {
  _moveFullFixedVector<int, 2>();
  _moveFullFixedVector<MoveObject, 2>();
  _moveFullFixedVector<CopyMoveObject, 2>();
  _moveFullFixedVector<int, 20>();
  _moveFullFixedVector<MoveObject, 20>();
  _moveFullFixedVector<CopyMoveObject, 20>();
}

#else
  
TEST_F(FixedSizeVectorTestData, baseDataTest) {
  FixedSizeVector<int, size_t{ 5u } > data;
  data.emplace_back(0);
  data.emplace_back(1);
  
  FixedSizeVector<int, size_t{ 5u } > copied(data);
  EXPECT_EQ(size_t{ 2u }, copied.size());
  EXPECT_EQ((int)0, copied[0]);
  EXPECT_EQ((int)1, copied[1]);
  
  FixedSizeVector<int, size_t{ 5u } > moved(std::move(data));
  EXPECT_EQ(size_t{ 2u }, moved.size());
  EXPECT_EQ((int)0, moved[0]);
  EXPECT_EQ((int)1, moved[1]);
}

#endif
