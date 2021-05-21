/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora { 
  namespace io {
    // -- indent helpers -- ----------------------------------------------------

    // add indent level suffix
    static inline void __addIndent(size_t indentSize, std::string& inOutIndent) {
      while (indentSize) {
        inOutIndent += ' ';
        --indentSize;
      }
    }
    // remove indent level suffix
    static inline void __removeIndent(size_t indentSize, std::string& inOutIndent) noexcept {
      while (indentSize && !inOutIndent.empty()) {
        inOutIndent.pop_back();
        --indentSize;
      }
    }


    // -- text serialization helpers --
    
    // copy text value between quotes + escape quote characters (single-line) -- all text values / JSON keys
    void _copyEscapedTextInQuotes(const char* text, std::string& outBuffer);
    
    // copy comment value behing prefix (mult-line allowed)
    void _copyComment(const char* comment, bool isInline, const char* commentPrefix, const std::string& indent, std::string& outBuffer);
    
    
    // -- deserialization helpers --
    
    enum class __KeyState : uint32_t {
      none = 0u,
      keyFound = 1u,
      sepFound = 2u,
      valueFound = 3u
    };
    union __Number {
      int32_t integer;
      double number;
    };
    
    // read boolean value from serialized data
    const char* _readBoolean(const char* serialized, bool& outValue) noexcept;
    // read number value from serialized data
    const char* _readNumber(const char* serialized, __Number& outValue, bool& outIsInteger) noexcept;
    
    // read text between quotes + restore escaped characters -- text values/keys
    // returns: pointer to next character after ending quote (or nullptr on failure: last character reached without finding it)
    // param "outValue": text value returned on success (note: if string size is 0, the value will be NULL)
    const char* _readText(const char* serialized, char** outValue, size_t& outSize);
    
    // read text (no quotes) until end symbol(s) + restore escaped characters -- text values/keys
    // returns: pointer to next character after end symbol (or end symbol if endSymbol==0)
    //          (or nullptr on failure: last character reached without finding end symbol(s))
    // param "outValue": text value returned on success (note: if string size is 0, the value will be NULL)
    // param "allowEndOfData": parse an entire file/payload (of no end symbol is found) -> will always return pointer to ending character if no end symbol
    // param "endSymbol#": can be a symbol like ':' or '=' to parse keys without quotes (ini/yaml/...) -> outValue may need to be trimmed!
    const char* _readText(const char* serialized, char** outValue, size_t& outSize, bool allowEndOfData, 
                          char endSymbol1, char endSymbol2 = (char)-1, char endSymbol3 = (char)-1);
  }
}
#undef __if_constexpr
