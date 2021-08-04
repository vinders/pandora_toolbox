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
#include <cstdio>
#include <chrono>
#include <array>
#include <algorithm>
#include <logic/sort.h>

#define _COLLECTION_MAX_SIZE size_t{ 30 }

using namespace pandora::logic;

class SortTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- collections & helpers --

enum class CollectionId : uint32_t {
  continuous = 0,
  singleValue = 1,
  repeats = 2,
  randomRepeats = 3,
  extremes = 4,
  negative = 5,
  negativePositive = 6,
};

void _fillCollection(CollectionId id, std::array<int, _COLLECTION_MAX_SIZE>& out) {
  switch (id) {
    case CollectionId::continuous:       out = { 27,3,11,14,20,18,28,2,1,13,4,23,7,16,8,5,9,26,25,12,15,17,22,29,19,21,0,24,6,10 }; break;
    case CollectionId::singleValue:      out = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }; break;
    case CollectionId::repeats:          out = { 4,0,2,5,0,1,2,1,4,2,3,1,5,2,3,3,5,0,3,4,1,0,4,3,5,1,0,5,2,4 }; break;
    case CollectionId::randomRepeats:    out = { 0,11,50,22,3,18,5,2,4,15,23,8,13,19,22,2,18,20,5,0,9,28,9,29,18,30,14,40,23,7 }; break;
    case CollectionId::extremes:         out = { 200,68,160,52,104,201,60,102,62,190,64,200,61,3000,50,100,0,65,101,68,66,192,191,53,63,193,100,67,51,204 }; break;
    case CollectionId::negative:         out = { -80,-185,-200,-180,-30,-27,-60,-198,-175,-1,-15,-190,-110,-9,-40,-190,-120,-28,-199,-100,-20,-190,-200,-70,-26,-10,-105,-50,-29,-110 }; break;
    case CollectionId::negativePositive: out = { 1003,-2,120,-1000,500,1001,6,150,20,1003,100,7,0,-4,4,120,-2,2,120,1002,3,-2,1003,0,-999,-20,120,4,1000,-1 }; break;
  }
}

void _sortCollection(CollectionId id, SortOrder order, void(*sorter)(int*, uint32_t)) {
  std::array<int, _COLLECTION_MAX_SIZE> expectedResult{ 0 };
  std::string title;
  switch (id) {
    case CollectionId::continuous: 
      expectedResult = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29 }; 
      title = "continuous :   "; break;
    case CollectionId::singleValue: 
      expectedResult = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }; 
      title = "singleValue :  "; break;
    case CollectionId::repeats: 
      expectedResult = { 0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5 }; 
      title = "repeats :      "; break;
    case CollectionId::randomRepeats: 
      expectedResult = { 0,0,2,2,3,4,5,5,7,8,9,9,11,13,14,15,18,18,18,19,20,22,22,23,23,28,29,30,40,50 }; 
      title = "randomRepeats :"; break;
    case CollectionId::extremes: 
      expectedResult = { 0,50,51,52,53,60,61,62,63,64,65,66,67,68,68,100,100,101,102,104,160,190,191,192,193,200,200,201,204,3000 }; 
      title = "extremes :     "; break;
    case CollectionId::negative: 
      expectedResult = { -200,-200,-199,-198,-190,-190,-190,-185,-180,-175,-120,-110,-110,-105,-100,-80,-70,-60,-50,-40,-30,-29,-28,-27,-26,-20,-15,-10,-9,-1 }; 
      title = "negative :     "; break;
    case CollectionId::negativePositive:  
      expectedResult = { -1000,-999,-20,-4,-2,-2,-2,-1,0,0,2,3,4,4,6,7,20,100,120,120,120,120,150,500,1000,1001,1002,1003,1003,1003 };
      title = "negativePosit :"; break;
  }
  if (order == SortOrder::desc)
    std::reverse(std::begin(expectedResult), std::end(expectedResult));

  std::array<int, _COLLECTION_MAX_SIZE> data{ 0 };
  _fillCollection(id, data);
  auto start = std::chrono::high_resolution_clock::now();
  sorter(data.data(), _COLLECTION_MAX_SIZE);
  printf("\t> %s %f ms\n", title.c_str(), static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
  EXPECT_EQ(expectedResult, data);
}


// -- sort operations --

TEST_F(SortTest, ascBubbleSort) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negative, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, bubbleSort<int, SortOrder::asc>);
}
TEST_F(SortTest, descBubbleSort) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negative, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, bubbleSort<int, SortOrder::desc>);
}

TEST_F(SortTest, ascInsertSort) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, insertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, insertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, insertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, insertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, insertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negative, SortOrder::asc, insertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, insertionSort<int, SortOrder::asc>);
}
TEST_F(SortTest, descInsertSort) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, insertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, insertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, insertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, insertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, insertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negative, SortOrder::desc, insertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, insertionSort<int, SortOrder::desc>);
}

TEST_F(SortTest, ascBinaryInsertSort) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negative, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, binaryInsertionSort<int, SortOrder::asc>);
}
TEST_F(SortTest, descBinaryInsertSort) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negative, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, binaryInsertionSort<int, SortOrder::desc>);
}

TEST_F(SortTest, ascHeapSort) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, heapSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, heapSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, heapSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, heapSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, heapSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negative, SortOrder::asc, heapSort<int, SortOrder::asc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, heapSort<int, SortOrder::asc>);
}
TEST_F(SortTest, descHeapSort) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, heapSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, heapSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, heapSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, heapSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, heapSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negative, SortOrder::desc, heapSort<int, SortOrder::desc>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, heapSort<int, SortOrder::desc>);
}

TEST_F(SortTest, ascQuickSortFirstPivot) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
  _sortCollection(CollectionId::negative, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::first>);
}
TEST_F(SortTest, descQuickSortFirstPivot) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
  _sortCollection(CollectionId::negative, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::first>);
}
TEST_F(SortTest, ascQuickSortCentralPivot) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
  _sortCollection(CollectionId::negative, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::center>);
}
TEST_F(SortTest, descQuickSortCentralPivot) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
  _sortCollection(CollectionId::negative, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::center>);
}
TEST_F(SortTest, ascQuickSortLastPivot) {
  _sortCollection(CollectionId::continuous, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
  _sortCollection(CollectionId::singleValue, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
  _sortCollection(CollectionId::repeats, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
  _sortCollection(CollectionId::extremes, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
  _sortCollection(CollectionId::negative, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
  _sortCollection(CollectionId::negativePositive, SortOrder::asc, quickSort<int, SortOrder::asc, SortPivotType::last>);
}
TEST_F(SortTest, descQuickSortLastPivot) {
  _sortCollection(CollectionId::continuous, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
  _sortCollection(CollectionId::singleValue, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
  _sortCollection(CollectionId::repeats, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
  _sortCollection(CollectionId::randomRepeats, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
  _sortCollection(CollectionId::extremes, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
  _sortCollection(CollectionId::negative, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
  _sortCollection(CollectionId::negativePositive, SortOrder::desc, quickSort<int, SortOrder::desc, SortPivotType::last>);
}
