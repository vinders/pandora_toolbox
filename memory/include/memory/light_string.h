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

#include <cstddef>

namespace pandora {
  namespace memory {
    /// @class LightString
    /// @brief Simple string container with dynamic allocation.
    ///        Useful to avoid the huge weight/overhead of std::string when not needed.
    /// @warning - No exception thrown: for required values, verify if not empty after setting it (or use assign() -> returns success).
    ///          - On allocation failure, constructors create an empty string (NULL data).
    class LightString final {
    public:
      LightString() noexcept = default;
      LightString(const LightString& rhs) noexcept { assign(rhs._value, rhs._size); }
      LightString(LightString&& rhs) noexcept : _value(rhs._value), _size(rhs._size) { rhs._value = nullptr; rhs._size = 0; }
      LightString& operator=(const LightString& rhs) noexcept { assign(rhs._value, rhs._size); return *this; }
      LightString& operator=(LightString&& rhs) noexcept { this->_value = rhs._value; this->_size = rhs._size; rhs._value = nullptr; rhs._size = 0; return *this; }
      ~LightString() noexcept { clear(); }
      
      LightString(size_t length, char repeated = ' ') noexcept; ///< Create string with repeated char (also useful to prealloc before calling assign / updating through data()[])
      LightString(const char* value) noexcept { assign(value); } ///< Create initialized string
      LightString(const char* value, size_t length) noexcept { assign(value, length); } ///< Create initialized string
      
      // -- accessors --
      
      inline const char* c_str() const noexcept { return (this->_value) ? this->_value : _emptyValue(); } ///< Get string content (never NULL)
      inline const char* data() const noexcept { return this->_value; }  ///< Get string content (NULL if empty)
      inline char* data() noexcept { return this->_value; }  ///< Get string content (NULL if empty, unless preallocated)
      
      constexpr inline size_t length() const noexcept { return this->_size; } ///< Get current length of the string
      constexpr inline size_t size() const noexcept { return this->_size; }   ///< Get current length of the string
      constexpr inline bool empty() const noexcept { return (this->_value == nullptr); } ///< Verify if the string is empty
      
      bool operator==(const LightString& rhs) const noexcept;
      inline bool operator!=(const LightString& rhs) const noexcept { return !(this->operator==(rhs)); }
      bool operator==(const char* rhs) const noexcept;
      inline bool operator!=(const char* rhs) const noexcept { return !(this->operator==(rhs)); }
      
      // -- operators --
      
      void clear() noexcept; ///< Clear string content (set to NULL)
      
      bool assign(const char* value) noexcept; ///< Copy another string (zero ended)
      bool assign(const char* value, size_t length) noexcept; ///< Copy another string/substring
      inline LightString& operator=(const char* value) noexcept { assign(value); return *this; }
      
      bool append(const char* suffix) noexcept; ///< Append another string (zero ended) -- causes realloc: if string changes often, prefer std::string
      bool append(const char* suffix, size_t length) noexcept; ///< Append another string/substring -- causes realloc: if string changes often, prefer std::string
      inline LightString& operator+=(const char* rhs) noexcept { append(rhs); return *this; }
      inline LightString operator+(const char* rhs) const noexcept { LightString copy(*this); copy += rhs; return copy; }
      inline LightString& operator+=(const LightString& rhs) noexcept { append(rhs._value); return *this; }
      inline LightString operator+(const LightString& rhs) const noexcept { LightString copy(*this); copy += rhs._value; return copy; }

    private:
      static constexpr const char* _emptyValue() noexcept { return ""; }
      char* _value = nullptr;
      size_t _size = 0u;
    };
    
    // ---
    
    /// @class LightWString
    /// @brief Simple wide string container with dynamic allocation.
    ///        Useful to avoid the huge weight/overhead of std::wstring when not needed.
    /// @warning - No exception thrown: for required values, verify if not empty after setting it (or use assign() -> returns success).
    ///          - On allocation failure, constructors create an empty string (NULL data).
    class LightWString final {
    public:
      LightWString() noexcept = default;
      LightWString(const LightWString& rhs) noexcept { assign(rhs._value, rhs._size); }
      LightWString(LightWString&& rhs) noexcept : _value(rhs._value), _size(rhs._size) { rhs._value = nullptr; rhs._size = 0; }
      LightWString& operator=(const LightWString& rhs) noexcept { assign(rhs._value, rhs._size); return *this; }
      LightWString& operator=(LightWString&& rhs) noexcept { this->_value = rhs._value; this->_size = rhs._size; rhs._value = nullptr; rhs._size = 0; return *this; }
      ~LightWString() noexcept { clear(); }
      
      LightWString(size_t length, wchar_t repeated = L' ') noexcept; ///< Create string with repeated char (also useful to prealloc before calling assign / updating through data()[])
      LightWString(const wchar_t* value) noexcept { assign(value); } ///< Create initialized string
      LightWString(const wchar_t* value, size_t length) noexcept { assign(value, length); } ///< Create initialized string
      
      // -- accessors --
      
      inline const wchar_t* c_str() const noexcept { return (this->_value) ? this->_value : _emptyValue(); } ///< Get string content (never NULL)
      inline const wchar_t* data() const noexcept { return this->_value; }  ///< Get string content (NULL if empty)
      inline wchar_t* data() noexcept { return this->_value; }  ///< Get string content (NULL if empty, unless preallocated)
      
      constexpr inline size_t length() const noexcept { return this->_size; } ///< Get current length of the string
      constexpr inline size_t size() const noexcept { return this->_size; }   ///< Get current length of the string
      constexpr inline bool empty() const noexcept { return (this->_value == nullptr); } ///< Verify if the string is empty
      
      bool operator==(const LightWString& rhs) const noexcept;
      inline bool operator!=(const LightWString& rhs) const noexcept { return !(this->operator==(rhs)); }
      bool operator==(const wchar_t* rhs) const noexcept;
      inline bool operator!=(const wchar_t* rhs) const noexcept { return !(this->operator==(rhs)); }
      
      // -- operators --
      
      void clear() noexcept; ///< Clear string content (set to NULL)
      
      bool assign(const wchar_t* value) noexcept; ///< Copy another string (zero ended)
      bool assign(const wchar_t* value, size_t length) noexcept; ///< Copy another string/substring
      inline LightWString& operator=(const wchar_t* value) noexcept { assign(value); return *this; }
      
      bool append(const wchar_t* suffix) noexcept; ///< Append another string (zero ended) -- causes realloc: if string changes often, prefer std::string
      bool append(const wchar_t* suffix, size_t length) noexcept; ///< Append another string/substring -- causes realloc: if string changes often, prefer std::string
      inline LightWString& operator+=(const wchar_t* rhs) noexcept { append(rhs); return *this; }
      inline LightWString operator+(const wchar_t* rhs) const noexcept { LightWString copy(*this); copy += rhs; return copy; }
      inline LightWString& operator+=(const LightWString& rhs) noexcept { append(rhs._value); return *this; }
      inline LightWString operator+(const LightWString& rhs) const noexcept { LightWString copy(*this); copy += rhs._value; return copy; }

    private:
      static constexpr const wchar_t* _emptyValue() noexcept { return L""; }
      wchar_t* _value = nullptr;
      size_t _size = 0u;
    };
  }
}
#undef __p_assert
