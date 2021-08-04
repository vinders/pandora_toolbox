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
--------------------------------------------------------------------------------
Description : measure duration of algorithms for benchmark utility
*******************************************************************************/
#pragma once

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <chrono>
#include <logic/search.h>
#include <logic/sort.h>
#include "array_generator.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

#define _BENCHMARK_REPEATS 500

// linear search algorithm (for comparison)
template <typename _ValueType, pandora::logic::SortOrder _Order = pandora::logic::SortOrder::asc>
uint32_t linearSearch(const _ValueType * collec, uint32_t n, pandora::logic::SearchValue<_ValueType> target) noexcept {
  __if_constexpr (_Order == pandora::logic::SortOrder::asc) {
    for (uint32_t i = 0; i < n; ++i) {
      if (collec[i] >= target)
        return (collec[i] == target) ? i : pandora::logic::indexNotFound();
    }
  }
  else {
    for (uint32_t i = 0; i < n; ++i) {
      if (collec[i] <= target)
        return (collec[i] == target) ? i : pandora::logic::indexNotFound();
    }
  }
  return pandora::logic::indexNotFound();
}


// -- search benchmark --

// execute search algorithm in a loop to measure average duration
template<uint32_t(* _Algorithm)(const int*,uint32_t,int), uint32_t _Size>
int64_t benchmarkSearchArray(int* collec, int target) noexcept {
  uint32_t pos = 0;
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = _BENCHMARK_REPEATS; i; --i)
    pos += (int64_t)_Algorithm(collec, _Size, target);
  auto end = std::chrono::high_resolution_clock::now();
  
  double nanosec = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / static_cast<double>(_BENCHMARK_REPEATS);
  nanosec += 0.0 * pos; // useless, but prevents loop replacement optimization
  return static_cast<int64_t>(nanosec);
}
// execute all search algorithms with a specific array
template <uint32_t _Size, pandora::logic::SortOrder _Order>
inline void measureBenchmarkSearchArray(int* collection, int target, int64_t results[5][8], uint32_t resultsIndex) noexcept {
  results[0][resultsIndex] = benchmarkSearchArray<linearSearch<int,_Order>, _Size>(collection, target);
  results[1][resultsIndex] = benchmarkSearchArray<pandora::logic::binarySearch<int,_Order>, _Size>(collection, target);
  results[2][resultsIndex] = benchmarkSearchArray<pandora::logic::jumpSearch<int,_Order>, _Size>(collection, target);
  results[3][resultsIndex] = benchmarkSearchArray<pandora::logic::exponentialSearch<int,_Order>, _Size>(collection, target);
  results[4][resultsIndex] = benchmarkSearchArray<pandora::logic::interpolationSearch<int,_Order>, _Size>(collection, target);
}


// -- sort benchmark --

// execute sort algorithm in a loop to measure average duration
template<void(* _Algorithm)(int*,uint32_t), uint32_t _Size>
int64_t benchmarkSortArray(int* collec) noexcept {
  std::unique_ptr<int[]> dataClones(new int[_Size*_BENCHMARK_REPEATS]());
  for (int i = _BENCHMARK_REPEATS - 1; i >= 0; --i)
    memcpy((void*)&dataClones.get()[static_cast<uint32_t>(i)*_Size], (void*)collec, _Size*sizeof(int));
  int* cursor = dataClones.get();
  
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = _BENCHMARK_REPEATS; i; --i, cursor += _Size)
    _Algorithm(cursor, _Size);
  auto end = std::chrono::high_resolution_clock::now();
  
  double nanosec = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / static_cast<double>(_BENCHMARK_REPEATS);
  return static_cast<int64_t>(nanosec);
}
// execute all sort algorithms with a specific array
template <uint32_t _Size, pandora::logic::SortOrder _Order>
inline void measureBenchmarkSortArray(int* collection, int64_t results[7][6], uint32_t resultsIndex) noexcept {
  results[0][resultsIndex] = benchmarkSortArray<pandora::logic::bubbleSort<int,_Order>, _Size>(collection);
  results[1][resultsIndex] = benchmarkSortArray<pandora::logic::insertionSort<int,_Order>, _Size>(collection);
  results[2][resultsIndex] = benchmarkSortArray<pandora::logic::binaryInsertionSort<int,_Order>, _Size>(collection);
  results[3][resultsIndex] = benchmarkSortArray<pandora::logic::heapSort<int,_Order>, _Size>(collection);
  results[4][resultsIndex] = benchmarkSortArray<pandora::logic::quickSort<int,_Order,pandora::logic::SortPivotType::first>, _Size>(collection);
  results[5][resultsIndex] = benchmarkSortArray<pandora::logic::quickSort<int,_Order,pandora::logic::SortPivotType::center>, _Size>(collection);
  results[6][resultsIndex] = benchmarkSortArray<pandora::logic::quickSort<int,_Order,pandora::logic::SortPivotType::last>, _Size>(collection);
}


