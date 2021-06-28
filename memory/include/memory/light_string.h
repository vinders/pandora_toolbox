/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>

namespace pandora {
  namespace memory {
    /// @class LightString
    /// @brief Simple string container with dynamic allocation.
    ///        Useful to avoid the huge weight/overhead of std::string when not needed.
    class LightString final {
    public:
      LightString() noexcept = default;
      LightString(const LightString& rhs) noexcept { assign(rhs._value, rhs._size); }
      LightString(LightString&& rhs) noexcept : _value(rhs._value), _size(rhs._size) { rhs._value = nullptr; rhs._size = 0; }
      LightString& operator=(const LightString& rhs) noexcept { assign(rhs._value, rhs._size); return *this; }
      LightString& operator=(LightString&& rhs) noexcept { this->_value = rhs._value; this->_size = rhs._size; rhs._value = nullptr; rhs._size = 0; return *this; }
      ~LightString() noexcept;
      
      LightString(size_t length) noexcept; ///< Create preallocated empty string
      LightString(const char* value) noexcept { assign(value); } ///< Create initialized string
      LightString(const char* value, size_t length) noexcept { assign(value, length); } ///< Create initialized string
      
      // -- accessors --
      
      inline const char* c_str() const noexcept { return this->_value; } ///< Get string content (NULL if empty)
      inline const char* data() const noexcept { return this->_value; }  ///< Get string content (NULL if empty)
      inline char* c_str() noexcept { return this->_value; } ///< Get string content (NULL if empty, unless preallocated)
      inline char* data() noexcept { return this->_value; }  ///< Get string content (NULL if empty, unless preallocated)
      
      constexpr inline size_t length() const noexcept { return this->_size; } ///< Get current length of the string
      constexpr inline size_t size() const noexcept { return this->_size; }   ///< Get current length of the string
      constexpr inline bool empty() const noexcept { return (this->_value == nullptr); } ///< Verify if the string is empty
      
      bool operator==(const LightString& rhs) const noexcept;
      inline bool operator!=(const LightString& rhs) const noexcept { return !(this->operator==(rhs)); }
      bool operator==(const char* rhs) const noexcept;
      inline bool operator!=(const char* rhs) const noexcept { return !(this->operator==(rhs)); }
      
      // -- operators --
      
      bool assign(const char* value) noexcept; ///< Copy another string (zero ended)
      bool assign(const char* value, size_t length) noexcept; ///< Copy another string/substring
      inline LightString& operator=(const char* value) noexcept { assign(value); return *this; }
      
      bool append(const char* value) noexcept; ///< Append another string (zero ended)
      bool append(const char* value, size_t length) noexcept; ///< Append another string/substring
      inline LightString& operator+=(const char* rhs) noexcept { append(rhs); return *this; }
      inline LightString operator+(const char* rhs) const noexcept { LightString copy(*this); copy += rhs; return copy; }
      inline LightString& operator+=(const LightString& rhs) noexcept { append(rhs._value); return *this; }
      inline LightString operator+(const LightString& rhs) const noexcept { LightString copy(*this); copy += rhs._value; return copy; }

    private:
      char* _value = nullptr;
      size_t _size = 0u;
    };
    
    // ---
    
    /// @class LightWString
    /// @brief Simple wide string container with dynamic allocation.
    ///        Useful to avoid the huge weight/overhead of std::wstring when not needed.
    class LightWString final {
    public:
      LightWString() noexcept = default;
      LightWString(const LightWString& rhs) noexcept { assign(rhs._value, rhs._size); }
      LightWString(LightWString&& rhs) noexcept : _value(rhs._value), _size(rhs._size) { rhs._value = nullptr; rhs._size = 0; }
      LightWString& operator=(const LightWString& rhs) noexcept { assign(rhs._value, rhs._size); return *this; }
      LightWString& operator=(LightWString&& rhs) noexcept { this->_value = rhs._value; this->_size = rhs._size; rhs._value = nullptr; rhs._size = 0; return *this; }
      ~LightWString() noexcept;
      
      LightWString(size_t length) noexcept; ///< Create preallocated empty string
      LightWString(const wchar_t* value) noexcept { assign(value); } ///< Create initialized string
      LightWString(const wchar_t* value, size_t length) noexcept { assign(value, length); } ///< Create initialized string
      
      // -- accessors --
      
      inline const wchar_t* c_str() const noexcept { return this->_value; } ///< Get string content (NULL if empty)
      inline const wchar_t* data() const noexcept { return this->_value; }  ///< Get string content (NULL if empty)
      inline wchar_t* c_str() noexcept { return this->_value; } ///< Get string content (NULL if empty, unless preallocated)
      inline wchar_t* data() noexcept { return this->_value; }  ///< Get string content (NULL if empty, unless preallocated)
      
      constexpr inline size_t length() const noexcept { return this->_size; } ///< Get current length of the string
      constexpr inline size_t size() const noexcept { return this->_size; }   ///< Get current length of the string
      constexpr inline bool empty() const noexcept { return (this->_value == nullptr); } ///< Verify if the string is empty
      
      bool operator==(const LightWString& rhs) const noexcept;
      inline bool operator!=(const LightWString& rhs) const noexcept { return !(this->operator==(rhs)); }
      bool operator==(const wchar_t* rhs) const noexcept;
      inline bool operator!=(const wchar_t* rhs) const noexcept { return !(this->operator==(rhs)); }
      
      // -- operators --
      
      bool assign(const wchar_t* value) noexcept; ///< Copy another string (zero ended)
      bool assign(const wchar_t* value, size_t length) noexcept; ///< Copy another string/substring
      inline LightWString& operator=(const wchar_t* value) noexcept { assign(value); return *this; }
      
      bool append(const wchar_t* value) noexcept; ///< Append another string (zero ended)
      bool append(const wchar_t* value, size_t length) noexcept; ///< Append another string/substring
      inline LightWString& operator+=(const wchar_t* rhs) noexcept { append(rhs); return *this; }
      inline LightWString operator+(const wchar_t* rhs) const noexcept { LightWString copy(*this); copy += rhs; return copy; }
      inline LightWString& operator+=(const LightWString& rhs) noexcept { append(rhs._value); return *this; }
      inline LightWString operator+(const LightWString& rhs) const noexcept { LightWString copy(*this); copy += rhs._value; return copy; }

    private:
      wchar_t* _value = nullptr;
      size_t _size = 0u;
    };
  }
}
#undef __p_assert
