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
#ifndef _P_CI_DISABLE_SLOW_TESTS
#include <gtest/gtest.h>
#include <vector>
#include <memory/fixed_size_vector.h>
#include "./_fake_classes_helper.h"

using namespace pandora::memory;

class FixedSizeVectorTestOp : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- assign / swap --

template <typename _DataType, size_t _Size>
void _countRepeatCtorFixedVector() {
  FixedSizeVector<_DataType, _Size> dataEmpty(size_t{ 0 }, _DataType(5));
  FixedSizeVector<_DataType, _Size> dataOne(size_t{ 1u }, _DataType(7));
  FixedSizeVector<_DataType, _Size> data(size_t{ 2u }, _DataType(9));
  FixedSizeVector<_DataType, _Size> dataAlmostFull(_Size - size_t{ 1u }, _DataType(12));
  FixedSizeVector<_DataType, _Size> dataFull(_Size, _DataType(42));
  FixedSizeVector<_DataType, _Size> dataExceed(_Size + size_t{ 1u }, _DataType(64));

  EXPECT_TRUE(dataEmpty.empty());
  EXPECT_EQ(size_t{ 0 }, dataEmpty.size());

  EXPECT_FALSE(dataOne.empty());
  EXPECT_EQ(size_t{ 1u }, dataOne.size());
  EXPECT_EQ(_DataType(7), dataOne.front());
  EXPECT_EQ(_DataType(7), dataOne.back());

  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(9), data.front());
  EXPECT_EQ(_DataType(9), data.back());

  EXPECT_FALSE(dataAlmostFull.empty());
  EXPECT_FALSE(dataAlmostFull.full());
  EXPECT_EQ(_Size - size_t{ 1u }, dataAlmostFull.size());
  EXPECT_EQ(_DataType(12), dataAlmostFull.front());
  EXPECT_EQ(_DataType(12), dataAlmostFull.back());

  EXPECT_FALSE(dataFull.empty());
  EXPECT_TRUE(dataFull.full());
  EXPECT_EQ(_Size, dataFull.size());
  EXPECT_EQ(_DataType(42), dataFull.front());
  EXPECT_EQ(_DataType(42), dataFull.back());

  EXPECT_FALSE(dataExceed.empty());
  EXPECT_TRUE(dataExceed.full());
  EXPECT_EQ(_Size, dataExceed.size());
  EXPECT_EQ(_DataType(64), dataExceed.front());
  EXPECT_EQ(_DataType(64), dataExceed.back());
}
template <typename _DataType, size_t _Size>
void _iterationCtorFixedVector() {
  FixedSizeVector<_DataType, _Size> sourceEmpty;
  FixedSizeVector<_DataType, _Size> sourceSingle1(size_t{ 1u }, _DataType(5));
  FixedSizeVector<_DataType, _Size> source1(size_t{ 2u }, _DataType(7));
  FixedSizeVector<_DataType, _Size> sourceFull1(_Size, _DataType(9));
  FixedSizeVector<_DataType, _Size> sourceExceed1(_Size * 2, _DataType(12));
  std::vector<_DataType> sourceSingle2(size_t{ 1u }, _DataType(5));
  std::vector<_DataType> source2(size_t{ 2u }, _DataType(7));
  std::vector<_DataType> sourceFull2(_Size, _DataType(9));
  std::vector<_DataType> sourceExceed2(_Size * 2, _DataType(12));

  ASSERT_EQ(size_t{ 0u }, std::distance(sourceEmpty.begin(), sourceEmpty.end()));
  FixedSizeVector<_DataType, _Size> dataEmpty(sourceEmpty.begin(), sourceEmpty.end());
  FixedSizeVector<_DataType, _Size> dataEmpty2(sourceFull1.begin() + 1, sourceFull1.begin());
  ASSERT_EQ(size_t{ 1u }, std::distance(sourceSingle1.begin(), sourceSingle1.end()));
  FixedSizeVector<_DataType, _Size> dataSingle1(sourceSingle1.begin(), sourceSingle1.end());
  ASSERT_EQ(size_t{ 2u }, std::distance(source1.begin(), source1.end()));
  FixedSizeVector<_DataType, _Size> data1(source1.begin(), source1.end());
  ASSERT_EQ(_Size, std::distance(sourceFull1.begin(), sourceFull1.end()));
  FixedSizeVector<_DataType, _Size> dataHalf1(sourceFull1.begin() + static_cast<int>(_Size / 4), sourceFull1.begin() + static_cast<int>(_Size * 3 / 4));
  FixedSizeVector<_DataType, _Size> dataFull1(sourceFull1.begin(), sourceFull1.end());
  FixedSizeVector<_DataType, _Size> dataExceed1(sourceExceed1.begin(), sourceExceed1.end());
  FixedSizeVector<_DataType, _Size> dataSingle2(sourceSingle2.begin(), sourceSingle2.end());
  FixedSizeVector<_DataType, _Size> data2(source2.begin(), source2.end());
  FixedSizeVector<_DataType, _Size> dataHalf2(sourceFull2.begin() + static_cast<int>(_Size / 4), sourceFull2.begin() + static_cast<int>(_Size * 3 / 4));
  FixedSizeVector<_DataType, _Size> dataFull2(sourceFull2.begin(), sourceFull2.end());
  FixedSizeVector<_DataType, _Size> dataExceed2(sourceExceed2.begin(), sourceExceed2.end());

  EXPECT_TRUE(dataEmpty.empty());
  EXPECT_EQ(size_t{ 0 }, dataEmpty.size());
  EXPECT_TRUE(dataEmpty2.empty());
  EXPECT_EQ(size_t{ 0 }, dataEmpty2.size());

  EXPECT_FALSE(dataSingle1.empty());
  EXPECT_EQ(size_t{ 1u }, dataSingle1.size());
  EXPECT_EQ(_DataType(5), dataSingle1.front());
  EXPECT_EQ(_DataType(5), dataSingle1.back());

  EXPECT_FALSE(data1.empty());
  EXPECT_EQ(size_t{ 2u }, data1.size());
  EXPECT_EQ(_DataType(7), data1.front());
  EXPECT_EQ(_DataType(7), data1.back());

  EXPECT_FALSE(dataHalf1.empty());
  EXPECT_EQ(_Size / 2, dataHalf1.size());
  EXPECT_EQ(_DataType(9), dataHalf1.front());
  EXPECT_EQ(_DataType(9), dataHalf1.back());

  EXPECT_FALSE(dataFull1.empty());
  EXPECT_EQ(_Size, dataFull1.size());
  EXPECT_EQ(_DataType(9), dataFull1.front());
  EXPECT_EQ(_DataType(9), dataFull1.back());

  EXPECT_FALSE(dataExceed1.empty());
  EXPECT_EQ(_Size, dataExceed1.size());
  EXPECT_EQ(_DataType(12), dataExceed1.front());
  EXPECT_EQ(_DataType(12), dataExceed1.back());

  EXPECT_FALSE(dataSingle2.empty());
  EXPECT_EQ(size_t{ 1u }, dataSingle2.size());
  EXPECT_EQ(_DataType(5), dataSingle2.front());
  EXPECT_EQ(_DataType(5), dataSingle2.back());

  EXPECT_FALSE(data2.empty());
  EXPECT_EQ(size_t{ 2u }, data2.size());
  EXPECT_EQ(_DataType(7), data2.front());
  EXPECT_EQ(_DataType(7), data2.back());

  EXPECT_FALSE(dataHalf2.empty());
  EXPECT_EQ(_Size / 2, dataHalf2.size());
  EXPECT_EQ(_DataType(9), dataHalf2.front());
  EXPECT_EQ(_DataType(9), dataHalf2.back());

  EXPECT_FALSE(dataFull2.empty());
  EXPECT_EQ(_Size, dataFull2.size());
  EXPECT_EQ(_DataType(9), dataFull2.front());
  EXPECT_EQ(_DataType(9), dataFull2.back());

  EXPECT_FALSE(dataExceed2.empty());
  EXPECT_EQ(_Size, dataExceed2.size());
  EXPECT_EQ(_DataType(12), dataExceed2.front());
  EXPECT_EQ(_DataType(12), dataExceed2.back());
}
template <typename _DataType, size_t _Size>
void _countRepeatAssignFixedVector() {
  FixedSizeVector<_DataType, _Size> dataEmpty;
  FixedSizeVector<_DataType, _Size> dataOne;
  FixedSizeVector<_DataType, _Size> data;
  FixedSizeVector<_DataType, _Size> dataAlmostFull;
  FixedSizeVector<_DataType, _Size> dataFull;
  FixedSizeVector<_DataType, _Size> dataExceed;
  EXPECT_TRUE(dataEmpty.empty());
  EXPECT_TRUE(dataOne.empty());
  EXPECT_TRUE(data.empty());
  EXPECT_TRUE(dataAlmostFull.empty());
  EXPECT_TRUE(dataFull.empty());
  EXPECT_TRUE(dataExceed.empty());

  dataEmpty.assign(size_t{ 0 }, _DataType(5));
  dataOne.assign(size_t{ 1u }, _DataType(7));
  data.assign(size_t{ 2u }, _DataType(9));
  dataAlmostFull.assign(_Size - size_t{ 1u }, _DataType(12));
  dataFull.assign(_Size, _DataType(42));
  dataExceed.assign(_Size + size_t{ 1u }, _DataType(64));

  EXPECT_TRUE(dataEmpty.empty());
  EXPECT_EQ(size_t{ 0 }, dataEmpty.size());

  EXPECT_FALSE(dataOne.empty());
  EXPECT_EQ(size_t{ 1u }, dataOne.size());
  EXPECT_EQ(_DataType(7), dataOne.front());
  EXPECT_EQ(_DataType(7), dataOne.back());

  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(9), data.front());
  EXPECT_EQ(_DataType(9), data.back());

  EXPECT_FALSE(dataAlmostFull.empty());
  EXPECT_FALSE(dataAlmostFull.full());
  EXPECT_EQ(_Size - size_t{ 1u }, dataAlmostFull.size());
  EXPECT_EQ(_DataType(12), dataAlmostFull.front());
  EXPECT_EQ(_DataType(12), dataAlmostFull.back());

  EXPECT_FALSE(dataFull.empty());
  EXPECT_TRUE(dataFull.full());
  EXPECT_EQ(_Size, dataFull.size());
  EXPECT_EQ(_DataType(42), dataFull.front());
  EXPECT_EQ(_DataType(42), dataFull.back());

  EXPECT_FALSE(dataExceed.empty());
  EXPECT_TRUE(dataExceed.full());
  EXPECT_EQ(_Size, dataExceed.size());
  EXPECT_EQ(_DataType(64), dataExceed.front());
  EXPECT_EQ(_DataType(64), dataExceed.back());
}
template <typename _DataType, size_t _Size>
void _iterationAssignFixedVector() {
  FixedSizeVector<_DataType, _Size> sourceEmpty;
  FixedSizeVector<_DataType, _Size> sourceSingle1(size_t{ 1u }, _DataType(5));
  FixedSizeVector<_DataType, _Size> source1(size_t{ 2u }, _DataType(7));
  FixedSizeVector<_DataType, _Size> sourceFull1(_Size, _DataType(9));
  FixedSizeVector<_DataType, _Size> sourceExceed1(_Size * 2, _DataType(12));
  std::vector<_DataType> sourceSingle2(size_t{ 1u }, _DataType(5));
  std::vector<_DataType> source2(size_t{ 2u }, _DataType(7));
  std::vector<_DataType> sourceFull2(_Size, _DataType(9));
  std::vector<_DataType> sourceExceed2(_Size * 2, _DataType(12));

  FixedSizeVector<_DataType, _Size> dataEmpty;
  FixedSizeVector<_DataType, _Size> dataSingle1;
  FixedSizeVector<_DataType, _Size> data1;
  FixedSizeVector<_DataType, _Size> dataHalf1;
  FixedSizeVector<_DataType, _Size> dataFull1;
  FixedSizeVector<_DataType, _Size> dataExceed1;
  EXPECT_TRUE(dataEmpty.empty());
  EXPECT_TRUE(dataSingle1.empty());
  EXPECT_TRUE(data1.empty());
  EXPECT_TRUE(dataHalf1.empty());
  EXPECT_TRUE(dataFull1.empty());
  EXPECT_TRUE(dataExceed1.empty());
  FixedSizeVector<_DataType, _Size> dataSingle2;
  FixedSizeVector<_DataType, _Size> data2;
  FixedSizeVector<_DataType, _Size> dataHalf2;
  FixedSizeVector<_DataType, _Size> dataFull2;
  FixedSizeVector<_DataType, _Size> dataExceed2;
  EXPECT_TRUE(dataSingle2.empty());
  EXPECT_TRUE(data2.empty());
  EXPECT_TRUE(dataHalf2.empty());
  EXPECT_TRUE(dataFull2.empty());
  EXPECT_TRUE(dataExceed2.empty());

  ASSERT_EQ(size_t{ 0u }, std::distance(sourceEmpty.begin(), sourceEmpty.end()));
  dataEmpty.assign(sourceEmpty.begin(), sourceEmpty.end());
  ASSERT_EQ(size_t{ 1u }, std::distance(sourceSingle1.begin(), sourceSingle1.end()));
  dataSingle1.assign(sourceSingle1.begin(), sourceSingle1.end());
  ASSERT_EQ(size_t{ 2u }, std::distance(source1.begin(), source1.end()));
  data1.assign(source1.begin(), source1.end());
  ASSERT_EQ(_Size, std::distance(sourceFull1.begin(), sourceFull1.end()));
  dataHalf1.assign(sourceFull1.begin() + static_cast<int>(_Size / 4), sourceFull1.begin() + static_cast<int>(_Size * 3 / 4));
  dataFull1.assign(sourceFull1.begin(), sourceFull1.end());
  dataExceed1.assign(sourceExceed1.begin(), sourceExceed1.end());
  dataSingle2.assign(sourceSingle2.begin(), sourceSingle2.end());
  data2.assign(source2.begin(), source2.end());
  dataHalf2.assign(sourceFull2.begin() + static_cast<int>(_Size / 4), sourceFull2.begin() + static_cast<int>(_Size * 3 / 4));
  dataFull2.assign(sourceFull2.begin(), sourceFull2.end());
  dataExceed2.assign(sourceExceed2.begin(), sourceExceed2.end());

  EXPECT_TRUE(dataEmpty.empty());
  EXPECT_EQ(size_t{ 0 }, dataEmpty.size());

  EXPECT_FALSE(dataSingle1.empty());
  EXPECT_EQ(size_t{ 1u }, dataSingle1.size());
  EXPECT_EQ(_DataType(5), dataSingle1.front());
  EXPECT_EQ(_DataType(5), dataSingle1.back());

  EXPECT_FALSE(data1.empty());
  EXPECT_EQ(size_t{ 2u }, data1.size());
  EXPECT_EQ(_DataType(7), data1.front());
  EXPECT_EQ(_DataType(7), data1.back());

  EXPECT_FALSE(dataHalf1.empty());
  EXPECT_EQ(_Size / 2, dataHalf1.size());
  EXPECT_EQ(_DataType(9), dataHalf1.front());
  EXPECT_EQ(_DataType(9), dataHalf1.back());

  EXPECT_FALSE(dataFull1.empty());
  EXPECT_EQ(_Size, dataFull1.size());
  EXPECT_EQ(_DataType(9), dataFull1.front());
  EXPECT_EQ(_DataType(9), dataFull1.back());

  EXPECT_FALSE(dataExceed1.empty());
  EXPECT_EQ(_Size, dataExceed1.size());
  EXPECT_EQ(_DataType(12), dataExceed1.front());
  EXPECT_EQ(_DataType(12), dataExceed1.back());

  EXPECT_FALSE(dataSingle2.empty());
  EXPECT_EQ(size_t{ 1u }, dataSingle2.size());
  EXPECT_EQ(_DataType(5), dataSingle2.front());
  EXPECT_EQ(_DataType(5), dataSingle2.back());

  EXPECT_FALSE(data2.empty());
  EXPECT_EQ(size_t{ 2u }, data2.size());
  EXPECT_EQ(_DataType(7), data2.front());
  EXPECT_EQ(_DataType(7), data2.back());

  EXPECT_FALSE(dataHalf2.empty());
  EXPECT_EQ(_Size / 2, dataHalf2.size());
  EXPECT_EQ(_DataType(9), dataHalf2.front());
  EXPECT_EQ(_DataType(9), dataHalf2.back());

  EXPECT_FALSE(dataFull2.empty());
  EXPECT_EQ(_Size, dataFull2.size());
  EXPECT_EQ(_DataType(9), dataFull2.front());
  EXPECT_EQ(_DataType(9), dataFull2.back());

  EXPECT_FALSE(dataExceed2.empty());
  EXPECT_EQ(_Size, dataExceed2.size());
  EXPECT_EQ(_DataType(12), dataExceed2.front());
  EXPECT_EQ(_DataType(12), dataExceed2.back());
}