// -- search/sort - display --

// display benchmark results for a search algorithm with a specific array
inline void printArraySearchBenchmarkResultLine(const std::string& algoName, int64_t results[5][8], uint32_t algoIndex) noexcept {
  int64_t average = (results[algoIndex][0] + results[algoIndex][1] + results[algoIndex][2] + results[algoIndex][3] 
                   + results[algoIndex][4] + results[algoIndex][5] + results[algoIndex][6] + results[algoIndex][7]) / 8LL;
  printf("%s| %7lld | %7lld | %7lld | %7lld | %7lld | %7lld | %7lld | %7lld | %7lld\n",
         algoName.c_str(), (long long)average, 
         (long long)(results[algoIndex][0]), (long long)(results[algoIndex][1]), (long long)(results[algoIndex][2]), (long long)(results[algoIndex][3]),
         (long long)(results[algoIndex][4]), (long long)(results[algoIndex][5]), (long long)(results[algoIndex][6]), (long long)(results[algoIndex][7]));
}
//display benchmark results for all search algorithms with a specific array
inline void printArraySearchBenchmarkResults(int64_t results[5][8]) noexcept {
  printf("     ALGORITHM      | average |   min   |   1/3   |   1/2   |   3/5   |   4/5   |   max   |  < min  |  > max  \n");
  printArraySearchBenchmarkResultLine("linear search       ", results, 0u);
  printArraySearchBenchmarkResultLine("binary search       ", results, 1u);
  printArraySearchBenchmarkResultLine("jump search         ", results, 2u);
  printArraySearchBenchmarkResultLine("exponential search  ", results, 3u);
  printArraySearchBenchmarkResultLine("interpolation search", results, 4u);
}

// display benchmark results for a sort algorithm with a specific array
inline void printArraySortBenchmarkResultLine(const std::string& algoName, int64_t results[7][6], uint32_t algoIndex) noexcept {
  int64_t average = (results[algoIndex][0] + results[algoIndex][1] + results[algoIndex][2] 
                   + results[algoIndex][3] + results[algoIndex][4] + results[algoIndex][5]) / 6LL;
  printf("%s| %8lld | %8lld | %8lld | %8lld | %8lld | %8lld | %8lld\n", 
         algoName.c_str(), (long long)average, 
         (long long)(results[algoIndex][0]), (long long)(results[algoIndex][1]), (long long)(results[algoIndex][2]), 
         (long long)(results[algoIndex][3]), (long long)(results[algoIndex][4]), (long long)(results[algoIndex][5]));
}
//display benchmark results for all sort algorithms with a specific array
inline void printArraySortBenchmarkResults(int64_t results[7][6]) noexcept {
  printf("     ALGORITHM     | average  | asc >=0  | asc <=0  | asc all  | desc >=0 | desc <=0 | desc all\n");
  printArraySortBenchmarkResultLine("bubble sort        ", results, 0u);
  printArraySortBenchmarkResultLine("insertion sort     ", results, 1u);
  printArraySortBenchmarkResultLine("binary insert sort ", results, 2u);
  printArraySortBenchmarkResultLine("heap sort          ", results, 3u);
  printArraySortBenchmarkResultLine("quick sort (first) ", results, 4u);
  printArraySortBenchmarkResultLine("quick sort (center)", results, 5u);
  printArraySortBenchmarkResultLine("quick sort (last)  ", results, 6u);
}


// -- search/sort - launchers --

