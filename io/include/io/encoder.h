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

#include <cstdint>
#include <string>
#include <memory>

#define _P_UTF8_MAX_CODE_SIZE  4
#define _P_UTF16_MAX_CODE_SIZE 2

namespace pandora {
  namespace io {
    ///< Unicode character encoding mode
    enum class Encoding : uint32_t {
      any     = 0u, ///< Undefined -> auto-detection in converters
      utf8    = 1u, ///< UTF-8 (byte/multi-byte)
      utf16BE = 2u, ///< UTF-16 big-endian (word/multi-word)
      utf16LE = 3u  ///< UTF-16 little-endian (word/multi-word)
    };
    
    /// ---
    
    /// @class Encoder
    /// @brief Unicode character encoding reader/writer/converter (UTF-8 / UTF-16BE / UTF-16LE)
    class Encoder final {
    public:
      /// @brief UTF-8 encoding
      class Utf8 final {
      public:
        Utf8() = default;
        static const char* byteOrderMark() noexcept { return "\xEF\xBB\xBF"; } ///< UTF-8 byte-order-mark
        
        /// @brief Encode unicode character code (outBuffer must not be NULL)
        /// @warning - 'outBuffer' must be at least 4 bytes long -> use _P_UTF8_MAX_CODE_SIZE for buffer length
        ///          - The buffer is filled with the character bytes, but no NULL/zero character is appended -> do not use it as a string
        /// @returns Buffer indexes used (single-byte: 1 / multi-byte: 2-4)
        static size_t encode(uint32_t charCode, char outBuffer[_P_UTF8_MAX_CODE_SIZE]) noexcept;
        /// @brief Decode unicode character code (value must not be NULL)
        /// @returns Unicode character code
        static inline uint32_t decode(const char value[_P_UTF8_MAX_CODE_SIZE]) noexcept {
          size_t codeSize = 0;
          return decode((const char*)value, size_t{_P_UTF8_MAX_CODE_SIZE}, codeSize);
        }
        /// @brief Decode first character of unicode string (value must not be NULL, length >= 1) + get source char-code size
        static uint32_t decode(const char* value, size_t length, size_t& outCodeSize) noexcept;
        
        /// @brief Convert file data (from specified encoding) to UTF-8 string (+ remove byte-order-mark)
        static std::string fromFile(const char* fileData, size_t length, Encoding fileEncoding = Encoding::any);
        /// @brief Convert UTF-8 string to UTF-8 file data (with byte-order-mark)
        /// @returns File content encoded in UTF-8 (or NULL if valueUtf8 is empty)
        static std::unique_ptr<char[]> toFile(const char* valueUtf8, size_t length, size_t& outFileLength, bool addBom = true);
      
        /// @brief Convert UTF-16 string to UTF-8 string 
        static std::string fromUtf16(const std::u16string& source);
      };

      /// @brief UTF-16 encoding
      class Utf16 final {
      public:
        Utf16() = default;
        static const char* byteOrderMarkBE() noexcept { return "\xFE\xFF"; } ///< UTF-16 big-endian byte-order-mark
        static const char* byteOrderMarkLE() noexcept { return "\xFF\xFE"; } ///< UTF-16 little-endian byte-order-mark
        
        /// @brief Encode unicode character code (outBuffer must not be NULL)
        /// @warning - 'outBuffer' must be at least 2 words long (4 bytes) -> use _P_UTF16_MAX_CODE_SIZE for buffer length
        ///          - The buffer is filled with the character words, but no NULL/zero character is appended -> do not use it as a string
        /// @returns Buffer indexes used (single-word: 1 / multi-word: 2)
        static size_t encode(uint32_t charCode, char16_t outBuffer[_P_UTF16_MAX_CODE_SIZE]) noexcept;
        /// @brief Decode unicode character code (value must not be NULL)
        /// @returns Unicode character code
        static inline uint32_t decode(const char16_t value[_P_UTF16_MAX_CODE_SIZE]) noexcept {
          size_t codeSize = 0;
          return decode((const char16_t*)value, size_t{_P_UTF16_MAX_CODE_SIZE}, codeSize);
        }
        /// @brief Decode first character of unicode string (value must not be NULL, length >= 1) + get source char-code size (number of char16_t indexes)
        static uint32_t decode(const char16_t* value, size_t length, size_t& outCodeSize) noexcept;
        
        /// @brief Convert file data (from specified encoding) to UTF-16 string (+ remove byte-order-mark)
        static std::u16string fromFile(const char* fileData, size_t length, Encoding fileEncoding = Encoding::any);
        /// @brief Convert UTF-16 string to UTF-16 big-endian file data (with byte-order-mark)
        /// @warning The file will need to be written in 'binary' mode (or else, fwrite may break new-line/control characters).
        /// @returns File content encoded in UTF-16BE (or NULL if valueUtf16 is empty)
        static std::unique_ptr<char[]> toFileBE(const char16_t* valueUtf16, size_t length, size_t& outFileLength);
        /// @brief Convert UTF-16 string to UTF-16 little-endian file data (with byte-order-mark)
        /// @warning The file will need to be written in 'binary' mode (or else, fwrite may break new-line/control characters).
        /// @returns File content encoded in UTF-16LE (or NULL if valueUtf16 is empty)
        static std::unique_ptr<char[]> toFileLE(const char16_t* valueUtf16, size_t length, size_t& outFileLength);
      
        /// @brief Convert UTF-8 string to UTF-16 string
        static std::u16string fromUtf8(const std::string& source);
      };
    
      /// @brief Auto-detect unicode file encoding (much more efficient if byte-order-mark is present)
      /// @returns Detected encoding mode (defaults to UTF-8 if empty file)
      static Encoding detectEncoding(const char* fileData, size_t length) noexcept;
    };
  }
}
