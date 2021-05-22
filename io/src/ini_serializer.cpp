/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cassert>
#include <cstring>
#include <stdexcept>
#include "io/_private/_key_value_serializer_common.h"
#include "io/ini_serializer.h"

using namespace pandora::io;


// -- Serializer -- ------------------------------------------------------------

// copy text value without quotes + escape special characters -- INI keys
static inline void __copyEscapedKeyText(const char* text, std::string& outBuffer) {
  outBuffer = "";
  if (text != nullptr) {
    while (*text == ' ')
      ++text;

    const char* partBegin = text;
    for (const char* it = text; *it; ++it) {
      // if character to escape/skip (INI -> []{};=)
      if (*it == '\\' || *it == '#' || ((int32_t)*it & (int32_t)0x9F) == (int32_t)0x1B || ((int32_t)*it & (int32_t)0x9F) == (int32_t)0x1D //3B-5B-7B-3D-5D-7D equals ;[{=]}
      ||  (unsigned char)*it < (unsigned char)0x20u) {
        if (it > partBegin) // don't add empty parts
          outBuffer += std::string(partBegin, it - partBegin);
          
        if ((unsigned char)*it >= (unsigned char)0x20u) { // value not skipped
          outBuffer += '\\';
          outBuffer += *it;
        }
        else if (*it == '\n') {
          outBuffer += '\\';
          outBuffer += 'n';
        }
        partBegin = it + 1;
      }
    }
    if (*partBegin)
      outBuffer += partBegin;

    while (!outBuffer.empty() && outBuffer.back() == ' ')
      outBuffer.pop_back();
  }
}

// INI array:
//   key[]=int
//   key[]="text"
//   key[]=[object]
//   key[][]=subarray-value1
void IniSerializer::_toArray(const SerializableValue::Array* arrayRef, const std::string& currentKey, std::string& outBuffer) {
  if (arrayRef != nullptr) {
    uint32_t length = (uint32_t)arrayRef->size();
    uint32_t i = 0;
    for (const SerializableValue* it = &(*arrayRef)[0]; i < length; ++i, ++it) {
      switch (it->type()) {
        // inline values
        case SerializableValue::Type::integer: outBuffer += currentKey; outBuffer += "[]="; outBuffer += std::to_string(it->getInteger()); break;
        case SerializableValue::Type::number:  outBuffer += currentKey; outBuffer += "[]="; outBuffer += std::to_string(it->getNumber()); break;
        case SerializableValue::Type::boolean: outBuffer += currentKey; outBuffer += "[]="; outBuffer += it->getBoolean() ? "true" : "false"; break;
        case SerializableValue::Type::text:    outBuffer += currentKey; outBuffer += "[]="; _copyEscapedTextInQuotes(it->getText(), outBuffer); break;
        case SerializableValue::Type::arrays:
          throw std::invalid_argument("IniSerializer: INI format does not allow sub-arrays in arrays");
        case SerializableValue::Type::object:
          throw std::invalid_argument("IniSerializer: INI format does not allow object sections in arrays");
        default: break;
      }
      if (it->comment() != nullptr)
        _copyComment(it->comment(), true, ";", "", outBuffer);
      outBuffer += '\n';
    }
  }
}

// INI object:
//   [object]
//   key1=int
//   key2="text"
//   key3=[subobject]
//   key4[]=subarray-value1
//   key4[]=subarray-value2
void IniSerializer::_toObject(const SerializableValue::Object* object, bool isRoot, std::string& outBuffer) {
  std::string currentKey, subObjectsBuffer;
  
  if (object != nullptr && !object->empty()) {
    for (const auto& it : *object) {
      __copyEscapedKeyText(it.first.c_str(), currentKey);
      if (currentKey.empty())
        throw std::invalid_argument("IniSerializer: INI format does not allow empty keys (or only made of spaces)");
      
      // object scope -> add comment after key (before sub-values) + process separately
      if (it.second.type() == SerializableValue::Type::object) {
        if (!isRoot)
          throw std::invalid_argument("IniSerializer: INI format does not allow nested object sections");
        
        if (!subObjectsBuffer.empty())
          subObjectsBuffer += "\n\n";
        subObjectsBuffer += '['; subObjectsBuffer += currentKey; subObjectsBuffer += "]\n";
        
        if (it.second.comment() != nullptr) {
          _copyComment(it.second.comment(), false, ";", "", subObjectsBuffer);
          subObjectsBuffer += '\n';
        }
        IniSerializer::_toObject(it.second.getObject(), false, subObjectsBuffer);
      }
      else {
        switch (it.second.type()) {
          // inline values
          case SerializableValue::Type::integer: outBuffer += currentKey; outBuffer += '='; outBuffer += std::to_string(it.second.getInteger()); break;
          case SerializableValue::Type::number:  outBuffer += currentKey; outBuffer += '='; outBuffer += std::to_string(it.second.getNumber()); break;
          case SerializableValue::Type::boolean: outBuffer += currentKey; outBuffer += '='; outBuffer += it.second.getBoolean() ? "true" : "false"; break;
          case SerializableValue::Type::text:    outBuffer += currentKey; outBuffer += '='; _copyEscapedTextInQuotes(it.second.getText(), outBuffer); break;
          // array values -> add comment before list
          case SerializableValue::Type::arrays:
            if (it.second.comment() != nullptr) {
              _copyComment(it.second.comment(), false, ";", "", outBuffer);
              outBuffer += '\n';
            }
            IniSerializer::_toArray(it.second.getArray(), currentKey, outBuffer); 
            break;
          default: break;
        }
        
        if (it.second.type() != SerializableValue::Type::arrays) {
          if (it.second.comment() != nullptr)
            _copyComment(it.second.comment(), true, ";", "", outBuffer);
          outBuffer += '\n';
        }
      }
    }
    // regroup sub-objects after inline definitions
    if (!subObjectsBuffer.empty()) {
      if (!outBuffer.empty())
        outBuffer += '\n';
      outBuffer += subObjectsBuffer;
    }
  }
}

