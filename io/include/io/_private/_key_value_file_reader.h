/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