template <typename _DataType, size_t _Size>
void _swapEmptyFixedVectors() {
  FixedSizeVector<_DataType, _Size> data;
  FixedSizeVector<_DataType, _Size> data2;

  data.swap(data2);
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_EQ(size_t{ 0 }, data2.size());
  data2.swap(data);
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_EQ(size_t{ 0 }, data2.size());
}
template <typename _DataType, size_t _Size>
void _swapEmptyAndNotFixedVectors() {
  FixedSizeVector<_DataType, _Size> data;
  FixedSizeVector<_DataType, _Size> data2;
  data.emplace_back(5);
  data.emplace_back(7);

  data.swap(data2);
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_EQ(size_t{ 2u }, data2.size());
  EXPECT_EQ(_DataType(5), data2.front());
  EXPECT_EQ(_DataType(7), data2.back());
  data2.swap(data);
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(5), data.front());
  EXPECT_EQ(_DataType(7), data.back());
  EXPECT_EQ(size_t{ 0 }, data2.size());
}
template <typename _DataType, size_t _Size>
void _swapFixedVectors() {
  FixedSizeVector<_DataType, _Size> data;
  FixedSizeVector<_DataType, _Size> data2;
  data.emplace_back(5);
  data.emplace_back(7);
  data2.emplace_back(9);

  data.swap(data2);
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(9), data.front());
  EXPECT_EQ(_DataType(9), data.back());
  EXPECT_EQ(size_t{ 2u }, data2.size());
  EXPECT_EQ(_DataType(5), data2.front());
  EXPECT_EQ(_DataType(7), data2.back());
  data2.swap(data);
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(5), data.front());
  EXPECT_EQ(_DataType(7), data.back());
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_EQ(_DataType(9), data2.front());
  EXPECT_EQ(_DataType(9), data2.back());
}
template <typename _DataType, size_t _Size>
void _swapFullFixedVectors() {
  FixedSizeVector<_DataType, _Size> data;
  FixedSizeVector<_DataType, _Size> data2;
  for (int i = 0; i < static_cast<int>(_Size); ++i) {
    data.emplace_back(i);
    data2.emplace_back(i + 1);
  }

  data.swap(data2);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(1), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size)), data.back());
  EXPECT_EQ(_Size, data2.size());
  EXPECT_EQ(_DataType(0), data2.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data2.back());
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  data2.swap(data);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  EXPECT_EQ(_Size, data2.size());
  EXPECT_EQ(_DataType(1), data2.front());
  EXPECT_EQ(_DataType(static_cast<int>(_Size)), data2.back());
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
}