// ---

std::string IniSerializer::toString(const SerializableValue::Object& keyValueList) const {
  std::string buffer;
  buffer.reserve(32 + keyValueList.size()*64);
  IniSerializer::_toObject(&keyValueList, true, buffer);
  return buffer;
}


// -- Deserializer -- ----------------------------------------------------------

// skip INI comment data
static const char* __skipIniComment(const char* serialized) noexcept {
  assert(*serialized == ';' || *serialized == '#');
  ++serialized;
  
  while (*serialized && *serialized != '\n')
    ++serialized;
  return (serialized - 1);
}

// right-trim text value
static size_t __rtrimText(char** value, size_t length) noexcept {
  if (*value == nullptr)
    return 0;
  
  // remove spaces/tabs/new-lines (but keep explicit '\0')
  char* last = &((*value)[length - 1]);
  while (length && (unsigned char)*last <= (unsigned char)' ' && *last) {
    --last;
    --length;
  }
  if (length > 0) {
    *(last + 1) = (char)0;
  }
  else {
    free(*value);
    *value = nullptr;
  }
  return length;
}

// read object key information
static inline const char* __readSectionKey(const char* serialized, std::string& outKeyName) {
  assert(*serialized == '[');
  ++serialized;
  
  size_t length = 0;
  char* text = nullptr;
  serialized = _readText(serialized, &text, length, true, ';', '#', '\n'); // until comment or end of ID line
  if (serialized == nullptr || text == nullptr || (length = __rtrimText(&text, length)) <= (size_t)1 || text[length - 1] != ']')
    return nullptr;
  while (*serialized && *serialized != '\n')
    ++serialized;
  
  text[length - 1] = (char)0; // ignore ']'
  outKeyName = std::string(text, length - 1);
  return serialized;
}

// insert object or array item
void IniSerializer::_insertProperty(const std::string& key, bool isInArray, SerializableValue::Object& parentObject, SerializableValue&& value) {
  assert(!key.empty());
  if (!isInArray) {
    parentObject[key] = std::move(value);
  }
  else {
    auto arrayIt = parentObject.find(key);
    if (arrayIt == parentObject.end()) {
      SerializableValue::Array target;
      target.emplace_back(std::move(value));
      parentObject[key] = SerializableValue(std::move(target));
    }
    else if (arrayIt->second.type() == SerializableValue::Type::arrays) {
      _pushArrayItem(arrayIt->second, std::move(value));
    }
    //else: ignore value
  }
}

// ---

