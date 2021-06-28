/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cstdlib>
#include <cstring>
#include "memory/light_string.h"

using namespace pandora::memory;


// -- LightString -- -----------------------------------------------------------

LightString::LightString(size_t length) noexcept : _value(length ? (char*)calloc(length + 1, sizeof(char)) : nullptr) {
  this->_size = (_value != nullptr) ? length : 0;
}
void LightString::clear() noexcept {
  if (this->_value != nullptr) {
    free(this->_value);
    this->_size = 0;
  }
}

bool LightString::operator==(const LightString& rhs) const noexcept { 
  return (this->_size == rhs._size && (this->_size == 0 || memcmp((void*)this->_value, (void*)rhs._value, (this->_size + 1u)*sizeof(char)) == 0));
}
bool LightString::operator==(const char* rhs) const noexcept {
  if (this->_value != nullptr)
    return (rhs != nullptr && strcmp(this->_value, rhs) == 0);
  return (rhs == nullptr || *rhs == '\0');
}

// Copy another string/substring
bool LightString::assign(const char* value, size_t length) noexcept {
  char* newValue = nullptr;
  if (length) {
    newValue = (char*)malloc((length + 1u)*sizeof(char));
    if (newValue == nullptr)
      return false;
    memcpy((void*)newValue, (void*)value, length*sizeof(char));
    newValue[length] = '\0';
  }

  if (this->_value != nullptr)
    free(this->_value);
  this->_value = newValue;
  this->_size = length;
  return true;
}
bool LightString::assign(const char* value) noexcept { return assign(value, (value != nullptr && *value != '\0') ? strlen(value) : 0);}

// Append another string/substring
bool LightString::append(const char* value, size_t length) noexcept {
  if (length != size_t{0}) {
    char* newValue = (char*)malloc((length + this->_size + 1u)*sizeof(char));
    if (newValue == nullptr)
      return false;
    
    if (this->_value != nullptr)
      memcpy((void*)newValue, (void*)this->_value, this->_size*sizeof(char));
    memcpy((void*)(newValue + this->_size), (void*)value, length*sizeof(char));
    
    if (this->_value != nullptr)
      free(this->_value);
    this->_value = newValue;
    this->_size += length;
    this->_value[this->_size] = '\0';
  }
  return true;
}
bool LightString::append(const char* value) noexcept { return (value != nullptr) ? append(value, strlen(value)) : true;}


// -- LightWString -- ----------------------------------------------------------

LightWString::LightWString(size_t length) noexcept : _value(length ? (wchar_t*)calloc(length + 1, sizeof(wchar_t)) : nullptr) {
  this->_size = (_value != nullptr) ? length : 0;
}
void LightWString::clear() noexcept {
  if (this->_value != nullptr) {
    free(this->_value);
    this->_size = 0;
  }
}

bool LightWString::operator==(const LightWString& rhs) const noexcept { 
  return (this->_size == rhs._size && (this->_size == 0 || memcmp((void*)this->_value, (void*)rhs._value, (this->_size + 1u)*sizeof(wchar_t)) == 0));
}
bool LightWString::operator==(const wchar_t* rhs) const noexcept {
  if (this->_value != nullptr)
    return (rhs != nullptr && wcscmp(this->_value, rhs) == 0);
  return (rhs == nullptr || *rhs == L'\0');
}

// Copy another string/substring
bool LightWString::assign(const wchar_t* value, size_t length) noexcept {
  wchar_t* newValue = nullptr;
  if (length) {
    newValue = (wchar_t*)malloc((length + 1u)*sizeof(wchar_t));
    if (newValue == nullptr)
      return false;
    memcpy((void*)newValue, (void*)value, length*sizeof(wchar_t));
    newValue[length] = L'\0';
  }

  if (this->_value != nullptr)
    free(this->_value);
  this->_value = newValue;
  this->_size = length;
  return true;
}
bool LightWString::assign(const wchar_t* value) noexcept { return assign(value, (value != nullptr && *value != L'\0') ? wcslen(value) : 0);}

// Append another string/substring
bool LightWString::append(const wchar_t* value, size_t length) noexcept {
  if (length != size_t{0}) {
    wchar_t* newValue = (wchar_t*)malloc((length + this->_size + 1u)*sizeof(wchar_t));
    if (newValue == nullptr)
      return false;
    
    if (this->_value != nullptr)
      memcpy((void*)newValue, (void*)this->_value, this->_size*sizeof(wchar_t));
    memcpy((void*)(newValue + this->_size), (void*)value, length*sizeof(wchar_t));
    
    if (this->_value != nullptr)
      free(this->_value);
    this->_value = newValue;
    this->_size += length;
    this->_value[this->_size] = L'\0';
  }
  return true;
}
bool LightWString::append(const wchar_t* value) noexcept { return (value != nullptr) ? append(value, wcslen(value)) : true;}
