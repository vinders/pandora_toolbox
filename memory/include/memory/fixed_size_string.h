/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstring>
#include <stdexcept>
#ifndef _P_SKIP_ASSERTS
# include <cassert>
# define __p_assert(arg) assert(arg)
#else
# define __p_assert(arg) ((void)0)
#endif

namespace pandora {
  namespace memory {
    /// @class FixedSizeString
    /// @brief A container that behaves like a string with a max length, without involving any dynamic allocation. 
    ///        Useful for real-time apps or for UI strings with fixed max size. The string is STL-compliant.
    template <size_t _MaxSize,            // Max length of strings that may be contained in the fixed-size string.
              typename _CharType = char>  // Character type (char, char16_t, char32_t, wchar_t, ...)
    class FixedSizeString final {
    public:
      using value_type = _CharType;
      using size_type = size_t;
      using reference = _CharType&;
      using const_reference = const _CharType&;
      using pointer = _CharType*;
      using const_pointer = const _CharType*;
      using Type = FixedSizeString<_MaxSize,_CharType>;
      static_assert((_MaxSize > 0u), "FixedSizeString: _MaxSize can't be 0.");
      static_assert((_MaxSize < 0xFFFFFFFFu), "FixedSizeString: _MaxSize can't exceed or equal 0xFFFFFFFF.");

      static constexpr size_t npos = static_cast<size_t>(0xFFFFFFFFu); ///< Undefined length or position value


      FixedSizeString() noexcept { clear(); }
      FixedSizeString(const Type& rhs) noexcept { assign(rhs); }
      FixedSizeString(Type&& rhs) noexcept { assign(rhs); }
      ~FixedSizeString() = default;

      /// @brief Create initialized string 
      /// @warning If (length > _MaxSize): truncates the string
      explicit FixedSizeString(const _CharType* value, size_t length = npos) noexcept { assign(value, length); }
      /// @brief Create initialized string 
      /// @warning If (length > _MaxSize): truncates the string
      explicit FixedSizeString(const Type& rhs, size_t offset, size_t length = npos) noexcept { assign(rhs, offset, length); }
      /// @brief Create initialized string
      /// @warning If (length > _MaxSize): truncates the string
      explicit FixedSizeString(size_t length, _CharType character) noexcept { _assignSingleChar(character, length); }

      void swap(Type& rhs) noexcept { Type buffer(*this); _assign(rhs._value,rhs._size); rhs._assign(buffer._value,buffer._size); }
      
      // -- getters --
      
      inline const _CharType* c_str() const noexcept { return this->_value; } ///< Get legacy string content
      inline const _CharType* data() const noexcept { return this->_value; }  ///< Get legacy string content
      
      constexpr inline size_t length() const noexcept { return this->_size; } ///< Get current length of the string
      constexpr inline size_t size() const noexcept { return this->_size; }   ///< Get current length of the string
      constexpr inline size_t max_size() const noexcept { return _MaxSize; }  ///< Get fixed max capacity of string
      constexpr inline size_t capacity() const noexcept { return _MaxSize; }  ///< Get fixed max capacity of string
      constexpr inline bool empty() const noexcept { return (this->_size == static_cast<size_t>(0u)); } ///< Verify if the string is empty
      constexpr inline bool full() const noexcept  { return (this->_size == _MaxSize); }                ///< Verify if the max fixed size of the string is reached
      
      
      inline _CharType& operator[](size_t index) noexcept { __p_assert(index < this->_size); return this->_value[index]; }             ///< Read character at index (not verified!)
      inline const _CharType& operator[](size_t index) const noexcept { __p_assert(index < this->_size); return this->_value[index]; } ///< Read character at index - const (not verified!)
      inline _CharType& at(size_t index) { ///< Read character at index (throws)
        if (index < this->_size)
          return this->_value[index];
        throw std::out_of_range("FixedSizeString.at: trying to read invalid index.");
      }
      inline const _CharType& at(size_t index) const { ///< Read character at index - const (throws)
        if (index < this->_size)
          return this->_value[index];
        throw std::out_of_range("FixedSizeString.at: trying to read invalid index.");
      }
      
      inline _CharType& front() noexcept { return this->_value[0]; } ///< Get first character
      inline const _CharType& front() const noexcept { return this->_value[0]; } ///< Get first character
      inline _CharType& back() noexcept { return (!empty()) ? this->_value[this->_size - 1u] : this->_value[0]; } ///< Get last character
      inline const _CharType& back() const noexcept { return (!empty()) ? this->_value[this->_size - 1u] : this->_value[0]; } ///< Get last character
      
      // -- comparisons --

      /// @brief Compare value of current string with another string ('rhs')
      inline int compare(const Type& rhs) const noexcept {
        return _compare(this->_value, rhs._value, this->_size + 1u);
      }
      /// @brief Compare value of current string with another string ('rhs')
      inline int compare(const _CharType* rhs) const noexcept {
        if (rhs != nullptr)
          return _compare(this->_value, rhs);
        return (this->_size == 0) ? 0 : 1;
      }

