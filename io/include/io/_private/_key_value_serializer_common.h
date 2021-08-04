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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>

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
    
    // read boolean value from serialized data (true / false)
    // returns: pointer to last letter (or NULL if not a bool value)
    const char* _readBoolean(const char* serialized, bool& outValue) noexcept;
    // read number value from serialized data
    // returns: pointer to last digit/decimal (same as 'serialized' if not digit/sign/decimal found)
    const char* _readNumber(const char* serialized, __Number& outValue, bool& outIsInteger) noexcept;
    
    // read text between quotes + restore escaped characters -- text values/keys
    // returns: pointer to ending quote (or NULL on failure: null/zero character reached without finding it)
    // param "outValue": text value returned on success (note: if string size is 0, the value will be NULL)
    const char* _readText(const char* serialized, char** outValue, size_t& outSize);
    
    // read text (no quotes) until end symbol(s) (not skipped) + restore escaped characters -- text values/keys
    // returns: pointer to end symbol (or NULL on failure: null/zero character reached without finding end symbol(s))
    // param "outValue": text value returned on success (note: if string size is 0, the value will be NULL)
    // param "allowEndOfData": parse an entire file/payload (of no end symbol is found) -> will always return pointer (to null/zero character if no end symbol)
    // param "endSymbol#": can be a symbol like ':' or '=' to parse keys without quotes (ini/yaml/...) -> outValue may need to be trimmed!
    const char* _readText(const char* serialized, char** outValue, size_t& outSize, bool allowEndOfData, 
                          char endSymbol1, char endSymbol2 = (char)-1, char endSymbol3 = (char)-1);
  }
}
