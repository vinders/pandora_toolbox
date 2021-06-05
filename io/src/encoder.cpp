/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cstring>
#include <system/force_inline.h>
#include "io/encoder.h"

#define __P_BUFFER8_SIZE 512
#define __P_BUFFER16_SIZE 256

#define __P_UNKNOWN_CHAR 0xFFFDu

using namespace pandora::io;


// -- Encoders/decoders -- -----------------------------------------------------

// Check system memory endianness
static inline bool __isMemoryBigEndian() noexcept {
  char16_t test = 0x0A0B;
  return (*reinterpret_cast<char*>(&test) == 0x0A);
}

// Encode unicode character code to UTF-8
// * U+0000 - U+007F : 1 byte (same value)
// * U+0080 - U+07FF : 2 bytes : 0xC0 + top 5 bits / 0x80 + low 6 bits
// * U+0800 - U+FFFF : 3 bytes : 0xE0 + top 4 bits / 0x80 + mid 6 bits / 0x80 + low 6 bits
// * U+10000 - U+10FFFF : 4 bytes : 0xF0 + top 3 bits / 0x80 + mid 6 bits / 0x80 + mid 6 bits / 0x80 + low 6 bits
static __forceinline size_t __encodeUtf8(uint32_t charCode, char* outBuffer) noexcept {
  // multi-byte
  if (charCode > (uint32_t)0x7Fu) {
    if (charCode > (uint32_t)0x7FFu) {
      if (charCode > (uint32_t)0xFFFFu) {
        outBuffer[0] = static_cast<char>(0xF0u | ((charCode >> 18) & 0x07)); // truncate values out of range
        outBuffer[1] = static_cast<char>(0x80u | ((charCode >> 12) & 0x3F));
        outBuffer[2] = static_cast<char>(0x80u | ((charCode >> 6) & 0x3F));
        outBuffer[3] = static_cast<char>(0x80u | (charCode & 0x3F));
        return size_t{4u};
      }
      outBuffer[0] = static_cast<char>(0xE0u | (charCode >> 12));
      outBuffer[1] = static_cast<char>(0x80u | ((charCode >> 6) & 0x3F));
      outBuffer[2] = static_cast<char>(0x80u | (charCode & 0x3F));
      return size_t{3u};
    }
    outBuffer[0] = static_cast<char>(0xC0u | (charCode >> 6));
    outBuffer[1] = static_cast<char>(0x80u | (charCode & 0x3F));
    return size_t{2u};
  }
  // single byte
  *outBuffer = (char)charCode;
  return size_t{1u};
}
// Decode unicode character code from UTF-8 
// (returned size must never be 0 -> would result in infinite loops for calling functions)
static __forceinline uint32_t __decodeUtf8(const char* value, size_t indexesAfterCurrent, size_t& outLength) noexcept {
  // multi-word
  if (*value & 0x80) {
    if ((*value & 0xE0) != 0xC0) {
      if ((*value & 0xF0) != 0xE0) {
        if ((*value & 0xF8) != 0xF0 || indexesAfterCurrent < size_t{3u}) {
          outLength = size_t{1u};
          return __P_UNKNOWN_CHAR;
        }
        outLength = size_t{4u};
        return ( ((static_cast<uint32_t>(*value)   & 0x07u) << 18) 
               | ((static_cast<uint32_t>(value[1]) & 0x3Fu) << 12) 
               | ((static_cast<uint32_t>(value[2]) & 0x3Fu) << 6) 
               |  (static_cast<uint32_t>(value[3]) & 0x3Fu) );
      }
      
      if (indexesAfterCurrent < size_t{2u}) {
        outLength = size_t{1u};
        return __P_UNKNOWN_CHAR;
      }
      outLength = size_t{3u};
      return ( ((static_cast<uint32_t>(*value)   & 0x0Fu) << 12) 
             | ((static_cast<uint32_t>(value[1]) & 0x3Fu) << 6) 
             |  (static_cast<uint32_t>(value[2]) & 0x3Fu) );
    }
    
    if (indexesAfterCurrent < size_t{1u}) {
      outLength = size_t{1u};
      return __P_UNKNOWN_CHAR;
    }
    outLength = size_t{2u};
    return ( ((static_cast<uint32_t>(*value) & 0x1Fu) << 6) | (static_cast<uint32_t>(value[1]) & 0x3Fu) );
  }
  // single word
  outLength = size_t{1u};
  return (static_cast<uint32_t>(*value) & 0x00FFu); // mask: to avoid duplication of most-significant-bit
}