      /// @brief Compare value of a substring of current string with another string ('rhs')
      inline int compare(size_t offset, size_t length, const Type& rhs) const noexcept {
        if (offset + length <= this->_size) {
          return _compare(&(this->_value[offset]), &(rhs._value[0]), length);
        }
        else if (offset < this->_size) {
          return _compare(&(this->_value[offset]), &(rhs._value[0]), this->_size + 1u - offset);
        }
        return (rhs._size == 0 || length == 0) ? 0 : -1;
      }
      /// @brief Compare value of a substring of current string with another string/substring ('rhs')
      inline int compare(size_t offset, size_t length, const _CharType* rhs, size_t /*rhsTotalLength*/ = npos) const noexcept {
        if (rhs != nullptr) {
          if (offset + length <= this->_size) {
            return _compare(&(this->_value[offset]), rhs, length);
          }
          else if (offset < this->_size) {
            return _compare(&(this->_value[offset]), rhs, this->_size + 1u - offset);
          }
          return (*rhs == static_cast<_CharType>(0) || length == 0) ? 0 : -1;
        }
        return (this->_size <= offset || length == 0) ? 0 : 1;
      }
      /// @brief Compare value of a substring of current string with another substring ('rhsLength' characters from 'rhs' at 'rhsOffset')
      inline int compare(size_t offset, size_t length, const Type& rhs, size_t rhsOffset) const noexcept { 
        if (offset + length <= this->_size) {
          return _compare(&(this->_value[offset]), (rhsOffset <= rhs._size) ? &(rhs._value[rhsOffset]) : &(rhs._value[_MaxSize]), length);
        }
        else if (offset < this->_size) {
          return _compare(&(this->_value[offset]), (rhsOffset <= rhs._size) ? &(rhs._value[rhsOffset]) : &(rhs._value[_MaxSize]), this->_size + 1u - offset);
        }
        return (rhs._size <= rhsOffset || length == 0) ? 0 : -1;
      }

      inline bool operator==(const Type& rhs) const noexcept { 
        return (this->_size == rhs._size && memcmp((void*)this->_value, (void*)rhs._value, (this->_size + 1u)*sizeof(_CharType)) == 0);
      }
      inline bool operator!=(const Type& rhs) const noexcept { return !(this->operator==(rhs)); }

      inline bool operator==(const _CharType* rhs) const noexcept { return (compare(rhs) == 0); }
      inline bool operator!=(const _CharType* rhs) const noexcept { return !(this->operator==(rhs)); }

      // -- change size --

      /// @brief Remove all content of the string
      inline void clear() noexcept {
        this->_size = static_cast<size_t>(0u);
        memset((void*)this->_value, 0, (_MaxSize + 1u)*sizeof(_CharType));
      }
      /// @brief Fill the string with a character (replace all content until reaching max fixed size)
      inline void fill(_CharType character) noexcept {
        this->_size = _MaxSize;
        _setSingleCharValue(size_t{ 0 }, character, _MaxSize);
      }

      /// @brief Change the size of the string - add padding at the end or truncate the end
      /// @warning If (length > _MaxSize): uses max size
      inline void resize(size_t length, _CharType paddingChar = static_cast<_CharType>(0x20)) noexcept {
        if (length < this->_size) {
          memset((void*)(&(this->_value[length])), 0, (this->_size - length)*sizeof(_CharType));
          this->_size = length;
        }
        else
          _appendSingleChar(paddingChar, length - this->_size);
      }
      /// @brief If current size is less than 'minSize', add padding characters before/after (centered)
      /// @warning If (minSize > _MaxSize): uses max size
      inline Type& pad(size_t minSize, _CharType paddingChar = static_cast<_CharType>(' ')) noexcept {
        if (minSize > this->_size) {
          size_t padding = (minSize <= _MaxSize) ? minSize - this->_size : _MaxSize - this->_size;
          _insertSingleChar(paddingChar, padding >> 1, 0u);
          _appendSingleChar(paddingChar, padding - (padding >> 1));
        }
        return *this;
      }

      // -- assignment --

      /// @brief Set the value of the string with the value of 'rhs'
      inline Type& assign(const _CharType* rhs, size_t length = npos) noexcept {
        if (rhs != nullptr)
          _assign(rhs, _getLength(rhs, length));
        else
          clear();
        return *this;
      }
      /// @brief Set the value of the string with the value of 'rhs'
      inline Type& assign(const Type& rhs) noexcept { _assign(rhs._value, rhs._size); return *this; }
      /// @brief Set the value of the string with a substring of 'rhs'
      inline Type& assign(const Type& rhs, size_t offset, size_t length = npos) noexcept {
        if (offset < rhs._size && length > 0u)
          _assign(&(rhs._value[offset]), (length == npos || (length > rhs._size - offset)) ? (rhs._size - offset) : length);
        else
          clear();
        return *this;
      }
      /// @brief Set the value of the string with 'character' (repeated 'charNb' times)
      inline Type& assign(size_t charNb, _CharType character) noexcept { _assignSingleChar(character, charNb); return *this; }