TEST_F(FixedSizeVectorTestOp, countRepeatCtor) {
  _countRepeatCtorFixedVector<int, 2>();
  _countRepeatCtorFixedVector<CopyObject, 2>();
  _countRepeatCtorFixedVector<CopyMoveObject, 2>();
  _countRepeatCtorFixedVector<int, 20>();
  _countRepeatCtorFixedVector<CopyObject, 20>();
  _countRepeatCtorFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, iterationCtor) {
  _iterationCtorFixedVector<int, 2>();
  _iterationCtorFixedVector<CopyObject, 2>();
  _iterationCtorFixedVector<CopyMoveObject, 2>();
  _iterationCtorFixedVector<int, 20>();
  _iterationCtorFixedVector<CopyObject, 20>();
  _iterationCtorFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, countRepeatAssign) {
  _countRepeatAssignFixedVector<int, 2>();
  _countRepeatAssignFixedVector<CopyObject, 2>();
  _countRepeatAssignFixedVector<CopyMoveObject, 2>();
  _countRepeatAssignFixedVector<int, 20>();
  _countRepeatAssignFixedVector<CopyObject, 20>();
  _countRepeatAssignFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, iterationAssign) {
  _iterationAssignFixedVector<int, 2>();
  _iterationAssignFixedVector<CopyObject, 2>();
  _iterationAssignFixedVector<CopyMoveObject, 2>();
  _iterationAssignFixedVector<int, 20>();
  _iterationAssignFixedVector<CopyObject, 20>();
  _iterationAssignFixedVector<CopyMoveObject, 20>();
}