// ---

// Encode unicode character code to UTF-16
// * U+0000 - U+D7FF and U+E000 - U+FFFF : 1 word (same value)
// * U+010000 - U+10FFFF : 2 words: 0xD800 + top 10 bits / 0xDC00 + low 10 bits
// * U+D800 - U+DFFF : reserved
static __forceinline size_t __encodeUtf16(uint32_t charCode, char16_t* outBuffer) noexcept {
  // single word
  if (charCode <= (uint32_t)0xFFFFu) {
    *outBuffer = (charCode < (uint32_t)0xD800u || charCode > (uint32_t)0xDFFFu) ? static_cast<char16_t>(charCode) : (char16_t)__P_UNKNOWN_CHAR;
    return size_t{1u};
  }
  // multi-word
  charCode -= 0x10000u;
  outBuffer[0] = static_cast<char16_t>((uint32_t)0xD800u | ((charCode >> 10) & 0x3FFu));
  outBuffer[1] = static_cast<char16_t>((uint32_t)0xDC00u | (charCode & 0x3FFu));
  return size_t{2u};
}
// Decode unicode character code from UTF-16
// (returned size must never be 0 -> would result in infinite loops for calling functions)
static __forceinline uint32_t __decodeUtf16(char highBitsWord1, char lowBitsWord1, 
                                            char highBitsWord2, char lowBitsWord2, size_t& outLength) noexcept {
  // multi-word
  if ((highBitsWord1 & 0xFC) == 0xD8) {
    if ((highBitsWord2 & 0xFC) != 0xDC) {
      outLength = size_t{2u};
      return __P_UNKNOWN_CHAR; // report invalid value
    }
    outLength = size_t{4u};
    return ( (((uint32_t)highBitsWord1 & 0x3u) << 18) | (((uint32_t)lowBitsWord1 & 0xFF) << 10) 
           | (((uint32_t)highBitsWord2 & 0x3u) << 8)  | ((uint32_t)lowBitsWord2 & 0xFF) )
           + 0x10000u;
  }
  // single word
  outLength = size_t{2u};
  return ((((uint32_t)highBitsWord1 & 0xFF) << 8) | ((uint32_t)lowBitsWord1 & 0xFF)); // masks: to avoid duplication of most-significant-bit
}
// Decode unicode character code from UTF-16
// (returned size must never be 0 -> would result in infinite loops for calling functions)
static __forceinline uint32_t __decodeUtf16(uint32_t word1, uint32_t word2, size_t& outLength) noexcept {
  // multi-word
  if ((word1 & 0xFC00) == 0xD800) {
    if ((word2 & 0xFC00) != 0xDC00) {
      outLength = size_t{2u};
      return __P_UNKNOWN_CHAR; // report invalid value
    }
    outLength = size_t{4u};
    return ( ((word1 & 0x3FFu) << 10) | (word2 & 0x3FFu) ) + 0x10000u;
  }
  // single word
  outLength = size_t{2u};
  return word1;
}


// -- UTF-8 -- -----------------------------------------------------------------

// Convert unicode string from UTF-16BE to UTF-8
static std::string __utf16BE_toUtf8(const char* data, size_t length) {
  std::string result;
  char buffer[__P_BUFFER8_SIZE];
  char* bufferIt = &buffer[0];
  size_t bufferLength = 0;
  
  uint32_t charCode = 0;
  size_t sourceCharLength = 0, outputCharLength = 0;
  char nextSourceWordHighBits = 0, nextSourceWordLowBits = 0;
  
  while (length >= size_t{2u}) {
    // decode UTF-16
    if (length >= size_t{4u}) {
      nextSourceWordHighBits = data[2];
      nextSourceWordLowBits = data[3];
    }
    else
      nextSourceWordHighBits = nextSourceWordLowBits = (char)0;
    
    charCode = __decodeUtf16(*data, data[1], nextSourceWordHighBits, nextSourceWordLowBits, sourceCharLength);
    data += sourceCharLength;
    length -= sourceCharLength;
    
    // encode UTF-8
    outputCharLength = __encodeUtf8(charCode, bufferIt);
    bufferLength += outputCharLength;
    bufferIt += outputCharLength;
    
    if (bufferLength + 5u >= __P_BUFFER8_SIZE) { // limit number of concats as much as possible
      result.append(&buffer[0], bufferLength);
      bufferLength = 0;
      bufferIt = &buffer[0];
    }
  }
  if (bufferLength > 0)
    result.append(&buffer[0], bufferLength);
  return result;
}

