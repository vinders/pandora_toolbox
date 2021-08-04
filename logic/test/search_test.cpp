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
#include <logic/search.h>

#define _COLLECTION_MAX_SIZE size_t{ 30 }

using namespace pandora::logic;

class SearchTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- collections & helpers --

enum class CollectionId: uint32_t {
  continuous = 0,
  singleValue = 1,
  repeats = 2,
  randomRepeats = 3,
  exponential = 4,
  logarithmic = 5,
  gaussian = 6,
  extremes = 7,
  negative = 8,
  negativePositive = 9,
};

void _fillCollection(CollectionId id, SortOrder order, std::array<int, _COLLECTION_MAX_SIZE>& out) {
  switch (id) {
    case CollectionId::continuous:       out = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29 }; break;
    case CollectionId::singleValue:      out = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }; break;
    case CollectionId::repeats:          out = { 0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5 }; break;
    case CollectionId::randomRepeats:    out = { 0,0,2,2,3,4,5,5,7,8,9,9,11,13,14,15,18,18,18,19,20,22,22,23,23,28,29,30,40,50 }; break;
    case CollectionId::exponential:      out = { 0,0,0,0,1,1,2,4,7,10,14,19,25,32,40,49,59,70,82,96,110,128,150,176,200,230,270,320,370,440 }; break;
    case CollectionId::logarithmic:      out = { 0,20,38,52,64,74,82,89,95,100,104,108,111,114,116,118,120,121,122,123,124,125,125,126,126,126,127,127,127,127 }; break;
    case CollectionId::gaussian:         out = { 4,4,4,5,5,6,7,8,10,13,18,25,35,42,48,52,58,65,75,82,87,90,92,93,94,95,95,96,96,96 }; break;
    case CollectionId::extremes:         out = { 0,50,51,52,53,60,61,62,63,64,65,66,67,68,68,100,100,101,102,104,160,190,191,192,193,200,200,201,204,3000 }; break;
    case CollectionId::negative:         out = { -200,-200,-199,-198,-190,-190,-190,-185,-180,-175,-120,-110,-110,-105,-100,-80,-70,-60,-50,-40,-30,-29,-28,-27,-26,-20,-15,-10,-9,-1 }; break;
    case CollectionId::negativePositive: out = { -1000,-999,-20,-4,-2,-2,-2,-1,0,0,2,3,4,4,6,7,20,100,120,120,120,120,150,500,1000,1001,1002,1003,1003,1003 }; break;
  }
  if (order == SortOrder::desc)
    std::reverse(std::begin(out), std::end(out));
}

