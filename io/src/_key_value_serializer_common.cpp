/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cassert>
#include <cstring>
#include <stdexcept>
#include "io/_private/_key_value_serializer_common.h"


// -- text serialization helpers -- --------------------------------------------

// copy text value between quotes + escape quote characters (single-line) -- all text values / JSON keys
void pandora::io::_copyEscapedTextInQuotes(const char* text, std::string& outBuffer) {
  outBuffer += '"';
  if (text != nullptr) {
    const char* partBegin = text;
    for (const char* it = text; *it; ++it) {
      if ( *it == '"' || *it == '\\' || (unsigned char)*it < (unsigned char)0x20u) {
        if (it > partBegin) // don't add empty parts
          outBuffer += std::string(partBegin, it - partBegin);
        
        if ((unsigned char)*it >= (unsigned char)0x20u) { // value not skipped
          outBuffer += '\\';
          outBuffer += *it;
        }
        else {
          switch (*it) {
            case '\n': outBuffer += '\\'; outBuffer += 'n'; break;
            case '\r': outBuffer += '\\'; outBuffer += 'r'; break;
            case '\a': outBuffer += '\\'; outBuffer += 'a'; break;
            case '\t': outBuffer += '\t'; break;
            default: break;
          }
        }
        partBegin = it + 1;
      }
    }
    if (*partBegin)
      outBuffer += partBegin;
  }
  outBuffer += '"';
}

// ---

// copy comment value behing prefix (mult-line allowed)
void pandora::io::_copyComment(const char* comment, bool isInline, const char* commentPrefix, const std::string& indent, std::string& outBuffer) {
  if (comment == nullptr || *comment == 0)
    return;
  if (isInline)
    outBuffer += ' '; // add space between text and comment
  
  const char* lineBegin = comment;
  for (const char* it = comment; *it; ++it) {
    if (*it == '\n') {
      uint32_t skippedLastChar = (it > comment && *(it - 1) == '\r') ? 1 : 0; // remove CR before LF
      if (it - skippedLastChar > lineBegin) { // don't add empty lines
        if (lineBegin > comment)
          outBuffer += indent;
        outBuffer += commentPrefix;
        outBuffer += std::string(lineBegin, it - lineBegin - skippedLastChar);
        outBuffer += '\n';
      }
      lineBegin = it + 1;
    }
  }
  if (*lineBegin) {
    if (lineBegin > comment)
      outBuffer += indent;
    outBuffer += commentPrefix;
    outBuffer += lineBegin;
  }
}


// -- deserialization helpers -- -----------------------------------------------

// read boolean value from serialized data
const char* pandora::io::_readBoolean(const char* serialized, bool& outValue) noexcept {
  if (*serialized == 't') {
    if (*(++serialized) == 'r' && *(++serialized) == 'u' && *(++serialized) == 'e') {
      outValue = true;
      return serialized;
    }
  }
  else if (*serialized == 'f') {
    if (*(++serialized) == 'a' && *(++serialized) == 'l' && *(++serialized) == 's' && *(++serialized) == 'e') {
      outValue = false;
      return serialized;
    }
  }
  return nullptr;
}

// read number value from serialized data
const char* pandora::io::_readNumber(const char* serialized, __Number& outValue, bool& outIsInteger) noexcept {
  bool isDecimalFound = false;
  bool isPositive = true;
  uint64_t integer = 0;
  uint64_t decimals = 0;
  uint64_t decimalWeight = 1uLL;
  
  const char* it = serialized;
  if (*it == '-') {
    isPositive = false;
    ++it;
  }
  else if (*it == '+')
    ++it;
  
  while (*it) {
    if (*it >= '0' && *it <= '9') {
      if (isDecimalFound) {
        decimals *= 10uLL;
        decimals += static_cast<uint64_t>(*it) - (uint64_t)'0';
        decimalWeight *= 10uLL;
      }
      else {
        integer *= 10uLL;
        integer += static_cast<uint64_t>(*it) - (uint64_t)'0';
      }
    }
    else if (*it == '.') {
      if (isDecimalFound) // already found -> end of number
        break;
      isDecimalFound = true;
    }
    else // other symbol -> end of number
      break;
    ++it;
  }
  
  outIsInteger = !isDecimalFound;
  if (outIsInteger)
    outValue.integer = isPositive ? static_cast<int32_t>(integer) : -static_cast<int32_t>(integer);
  else
    outValue.number = isPositive ? static_cast<double>(integer) + (double)decimals/(double)decimalWeight
                                 : -static_cast<double>(integer) - (double)decimals/(double)decimalWeight;
  return (it > serialized) ? --it : serialized;
}

// ---