// Convert unicode string from UTF-16LE to UTF-8
static std::string __utf16LE_toUtf8(const char* data, size_t length) {
  std::string result;
  char buffer[__P_BUFFER8_SIZE];
  char* bufferIt = &buffer[0];
  size_t bufferLength = 0;
  
  uint32_t charCode = 0;
  size_t sourceCharLength = 0, outputCharLength = 0;
  char nextSourceWordHighBits = 0, nextSourceWordLowBits = 0;
  
  while (length >= size_t{2u}) {
    // decode UTF-16
    if (length >= size_t{4u}) { // reversed
      nextSourceWordHighBits = data[3];
      nextSourceWordLowBits = data[2];
    }
    else
      nextSourceWordHighBits = nextSourceWordLowBits = (char)0;
    
    charCode = __decodeUtf16(data[1], *data, nextSourceWordHighBits, nextSourceWordLowBits, sourceCharLength); // reversed
    data += sourceCharLength;
    length -= sourceCharLength;
    
    // encode UTF-8
    outputCharLength = __encodeUtf8(charCode, bufferIt);
    bufferLength += outputCharLength;
    bufferIt += outputCharLength;
    
    if (bufferLength + 5u >= __P_BUFFER8_SIZE) { // limit number of concats as much as possible
      result.append(&buffer[0], bufferLength);
      bufferLength = 0;
      bufferIt = &buffer[0];
    }
  }
  if (bufferLength > 0)
    result.append(&buffer[0], bufferLength);
  return result;
}

// ---

// Encode unicode character code (outBuffer must not be NULL)
size_t Encoder::Utf8::encode(uint32_t charCode, char outBuffer[_P_UTF8_MAX_CODE_SIZE]) noexcept {
  return __encodeUtf8(charCode, (char*)outBuffer);
}
// Decode unicode character code (value must not be NULL)
uint32_t Encoder::Utf8::decode(const char value[_P_UTF8_MAX_CODE_SIZE]) noexcept {
  size_t length = 0;
  return __decodeUtf8((const char*)value, size_t{_P_UTF8_MAX_CODE_SIZE - 1}, length);
}

// ---

// Convert file data (from specified encoding) to UTF-8 string (+ remove byte-order-mark)
std::string Encoder::Utf8::fromFile(const char* fileData, size_t length, Encoding fileEncoding) {
  if (fileData != nullptr) {
    if (length > size_t{1u}) {
      if (fileEncoding == Encoding::any)
        fileEncoding = Encoder::detectEncoding(fileData, length);
    
      switch (fileEncoding) {
        // UTF-8 -> copy as is
        case Encoding::utf8: {
          if (length >= 3 && memcmp((void*)fileData, (void*)Encoder::Utf8::byteOrderMark(), 3*sizeof(char)) == 0) {
            fileData += 3; // skip BOM
            length -= 3u;
          }
          return (length > 0) ? std::string(fileData, length) : std::string{};
        }
        // UTF-16BE -> re-encode
        case Encoding::utf16BE: {
          if (*fileData == '\xFE' && *(fileData + 1) == '\xFF') {
            fileData += 2; // skip BOM
            length -= 2u;
          }
          return __utf16BE_toUtf8(fileData, length);
        }
        // UTF-16LE -> re-encode
        default: {
          if (*fileData == '\xFF' && *(fileData + 1) == '\xFE') { // reversed
            fileData += 2; // skip BOM
            length -= 2u;
          }
          return __utf16LE_toUtf8(fileData, length);
        }
      }
    }
    else if (length == size_t{1u}) // single character
      return std::string(size_t{1u}, *fileData);
  }
  return std::string{}; // NULL fileData
}

// ---

// Convert UTF-8 string to UTF-8 file data (with byte-order-mark)
std::unique_ptr<char[]> Encoder::Utf8::toFile(const char* valueUtf8, size_t length, size_t& outFileLength, bool addBom) {
  if (valueUtf8 != nullptr && length > 0) {
    std::unique_ptr<char[]> outFileData = nullptr;
    if (addBom) {
      outFileLength = length + 3u;
      outFileData = std::unique_ptr<char[]>(new char[outFileLength]);
      memcpy((void*)&outFileData[0], (void*)Encoder::Utf8::byteOrderMark(), 3u*sizeof(char));
      memcpy((void*)&outFileData[3], (void*)valueUtf8, length*sizeof(char));
    }
    else {
      outFileLength = length;
      outFileData = std::unique_ptr<char[]>(new char[outFileLength]);
      memcpy((void*)&outFileData[0], (void*)valueUtf8, length*sizeof(char));
    }
    return outFileData;
  }
  outFileLength = 0;
  return nullptr;
}