// parse INI object data
const char* IniSerializer::_readObject(const char* serialized, bool isRoot, SerializableValue::Object& outObject) {
  if (serialized == nullptr || *serialized == 0)
    return nullptr;

  std::string lastKey;
  __KeyState lastKeyState = __KeyState::none;
  bool isInArray = false;

  --serialized; // move "before" first char, to avoid skipping it with incrementation below
  do { // do{...}while(*serialized); -> no check in 1st iteration (pointer is "before" first char!)
    ++serialized; // increment AFTER checking *serialized!=0 (first iteration is also incremented to skip '{')

    if ((unsigned char)*serialized > (unsigned char)' ') { // ignore white-spaces/tabs/line-feeds/return...
      if (*serialized == ';' || *serialized == '#') { // ignore comments
        serialized = __skipIniComment(serialized);
      }
      else {
        switch (lastKeyState) {
          // find property-key or section-ID
          case __KeyState::none: {
            // object section ID
            if (*serialized == '[') {
              if (!isRoot) // only process sections at root level
                return (serialized - 1); // beginning of other object -> end of current object
            
              std::string sectionKey;
              const char* next = __readSectionKey(serialized, sectionKey);
              if (next == nullptr) { // invalid line -> ignore
                while (*serialized && *serialized != '\n')
                  ++serialized;
                break;
              }
              SerializableValue::Object object;
              serialized = _readObject(next, false, object);
              IniSerializer::_insertProperty(sectionKey, false, outObject, SerializableValue(std::move(object)));
              if (serialized == nullptr) // end of data reached -> stop parsing
                return nullptr;
            }
            // property key
            else {
              size_t length = 0;
              char* text = nullptr;
              const char* next = _readText(serialized, &text, length, false, '=');
              if (next != nullptr && (length = __rtrimText(&text, length)) > 0) {
                serialized = next; // iterator on '=' symbol -> it will be skipped by iteration increment

                // detect arrays
                isInArray = (length >= size_t{3u} && text[length - 2] == '[' && text[length - 1u] == ']');
                if (isInArray) {
                  length -= 2u;
                  text[length] = (char)0;
                  if ((length = __rtrimText(&text, length)) == 0) { // no key before brackets -> invalid
                    while (*serialized && *serialized != '\n')
                      ++serialized;
                    break;
                  }
                }
                // store key
                lastKey = std::string(text, length);
                free(text);
                lastKeyState = __KeyState::sepFound;
              }
              else { // invalid -> ignore line
                while (*serialized && *serialized != '\n')
                  ++serialized;
              }
            }
            break;
          }
          // find value after separator
          case __KeyState::sepFound: {
            switch (*serialized) {
              case '\'':
              case '"': { // text value between quotes
                size_t length = 0;
                char* text = nullptr;
                const char* next = _readText(serialized + 1, &text, length, false, *serialized, '\n');
                if (next != nullptr && *next == *serialized)
                  serialized = next; // quote symbol found -> will be skipped by next iteration increment
                else { // not in quotes (first quote is part of the string)
                  serialized = _readText(serialized, &text, length, true, ';', '#', '\n'); // until end of line (or end of data, if last line)
                  --serialized; // move before end/comment symbol (so it can be parsed after next iteration increment)
                  length = __rtrimText(&text, length);
                }
                IniSerializer::_insertProperty(lastKey, isInArray, outObject, _valueFromMovedText(length, text));
                break;
              }
              default: {
                if ((*serialized >= '0' && *serialized <= '9') || *serialized == '-' || *serialized == '+' || *serialized == '.') {
                  __Number value;
                  bool isInteger = true;
                  const char* next = _readNumber(serialized, value, isInteger);
                
                  // only use it as a number if the whole line is numeric (except for white-spaces)
                  ++next; // skip last digit
                  while (*next && *next != '\n' && (unsigned char)*next <= (unsigned char)' ')
                    ++next;
                  if (*next == '\n' || *next == 0 || *next == ';' || *next == '#') {
                    serialized = (next - 1); // before end/comment symbol (so it can be parsed after next iteration increment)
                    IniSerializer::_insertProperty(lastKey, isInArray, outObject, isInteger ? SerializableValue(value.integer) : SerializableValue(value.number));
                    break;
                  } // else: use as text
                }
            
                // text value (not in quotes) / boolean value
                size_t length = 0;
                char* text = nullptr;
                serialized = _readText(serialized, &text, length, true, ';', '#', '\n'); // until end of line (or end of data, if last line)
                --serialized; // move before end/comment symbol (so it can be parsed after next iteration increment)
                length = __rtrimText(&text, length);
              
                // boolean values
                if (length == size_t{4u} && memcmp((void*)text, (void*)"true", length*sizeof(char)) == 0)
                  IniSerializer::_insertProperty(lastKey, isInArray, outObject, SerializableValue(true));
                else if (length == size_t{5u} && memcmp((void*)text, (void*)"false", length*sizeof(char)) == 0)
                  IniSerializer::_insertProperty(lastKey, isInArray, outObject, SerializableValue(false));
                else // text value
                  IniSerializer::_insertProperty(lastKey, isInArray, outObject, _valueFromMovedText(length, text));
                break;
              }
            }
            lastKeyState = __KeyState::none;
            break;
          }
        }
      }
    }
    else if (*serialized == '\n' && lastKeyState == __KeyState::sepFound) { // no value after separator: use empty string
      IniSerializer::_insertProperty(lastKey, isInArray, outObject, _valueFromMovedText(0, nullptr));
      lastKeyState = __KeyState::none;
    }
  }
  while (*serialized);
  return nullptr;
}

// ---

SerializableValue::Object IniSerializer::fromString(const char* serialized) const {
  SerializableValue::Object rootMap;
  IniSerializer::_readObject(serialized, true, rootMap);
  return rootMap;
}
