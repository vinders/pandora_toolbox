/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : display helpers for benchmark utility
*******************************************************************************/
#pragma once

#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstddef>
#include <cstdio>
#include <string>
#include <array>
#include "array_generator.h"

// -- general --

// remove all screen content
inline void clearScreen() noexcept {
# ifdef _WINDOWS
    system("cls");
# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
    if (system("clear") == -1)
      printf("\n____________________________________________________________\n");
# endif
}

// display section title
inline void printTitle(const std::string& title) {
  printf("------------------------------------------------------------\n %s\n------------------------------------------------------------\n\n", title.c_str());
}


// -- menu --

// display section menu (first entry should be return/exit command)
template <size_t _Size>
inline void printMenu(const std::array<std::string, _Size>& items) {
  for (int i = 1; i < static_cast<int>(_Size); ++i)
    printf("> %d - %s\n", i, items[i].c_str());
  printf("> 0 - %s\n\n", items[0].c_str());
}

// get numeric user input
inline int readNumericInput(int minValue, int maxValue) noexcept {
  int val = -1;
  bool isValid = false;
  printf("Enter a value (%d-%d, or 0) :\n> ", minValue, maxValue);

  do {
    fflush(stdin);
    isValid = (scanf("%d", &val) > 0 && (val == 0 || (val >= minValue && val <= maxValue)) );
    if (!isValid)
      printf("Invalid value. Please try again (%d-%d or 0) :\n> ", minValue, maxValue);
  } while (!isValid);

  while (getchar() != '\n'); // clear buffer
  return val;
}

// print "back to menu" message
inline void printReturn() {
  printf("> Press ENTER to return to menu...\n"); 
  getchar();
}


// -- content --

// display array content
inline void printArray(const std::string& label, const int* collection, size_t length) {
  std::string arrayValue;
  arrayValue.reserve(length * 5);
  arrayValue += "[ ";
  for (size_t i = length; i > 1; --i, ++collection)
    arrayValue += std::to_string(*collection) + ", ";
  arrayValue += std::to_string(*collection) + " ]";

  printf("%s: %s \n", label.c_str(), arrayValue.c_str());
}

// display all arrays
template <size_t _Size>
void printAllArrays() {
  printf("\n---\n\n");

  int collection[_Size];
  for (CollectionId id = CollectionId::continuous; (uint32_t)id <= (uint32_t)CollectionId::extremes; id = (CollectionId)((uint32_t)id + 1)) {
    std::string title = toString(id);
    printf("* %s :\n", title.c_str());

    generateArray<ArrayOrder::asc, ValueSign::positive>(id, collection, _Size);
    printArray("  - pos", collection, _Size);
    generateArray<ArrayOrder::asc, ValueSign::negative>(id, collection, _Size);
    printArray("  - neg", collection, _Size);
    generateArray<ArrayOrder::asc, ValueSign::both>(id, collection, _Size);
    printArray("  - all", collection, _Size);
    printf("\n---\n\n");
  }
}
