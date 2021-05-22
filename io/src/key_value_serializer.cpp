/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cassert>
#include <cstring>
#include <stdexcept>
#include "io/key_value_serializer.h"

using namespace pandora::io;


// -- SerializableValue -- -----------------------------------------------------

SerializableValue::SerializableValue(SerializableValue&& rhs) noexcept {
  _value.integer = 0;
  memcpy((void*)this, (void*)&rhs, sizeof(SerializableValue));
  rhs._comment = nullptr;
  rhs._valueType = Type::integer;
}
SerializableValue& SerializableValue::operator=(SerializableValue&& rhs) noexcept {
  _destroy();
  memcpy((void*)this, (void*)&rhs, sizeof(SerializableValue));
  rhs._comment = nullptr;
  rhs._valueType = Type::integer;
  return *this;
}

void SerializableValue::_copy(const SerializableValue& rhs) {
  this->_valueType = rhs._valueType;
  this->_length = rhs._length;

  if (rhs._comment != nullptr && *rhs._comment) {
    size_t commentLength = strlen(rhs._comment);
    this->_comment = (char*)malloc((commentLength + 1)*sizeof(char));
    if (this->_comment == nullptr)
      throw std::bad_alloc();
    memcpy((void*)this->_comment, (void*)rhs._comment, commentLength*sizeof(char));
    this->_comment[commentLength] = (char)0;
  }

  switch (rhs._valueType) {
    case Type::integer: this->_value.integer = rhs._value.integer; break;
    case Type::number: this->_value.number = rhs._value.number; break;
    case Type::boolean: this->_value.boolean = rhs._value.boolean; break;
    case Type::text: 
      this->_value.text = nullptr;
      if (rhs._value.text != nullptr && rhs._length > 0) {
        this->_value.text = (char*)malloc((rhs._length + 1)*sizeof(char));
        if (this->_value.text == nullptr)
          throw std::bad_alloc();

        memcpy((void*)this->_value.text, (void*)rhs._value.text, rhs._length*sizeof(char));
        this->_value.text[rhs._length] = (char)0;
      }
      break;
    case Type::arrays:
      this->_value.arrayRef = nullptr;
      if (rhs._value.arrayRef != nullptr) {
        try { this->_value.arrayRef = new Array(*rhs._value.arrayRef); }
        catch (...) { free(this->_value.arrayRef); this->_value.arrayRef = nullptr; this->_length = 0; throw; }
      }
      break;
    case Type::object:
      this->_value.objectRef = nullptr;
      if (rhs._value.objectRef != nullptr) {
        try { this->_value.objectRef = new Object(*rhs._value.objectRef); }
        catch (...) { free(this->_value.objectRef); this->_value.objectRef = nullptr; this->_length = 0; throw; }
      }
      break;
    default: this->_valueType = Type::integer; this->_value.integer = 0; this->_length = 1; break;
  }
}

// ---

void SerializableValue::_destroy() noexcept {
  switch (_valueType) {
    case Type::text: 
      if (this->_value.text != nullptr) {
        free(this->_value.text);
        this->_value.text = nullptr;
      }
      break;
    case Type::arrays: 
      if (this->_value.arrayRef != nullptr) {
        delete this->_value.arrayRef;
        this->_value.arrayRef = nullptr;
      }
      break;
    case Type::object: 
      if (this->_value.objectRef != nullptr) { 
        delete this->_value.objectRef;
        this->_value.objectRef = nullptr;
      } 
      break;
    default: break;
  }
  if (this->_comment != nullptr) {
    free(this->_comment);
    this->_comment = nullptr;
  }
}

bool SerializableValue::operator==(const SerializableValue& rhs) const noexcept {
  if (this->_valueType == rhs._valueType && this->_comment == rhs._comment && this->_length == rhs._length) {
    switch (_valueType) {
      case Type::integer: return (this->_value.integer == rhs._value.integer);
      case Type::number: return (this->_value.number == rhs._value.number);
      case Type::boolean: return (this->_value.boolean == rhs._value.boolean);
      case Type::text: return (this->_value.text == rhs._value.text);
      case Type::arrays: return (this->_value.arrayRef == rhs._value.arrayRef);
      case Type::object: return (this->_value.objectRef == rhs._value.objectRef);
      default: return false;
    }
  }
  return false;
}


// -- create immutable value -- ------------------------------------------------

SerializableValue::SerializableValue(const char* value) : _valueType(Type::text) {
  if (value != nullptr) {
    if (*value) {
      this->_length = strlen(value);
      this->_value.text = (char*)malloc((this->_length + 1)*sizeof(char));
      if (this->_value.text == nullptr)
        throw std::bad_alloc();
      memcpy((void*)this->_value.text, (void*)value, this->_length*sizeof(char));
      this->_value.text[this->_length] = (char)0;
    }
    else
      this->_value.text = nullptr;
  }
  else
    this->_value.text = nullptr;
}