// ---

// Convert UTF-16 string to UTF-8 string 
std::string Encoder::Utf8::fromUtf16(const std::u16string& source) {
  if (__isMemoryBigEndian())
    return __utf16BE_toUtf8(reinterpret_cast<const char*>(source.c_str()), source.size() << 1);
  else
    return __utf16LE_toUtf8(reinterpret_cast<const char*>(source.c_str()), source.size() << 1);
}


// -- UTF-16 -- ----------------------------------------------------------------

// Convert unicode string from UTF-8 to UTF-16
static std::u16string __utf8_toUtf16(const char* data, size_t length) {
  std::u16string result;
  char16_t buffer[__P_BUFFER16_SIZE];
  char16_t* bufferIt = &buffer[0];
  size_t bufferLength = 0;
  
  uint32_t charCode = 0;
  size_t sourceCharLength = 0, outputCharLength = 0;
  
  while (length) {
    // decode UTF-16
    charCode = __decodeUtf8(data, length - 1u, sourceCharLength);
    data += sourceCharLength;
    length -= sourceCharLength;
    
    // encode UTF-16
    outputCharLength = __encodeUtf16(charCode, bufferIt);
    bufferLength += outputCharLength;
    bufferIt += outputCharLength;
    
    if (bufferLength + 3u >= __P_BUFFER16_SIZE) { // limit number of concats as much as possible
      result.append(&buffer[0], bufferLength);
      bufferLength = 0;
      bufferIt = &buffer[0];
    }
  }
  if (bufferLength > 0)
    result.append(&buffer[0], bufferLength);
  return result;
}

// ---

// Encode unicode character code (outBuffer must not be NULL)
size_t Encoder::Utf16::encode(uint32_t charCode, char16_t outBuffer[_P_UTF16_MAX_CODE_SIZE]) noexcept {
  return __encodeUtf16(charCode, (char16_t*)outBuffer);
}
// Decode unicode character code (value must not be NULL)
uint32_t Encoder::Utf16::decode(const char16_t value[_P_UTF16_MAX_CODE_SIZE]) noexcept {
  size_t length = 0;
  return __decodeUtf16((uint32_t)value[0] & 0xFFFF, (uint32_t)value[1] & 0xFFFF, length);
}

// ---

// Convert file data (from specified encoding) to UTF-16 string (+ remove byte-order-mark)
std::u16string Encoder::Utf16::fromFile(const char* fileData, size_t length, Encoding fileEncoding) {
  if (fileData != nullptr && length >= size_t{2u}) {
    if (fileEncoding == Encoding::any)
      fileEncoding = Encoder::detectEncoding(fileData, length);
    
    switch (fileEncoding) {
      // UTF-8 -> re-encode
      case Encoding::utf8: {
        if (length >= 3 && memcmp((void*)fileData, (void*)Encoder::Utf8::byteOrderMark(), 3*sizeof(char)) == 0) {
          fileData += 3; // skip BOM
          length -= 3u;
        }
        return __utf8_toUtf16(fileData, length);
      }
      // UTF-16BE -> copy (fix byte order)
      case Encoding::utf16BE: {
        if (*fileData == '\xFE' && *(fileData + 1) == '\xFF') {
          fileData += 2; // skip BOM
          length -= 2u;
        }
        if (__isMemoryBigEndian())
          return (length >= size_t{2u}) ? std::u16string(reinterpret_cast<const char16_t*>(fileData), length >> 1) : std::u16string{};
        
        std::u16string result;
        while (length >= size_t{2u}) {
          result += static_cast<char16_t>(((uint32_t)*fileData << 8) | ((uint32_t)fileData[1] & 0xFF));
          fileData += 2;
          length -= 2u;
        }
        return result;
      }
      // UTF-16LE -> copy (fix byte order)
      default: {
        if (*fileData == '\xFF' && *(fileData + 1) == '\xFE') { // reversed
          fileData += 2; // skip BOM
          length -= 2u;
        }
        if (!__isMemoryBigEndian())
          return (length >= size_t{2u}) ? std::u16string(reinterpret_cast<const char16_t*>(fileData), length >> 1) : std::u16string{};
        
        std::u16string result;
        while (length >= size_t{2u}) {
          result += static_cast<char16_t>(((uint32_t)fileData[1] << 8) | ((uint32_t)fileData[1] & 0xFF));
          fileData += 2;
          length -= 2u;
        }
        return result;
      }
    }
  }
  return std::u16string{};
}