// execute and display benchmark results of search algorithms for a specific category of array
template <uint32_t _Size, pandora::logic::SortOrder _Order>
void measurePrintSortedArraySearchBenchmarks(CollectionId type) noexcept {
  int collection[_Size];
  std::string title = toString(type);
  printf("* %s array (%s) : algorithms benchmark (ns) :\n", title.c_str(), (_Order == pandora::logic::SortOrder::asc) ? "asc" : "desc");

  int64_t results[5][8];
  generateArray<(_Order == pandora::logic::SortOrder::asc) ? ArrayOrder::asc : ArrayOrder::desc, ValueSign::positive>(type, collection, _Size);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[0], results, 0u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size / 3u], results, 1u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[(_Size >> 1) - 1u], results, 2u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size*3u / 5u], results, 3u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size*4u / 5u], results, 4u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size - 1u], results, 5u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[0] - 1, results, 6u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size - 1u] + 1, results, 7u);
  int64_t resultsNegative[5][8];
  generateArray<(_Order == pandora::logic::SortOrder::asc) ? ArrayOrder::asc : ArrayOrder::desc, ValueSign::negative>(type, collection, _Size);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[0], resultsNegative, 0u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size / 3u], resultsNegative, 1u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[(_Size >> 1) - 1u], resultsNegative, 2u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size * 3u / 5u], resultsNegative, 3u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size * 4u / 5u], resultsNegative, 4u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size - 1u], resultsNegative, 5u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[0] - 1, resultsNegative, 6u);
  measureBenchmarkSearchArray<_Size, _Order>(collection, collection[_Size - 1u] + 1, resultsNegative, 7u);

  for (uint32_t algo = 0; algo < 5u; ++algo)
    for (uint32_t i = 0; i < 8u; ++i)
      results[algo][i] = (results[algo][i] + resultsNegative[algo][i]) >> 1;
  printArraySearchBenchmarkResults(results);
}

// execute and display benchmark results of sort algorithms for a specific category of array
template <uint32_t _Size>
void measurePrintArraySortBenchmarks(CollectionId type) noexcept {
  int64_t results[7][6];
  int collection[_Size];
  std::string title = toString(type);
  printf("* %s array : algorithms benchmark (ns) :\n", title.c_str());

  generateArray<ArrayOrder::unordered, ValueSign::positive>(type, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::asc>(collection, results, 0u);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::desc>(collection, results, 3u);
  generateArray<ArrayOrder::unordered, ValueSign::negative>(type, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::asc>(collection, results, 1u);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::desc>(collection, results, 4u);
  generateArray<ArrayOrder::unordered, ValueSign::both>(type, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::asc>(collection, results, 2u);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::desc>(collection, results, 5u);
  printArraySortBenchmarkResults(results);
}

// execute and display benchmark results of sort algorithms on an already sorted type of array
template <uint32_t _Size, bool _IsReversed>
void measurePrintSortedArraySortBenchmarks(const std::string& title) noexcept {
  int64_t results[7][6];
  int collection[_Size];
  printf("* %s array : algorithms benchmark (ns) :\n", title.c_str());

  generateArray<(_IsReversed) ? ArrayOrder::desc : ArrayOrder::asc, ValueSign::positive>(CollectionId::randomRepeats, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::asc>(collection, results, 0u);
  generateArray<(_IsReversed) ? ArrayOrder::asc : ArrayOrder::desc, ValueSign::positive>(CollectionId::randomRepeats, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::desc>(collection, results, 3u);
  generateArray<(_IsReversed) ? ArrayOrder::desc : ArrayOrder::asc, ValueSign::negative>(CollectionId::randomRepeats, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::asc>(collection, results, 1u);
  generateArray<(_IsReversed) ? ArrayOrder::asc : ArrayOrder::desc, ValueSign::negative>(CollectionId::randomRepeats, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::desc>(collection, results, 4u);
  generateArray<(_IsReversed) ? ArrayOrder::desc : ArrayOrder::asc, ValueSign::both>(CollectionId::randomRepeats, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::asc>(collection, results, 2u);
  generateArray<(_IsReversed) ? ArrayOrder::asc : ArrayOrder::desc, ValueSign::both>(CollectionId::randomRepeats, collection, _Size);
  measureBenchmarkSortArray<_Size, pandora::logic::SortOrder::desc>(collection, results, 5u);
  printArraySortBenchmarkResults(results);
}

#undef __if_constexpr