      inline Type& operator=(const Type& rhs) noexcept { return assign(rhs); }
      inline Type& operator=(Type&& rhs) noexcept { return assign(rhs); }
      inline Type& operator=(const _CharType* rhs) noexcept { return assign(rhs, npos); }
      
      // -- append --

      /// @brief Append another string ('rhs')
      inline Type& append(const _CharType* rhs, size_t length = npos) noexcept {
        if (rhs != nullptr)
          _append(rhs, _getLength(rhs, length));
        return *this;
      }
      /// @brief Append another string ('rhs')
      inline Type& append(const Type& rhs) noexcept { _append(rhs._value, rhs._size); return *this; }
      /// @brief Append another substring ('length' character(s) from 'rhs' at 'offset' position)
      inline Type& append(const Type& rhs, size_t offset, size_t length = npos) noexcept {
        if (offset < rhs._size && length > 0u)
          _append(&(rhs._value[offset]), (length == npos || (length > rhs._size - offset)) ? (rhs._size - offset) : length);
        return *this;
      }
      /// @brief Append 'character' value ('charNb' times)
      inline Type& append(size_t charNb, _CharType character) noexcept { _appendSingleChar(character, charNb); return *this; }

      /// @brief Append a character at the end of the string (if possible)
      inline bool push_back(_CharType character) noexcept {
        __p_assert(character != static_cast<_CharType>(0));
        if (this->_size < _MaxSize) {
          this->_value[this->_size] = character;
          ++(this->_size);
          return true;
        }
        return false;
      }
      /// @brief Pad the end of the string with a padding character (enough times to reach desired 'minSize' length)
      /// @warning If (minSize > _MaxSize): uses max size
      inline Type& rpad(size_t minSize, _CharType paddingChar = static_cast<_CharType>(' ')) noexcept {
        if (minSize > this->_size)
          _appendSingleChar(paddingChar, minSize - this->_size);
        return *this;
      }

      inline Type& operator+=(const _CharType* rhs) noexcept { return append(rhs); }
      inline Type operator+(const _CharType* rhs) const noexcept { Type copy(*this); copy += rhs; return copy; }
      inline Type& operator+=(const Type& rhs) noexcept { return append(rhs); }
      inline Type operator+(const Type& rhs) const noexcept { Type copy(*this); copy += rhs; return copy; }

      // -- insert --

      /// @brief Insert another string ('rhs') at 'position'
      inline Type& insert(size_t position, const Type& rhs) noexcept { _insertOrAppend(rhs._value, rhs._size, position); return *this; }
      /// @brief Insert another substring ('rhsLength' characters from 'rhs' at 'rhsOffset') at 'position'
      inline Type& insert(size_t position, const Type& rhs, size_t rhsOffset, size_t rhsLength = npos) noexcept {
        if (rhsOffset < rhs._size) {
          if (rhsLength == npos || rhsLength > rhs._size - rhsOffset)
            rhsLength = rhs._size - rhsOffset;
          _insertOrAppend(&(rhs._value[rhsOffset]), rhsLength, position);
        }
        return *this;
      }
      /// @brief Insert another string ('rhs') at 'position'
      inline Type& insert(size_t position, const _CharType* rhs, size_t length = npos) noexcept {
        if (rhs != nullptr)
          _insertOrAppend(rhs, _getLength(rhs, length), position);
        return *this;
      }
      /// @brief Insert a repeated 'character' ('charNb' times) at 'position'
      inline Type& insert(size_t position, size_t charNb, _CharType character) noexcept { _insertOrAppendChar(character, charNb, position); return *this; }

      /// @brief Pad the beginning of the string with a padding character (enough times to reach desired 'minSize' length)
      /// @warning If (minSize > _MaxSize): uses max size
      inline Type& lpad(size_t minSize, _CharType paddingChar = static_cast<_CharType>(' ')) noexcept {
        if (minSize > _MaxSize)
          minSize = _MaxSize;
        if (minSize > this->_size)
          _insertSingleChar(paddingChar, minSize - this->_size, 0u);
        return *this;
      }
      
      // -- replace --

      /// @brief Replace N character(s) (N = 'length') by another string ('rhs')
      inline Type& replace(size_t offset, size_t length, const Type& rhs) noexcept { _replaceOrAppend(rhs._value, rhs._size, offset, length); return *this; }
      /// @brief Replace N character(s) (N = 'length') by another substring ('rhsLength' characters from 'rhs' at 'rhsOffset')
      inline Type& replace(size_t offset, size_t length, const Type& rhs, size_t rhsOffset, size_t rhsLength) noexcept {
        if (rhsOffset < rhs._size) {
          if (rhsLength > rhs._size - rhsOffset) // too long, or npos
            rhsLength = rhs._size - rhsOffset;
        }
        else
          rhsOffset = rhsLength = size_t{ 0 };
        _replaceOrAppend(&(rhs._value[rhsOffset]), rhsLength, offset, length);
        return *this;
      }
      /// @brief Replace N character(s) (N = 'length') by another string ('rhs')
      inline Type& replace(size_t offset, size_t length, const _CharType* rhs, size_t rhsLength = npos) noexcept {
        _replaceOrAppend(rhs, (rhs != nullptr) ? _getLength(rhs, rhsLength) : size_t{ 0 }, offset, length);
        return *this;
      }
      /// @brief Replace N character(s) (N = 'length') by a repeated 'character' ('charNb' times)
      inline Type& replace(size_t offset, size_t length, size_t charNb, _CharType character) noexcept { _replaceOrAppendChar(character, charNb, offset, length); return *this; }