// read text between quotes + restore escaped characters -- text values/keys
const char* pandora::io::_readText(const char* serialized, char** outValue, size_t& outSize) {
  char endSymbol = *serialized;
  ++serialized;
  
  size_t bufferSize = 64;
  char* buffer = (char*)malloc(bufferSize*sizeof(char));
  char* bufferIt = buffer;
  if (buffer == nullptr)
    throw std::bad_alloc();

  size_t currentSize = 0;
  while (*serialized) {
    // end of string
    if (*serialized == endSymbol) {
      // resize buffer to fit actual string
      *bufferIt = (char)0;
      if (currentSize + 1 < bufferSize) {
        if (currentSize > 0) {
          buffer = (char*)realloc(buffer, currentSize + 1);
          assert(buffer != nullptr);
        }
        else {
          free(buffer);
          buffer = nullptr;
        }
      }
      // output value
      outSize = currentSize;
      *outValue = buffer;
      return serialized;
    }
    
    if (*serialized == '\\') {
      if (*(++serialized) == 0)
        break;
      switch (*serialized) {
        case 'n': *bufferIt = '\n'; break;
        case 'r': *bufferIt = '\r'; break;
        case 't': *bufferIt = '\t'; break;
        case 'b': *bufferIt = '\b'; break;
        case 'a': *bufferIt = '\a'; break;
        case '0': *bufferIt = '\0'; break;
        //case 'x': TODO unicode; break;
        //case 'u': TODO unicode; break;
        default: *bufferIt = *serialized; break;
      }
    }
    else
      *bufferIt = *serialized;
    ++serialized;
    
    // adjust buffer size if needed
    ++currentSize;
    if (currentSize + 1u >= bufferSize) {
      bufferSize = bufferSize << 1;
      char* newBuffer = (char*)realloc(buffer, bufferSize);
      if (newBuffer == nullptr) {
        free(buffer); // if failed, old block still exists
        throw std::bad_alloc();
      }
      buffer = newBuffer;
      bufferIt = &buffer[currentSize];
    }
    else
      ++bufferIt;
  }
  // failure -> release buffer
  free(buffer);
  return nullptr;
}

// read text + restore escaped characters -- text values/keys
const char* pandora::io::_readText(const char* serialized, char** outValue, size_t& outSize, 
                                   bool allowEndOfData, char endSymbol1, char endSymbol2, char endSymbol3) {
  if (endSymbol2 == (char)-1)
    endSymbol2 = endSymbol1;
  if (endSymbol3 == (char)-1)
    endSymbol3 = endSymbol1;
  
  size_t bufferSize = 64;
  char* buffer = (char*)malloc(bufferSize*sizeof(char));
  char* bufferIt = buffer;
  if (buffer == nullptr)
    throw std::bad_alloc();

  size_t currentSize = 0;
  while (*serialized) {
    // end of string
    if (*serialized == endSymbol1 || *serialized == endSymbol2 || *serialized == endSymbol3) {
      if (currentSize == 0) {
        free(buffer);
        buffer = nullptr;
      }
      else
        *bufferIt = (char)0;
      
      outSize = currentSize; // no buffer resize: value must be trimmed -> too big anyway
      *outValue = buffer;
      return serialized;
    }
    
    if (*serialized == '\\') {
      if (*(++serialized) == 0)
        break;
      switch (*serialized) {
        case 'n': *bufferIt = '\n'; break;
        case 'r': *bufferIt = '\r'; break;
        case 't': *bufferIt = '\t'; break;
        case 'b': *bufferIt = '\b'; break;
        case 'a': *bufferIt = '\a'; break;
        case '0': *bufferIt = '\0'; break;
        //case 'x': TODO unicode; break;
        //case 'u': TODO unicode; break;
        default: *bufferIt = *serialized; break;
      }
    }
    else
      *bufferIt = *serialized;
    ++serialized;
    
    // adjust buffer size if needed
    ++currentSize;
    if (currentSize + 1u >= bufferSize) {
      bufferSize = bufferSize << 1;
      char* newBuffer = (char*)realloc(buffer, bufferSize);
      if (newBuffer == nullptr) {
        free(buffer); // if failed, old block still exists
        throw std::bad_alloc();
      }
      buffer = newBuffer;
      bufferIt = &buffer[currentSize];
    }
    else
      ++bufferIt;
  }
  
  if (!allowEndOfData) {
    // failure -> release buffer
    free(buffer);
    return nullptr;
  }
  else {
    if (currentSize == 0) {
      free(buffer);
      buffer = nullptr;
    }
    else
      *bufferIt = (char)0;
    
    outSize = currentSize;
    *outValue = buffer;
    return serialized;
  }
}