void _searchAsc(CollectionId id, uint32_t (*searcher)(const int*, uint32_t, int)) {
  std::array<int, _COLLECTION_MAX_SIZE> collection;
  _fillCollection(id, SortOrder::asc, collection);
  uint32_t calls = 0;
  auto start = std::chrono::high_resolution_clock::now();

  switch (id) {
    // { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29 };
    case CollectionId::continuous: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 0)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 2)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 30)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // min
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 29)); ++calls; // max
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 14)); ++calls; // middle
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE - 1u, 14)); ++calls; // real middle
      EXPECT_EQ(7u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 7)); ++calls;  // low value
      EXPECT_EQ(24u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 24)); ++calls; // high value
      printf("\t> continuous (%d) :    %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
    case CollectionId::singleValue: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 2)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 2)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // min/max/middle/low/high/repeated
      printf("\t> singleValue (%d) :    %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5 };
    case CollectionId::repeats: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 0)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 6)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // min
      EXPECT_EQ(25u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 5)); ++calls;  // max
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 3)); ++calls;  // middle
      EXPECT_EQ(5u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // low value/repeated value 1
      EXPECT_EQ(20u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 4)); ++calls;  // high value/repeated value 2
      printf("\t> repeats (%d) :       %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,0,2,2,3,4,5,5,7,8,9,9,11,13,14,15,18,18,18,19,20,22,22,23,23,28,29,30,40,50 };
    case CollectionId::randomRepeats: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 0)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 2)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 2)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 51)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 25)); ++calls; // non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // min
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 50)); ++calls; // max
      EXPECT_EQ(16u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 18)); ++calls; // middle
      EXPECT_EQ(8u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 7)); ++calls;  // low value
      EXPECT_EQ(25u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 28)); ++calls; // high value
      EXPECT_EQ(10u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 9)); ++calls;  // repeated value 1
      EXPECT_EQ(23u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 23)); ++calls; // repeated value 2
      printf("\t> randomRepeats (%d) : %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,0,0,0,1,1,2,4,7,10,14,19,25,32,40,49,59,70,82,96,110,128,150,176,200,230,270,320,370,440 };
    case CollectionId::exponential: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 0)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 441)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 6)); ++calls;  // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 97)); ++calls; // non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // min/repeated
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 440)); ++calls;// max
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 49)); ++calls; // middle
      EXPECT_EQ(8u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 7)); ++calls;  // low value
      EXPECT_EQ(24u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 200)); ++calls;// high value
      EXPECT_EQ(4u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // repeated value
      printf("\t> exponential (%d) :   %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,20,38,52,64,74,82,89,95,100,104,108,111,114,116,118,120,121,122,123,124,125,125,126,126,126,127,127,127,127 };
    case CollectionId::logarithmic: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 0)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 20)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 38)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 128)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 40)); ++calls; // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 119)); ++calls;// non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // min
      EXPECT_EQ(26u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 127)); ++calls;// max/repeated
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 118)); ++calls;// middle
      EXPECT_EQ(7u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 89)); ++calls; // low value
      EXPECT_EQ(20u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 124)); ++calls;// high value
      EXPECT_EQ(21u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 125)); ++calls;// repeated value 1
      EXPECT_EQ(23u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 126)); ++calls;// repeated value 2
      printf("\t> logarithmic (%d) :   %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 4,4,4,5,5,6,7,8,10,13,18,25,35,42,48,52,58,65,75,82,87,90,92,93,94,95,95,96,96,96 };
    case CollectionId::gaussian: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 4)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 4)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 5)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 4)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 5)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 3)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 97)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 20)); ++calls; // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 88)); ++calls; // non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 4)); ++calls;  // min/repeated
      EXPECT_EQ(27u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 96)); ++calls; // max/repeated
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 48)); ++calls; // middle
      EXPECT_EQ(11u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 25)); ++calls;// low value
      EXPECT_EQ(23u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 93)); ++calls; // high value
      EXPECT_EQ(3u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 5)); ++calls;  // repeated value 1
      EXPECT_EQ(25u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 95)); ++calls; // repeated value 2
      printf("\t> gaussian (%d) :      %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,50,51,52,53,60,61,62,63,64,65,66,67,68,68,100,100,101,102,104,160,190,191,192,193,200,200,201,204,3000 };
    case CollectionId::extremes: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 0)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 50)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 50)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 51)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data() ,_COLLECTION_MAX_SIZE, 3001));++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,10001));++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 15)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 55)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 70)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 189)); ++calls;// non existing
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // min
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE,3000)); ++calls;// max
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 100)); ++calls;// middle
      EXPECT_EQ(7u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 62)); ++calls; // low value
      EXPECT_EQ(23u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 192)); ++calls;// high value
      EXPECT_EQ(25u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 200)); ++calls;// repeated value
      printf("\t> extremes (%d) :      %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { -200,-200,-199,-198,-190,-190,-190,-185,-180,-175,-120,-110,-110,-105,-100,-80,-70,-60,-50,-40,-30,-29,-28,-27,-26,-20,-15,-10,-9,-1 };
    case CollectionId::negative: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, -200)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, -200)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, -199)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, -200)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, -199)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -201)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,-5000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;   // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -178)); ++calls;// non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -42)); ++calls; // non existing
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, -200)); ++calls;// min/repeat
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls;  // max
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -80)); ++calls; // middle
      EXPECT_EQ(7u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, -185)); ++calls;// low value
      EXPECT_EQ(25u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -20)); ++calls; // high value
      EXPECT_EQ(4u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, -190)); ++calls;// repeated value 1
      EXPECT_EQ(11u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -110)); ++calls;// repeated value 2
      printf("\t> negative (%d) :      %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { -1000,-999,-20,-4,-2,-2,-2,-1,0,0,2,3,4,4,6,7,20,100,120,120,120,120,150,500,1000,1001,1002,1003,1003,1003 };
    case CollectionId::negativePositive: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, -1000)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, -1000)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, -999)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, -1000)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, -999)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, -20)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,-1001)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 1004)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,-9000)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,10000)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -500)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -21)); ++calls;  // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -19)); ++calls;  // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 200)); ++calls;  // non existing
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE,-1000)); ++calls; // min
      EXPECT_EQ(27u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 1003)); ++calls; // max/repeat
      EXPECT_EQ(17u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 100)); ++calls;  // middle
      EXPECT_EQ(7u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls;   // low value
      EXPECT_EQ(23u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 500)); ++calls;  // high value
      EXPECT_EQ(8u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;    // repeated value 1
      EXPECT_EQ(12u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 4)); ++calls;    // repeated value 2
      EXPECT_EQ(18u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 120)); ++calls;  // repeated value 3
      printf("\t> negativePosit (%d) : %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
  }
}