// ---

// Convert UTF-16 string to UTF-16 big-endian file data (with byte-order-mark)
std::unique_ptr<char[]> Encoder::Utf16::toFileBE(const char16_t* valueUtf16, size_t length, size_t& outFileLength) {
  if (valueUtf16 != nullptr && length > 0) {
    outFileLength = (length << 1) + 2u;
    std::unique_ptr<char[]>  outFileData = std::unique_ptr<char[]>(new char[outFileLength]);
    memcpy((void*)outFileData.get(), (void*)Encoder::Utf16::byteOrderMarkBE(), 2u*sizeof(char));
    
    if (__isMemoryBigEndian()) {
      memcpy((void*)&(outFileData.get()[2]), (void*)valueUtf16, (length << 1)*sizeof(char));
    }
    else {
      const char* byteValue = (const char*)valueUtf16;
      for (char* it = &(outFileData.get()[2]); length; it += 2, byteValue += 2, --length) {
        *it = byteValue[1];
        it[1] = *byteValue;
      }
    }
    return outFileData;
  }
  outFileLength = 0;
  return nullptr;
}

// Convert UTF-16 string to UTF-16 little-endian file data (with byte-order-mark)
std::unique_ptr<char[]> Encoder::Utf16::toFileLE(const char16_t* valueUtf16, size_t length, size_t& outFileLength) {
  if (valueUtf16 != nullptr && length > 0) {
    outFileLength = (length << 1) + 2u;
    std::unique_ptr<char[]>  outFileData = std::unique_ptr<char[]>(new char[outFileLength]);
    memcpy((void*)outFileData.get(), (void*)Encoder::Utf16::byteOrderMarkLE(), 2u*sizeof(char));
    
    if (!__isMemoryBigEndian()) {
      memcpy((void*)&(outFileData.get()[2]), (void*)valueUtf16, (length << 1)*sizeof(char));
    }
    else {
      const char* byteValue = (const char*)valueUtf16;
      for (char* it = &(outFileData.get()[2]); length; it += 2, byteValue += 2, --length) {
        *it = byteValue[1];
        it[1] = *byteValue;
      }
    }
    return outFileData;
  }
  outFileLength = 0;
  return nullptr;
}

// ---

// Convert UTF-8 string to UTF-16 string
std::u16string Encoder::Utf16::fromUtf8(const std::string& source) {
  return __utf8_toUtf16(source.c_str(), source.size());
}


// -- Encoder -- ---------------------------------------------------------------

// Auto-detect unicode file encoding
Encoding Encoder::detectEncoding(const char* fileData, size_t length) noexcept {
  if (fileData != nullptr) {
    // byte order mark
    if (length >= 2) {
      if (length >= 3 && memcmp((void*)fileData, (void*)Encoder::Utf8::byteOrderMark(), 3*sizeof(char)) == 0)
        return Encoding::utf8;
      if (*fileData == *(Encoder::Utf16::byteOrderMarkBE()) && fileData[1] == Encoder::Utf16::byteOrderMarkBE()[1])
        return Encoding::utf16BE;
      if (*fileData == *(Encoder::Utf16::byteOrderMarkLE()) && fileData[1] == Encoder::Utf16::byteOrderMarkLE()[1])
        return Encoding::utf16LE;
    }
    
    uint32_t pairFirstZero = 0, pairLastZero = 0;
    for (size_t i = 0; i < length; ++i, ++fileData) {
      if (*fileData == (char)0) {
        if (i & 0x1) { // 2nd character of current pair
          if (*(fileData - 1) == '\n') // i&0x1 : i>=1 : we can safely check index i-1
            return Encoding::utf16LE;
          ++pairLastZero;
        }
        else if (i < length - 1) { // 1st character of current pair
          if (*(fileData + 1) == '\n') // (i < length - 1) : not last index : we can safely check index i+1
            return Encoding::utf16BE;
          ++pairFirstZero;
        }
      }
    }
    
    if (pairFirstZero > 0 || pairLastZero > 0) // 'NULL' character found before the end -> UTF-16
      return (pairFirstZero >= pairLastZero) ? Encoding::utf16BE : Encoding::utf16LE;
  }
  return Encoding::utf8; // no 'NULL' character found before the end (or empty string) -> UTF-8
}
