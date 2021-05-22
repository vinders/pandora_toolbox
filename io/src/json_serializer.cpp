/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cassert>
#include <stdexcept>
#include "io/_private/_key_value_serializer_common.h"
#include "io/json_serializer.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

#define __P_COMMENT_PREFIX "// "
#define __P_COMMENT_CHAR   '/'

using namespace pandora::io;


// -- Serializer -- ------------------------------------------------------------

// JSON array
void JsonSerializer::_toArray(const SerializableValue::Array* arrayRef, const char* comment, size_t indentSize, 
                              std::string& currentIndent, std::string& outBuffer) {
  outBuffer += "[\n";
  __addIndent(indentSize, currentIndent); // new level
  if (comment != nullptr) {
    outBuffer += currentIndent;
    _copyComment(comment, false, __P_COMMENT_PREFIX, currentIndent, outBuffer);
    outBuffer += '\n';
  }
  
  if (arrayRef != nullptr) {
    bool hasComment;
    size_t length = arrayRef->size();

    uint32_t i = 0;
    for (const SerializableValue* it = &(*arrayRef)[0]; i < length; ++i, ++it) {
      hasComment = (it->comment() != nullptr);
      outBuffer += currentIndent;

      switch (it->type()) {
        // inline values
        case SerializableValue::Type::integer: outBuffer += std::to_string(it->getInteger()); break;
        case SerializableValue::Type::number:  outBuffer += std::to_string(it->getNumber()); break;
        case SerializableValue::Type::boolean: outBuffer += it->getBoolean() ? "true" : "false"; break;
        case SerializableValue::Type::text:    _copyEscapedTextInQuotes(it->getText(), outBuffer); break;

        // array scope -> add comment before sub-values
        case SerializableValue::Type::arrays: {
          JsonSerializer::_toArray(it->getArray(), it->comment(), indentSize, currentIndent, outBuffer); 
          hasComment = false;
          break;
        }
        // object scope -> add comment before sub-values
        case SerializableValue::Type::object: {
          JsonSerializer::_toObject(it->getObject(), it->comment(), indentSize, currentIndent, outBuffer);
          hasComment = false;
          break;
        }
        default: break;
      }
      
      // comma separated items
      if (i + 1 < (uint32_t)length)
        outBuffer += ",";
      // add comment after value (only for inline values)
      if (hasComment)
        _copyComment(it->comment(), true, __P_COMMENT_PREFIX, currentIndent, outBuffer);
      outBuffer += '\n';
    }
  }
  __removeIndent(indentSize, currentIndent); // end level
  outBuffer += currentIndent; 
  outBuffer += ']';
}

// JSON object
void JsonSerializer::_toObject(const SerializableValue::Object* object, const char* comment, size_t indentSize, 
                               std::string& currentIndent, std::string& outBuffer) {
  outBuffer += '{';
  outBuffer += '\n';
  __addIndent(indentSize, currentIndent); // new level
  if (comment != nullptr) {
    outBuffer += currentIndent;
    _copyComment(comment, false, __P_COMMENT_PREFIX, currentIndent, outBuffer);
    outBuffer += '\n';
  }

  if (object != nullptr && !object->empty()) {
    bool hasComment = false;
    size_t length = object->size();

    uint32_t i = 0;
    for (const auto& it : *object) {
      hasComment = (it.second.comment() != nullptr);
      outBuffer += currentIndent;
      _copyEscapedTextInQuotes(it.first.c_str(), outBuffer);
      outBuffer += ": ";

      switch (it.second.type()) {
        // inline values
        case SerializableValue::Type::integer: outBuffer += std::to_string(it.second.getInteger()); break;
        case SerializableValue::Type::number:  outBuffer += std::to_string(it.second.getNumber()); break;
        case SerializableValue::Type::boolean: outBuffer += it.second.getBoolean() ? "true" : "false"; break;
        case SerializableValue::Type::text:    _copyEscapedTextInQuotes(it.second.getText(), outBuffer); break;
        
        // array scope -> add comment before sub-values
        case SerializableValue::Type::arrays: {
          JsonSerializer::_toArray(it.second.getArray(), it.second.comment(), indentSize, currentIndent, outBuffer); 
          hasComment = false;
          break;
        }
        // object scope -> add comment before sub-values
        case SerializableValue::Type::object: {
          JsonSerializer::_toObject(it.second.getObject(), it.second.comment(), indentSize, currentIndent, outBuffer);
          hasComment = false;
          break;
        }
        default: break;
      }
      
      // comma separated items
      if (i + 1 < (uint32_t)length)
        outBuffer += ",";
      // add comment after value (only for inline values)
      if (hasComment)
        _copyComment(it.second.comment(), true, __P_COMMENT_PREFIX, currentIndent, outBuffer);
      outBuffer += '\n';
      ++i;
    }
  }
  __removeIndent(indentSize, currentIndent); // remove level

  if (!currentIndent.empty()) {
    outBuffer += currentIndent; 
    outBuffer += '}';
  }
  else { // end of file -> add new-line
    outBuffer += '}';
    outBuffer += '\n';
  }
}