      /// @brief Replace N character(s) between iterators of another string ('rhs', 'iterEnd' is excluded)
      inline Type& replace(const _CharType* iterFirst, const _CharType* iterEnd, const Type& rhs) noexcept {
        if (_verifyIteration(iterFirst, iterEnd))
          _replace(rhs._value, rhs._size, _iteratorToPosition(iterFirst), _iteratorsDistance(iterFirst, iterEnd));
        return *this;
      }
      /// @brief Replace N character(s) between iterators of another string ('rhs', 'iterEnd' is excluded)
      inline Type& replace(const _CharType* iterFirst, const _CharType* iterEnd, const _CharType* rhs, size_t rhsLength = npos) noexcept {
        if (_verifyIteration(iterFirst, iterEnd))
          _replace(rhs, (rhs != nullptr) ? _getLength(rhs, rhsLength) : size_t{ 0 }, _iteratorToPosition(iterFirst), _iteratorsDistance(iterFirst, iterEnd));
        return *this;
      }

      // -- erase --

      /// @brief Remove N character(s) at 'offset' position (N = 'length')
      inline Type& erase(size_t offset = 0u, size_t length = npos) noexcept {
        if (offset < this->_size)
          _erase(offset, (length <= this->_size - offset) ? length : this->_size - offset);
        return *this;
      }
      /// @brief Remove character at iterator position
      inline const _CharType* erase(const _CharType* iter) noexcept {
        if (iter >= &(this->_value[0]) && iter < &(this->_value[this->_size]))
          _erase(_iteratorToPosition(iter), static_cast<size_t>(1u));
        return iter;
      }
      /// @brief Remove character between iterator positions ('iterEnd' is excluded)
      inline const _CharType* erase(const _CharType* iterFirst, const _CharType* iterEnd) noexcept {
        if (_verifyIteration(iterFirst, iterEnd))
          _erase(_iteratorToPosition(iterFirst), _iteratorsDistance(iterFirst, iterEnd));
        return iterFirst;
      }

      /// @brief Remove last character
      inline bool pop_back() noexcept {
        if (this->_size > 0u) {
          --(this->_size);
          this->_value[this->_size] = 0;
          return true;
        }
        return false;
      }

      /// @brief Remove all spaces/tabs/controls chars at the beginning/end of current string
      inline Type& trim() noexcept {
        ltrim();
        return rtrim();
      }
      /// @brief Remove all spaces/tabs/controls chars at the beginning of current string
      inline Type& ltrim() noexcept {
        size_t firstIndex = _getFirstNonTrimmableIndex();
        if (firstIndex > static_cast<size_t>(0u) && firstIndex < this->_size) {
          this->_size -= firstIndex;
          memmove((void*)&(this->_value[0]), (void*)&(this->_value[firstIndex]), this->_size*sizeof(_CharType));
          memset((void*)&(this->_value[this->_size]), 0, firstIndex*sizeof(_CharType));
        }
        return *this;
      }
      /// @brief Remove all spaces/tabs/controls chars at the end of current string
      inline Type& rtrim() noexcept {
        size_t afterLastIndex = _getIndexAfterLastNonTrimmable();
        if (afterLastIndex < this->_size) {
          memset((void*)&(this->_value[afterLastIndex]), 0, (this->_size - afterLastIndex)*sizeof(_CharType));
          this->_size = afterLastIndex;
        }
        return *this;
      }
      
      // -- search --

      /// @brief Extract a substring of current object
      inline Type substr(size_t offset = 0u, size_t length = npos) const noexcept { Type sub; sub._size = copy(sub._value, offset, length); return sub; }
      /// @brief Copy a substring of current object into passed array
      inline size_t copy(_CharType* out, size_t offset, size_t length = npos) const noexcept {
        __p_assert(out != nullptr);
        if (offset < this->_size && length > 0u) {
          size_t copySize = (length == npos || (length > this->_size - offset)) ? (this->_size - offset) : length;
          memcpy((void*)out, (void*)(&(this->_value[offset])), copySize*sizeof(_CharType));
          out[copySize] = static_cast<_CharType>(0);
          return copySize;
        }
        return 0u;
      }

      /// @brief Find the first position of the substring 'query' ('npos' returned if not found)
      size_t find(const Type& query, size_t offset = 0u) const noexcept {
        return (offset < this->_size) ? _findString(query._value, query._size, &(this->_value[offset]), &(this->_value[this->_size - 1u]) ) : npos;
      }
      size_t find(const _CharType* query, size_t offset = 0u, size_t length = npos) const noexcept {
        return (offset < this->_size) ? _findString(query, _getLength(query, length), &(this->_value[offset]), &(this->_value[this->_size - 1u]) ) : npos;
      }
      inline size_t find(_CharType character, size_t offset = 0u) const noexcept {
        return (offset < this->_size) ? _find(character, &(this->_value[offset]), &(this->_value[this->_size - 1u]) ) : npos;
      }