TEST_F(FixedSizeVectorTestOp, swapEmpty) {
  _swapEmptyFixedVectors<int, 2>();
  _swapEmptyFixedVectors<CopyObject, 2>();
  _swapEmptyFixedVectors<MoveObject, 2>();
  _swapEmptyFixedVectors<CopyMoveObject, 2>();
  _swapEmptyFixedVectors<int, 20>();
  _swapEmptyFixedVectors<CopyObject, 20>();
  _swapEmptyFixedVectors<MoveObject, 20>();
  _swapEmptyFixedVectors<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, swapEmptyAndNonEmpty) {
  _swapEmptyAndNotFixedVectors<int, 2>();
  _swapEmptyAndNotFixedVectors<CopyObject, 2>();
  _swapEmptyAndNotFixedVectors<MoveObject, 2>();
  _swapEmptyAndNotFixedVectors<CopyMoveObject, 2>();
  _swapEmptyAndNotFixedVectors<int, 20>();
  _swapEmptyAndNotFixedVectors<CopyObject, 20>();
  _swapEmptyAndNotFixedVectors<MoveObject, 20>();
  _swapEmptyAndNotFixedVectors<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, swapNormal) {
  _swapFixedVectors<int, 2>();
  _swapFixedVectors<CopyObject, 2>();
  _swapFixedVectors<MoveObject, 2>();
  _swapFixedVectors<CopyMoveObject, 2>();
  _swapFixedVectors<int, 20>();
  _swapFixedVectors<CopyObject, 20>();
  _swapFixedVectors<MoveObject, 20>();
  _swapFixedVectors<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, swapFull) {
  _swapFullFixedVectors<int, 2>();
  _swapFullFixedVectors<CopyObject, 2>();
  _swapFullFixedVectors<MoveObject, 2>();
  _swapFullFixedVectors<CopyMoveObject, 2>();
  _swapFullFixedVectors<int, 20>();
  _swapFullFixedVectors<CopyObject, 20>();
  _swapFullFixedVectors<MoveObject, 20>();
  _swapFullFixedVectors<CopyMoveObject, 20>();
}