// ---

std::string JsonSerializer::toString(const SerializableValue::Object& keyValueList) const {
  std::string buffer, indentBuffer;
  JsonSerializer::_toObject(&keyValueList, nullptr, this->_indentSize, indentBuffer, buffer);
  return buffer;
}


// -- Deserializer -- ----------------------------------------------------------

// skip JSON comment data
static const char* __skipJsonComment(const char* serialized) noexcept {
  assert(*serialized == '/');
  ++serialized;
  
  if (*serialized == '/') {
    ++serialized;
    
    while (*serialized && *serialized != '\n')
      ++serialized;
    if (*serialized)
      return (serialized + 1);
  }
  else if (*serialized == '*') {
    ++serialized;
    
    while (*serialized && (*serialized != '*' || *(serialized + 1) != '/'))
      ++serialized;
    if (*serialized)
      return (serialized + 2);
  }
  return nullptr;
}

// ---

// parse JSON array data
const char* JsonSerializer::_readArray(const char* serialized, SerializableValue::Array& outArray) {
  assert(*serialized == '[');

  __KeyState lastValueState = __KeyState::none;
  while (*serialized) {
    ++serialized; // increment AFTER checking *serialized!=0 (first iteration is also incremented to skip '[')
                  //-> if ++serialized is 0, the next IF will prevent from using it

    if ((unsigned char)*serialized > (unsigned char)' ') { // ignore white-spaces/tabs/line-feeds/return/ending-zero...
      if (*serialized == '/') { // ignore comments
        serialized = __skipJsonComment(serialized);
        if (serialized == nullptr)
          return nullptr;
        continue;
      }
      
      switch (lastValueState) {
        // find value or end symbol
        case __KeyState::none: {
          switch (*serialized) {
            case ']': return serialized; // end symbol
            case '\'':
            case '"': { // string value
              size_t length = 0;
              char* text = nullptr;
              if ((serialized = _readText(serialized, &text, length)) == nullptr)
                return nullptr;
              outArray.emplace_back(_valueFromMovedText(length, text));
              break;
            }
            case '{': { // object value
              SerializableValue::Object object;
              if ((serialized = _readObject(serialized, object)) == nullptr)
                return nullptr;
              outArray.emplace_back(std::move(object));
              break;
            }
            case '[': { // array value
              SerializableValue::Array array;
              if ((serialized = _readArray(serialized, array)) == nullptr)
                return nullptr;
              outArray.emplace_back(std::move(array));
              break;
            }
            case 'f':
            case 't': { // boolean value
              bool value = false;
              serialized = _readBoolean(serialized, value);
              if (serialized == nullptr)
                return nullptr;
              outArray.emplace_back(value);
              break;
            }
            case '.':
            case '+':
            case '-': { // number value (with sign, or floating-point with no integer part)
              __Number value;
              bool isInteger = true;
              serialized = _readNumber(serialized, value, isInteger);
              if (serialized == nullptr)
                return nullptr;
              outArray.push_back(isInteger ? SerializableValue(value.integer) : SerializableValue(value.number));
              break;
            }
            default: {
              if (*serialized >= '0' && *serialized <= '9') { // number value (no sign)
                __Number value;
                bool isInteger = true;
                serialized = _readNumber(serialized, value, isInteger);
                if (serialized == nullptr)
                  return nullptr;
                outArray.push_back(isInteger ? SerializableValue(value.integer) : SerializableValue(value.number));
                break;
              }
              return nullptr;
            }
          }
          lastValueState = __KeyState::valueFound;
          break;
        }
        // find comma after value or end symbol
        case __KeyState::valueFound: {
          if (*serialized != ',')
            return (*serialized == ']') ? serialized : nullptr; // end symbol (or invalid)
          // comma -> now expects next key or end symbol
          lastValueState = __KeyState::none;
          break;
        }
        default: return nullptr;
      }
    }
  }
  return nullptr;
}

// ---