      /// @brief Find the last position of the substring 'query' ('npos' returned if not found)
      size_t rfind(const Type& query, size_t offset = npos) const noexcept {
        return (offset < this->_size) ? _rfindString(query._value, query._size, &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }
      size_t rfind(const _CharType* query, size_t offset = npos, size_t length = npos) const noexcept {
        return(offset < this->_size) ? _rfindString(query, _getLength(query, length), &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }
      inline size_t rfind(_CharType character, size_t offset = npos) const noexcept {
        return (offset < this->_size) ? _rfind(character, &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }

      /// @brief Find the first position of any of the character(s) in 'charList' ('npos' returned if not found)
      inline size_t find_first_of(const Type& charList, size_t offset = 0u) const noexcept {
        return (offset < this->_size) ? _find(charList._value, charList._size, &(this->_value[offset]), &(this->_value[this->_size - 1u]) ) : npos;
      }
      inline size_t find_first_of(const _CharType* charList, size_t offset = 0u, size_t length = npos) const noexcept  {
        if (offset < this->_size)
          return _find(charList, _getLength(charList, length), &(this->_value[offset]), &(this->_value[this->_size - 1u]) );
        return npos;
      }
      inline size_t find_first_of(_CharType character, size_t offset = 0u) const noexcept { return find(character, offset); }

      /// @brief Find the first position of any character different from the character(s) in 'charList' ('npos' returned if not found)
      inline size_t find_first_not_of(const Type& charList, size_t offset = 0u) const noexcept {
        return (offset < this->_size) ? _findNot(charList._value, charList._size, &(this->_value[offset]), &(this->_value[this->_size - 1u]) ) : npos;
      }
      inline size_t find_first_not_of(const _CharType* charList, size_t offset = 0u, size_t length = npos) const noexcept {
        if (offset < this->_size)
          return _findNot(charList, _getLength(charList, length), &(this->_value[offset]), &(this->_value[this->_size - 1u]) );
        return npos;
      }
      inline size_t find_first_not_of(_CharType character, size_t offset = 0u) const noexcept {
        return (offset < this->_size) ? _findNot(character, &(this->_value[offset]), &(this->_value[this->_size - 1u]) ) : npos;
      }

      /// @brief Find the last position of any of the character(s) in 'charList' ('npos' returned if not found)
      size_t find_last_of(const Type& charList, size_t offset = npos) const noexcept {
        return (offset < this->_size) ? _rfind(charList._value, charList._size, &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }
      size_t find_last_of(const _CharType* charList, size_t offset = npos, size_t length = npos) const noexcept {
        return (offset < this->_size) ? _rfind(charList, _getLength(charList, length), &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }
      inline size_t find_last_of(_CharType character, size_t offset = npos) const noexcept { return rfind(character, offset); }

      /// @brief Find the last position of any character different from the character(s) in 'charList' ('npos' returned if not found)
      size_t find_last_not_of(const Type& charList, size_t offset = npos) const noexcept {
        return (offset < this->_size) ? _rfindNot(charList._value, charList._size, &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }
      size_t find_last_not_of(const _CharType* charList, size_t offset = npos, size_t length = npos) const noexcept {
        return (offset < this->_size) ? _rfindNot(charList, _getLength(charList, length), &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }
      inline size_t find_last_not_of(_CharType character, size_t offset = npos) const noexcept {
        return (offset < this->_size) ? _rfindNot(character, &(this->_value[_rOffsetIndex(offset)]), &(this->_value[0]) ) : npos;
      }


    private:
      // -- private - operations --

      // assign instance
      inline void _assign(const _CharType* verifiedValue, size_t verifiedLength) noexcept {
        this->_size = _fixAssignSize(verifiedLength);
        if (this->_size > 0u)
          memcpy((void*)(this->_value), (void*)verifiedValue, this->_size*sizeof(_CharType));
        memset((void*)&(this->_value[this->_size]), 0, (_MaxSize + 1 - this->_size)*sizeof(_CharType));
      }
      inline void _assignSingleChar(_CharType value, size_t verifiedLength) noexcept {
        this->_size = _fixAssignSize(verifiedLength);
        if (this->_size > 0u)
          _setSingleCharValue(0u, value, this->_size);
        memset((void*)&(this->_value[this->_size]), 0, (_MaxSize + 1 - this->_size)*sizeof(_CharType));
      }

      // append instance
      inline void _append(const _CharType* verifiedValue, size_t verifiedLength) noexcept {
        size_t appendSize = _fixAppendSize(verifiedLength);
        if (appendSize > 0u) {
          memcpy((void*)&(this->_value[this->_size]), (void*)verifiedValue, appendSize*sizeof(_CharType));
          this->_size += appendSize;
        }
      }
      inline void _appendSingleChar(_CharType value, size_t verifiedLength) noexcept {
        size_t appendSize = _fixAppendSize(verifiedLength);
        if (appendSize > 0u) {
          _setSingleCharValue(this->_size, value, appendSize);
          this->_size += appendSize;
        }
      }

      // insert instance (position must be valid)
      inline void _insert(const _CharType* verifiedValue, size_t verifiedLength, size_t position) noexcept {
        if (verifiedLength > size_t{ 0 }) {
          _shiftForInsert(verifiedLength, position); // updates verifiedLength if necessary
          memcpy((void*)&(this->_value[position]), (void*)verifiedValue, verifiedLength*sizeof(_CharType));
        }
      }
      inline void _insertSingleChar(_CharType value, size_t verifiedLength, size_t position) noexcept {
        if (verifiedLength > size_t{ 0 }) {
          _shiftForInsert(verifiedLength, position); // updates verifiedLength if necessary
          _setSingleCharValue(position, value, verifiedLength);
        }
      }

      // replace by instance
      inline void _replace(const _CharType* verifiedValue, size_t verifiedLength, size_t position, size_t replacedLength) noexcept {
        _shiftForReplace(verifiedLength, position, replacedLength); // update verifiedLength if necessary
        if (verifiedLength > size_t{ 0 })
          memcpy((void*)&(this->_value[position]), (void*)verifiedValue, verifiedLength*sizeof(_CharType));
      }
      inline void _replaceSingleChar(_CharType value, size_t verifiedLength, size_t position, size_t replacedLength) noexcept {
        _shiftForReplace(verifiedLength, position, replacedLength); // update verifiedLength if necessary
        if (verifiedLength > size_t{ 0 })
          _setSingleCharValue(position, value, verifiedLength);
      }

      // erase range
      inline void _erase(size_t verifiedPosition, size_t verifiedLength) noexcept {
        if (verifiedLength > 0u) {
          if (verifiedPosition + verifiedLength < this->_size)
            memmove((void*)&(this->_value[verifiedPosition]), (void*)&(this->_value[verifiedPosition + verifiedLength]), (this->_size - (verifiedPosition + verifiedLength))*sizeof(_CharType));
          this->_size -= verifiedLength;
          memset((void*)&(this->_value[this->_size]), 0, (_MaxSize - this->_size)*sizeof(_CharType));
        }
      }

      // insert or append range, depending on position
      inline void _insertOrAppend(const _CharType* verifiedValue, size_t verifiedLength, size_t position) noexcept {
        if (position < this->_size)
          _insert(verifiedValue, verifiedLength, position);
        else
          _append(verifiedValue, verifiedLength);
      }
      inline void _insertOrAppendChar(_CharType value, size_t verifiedLength, size_t position) noexcept {
        if (position < this->_size)
          _insertSingleChar(value, verifiedLength, position);
        else
          _appendSingleChar(value, verifiedLength);
      }
      // replace or append range, depending on position
      inline void _replaceOrAppend(const _CharType* verifiedValue, size_t verifiedLength, size_t offset, size_t length) noexcept {
        if (offset < this->_size)
          _replace(verifiedValue, verifiedLength, offset, length);
        else
          _append(verifiedValue, verifiedLength);
      }
      inline void _replaceOrAppendChar(_CharType value, size_t verifiedLength, size_t offset, size_t length) noexcept {
        if (offset < this->_size)
          _replaceSingleChar(value, verifiedLength, offset, length);
        else
          _appendSingleChar(value, verifiedLength);
      }

      // set single repeated char value
      inline void _setSingleCharValue(size_t position, char value, size_t repeats) noexcept {
        __p_assert(value != static_cast<char>(0));
        memset((void*)&(this->_value[position]), value, repeats*sizeof(_CharType));
      }
      // set single repeated multi-byte char value
      template <typename _Type>
      inline void _setSingleCharValue(size_t position, _Type value, size_t repeats) noexcept {
        __p_assert(value != static_cast<_Type>(0));
        const _Type* end = &(this->_value[position + repeats]);
        for (_Type* it = &(this->_value[position]); it < end; ++it)
          *it = value;
      }

      // -- private - size management --
      
      // get length of string argument
      static inline size_t _getLength(const _CharType* value, size_t maxLengthCheck) noexcept {
        const _CharType* it = value;
        while (maxLengthCheck && *it != static_cast<_CharType>(0)) {
          --maxLengthCheck;
          ++it;
        }
        return static_cast<size_t>(it - value);
      }

      // get absolute index from a right-side offset
      constexpr inline size_t _rOffsetIndex(size_t offset) const noexcept {
        return (offset == npos || offset >= this->_size) ? (this->_size - 1u) : (this->_size - offset - 1u);
      }

      // fix the length of an assigned string
      constexpr inline size_t _fixAssignSize(size_t assignSize) const noexcept {
        return (assignSize <= _MaxSize) ? assignSize : _MaxSize;
      }
      // fix the length of an appended string
      constexpr inline size_t _fixAppendSize(size_t appendSize) const noexcept {
        return (appendSize <= _MaxSize - this->_size) ? appendSize : (_MaxSize - this->_size);
      }

      // shift suffix for insert + fix verifiedLength
      void _shiftForInsert(size_t& verifiedLength, size_t position) noexcept {
        if (verifiedLength <= _MaxSize - this->_size) {
          memmove((void*)&(this->_value[position + verifiedLength]), (void*)&(this->_value[position]), (this->_size - position)*sizeof(_CharType));
          this->_size += verifiedLength;
        }
        else {
          if (verifiedLength < _MaxSize - position) // suffix truncated
            memmove((void*)&(this->_value[position + verifiedLength]), (void*)&(this->_value[position]), (_MaxSize - position - verifiedLength)*sizeof(_CharType));
          else // suffix removed
            verifiedLength = _MaxSize - position;
          this->_size = _MaxSize;
        }
      }
      // remove replaced part + shift suffix to appropriate location + fix verifiedLength
      void _shiftForReplace(size_t& verifiedLength, size_t position, size_t replacedLength) noexcept {
        if (verifiedLength < _MaxSize - position) { 
          size_t insertEndSize = position + verifiedLength;
          if (replacedLength < this->_size - position) { // suffix exists
            if (verifiedLength != replacedLength) { // move suffix
              size_t suffixSize = this->_size - (position + replacedLength);
              if (insertEndSize + suffixSize < _MaxSize) {
                memmove((void*)&(this->_value[position + verifiedLength]), (void*)&(this->_value[position + replacedLength]), suffixSize*sizeof(_CharType));
                
                this->_size = insertEndSize + suffixSize;
                if (verifiedLength < replacedLength)
                  memset((void*)&(this->_value[this->_size]), 0, (replacedLength - verifiedLength)*sizeof(_CharType));
              }
              else {
                memmove((void*)&(this->_value[position + verifiedLength]), (void*)&(this->_value[position + replacedLength]), (_MaxSize - insertEndSize)*sizeof(_CharType));
                this->_size = _MaxSize;
              }
            }
          }
          else { // no suffix
            if (insertEndSize < this->_size)
              memset((void*)&(this->_value[insertEndSize]), 0, (this->_size - insertEndSize)*sizeof(_CharType));
            this->_size = insertEndSize;
          }
        }
        else { // fill string with new value
          verifiedLength = _MaxSize - position;
          this->_size = _MaxSize;
        }
      }

      // -- private - searches --

      // basic string comparison - fixed size
      static inline int _compare(const _CharType* lhs, const _CharType* rhs, size_t length) noexcept {
        for (; length > 0; --length, ++lhs, ++rhs) {
          if (*lhs != *rhs)
            return (*lhs < *rhs) ? -1 : 1;
        }
        return 0;
      }
      // basic string comparison - zero ended
      static inline int _compare(const _CharType* lhs, const _CharType* rhs) noexcept {
        while (*lhs == *rhs && *lhs != static_cast<_CharType>(0)) {
          ++lhs;
          ++rhs;
        }
        if (*lhs == *rhs)
          return 0;
        return (*lhs < *rhs) ? -1 : 1;
      }

      // find first non-trimmable character in current string
      inline size_t _getFirstNonTrimmableIndex() const noexcept {
        const _CharType* it = &(this->_value[0]);
        while (*it <= 0x20 && *it > 0)
          ++it;
        return _iteratorToPosition(it);
      }
      // find last non-trimmable character in current string
      inline size_t _getIndexAfterLastNonTrimmable() const noexcept {
        if (this->_size > size_t{ 0 }) {
          const _CharType* it = &(this->_value[this->_size - 1u]);
          while (it >= &(this->_value[0]) && static_cast<uint32_t>(*it) <= 0x20)
            --it;
          ++it; // index after last -> +1
          return _iteratorToPosition(it);
        }
        return static_cast<size_t>(0u);
      }

      // control validity of range between iterators
      inline bool _verifyIteration(const _CharType* iterFirst, const _CharType* iterEnd) const noexcept {
        return (iterFirst >= &(this->_value[0]) && iterFirst < &(this->_value[this->_size]) && iterFirst < iterEnd && iterEnd <= &(this->_value[_MaxSize]));
      }
      // get position from iteration pointer
      inline size_t _iteratorToPosition(const _CharType* iter) const noexcept { return static_cast<size_t>(iter - &(this->_value[0])); }
      // get distance between iteration pointers
      inline size_t _iteratorsDistance(const _CharType* iterFirst, const _CharType* iterEnd) const noexcept {
        return (iterEnd <= &(this->_value[this->_size])) ? static_cast<size_t>(iterEnd - iterFirst) : static_cast<size_t>(&(this->_value[this->_size]) - iterFirst);
      }

      // first first character occurrence
      inline size_t _find(_CharType character, const _CharType* iter, const _CharType* iterLast) const noexcept {
        while (iter <= iterLast && *iter != character)
          ++iter;
        return (iter <= iterLast && *iter == character) ? _iteratorToPosition(iter) : npos;
      }
      // first first character non-occurrence
      inline size_t _findNot(_CharType character, const _CharType* iter, const _CharType* iterLast) const noexcept {
        while (iter <= iterLast && *iter == character)
          ++iter;
        return (iter <= iterLast && *iter != character) ? _iteratorToPosition(iter) : npos;
      }
      // find last character occurrence
      inline size_t _rfind(_CharType character, const _CharType* iter, const _CharType* iterRevLast) const noexcept {
        while (iter >= iterRevLast && *iter != character)
          --iter;
        return (iter >= iterRevLast && *iter == character) ? _iteratorToPosition(iter) : npos;
      }
      // find last character non-occurrence
      inline size_t _rfindNot(_CharType character, const _CharType* iter, const _CharType* iterRevLast) const noexcept {
        while (iter >= iterRevLast && *iter == character)
          --iter;
        return (iter >= iterRevLast && *iter != character) ? _iteratorToPosition(iter) : npos;
      }

      // first first character occurrence
      inline size_t _find(const _CharType* charList, size_t charNumber, const _CharType* iter, const _CharType* iterLast) const noexcept {
        const _CharType* charEnd = &(charList[charNumber]);
        while (iter <= iterLast) {
          for (const _CharType* charIt = charList; charIt < charEnd; ++charIt)
            if (*iter == *charIt)
              return _iteratorToPosition(iter);
          ++iter;
        }
        return npos;
      }
      // first first character non-occurrence
      inline size_t _findNot(const _CharType* charList, size_t charNumber, const _CharType* iter, const _CharType* iterLast) const noexcept {
        const _CharType* charEnd = &(charList[charNumber]);
        while (iter <= iterLast) {
          const _CharType* charIt;
          for (charIt = charList; charIt < charEnd; ++charIt)
            if (*iter == *charIt)
              break;
          if (charIt == charEnd)
            return _iteratorToPosition(iter);

          ++iter;
        }
        return npos;
      }
      // find last character occurrence
      inline size_t _rfind(const _CharType* charList, size_t charNumber, const _CharType* iter, const _CharType* iterRevLast) const noexcept {
        const _CharType* charEnd = &(charList[charNumber]);
        while (iter >= iterRevLast) {
          for (const _CharType* charIt = charList; charIt < charEnd; ++charIt)
            if (*iter == *charIt)
              return _iteratorToPosition(iter);
          --iter;
        }
        return npos;
      }
      // find last character non-occurrence
      inline size_t _rfindNot(const _CharType* charList, size_t charNumber, const _CharType* iter, const _CharType* iterRevLast) const noexcept {
        const _CharType* charEnd = &(charList[charNumber]);
        while (iter >= iterRevLast) {
          const _CharType* charIt;
          for (charIt = charList; charIt < charEnd; ++charIt)
            if (*iter == *charIt)
              break;
          if (charIt == charEnd)
            return _iteratorToPosition(iter);

          --iter;
        }
        return npos;
      }

      // first first string occurrence
      inline size_t _findString(const _CharType* query, size_t queryLength, const _CharType* iter, const _CharType* iterLast) const noexcept {
        const _CharType* lastPossibleQuery = iterLast + 1u - queryLength;
        while (iter <= lastPossibleQuery) {
          if (*iter == *query) {
            const _CharType* subIter = iter + queryLength - 1u;
            const _CharType* queryIter = query + queryLength - 1u;
            while (queryIter > query && *subIter == *queryIter) {
              --subIter;
              --queryIter;
            }
            if (queryIter == query)
              return _iteratorToPosition(iter);
          }
          ++iter;
        }
        return npos;
      }
      // find last string occurrence
      inline size_t _rfindString(const _CharType* query, size_t queryLength, const _CharType* iter, const _CharType* iterRevLast) const noexcept {
        const _CharType* queryLast = query + queryLength - 1u;
        iterRevLast = iterRevLast + queryLength - 1u;
        while (iter >= iterRevLast) {
          if (*iter == *queryLast) {
            const _CharType* subIter = iter - queryLength + 1u;
            const _CharType* queryIter = query;
            while (queryIter < queryLast && *subIter == *queryIter) {
              ++subIter;
              ++queryIter;
            }
            if (queryIter == queryLast)
              return _iteratorToPosition(iter - queryLength + 1u);
          }
          --iter;
        }
        return npos;
      }

    private:
      _CharType _value[_MaxSize + 1u];
      size_t _size = 0u;
    };

    // -- aliases --
  
    /// @brief Fixed-size version of standard strings
    template <size_t _MaxSize>
    using fixed_string = FixedSizeString<_MaxSize, char>;

    /// @brief Fixed-size version of UTF-16/UCS-2 strings
    template <size_t _MaxSize>
    using fixed_u16string = FixedSizeString<_MaxSize, char16_t>;

    /// @brief Fixed-size version of UTF-32 strings
    template <size_t _MaxSize>
    using fixed_u32string = FixedSizeString<_MaxSize, char32_t>;

    /// @brief Fixed-size version of wide-character strings
    template <size_t _MaxSize>
    using fixed_wstring = FixedSizeString<_MaxSize, wchar_t>;
  }
}
#undef __p_assert