// -- add operations --

template <typename _DataType, size_t _Size>
void _clearResizeFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  data.emplace_back(5);
  data.emplace_back(7);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());

  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());
  data.resize(size_t{ 0 }, _DataType(42));
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.resize(_Size - static_cast<size_t>(1u), _DataType(42));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_Size - static_cast<size_t>(1u), data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.resize(_Size - static_cast<size_t>(1u), _DataType(64));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_Size - static_cast<size_t>(1u), data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.resize(_Size, _DataType(64));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(64), data.back());
  data.resize(_Size - static_cast<size_t>(1u), _DataType(64));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_Size - static_cast<size_t>(1u), data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());

  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.emplace_back(5);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  data.resize(_Size, _DataType(64));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(5), data.front());
  EXPECT_EQ(_DataType(64), data.back());
  data.resize(_Size * 2, _DataType(42));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(5), data.front());
  EXPECT_EQ(_DataType(64), data.back());

  data.resize(size_t{ 0 }, _DataType(42));
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());
  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());
}

template <typename _DataType, size_t _Size>
void _pushCopyFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  _DataType item0(0);
  EXPECT_TRUE(data.push_back(item0));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(item0, data.front());
  EXPECT_EQ(item0, data.back());

  for (int i = 1; i < static_cast<int>(_Size); ++i) {
    _DataType item(i);
    EXPECT_TRUE(data.push_back(item));
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(item0, data.front());
    EXPECT_EQ(item, data.back());
  }
  ASSERT_EQ(_Size, data.size());
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    EXPECT_EQ(_DataType(i), data[i]);

  EXPECT_FALSE(data.push_back(item0));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  data.pop_back();
  EXPECT_TRUE(data.push_back(item0));
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(item0, data.front());
  EXPECT_EQ(item0, data.back());
}
template <typename _DataType, size_t _Size>
void _pushMoveFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  _DataType item0(0);
  EXPECT_TRUE(data.push_back(std::move(item0)));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());

  for (int i = 1; i < static_cast<int>(_Size); ++i) {
    _DataType item(i);
    EXPECT_TRUE(data.push_back(std::move(item)));
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(_DataType(0), data.front());
    EXPECT_EQ(_DataType(i), data.back());
  }
  ASSERT_EQ(_Size, data.size());
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    EXPECT_EQ(_DataType(i), data[i]);

  EXPECT_FALSE(data.push_back(_DataType(0)));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  data.pop_back();
  EXPECT_TRUE(data.push_back(_DataType(0)));
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());
}
template <typename _DataType, size_t _Size>
void _emplaceFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  EXPECT_TRUE(data.emplace_back(0));
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());

  for (int i = 1; i < static_cast<int>(_Size); ++i) {
    EXPECT_TRUE(data.emplace_back(i));
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(_DataType(0), data.front());
    EXPECT_EQ(_DataType(i), data.back());
  }
  ASSERT_EQ(_Size, data.size());
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    EXPECT_EQ(_DataType(i), data[i]);

  EXPECT_FALSE(data.emplace_back(0));
  EXPECT_EQ(_DataType(static_cast<int>(_Size) - 1), data.back());
  data.pop_back();
  EXPECT_TRUE(data.emplace_back(0));
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());
}

