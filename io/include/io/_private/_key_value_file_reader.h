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
#pragma once

#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace pandora { 
  namespace io {
    /// @class KeyValueFileReader
    /// @brief Property value reader in 'KEY=VALUE' formatted files
    class KeyValueFileReader final {
    public:
      /// @brief Open file and search for a property
      /// @warning The buffer should be wide enough to store an entire line of the file
      static inline size_t searchFileForValue(const char* filePath, const char* keyId, char* outBuffer, size_t bufferSize) noexcept {
        assert(filePath != nullptr && keyId != nullptr && outBuffer != nullptr && bufferSize > 0u);
        FILE* fileAccess;
        fileAccess = fopen(filePath, "r");
        if (fileAccess != nullptr) {
          size_t valueLength = searchForValue(fileAccess, keyId, outBuffer, bufferSize);
          fclose(fileAccess);
          return valueLength;
        }
        return 0u;
      }
      
      /// @brief Search for a property (in an open file)
      /// @warning The buffer should be wide enough to store an entire line of the file
      static inline size_t searchForValue(FILE* fileAccess, const char* keyId, char* outBuffer, size_t bufferSize) noexcept {
        if (fileAccess == nullptr || keyId == nullptr || outBuffer == nullptr)
          return static_cast<size_t>(0u);
        assert(bufferSize > 0u);
        size_t valueLength = 0u;
        size_t keyIdLength = strlen(keyId);

        while (!feof(fileAccess)) {
          if (fgets(outBuffer, static_cast<int>(bufferSize), fileAccess) != nullptr && memcmp(outBuffer, keyId, keyIdLength) == 0) {
            char* iter = &outBuffer[keyIdLength];
            while (*iter != 0 && *iter <= ' ')
              ++iter;
            if (*iter != 0 && *iter == '=') {
              ++iter;
              while (*iter != 0 && *iter <= ' ')
                ++iter;
            }
            
            size_t outBufferLength = strlen(outBuffer);
            valueLength = outBufferLength - static_cast<size_t>(iter - outBuffer);
            for (char* endIter = &outBuffer[outBufferLength - 1u]; endIter >= iter && *endIter <= ' '; --endIter)
              --valueLength;

            memmove(outBuffer, iter, valueLength);
            outBuffer[valueLength] = '\0';
            break;
          }
        }
        return valueLength;
      }
    };

  }
}