void _searchDesc(CollectionId id, uint32_t (*searcher)(const int*, uint32_t, int)) {
  std::array<int, _COLLECTION_MAX_SIZE> collection;
  _fillCollection(id, SortOrder::desc, collection);
  uint32_t calls = 0;
  auto start = std::chrono::high_resolution_clock::now();

  switch (id) {
    // { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29 };
    case CollectionId::continuous: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 29)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 29)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 28)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 29)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 28)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 27)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 30)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 29)); ++calls;  // min
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls; // max
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 14)); ++calls; // middle
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE - 1u, 15)); ++calls; // real middle
      EXPECT_EQ(22u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 7)); ++calls;  // low value
      EXPECT_EQ(5u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 24)); ++calls; // high value
      printf("\t> continuous (%d) :    %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
    case CollectionId::singleValue: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 2)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 0)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 2)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // min/max/middle/low/high/repeated
      printf("\t> singleValue (%d) :    %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5 };
    case CollectionId::repeats: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 5)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 5)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 4)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 5)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 4)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 6)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 5)); ++calls;  // min
      EXPECT_EQ(25u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // max
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 2)); ++calls;  // middle
      EXPECT_EQ(5u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 4)); ++calls;  // low value/repeated value 1
      EXPECT_EQ(20u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // high value/repeated value 2
      printf("\t> repeats (%d) :       %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,0,2,2,3,4,5,5,7,8,9,9,11,13,14,15,18,18,18,19,20,22,22,23,23,28,29,30,40,50 };
    case CollectionId::randomRepeats: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 50)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 50)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 40)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 50)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 40)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 30)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 51)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls;  // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 25)); ++calls; // non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 50)); ++calls; // min
      EXPECT_EQ(28u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // max
      EXPECT_EQ(11u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 18)); ++calls; // middle
      EXPECT_EQ(4u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 28)); ++calls; // low value
      EXPECT_EQ(21u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 7)); ++calls;  // high value
      EXPECT_EQ(5u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 23)); ++calls; // repeated value 1
      EXPECT_EQ(18u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 9)); ++calls;  // repeated value 2
      printf("\t> randomRepeats (%d) : %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,0,0,0,1,1,2,4,7,10,14,19,25,32,40,49,59,70,82,96,110,128,150,176,200,230,270,320,370,440 };
    case CollectionId::exponential: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 440)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 440)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 370)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 440)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 370)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 320)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 441)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 6)); ++calls;  // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 97)); ++calls; // non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 440)); ++calls;// min
      EXPECT_EQ(26u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // max/repeated
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 49)); ++calls; // middle
      EXPECT_EQ(21u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 7)); ++calls;  // low value
      EXPECT_EQ(5u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 200)); ++calls;// high value
      EXPECT_EQ(24u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 1)); ++calls; // repeated value
      printf("\t> exponential (%d) :   %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,20,38,52,64,74,82,89,95,100,104,108,111,114,116,118,120,121,122,123,124,125,125,126,126,126,127,127,127,127 };
    case CollectionId::logarithmic: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 127)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 127)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 126)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 127)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 126)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 128)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 40)); ++calls; // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 119)); ++calls;// non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 127)); ++calls;// min/repeated
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // max
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 118)); ++calls;// middle
      EXPECT_EQ(9u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 124)); ++calls;// low value
      EXPECT_EQ(22u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 89)); ++calls; // high value
      EXPECT_EQ(7u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 125)); ++calls; // repeated value 1
      EXPECT_EQ(4u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 126)); ++calls; // repeated value 2
      printf("\t> logarithmic (%d) :   %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 4,4,4,5,5,6,7,8,10,13,18,25,35,42,48,52,58,65,75,82,87,90,92,93,94,95,95,96,96,96 };
    case CollectionId::gaussian: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 96)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 96)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 95)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 96)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 95)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 3)); ++calls;  // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 97)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,1000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 20)); ++calls; // non existing 1
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 88)); ++calls; // non existing 2
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 96)); ++calls; // min/repeated
      EXPECT_EQ(27u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 4)); ++calls;  // max/repeated
      EXPECT_EQ(15u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 48)); ++calls; // middle
      EXPECT_EQ(6u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 93)); ++calls; // low value
      EXPECT_EQ(18u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 25)); ++calls; // high value
      EXPECT_EQ(25u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 5)); ++calls; // repeated value 1
      EXPECT_EQ(3u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 95)); ++calls;  // repeated value 2
      printf("\t> gaussian (%d) :      %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { 0,50,51,52,53,60,61,62,63,64,65,66,67,68,68,100,100,101,102,104,160,190,191,192,193,200,200,201,204,3000 };
    case CollectionId::extremes: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 3000)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 3000)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 204)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 3000)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, 204)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 201)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data() ,_COLLECTION_MAX_SIZE, 3001));++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,10001));++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 15)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 55)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 70)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 189)); ++calls;// non existing
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE,3000)); ++calls;// min
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;  // max
      EXPECT_EQ(13u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 100)); ++calls;// middle
      EXPECT_EQ(6u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 192)); ++calls;// low value
      EXPECT_EQ(22u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 62)); ++calls; // high value
      EXPECT_EQ(3u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 200)); ++calls; // repeated value
      printf("\t> extremes (%d) :      %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { -200,-200,-199,-198,-190,-190,-190,-185,-180,-175,-120,-110,-110,-105,-100,-80,-70,-60,-50,-40,-30,-29,-28,-27,-26,-20,-15,-10,-9,-1 };
    case CollectionId::negative: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, -1)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, -1)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, -9)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, -1)); ++calls;
      EXPECT_EQ(1u,              searcher(collection.data(), 2u, -9)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, -10)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -201)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,-5000)); ++calls;// out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;   // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -178)); ++calls;// non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -42)); ++calls; // non existing
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls;  // min
      EXPECT_EQ(28u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -200)); ++calls;// max/repeat
      EXPECT_EQ(14u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -80)); ++calls; // middle
      EXPECT_EQ(4u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, -20)); ++calls; // low value
      EXPECT_EQ(22u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -185)); ++calls;// high value
      EXPECT_EQ(23u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -190)); ++calls;// repeated value 1
      EXPECT_EQ(17u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -110)); ++calls;// repeated value 2
      printf("\t> negative (%d) :      %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
    // { -1000,-999,-20,-4,-2,-2,-2,-1,0,0,2,3,4,4,6,7,20,100,120,120,120,120,150,500,1000,1001,1002,1003,1003,1003 };
    case CollectionId::negativePositive: {
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 0u, 1003)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 1u, 1003)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 1u, 1001)); ++calls;
      EXPECT_EQ(0u,              searcher(collection.data(), 2u, 1003)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), 2u, 1001)); ++calls;
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,-1001)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 1004)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,-9000)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE,10000)); ++calls; // out of range
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -500)); ++calls; // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -21)); ++calls;  // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, -19)); ++calls;  // non existing
      EXPECT_EQ(indexNotFound(), searcher(collection.data(), _COLLECTION_MAX_SIZE, 200)); ++calls;  // non existing
      EXPECT_EQ(0u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 1003)); ++calls; // min/repeat
      EXPECT_EQ(29u,             searcher(collection.data(), _COLLECTION_MAX_SIZE,-1000)); ++calls; // max
      EXPECT_EQ(12u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 100)); ++calls;  // middle
      EXPECT_EQ(6u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 500)); ++calls;  // low value
      EXPECT_EQ(22u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, -1)); ++calls;   // high value
      EXPECT_EQ(20u,              searcher(collection.data(), _COLLECTION_MAX_SIZE, 0)); ++calls;   // repeated value 1
      EXPECT_EQ(16u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 4)); ++calls;    // repeated value 2
      EXPECT_EQ(8u,             searcher(collection.data(), _COLLECTION_MAX_SIZE, 120)); ++calls;   // repeated value 3
      printf("\t> negativePosit (%d) : %f ms\n", calls, static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000000.0);
      break;
    }
  }
}