template <typename _DataType, size_t _Size>
void _insertCopyFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  _DataType item0(0); // empty vector
  auto it0 = data.insert(data.begin(), item0);
  EXPECT_TRUE(it0 != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), *it0);
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());

  _DataType item1(1); // end of vector
  EXPECT_TRUE(data.insert(data.end(), item1) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.back());

  _DataType item2(2); // before first
  EXPECT_TRUE(data.insert(data.begin(), item2) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(2), data.front());
  EXPECT_EQ(_DataType(0), data.at(1));
  EXPECT_EQ(_DataType(1), data.back());

  _DataType item3(3); // after first
  EXPECT_TRUE(data.insert(data.begin() + 1, item3) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(2), data.front());
  EXPECT_EQ(_DataType(3), data.at(1));
  EXPECT_EQ(_DataType(0), data.at(2));
  EXPECT_EQ(_DataType(1), data.back());

  for (int i = 4; i < static_cast<int>(_Size); ++i) { // fill
    _DataType item(i);
    size_t index = data.size() / 2u;
    auto oldAtIndex = data.at(index);
    auto it = data.insert(data.begin() + index, item);
    EXPECT_TRUE(it != data.end());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(_DataType(2), data.front());
    EXPECT_EQ(_DataType(i), *it);
    EXPECT_EQ(_DataType(i), data.at(index));
    EXPECT_EQ(oldAtIndex, data.at(index + 1u));
    EXPECT_EQ(_DataType(1), data.back());
  }

  _DataType itemN(42);
  EXPECT_FALSE(data.insert(data.begin(), itemN) != data.end());
  data.pop_back();
  EXPECT_TRUE(data.insert(data.begin(), itemN) != data.end());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(2), data.at(1));
  EXPECT_EQ(_DataType(3), data.at(2));
  EXPECT_EQ(_DataType(0), data.back());
}
template <typename _DataType, size_t _Size>
void _insertMoveFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  _DataType item0(0); // empty vector
  auto it0 = data.insert(data.begin(), std::move(item0));
  EXPECT_TRUE(it0 != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), *it0);
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());

  _DataType item1(1); // end of vector
  EXPECT_TRUE(data.insert(data.end(), std::move(item1)) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.back());

  _DataType item2(2); // before first
  EXPECT_TRUE(data.insert(data.begin(), std::move(item2)) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(2), data.front());
  EXPECT_EQ(_DataType(0), data.at(1));
  EXPECT_EQ(_DataType(1), data.back());

  _DataType item3(3); // after first
  EXPECT_TRUE(data.insert(data.begin() + 1, std::move(item3)) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(2), data.front());
  EXPECT_EQ(_DataType(3), data.at(1));
  EXPECT_EQ(_DataType(0), data.at(2));
  EXPECT_EQ(_DataType(1), data.back());

  for (int i = 4; i < static_cast<int>(_Size); ++i) { // fill
    _DataType item(i);
    size_t index = data.size() / 2u;
    auto it = data.insert(data.begin() + index, std::move(item));
    EXPECT_TRUE(it != data.end());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(_DataType(2), data.front());
    EXPECT_EQ(_DataType(i), *it);
    EXPECT_EQ(_DataType(i), data.at(index));
    EXPECT_EQ(_DataType(1), data.back());
  }

  _DataType itemN(42);
  EXPECT_FALSE(data.insert(data.begin(), std::move(itemN)) != data.end());
  data.pop_back();
  _DataType itemN2(42);
  EXPECT_TRUE(data.insert(data.begin(), std::move(itemN2)) != data.end());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(2), data.at(1));
  EXPECT_EQ(_DataType(3), data.at(2));
  EXPECT_EQ(_DataType(0), data.back());
}
template <typename _DataType, size_t _Size>
void _insertEmplaceFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  auto it0 = data.emplace(data.begin(), 0);
  EXPECT_TRUE(it0 != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(0), *it0);
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(0), data.back());

  EXPECT_TRUE(data.emplace(data.end(), 1) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(0), data.front());
  EXPECT_EQ(_DataType(1), data.back());

  EXPECT_TRUE(data.emplace(data.begin(), 2) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(2), data.front());
  EXPECT_EQ(_DataType(0), data.at(1));
  EXPECT_EQ(_DataType(1), data.back());

  EXPECT_TRUE(data.emplace(data.begin() + 1, 3) != data.end());
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(2), data.front());
  EXPECT_EQ(_DataType(3), data.at(1));
  EXPECT_EQ(_DataType(0), data.at(2));
  EXPECT_EQ(_DataType(1), data.back());

  for (int i = 4; i < static_cast<int>(_Size); ++i) { // fill
    size_t index = data.size() / 2u;
    auto it = data.emplace(data.begin() + index, i);
    EXPECT_TRUE(it != data.end());
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(static_cast<size_t>(i + 1), data.size());
    EXPECT_EQ(_DataType(2), data.front());
    EXPECT_EQ(_DataType(i), *it);
    EXPECT_EQ(_DataType(i), data.at(index));
    EXPECT_EQ(_DataType(1), data.back());
  }

  EXPECT_FALSE(data.emplace(data.begin(), 42) != data.end());
  data.pop_back();
  EXPECT_TRUE(data.emplace(data.begin(), 42) != data.end());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(2), data.at(1));
  EXPECT_EQ(_DataType(3), data.at(2));
  EXPECT_EQ(_DataType(0), data.back());
}