SerializableValue::SerializableValue(const std::string& value) : _valueType(Type::text) {
  if (!value.empty()) {
    this->_length = value.size();
    this->_value.text = (char*)calloc(this->_length + 1, sizeof(char));
    if (this->_value.text == nullptr)
      throw std::bad_alloc();
    
    memcpy((void*)this->_value.text, (void*)value.c_str(), this->_length*sizeof(char));
    this->_value.text[this->_length] = (char)0;
  }
  else
    this->_value.text = nullptr;
}

SerializableValue::SerializableValue(Array&& value) : _valueType(Type::arrays) {
  if (!value.empty()) {
    this->_length = value.size();
    try { this->_value.arrayRef = new Array(std::move(value)); }
    catch (...) { this->_value.arrayRef = nullptr; throw; }
  }
  else
    this->_value.arrayRef = nullptr;
}

SerializableValue::SerializableValue(Object&& value) : _valueType(Type::object) {
  if (!value.empty()) {
    this->_length = value.size();
    try { this->_value.objectRef = new Object(std::move(value)); }
    catch (...) { this->_value.objectRef = nullptr; throw; }
  }
  else
    this->_value.objectRef = nullptr;
}


// -- accessors -- -------------------------------------------------------------

void SerializableValue::setComment(const char* comment) {
  if (this->_comment != nullptr) {
    free(this->_comment);
    this->_comment = nullptr;
  }
  if (comment != nullptr && *comment) {
    size_t length = strlen(comment);
    this->_comment = (char*)malloc((length + 1)*sizeof(char));
    if (this->_comment == nullptr)
      throw std::bad_alloc();
    memcpy((void*)this->_comment, (void*)comment, length*sizeof(char));
    this->_comment[length] = (char)0;
  }
}


// -- read value -- ------------------------------------------------------------

int32_t SerializableValue::getInteger() const noexcept {
  if (this->_valueType == Type::integer)
    return this->_value.integer;
  else if (this->_valueType == Type::number)
    return (int32_t)this->_value.number;
  else if (this->_valueType == Type::boolean)
    return this->_value.boolean ? 1 : 0;
  else if (this->_valueType == Type::text && this->_value.text != nullptr)
    return (int32_t)atoi(this->_value.text);
  return 0;
}
double SerializableValue::getNumber() const noexcept {
  if (this->_valueType == Type::number)
    return this->_value.number;
  else if (this->_valueType == Type::integer)
    return (double)this->_value.integer;
  else if (this->_valueType == Type::boolean)
    return this->_value.boolean ? 1.0 : 0.0;
  else if (this->_valueType == Type::text && this->_value.text != nullptr)
    return (double)atof(this->_value.text);
  return 0.0;
}
bool SerializableValue::getBoolean() const noexcept {
  if (this->_valueType == Type::boolean)
    return this->_value.boolean;
  else if (this->_valueType == Type::integer)
    return (this->_value.integer != 0);
  else if (this->_valueType == Type::number)
    return (this->_value.number != 0.0);
  return (this->_length > 0);
}
const char* SerializableValue::getText() const {
  if (this->_valueType == Type::text)
    return (const char*)this->_value.text;
  else if (this->_valueType == Type::boolean)
    return this->_value.boolean ? "true" : "false";
  return nullptr;
}


// -- private helpers -- -------------------------------------------------------

SerializableValue::SerializableValue(size_t length, char* movedValue) noexcept : _valueType(SerializableValue::Type::text) {
  this->_length = (movedValue != nullptr) ? length : 0;
  this->_value.text = movedValue;
}

SerializableValue::Array* SerializableValue::_getArray() {
  if (this->_valueType != Type::arrays) {
    assert(false);
    throw std::logic_error("SerializableValue: type mismatch (cannot read array of other type)");
  }
  return (SerializableValue::Array*)this->_value.arrayRef;
}

void SerializableValue::_pushArrayItem(SerializableValue&& value) {
  if (this->_valueType != Type::arrays) {
    assert(false);
    return;
  }
  
  if (this->_value.arrayRef == nullptr) {
    try { this->_value.arrayRef = new Array(); }
    catch (...) { this->_value.arrayRef = nullptr; this->_length = 0; throw; }
  }
  ((SerializableValue::Array*)this->_value.arrayRef)->emplace_back(std::move(value));
  this->_length = ((SerializableValue::Array*)this->_value.arrayRef)->size();
}