// -- search operations --

TEST_F(SearchTest, ascBinarySearch) {
  _searchAsc(CollectionId::continuous, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::singleValue, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::repeats, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::randomRepeats, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::exponential, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::logarithmic, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::gaussian, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::extremes, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negative, binarySearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negativePositive, binarySearch<int, SortOrder::asc>);
}
TEST_F(SearchTest, descBinarySearch) {
  _searchDesc(CollectionId::continuous, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::singleValue, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::repeats, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::randomRepeats, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::exponential, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::logarithmic, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::gaussian, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::extremes, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negative, binarySearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negativePositive, binarySearch<int, SortOrder::desc>);
}

TEST_F(SearchTest, ascJumpSearch) {
  _searchAsc(CollectionId::continuous, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::singleValue, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::repeats, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::randomRepeats, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::exponential, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::logarithmic, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::gaussian, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::extremes, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negative, jumpSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negativePositive, jumpSearch<int, SortOrder::asc>);
  // array size that isn't a multiple
  std::array<int,29> notMultiple = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28 };
  uint32_t pos = jumpSearch<int, SortOrder::asc>(notMultiple.data(), static_cast<uint32_t>(notMultiple.size()), 27);
  EXPECT_EQ(uint32_t{ 27u }, pos);

}
TEST_F(SearchTest, descJumpSearch) {
  _searchDesc(CollectionId::continuous, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::singleValue, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::repeats, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::randomRepeats, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::exponential, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::logarithmic, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::gaussian, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::extremes, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negative, jumpSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negativePositive, jumpSearch<int, SortOrder::desc>);
}