template <typename _DataType, size_t _Size>
void _insertCountRepeatFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  // empty vector
  auto it0 = data.insert(data.begin(), size_t{ 0 }, _DataType(42)); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.end(), size_t{ 0 }, _DataType(42));
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.begin(), size_t{ 1u }, _DataType(42)); // add some
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  it0 = data.insert(data.end(), size_t{ 2u }, _DataType(42));
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  it0 = data.insert(data.begin(), _Size, _DataType(42)); // add max
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  for (int i = 0; i < static_cast<int>(_Size); ++i) {
    EXPECT_EQ(_DataType(42), data.at(i));
  }
  data.clear();
  it0 = data.insert(data.end(), _Size, _DataType(42));
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  it0 = data.insert(data.begin(), _Size * 2u, _DataType(42)); // add excess
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  it0 = data.insert(data.end(), _Size * 2u, _DataType(42));
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());

  // full vector
  EXPECT_TRUE(data.full());
  it0 = data.insert(data.begin(), size_t{ 0 }, _DataType(64)); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.end(), size_t{ 0 }, _DataType(64));
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin(), size_t{ 1u }, _DataType(64)); // add some
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.end(), size_t{ 2u }, _DataType(64));
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin(), _Size, _DataType(64)); // add max
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.end(), _Size, _DataType(64));
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());

  // partially full vector
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin(), size_t{ 0 }, _DataType(64)); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin() + 1, size_t{ 0 }, _DataType(64));
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.end(), size_t{ 0 }, _DataType(64));
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin(), size_t{ 1u }, _DataType(64)); // add some
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(64), *it0);
  EXPECT_EQ(_DataType(64), data.front());
  EXPECT_EQ(_DataType(42), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(42), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, size_t{ 2u }, _DataType(32));
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(32), *it0);
  EXPECT_EQ(_DataType(64), data.front());
  EXPECT_EQ(_DataType(32), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(32), data.at(size_t{ 2u }));
  EXPECT_EQ(_DataType(42), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.end(), size_t{ 1u }, _DataType(28));
  EXPECT_TRUE(it0 == data.begin() + 2);
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(_DataType(28), *it0);
  EXPECT_EQ(_DataType(64), data.front());
  EXPECT_EQ(_DataType(32), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(28), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin(), _Size - 2u, _DataType(28)); // fill
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(28), *it0);
  EXPECT_EQ(_DataType(28), data.front());
  EXPECT_EQ(_DataType(28), data.at(data.size() - 3u));
  EXPECT_EQ(_DataType(64), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(32), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, _Size - 2u, _DataType(24));
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(24), *it0);
  EXPECT_EQ(_DataType(28), data.front());
  EXPECT_EQ(_DataType(24), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(24), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(28), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.end(), _Size - 2u, _DataType(64));
  EXPECT_TRUE(it0 == data.begin() + 2);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(64), *it0);
  EXPECT_EQ(_DataType(28), data.front());
  EXPECT_EQ(_DataType(24), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(64), data.at(size_t{ 2u }));
  EXPECT_EQ(_DataType(64), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin(), _Size, _DataType(42)); // add max
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(data.size() - 3u));
  EXPECT_EQ(_DataType(28), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(24), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, _Size, _DataType(64));
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(64), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(64), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(64), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(42), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.end(), _Size, _DataType(28));
  EXPECT_TRUE(it0 == data.begin() + 2);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(28), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(64), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(28), data.at(size_t{ 2u }));
  EXPECT_EQ(_DataType(28), data.back());
}
template <typename _DataType, size_t _Size>
void _insertIterationFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  FixedSizeVector<_DataType, size_t{2u}> source;
  source.emplace_back(42);
  source.emplace_back(57);
  FixedSizeVector<_DataType, _Size> sourceFull;
  for (int i = 0; i < static_cast<int>(_Size); ++i)
    sourceFull.emplace_back(i);
  FixedSizeVector<_DataType, _Size * 2> sourceExceed;
  for (int i = 0; i < static_cast<int>(_Size) * 2; ++i)
    sourceExceed.emplace_back(i);

  // empty vector
  auto it0 = data.insert(data.begin(), source.begin(), source.begin()); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.end(), source.begin() + 1, source.begin());
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.end(), source.end(), source.begin());
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.end(), source.end(), source.end());
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.begin(), source.begin(), source.begin() + 1); // add some
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  it0 = data.insert(data.end(), source.begin(), source.end());
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(57), data.back());
  data.clear();
  it0 = data.insert(data.begin(), sourceFull.begin(), sourceFull.end()); // add max
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceFull[0], *it0);
  EXPECT_EQ(sourceFull[0], data.front());
  EXPECT_EQ(sourceFull.back(), data.back());
  for (int i = 0; i < static_cast<int>(_Size); ++i) {
    EXPECT_EQ(sourceFull[i], data.at(i));
  }
  data.clear();
  it0 = data.insert(data.end(), sourceFull.begin(), sourceFull.end());
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceFull[0], *it0);
  EXPECT_EQ(sourceFull[0], data.front());
  EXPECT_EQ(sourceFull.back(), data.back());
  data.clear();
  it0 = data.insert(data.begin(), sourceExceed.begin(), sourceExceed.end()); // add excess
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceExceed[0], *it0);
  EXPECT_EQ(sourceExceed[0], data.front());
  EXPECT_EQ(sourceExceed[_Size - 1u], data.back());
  data.clear();
  it0 = data.insert(data.end(), sourceExceed.begin(), sourceExceed.end());
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceExceed[0], *it0);
  EXPECT_EQ(sourceExceed[0], data.front());
  EXPECT_EQ(sourceExceed[_Size - 1u], data.back());

  // full vector
  EXPECT_TRUE(data.full());
  it0 = data.insert(data.begin(), source.begin(), source.begin()); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(sourceExceed[0], data.front());
  EXPECT_EQ(sourceExceed[_Size - 1u], data.back());
  it0 = data.insert(data.end(), source.end(), source.end());
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(sourceExceed[0], data.front());
  EXPECT_EQ(sourceExceed[_Size - 1u], data.back());
  it0 = data.insert(data.begin(), source.begin(), source.begin() + 1); // add some
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(sourceExceed[0], data.front());
  EXPECT_EQ(sourceExceed[_Size - 1u], data.back());
  it0 = data.insert(data.end(), sourceFull.begin(), sourceFull.end()); // add max
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(sourceExceed[0], data.front());
  EXPECT_EQ(sourceExceed[_Size - 1u], data.back());

  // partially full vector
  data.clear();
  data.resize(size_t{ 2u }, _DataType(42));
  it0 = data.insert(data.begin(), source.begin(), source.begin()); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin() + 1, source.begin() + 1, source.begin());
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.end(), source.end(), source.end());
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin(), source.begin(), source.begin() + 1); // add some
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 3u }, data.size());
  EXPECT_EQ(source[0], *it0);
  EXPECT_EQ(source[0], data.front());
  EXPECT_EQ(_DataType(42), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(42), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, sourceFull.begin(), sourceFull.begin() + 2);
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(sourceFull[0], *it0);
  EXPECT_EQ(source[0], data.front());
  EXPECT_EQ(sourceFull[0], data.at(size_t{ 1u }));
  EXPECT_EQ(sourceFull[1], data.at(size_t{ 2u }));
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  data.resize(size_t{ 2u }, _DataType(42));
  it0 = data.insert(data.begin(), sourceFull.begin(), sourceFull.begin() + (_Size - 2u)); // fill
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceFull[0], *it0);
  EXPECT_EQ(sourceFull[0], data.front());
  EXPECT_EQ(sourceFull[_Size - 3u], data.at(data.size() - 3u));
  EXPECT_EQ(_DataType(42), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  data.resize(size_t{ 2u }, _DataType(42));
  it0 = data.insert(data.begin() + 1, sourceFull.begin() + 2, sourceFull.end());
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceFull[2], *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(sourceFull[2], data.at(size_t{ 1u }));
  EXPECT_EQ(sourceFull.back(), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  data.resize(size_t{ 2u }, _DataType(42));
  it0 = data.insert(data.end(), sourceFull.begin() + 2, sourceFull.end());
  EXPECT_TRUE(it0 == data.begin() + 2);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceFull[2], *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(42), data.at(size_t{ 1u }));
  EXPECT_EQ(sourceFull[2], data.at(size_t{ 2u }));
  EXPECT_EQ(sourceFull.back(), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, sourceExceed.begin(), sourceExceed.end()); // add max
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(sourceExceed[0], *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(sourceExceed[0], data.at(1u));
  EXPECT_EQ(sourceExceed[_Size - 3u], data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(42), data.back());
}
template <typename _DataType, size_t _Size>
void _insertInitListFixedVector() {
  FixedSizeVector<_DataType, _Size> data;
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t{ 0 }, data.size());

  // empty vector
  auto it0 = data.insert(data.begin(), std::initializer_list<_DataType>{}); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.end(), std::initializer_list<_DataType>{});
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 0 }, data.size());
  it0 = data.insert(data.begin(), { _DataType(42), _DataType(57) }); // add some
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(57), data.back());
  data.clear();
  it0 = data.insert(data.end(), { _DataType(42), _DataType(57) });
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(57), data.back());
  data.clear();
  it0 = data.insert(data.begin(), { _DataType(22),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),
                                    _DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),
                                    _DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(57) }); // add excess
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(22), *it0);
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.at(1));
  EXPECT_EQ(_DataType(42), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(42), data.back());
  data.clear();
  it0 = data.insert(data.end(), { _DataType(22),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),
                                  _DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),
                                  _DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(57) });
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(22), *it0);
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.at(1));
  EXPECT_EQ(_DataType(42), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(42), data.back());

  // full vector
  EXPECT_TRUE(data.full());
  it0 = data.insert(data.begin(), std::initializer_list<_DataType>{}); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());
  it0 = data.insert(data.begin() + 1, { _DataType(42), _DataType(57) }); // add some
  EXPECT_TRUE(it0 == data.end());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(42), data.back());

  // partially full vector
  data.clear();
  data.resize(size_t{ 2u }, _DataType(22));
  it0 = data.insert(data.begin() + 1, std::initializer_list<_DataType>{}); // add 0
  EXPECT_TRUE(it0 == data.end());
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(_DataType(22), data.front());
  EXPECT_EQ(_DataType(22), data.back());
  it0 = data.insert(data.begin(), { _DataType(42), _DataType(57) }); // add some
  EXPECT_TRUE(it0 == data.begin());
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(42), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(57), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(22), data.at(size_t{ 2u }));
  EXPECT_EQ(_DataType(22), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, { _DataType(32), _DataType(64) });
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(size_t{ 4u }, data.size());
  EXPECT_EQ(_DataType(32), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(32), data.at(size_t{ 1u }));
  EXPECT_EQ(_DataType(64), data.at(size_t{ 2u }));
  EXPECT_EQ(_DataType(57), data.back());
  data.resize(size_t{ 2u }, _DataType(0));
  it0 = data.insert(data.begin() + 1, { _DataType(22),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),
                                        _DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),
                                        _DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(42),_DataType(57) }); // add max
  EXPECT_TRUE(it0 == data.begin() + 1);
  EXPECT_EQ(_Size, data.size());
  EXPECT_EQ(_DataType(22), *it0);
  EXPECT_EQ(_DataType(42), data.front());
  EXPECT_EQ(_DataType(22), data.at(1u));
  EXPECT_EQ(_DataType(42), data.at(data.size() - 2u));
  EXPECT_EQ(_DataType(32), data.back());
}