// parse JSON object data
const char* JsonSerializer::_readObject(const char* serialized, SerializableValue::Object& outObject) {
  bool isBracketless = (*serialized != '{');
  
  __KeyState lastKeyState = __KeyState::none;
  std::string lastKey;
  while (*serialized) {
    ++serialized; // increment AFTER checking *serialized!=0 (first iteration is also incremented to skip '{')
                  //-> if ++serialized is 0, the next IF will prevent from using it

    if ((unsigned char)*serialized > (unsigned char)' ') { // ignore white-spaces/tabs/line-feeds/return/ending-zero...
      if (*serialized == '/') { // ignore comments
        serialized = __skipJsonComment(serialized);
        if (serialized == nullptr)
          return nullptr;
        continue;
      }

      switch (lastKeyState) {
        // find key or end symbol
        case __KeyState::none: {
          switch (*serialized) {
            case '\'':
            case '"': { // key string
              size_t length = 0;
              char* text = nullptr;
              if ((serialized = _readText(serialized, &text, length)) == nullptr || text == nullptr)
                return nullptr;
              lastKey = std::string(text, length);
              free(text);
              break;
            }
            case '}': return serialized; // end symbol
            default: return nullptr;
          }
          lastKeyState = __KeyState::keyFound;
          break;
        }
        // find separator after key
        case __KeyState::keyFound: {
          if (*serialized != ':')
            return nullptr;
          lastKeyState = __KeyState::sepFound;
          break;
        }
        // find value after separator
        case __KeyState::sepFound: {
          switch (*serialized) {
            case '\'':
            case '"': { // string value
              size_t length = 0;
              char* text = nullptr;
              if ((serialized = _readText(serialized, &text, length)) == nullptr)
                return nullptr;
              outObject[lastKey] = _valueFromMovedText(length, text);
              break;
            }
            case '{': { // object value
              SerializableValue::Object object;
              if ((serialized = _readObject(serialized, object)) == nullptr)
                return nullptr;
              outObject[lastKey] = SerializableValue(std::move(object));
              break;
            }
            case '[': { // array value
              SerializableValue::Array arrayVal;
              if ((serialized = _readArray(serialized, arrayVal)) == nullptr)
                return nullptr;
              outObject[lastKey] = SerializableValue(std::move(arrayVal));
              break;
            }
            case 'f':
            case 't': { // boolean value
              bool value = false;
              serialized = _readBoolean(serialized, value);
              if (serialized == nullptr)
                return nullptr;
              outObject[lastKey] = SerializableValue(value);
              break;
            }
            case '.':
            case '+':
            case '-': { // number value (with sign, or floating-point with no integer part)
              __Number value;
              bool isInteger = true;
              serialized = _readNumber(serialized, value, isInteger);
              if (serialized == nullptr)
                return nullptr;
              outObject[lastKey] = isInteger ? SerializableValue(value.integer) : SerializableValue(value.number);
              break;
            }
            default: {
              if (*serialized >= '0' && *serialized <= '9') { // number value (no sign)
                __Number value;
                bool isInteger = true;
                serialized = _readNumber(serialized, value, isInteger);
                if (serialized == nullptr)
                  return nullptr;
                outObject[lastKey] = isInteger ? SerializableValue(value.integer) : SerializableValue(value.number);
                break;
              }
              return nullptr;
            }
          }
          lastKeyState = __KeyState::valueFound;
          break;
        }
        // find comma after value or end symbol
        case __KeyState::valueFound: {
          if (*serialized != ',')
            return (*serialized == '}') ? serialized : nullptr; // end symbol (or invalid)
          // comma -> now expects new key or end symbol
          lastKeyState = __KeyState::none;
          break;
        }
      }
    }
  }
  return isBracketless ? serialized : nullptr;
}

// ---

SerializableValue::Object JsonSerializer::fromString(const char* serialized) const {
  if (serialized == nullptr)
    return SerializableValue::Object{};
  
  // find root opening bracket
  while (*serialized != '{') {
    while (*serialized && (unsigned char)*serialized <= (unsigned char)' ') // skip white-spaces
      ++serialized;
    if (*serialized == '/') // skip comments
      serialized = __skipJsonComment(serialized);
    else if (*serialized != '{') // not a comment after white-spaces -> must be bracket (or bracketless mode)
      break;
  }
  
  SerializableValue::Object rootMap;
  if (JsonSerializer::_readObject(serialized, rootMap) == nullptr)
    throw std::invalid_argument("JsonSerializer: invalid JSON format: missing symbol after value (bracket/quote/comma)");
  return rootMap;
}
