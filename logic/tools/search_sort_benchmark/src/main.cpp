/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Benchmark utility to test the efficiency of search & sorting algorithms, depending on array type.
*******************************************************************************/
#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstdio>
#include <string>
#include <chrono>
#include <logic/search.h>
#include <logic/sort.h>
#include "array_generator.h"
#include "benchmark.h"
#include "display.h"

#define _TINY_ARRAY_SIZE   10
#define _SMALL_ARRAY_SIZE  30
#define _MEDIUM_ARRAY_SIZE 100
#define _GREAT_ARRAY_SIZE  2000

using namespace pandora::logic;

// -- search benchmarks --

// benchmark - search algorithms
template <uint32_t _Size>
void _showSearchBenchmarks() noexcept {
  printf("\n---\n\n");
  for (CollectionId id = CollectionId::continuous; (uint32_t)id <= (uint32_t)CollectionId::extremes; id = (CollectionId)((uint32_t)id + 1u)) {
    measurePrintSortedArraySearchBenchmarks<_Size, pandora::logic::SortOrder::asc>(id);
    printf("\n");
    measurePrintSortedArraySearchBenchmarks<_Size, pandora::logic::SortOrder::desc>(id);
    printf("\n---\n\n");
  }
}


// -- sort benchmarks --

// benchmark - sort algorithms
template <uint32_t _Size>
void _showSortBenchmarks() noexcept {
  printf("\n---\n\n");
  measurePrintSortedArraySortBenchmarks<_Size,false>("Already sorted");
  printf("\n---\n\n");
  measurePrintSortedArraySortBenchmarks<_Size,true>("Reverse sorted");
  printf("\n---\n\n");

  for (CollectionId id = CollectionId::continuous; (uint32_t)id <= (uint32_t)CollectionId::extremes; id = (CollectionId)((uint32_t)id + 1u)) {
    measurePrintArraySortBenchmarks<_Size>(id);
    printf("\n---\n\n");
  }
}


// -- menus --

// menu - show test collections (size choice)
void showTestCollections() noexcept {
  clearScreen();
  printTitle("Benchmark utility: show test collections");

  printf("Collection size :\n");
  printMenu<5>({ "Return to menu...", "10 samples", "30 samples", "100 samples", "2000 samples" });
  int option = readNumericInput(0, 4);
  switch (option) {
    case 1: printAllArrays<_TINY_ARRAY_SIZE>(); break;
    case 2: printAllArrays<_SMALL_ARRAY_SIZE>(); break;
    case 3: printAllArrays<_MEDIUM_ARRAY_SIZE>(); break;
    case 4: printAllArrays<_GREAT_ARRAY_SIZE>(); break;
    case 0:
    default: return;
  }
}

// menu - show search benchmarks (size choice)
void showSearchBenchmarksMenu() noexcept {
  clearScreen();
  printTitle("Benchmark utility: search algorithms");

  printf("Collection size :\n");
  printMenu<5>({ "Return to menu...", "10 samples", "30 samples", "100 samples", "2000 samples" });
  int option = readNumericInput(0, 4);
  switch (option) {
    case 1: _showSearchBenchmarks<_TINY_ARRAY_SIZE>(); break;
    case 2: _showSearchBenchmarks<_SMALL_ARRAY_SIZE>(); break;
    case 3: _showSearchBenchmarks<_MEDIUM_ARRAY_SIZE>(); break;
    case 4: _showSearchBenchmarks<_GREAT_ARRAY_SIZE>(); break;
    case 0:
    default: return;
  }
}

// menu - show sort benchmarks (size choice)
void showSortBenchmarksMenu() noexcept {
  clearScreen();
  printTitle("Benchmark utility: sort algorithms");

  printf("Collection size :\n");
  printMenu<5>({ "Return to menu...", "10 samples", "30 samples", "100 samples", "2000 samples" });
  int option = readNumericInput(1, 4);
  switch (option) {
    case 1: _showSortBenchmarks<_TINY_ARRAY_SIZE>(); break;
    case 2: _showSortBenchmarks<_SMALL_ARRAY_SIZE>(); break;
    case 3: _showSortBenchmarks<_MEDIUM_ARRAY_SIZE>(); break;
    case 4: _showSortBenchmarks<_GREAT_ARRAY_SIZE>(); break;
    case 0:
    default: return;
  }
}

// ---

// Main loop of benchmark utility
int main() {
  bool isRunning = true;
  while (isRunning) {
    clearScreen();
    printTitle("Benchmark utility: search/sort algorithms");

    printMenu<4>({ "Exit...", "Show test collections", "Benchmark search algorithms", "Benchmark sort algorithms" });
    int option = readNumericInput(1, 3);
    switch (option) {
      case 1: showTestCollections(); break;
      case 2: showSearchBenchmarksMenu(); break;
      case 3: showSortBenchmarksMenu(); break;
      case 0:
      default: isRunning = false; break;
    }
    if (option != 0)
      printReturn();
  }
  exit(0);
}