TEST_F(SearchTest, ascExponentialSearch) {
  _searchAsc(CollectionId::continuous, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::singleValue, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::repeats, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::randomRepeats, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::exponential, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::logarithmic, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::gaussian, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::extremes, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negative, exponentialSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negativePositive, exponentialSearch<int, SortOrder::asc>);
}
TEST_F(SearchTest, descExponentialSearch) {
  _searchDesc(CollectionId::continuous, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::singleValue, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::repeats, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::randomRepeats, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::exponential, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::logarithmic, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::gaussian, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::extremes, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negative, exponentialSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negativePositive, exponentialSearch<int, SortOrder::desc>);
}

TEST_F(SearchTest, ascInterpolationSearch) {
  _searchAsc(CollectionId::continuous, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::singleValue, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::repeats, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::randomRepeats, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::exponential, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::logarithmic, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::gaussian, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::extremes, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negative, interpolationSearch<int, SortOrder::asc>);
  _searchAsc(CollectionId::negativePositive, interpolationSearch<int, SortOrder::asc>);
}
TEST_F(SearchTest, descInterpolationSearch) {
  _searchDesc(CollectionId::continuous, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::singleValue, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::repeats, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::randomRepeats, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::exponential, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::logarithmic, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::gaussian, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::extremes, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negative, interpolationSearch<int, SortOrder::desc>);
  _searchDesc(CollectionId::negativePositive, interpolationSearch<int, SortOrder::desc>);
}
