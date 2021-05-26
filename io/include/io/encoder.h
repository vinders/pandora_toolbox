/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <string>
#include <memory>

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
        /// @warning - 'outBuffer' must be at least 4 bytes long!
        ///          - The buffer is filled with the character bytes, but no NULL/zero character is appended -> do not use it as a string
        /// @returns Buffer indexes used (single-byte: 1 / multi-byte: 2-4)
        static size_t encode(uint32_t charCode, char outBuffer[4]) noexcept;
        /// @brief Decode unicode character code (value must not be NULL)
        /// @returns Unicode character code
        static uint32_t decode(const char value[4]) noexcept;
        
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
        /// @warning - 'outBuffer' must be at least 2 words long (4 bytes)!
        ///          - The buffer is filled with the character words, but no NULL/zero character is appended -> do not use it as a string
        /// @returns Buffer indexes used (single-word: 1 / multi-word: 2)
        static size_t encode(uint32_t charCode, char16_t outBuffer[2]) noexcept;
        /// @brief Decode unicode character code (value must not be NULL)
        /// @returns Unicode character code
        static uint32_t decode(const char16_t value[2]) noexcept;
        
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