TEST_F(FixedSizeVectorTestOp, clearResize) {
  _clearResizeFixedVector<int, 2>();
  _clearResizeFixedVector<CopyObject, 2>();
  _clearResizeFixedVector<CopyMoveObject, 2>();
  _clearResizeFixedVector<int, 20>();
  _clearResizeFixedVector<CopyObject, 20>();
  _clearResizeFixedVector<CopyMoveObject, 20>();
}

TEST_F(FixedSizeVectorTestOp, pushCopy) {
  _pushCopyFixedVector<int, 2>();
  _pushCopyFixedVector<CopyObject, 2>();
  _pushCopyFixedVector<CopyMoveObject, 2>();
  _pushCopyFixedVector<int, 20>();
  _pushCopyFixedVector<CopyObject, 20>();
  _pushCopyFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, pushMove) {
  _pushMoveFixedVector<int, 2>();
  _pushMoveFixedVector<MoveObject, 2>();
  _pushMoveFixedVector<CopyMoveObject, 2>();
  _pushMoveFixedVector<int, 20>();
  _pushMoveFixedVector<MoveObject, 20>();
  _pushMoveFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, emplaceData) {
  _emplaceFixedVector<int, 2>();
  _emplaceFixedVector<CopyObject, 2>();
  _emplaceFixedVector<MoveObject, 2>();
  _emplaceFixedVector<CopyMoveObject, 2>();
  _emplaceFixedVector<int, 20>();
  _emplaceFixedVector<CopyObject, 20>();
  _emplaceFixedVector<MoveObject, 20>();
  _emplaceFixedVector<CopyMoveObject, 20>();
}

TEST_F(FixedSizeVectorTestOp, insertCopyAt) {
  _insertCopyFixedVector<int, 20>();
  _insertCopyFixedVector<CopyObject, 20>();
  _insertCopyFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, insertMoveAt) {
  _insertMoveFixedVector<int, 20>();
  _insertMoveFixedVector<MoveObject, 20>();
  _insertMoveFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, emplaceAt) {
  _insertEmplaceFixedVector<int, 20>();
  _insertEmplaceFixedVector<CopyObject, 20>();
  _insertEmplaceFixedVector<MoveObject, 20>();
  _insertEmplaceFixedVector<CopyMoveObject, 20>();
}

TEST_F(FixedSizeVectorTestOp, insertCountRepeatAt) {
  _insertCountRepeatFixedVector<int, 20>();
  _insertCountRepeatFixedVector<CopyObject, 20>();
  _insertCountRepeatFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, insertIterationAt) {
  _insertIterationFixedVector<int, 20>();
  _insertIterationFixedVector<CopyObject, 20>();
  _insertIterationFixedVector<CopyMoveObject, 20>();
}
TEST_F(FixedSizeVectorTestOp, insertInitializerListAt) {
  _insertInitListFixedVector<int, 20>();
# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
    _insertInitListFixedVector<CopyObject, 20>();
# endif
  _insertInitListFixedVector<CopyMoveObject, 20>();
}

#endif
