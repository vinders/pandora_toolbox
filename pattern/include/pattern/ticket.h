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
#include <cstdint>
#include <atomic>

namespace pandora {
  namespace pattern {
    /// @brief Unique ticket for transactions and promises
    template <typename _ContentType,                                    // Inner value type
              typename _AssignedValueType = _ContentType,               // Constant assignment value type for inner value
              _AssignedValueType _InvalidState = (_AssignedValueType)0> // "Empty" value for inner type
    class Ticket final {
    public:
      using Id = uint64_t; ///> Unique identifier type
      using Type = Ticket<_ContentType,_AssignedValueType,_InvalidState>;
    
      /// @brief Create invalid ticket (empty)
      inline Ticket() noexcept : _data(_InvalidState) { _generateId(); }
      /// @brief Create ticket with initial value (with (value != _InvalidState) for the ticket to be valid)
      inline Ticket(_ContentType&& data) noexcept : _data(std::move(data)) { _generateId(); }

      Ticket(const Type&) = default;
      Ticket(Type&& rhs) noexcept : _id(rhs._id), _data(std::move(rhs._data)) { rhs._data = _ContentType(_InvalidState); }
      Type& operator=(const Type&) = default;
      Type& operator=(Type&& rhs) noexcept {
        this->_id = rhs._id; 
        this->_data = std::move(rhs._data); 
        rhs._data = _ContentType(_InvalidState);
        return *this; 
      }
      ~Ticket() = default;
      
      // -- getters --

      constexpr inline Id id() const noexcept { return this->_id; }
      inline const _ContentType& data() const noexcept { return this->_data; }
      inline bool isValid() const noexcept { return (this->_data != _ContentType(_InvalidState)); }
      inline operator bool() const noexcept { return isValid(); }
      
      // -- operations --

      inline uint64_t operator()() const { return static_cast<uint64_t>(this->_id); } // hash
      inline operator uint64_t() const { return static_cast<uint64_t>(this->_id); } // hash
      inline void invalidate() noexcept { this->_data = _ContentType(_InvalidState); }
      
      // -- comparisons --
      
      constexpr inline bool operator==(const Type& rhs) const noexcept { return (this->_id == rhs._id); }
      constexpr inline bool operator!=(const Type& rhs) const noexcept { return (this->_id != rhs._id); }
      constexpr inline bool operator<(const Type& rhs) const noexcept { return (this->_id < rhs._id); }
      constexpr inline bool operator<=(const Type& rhs) const noexcept { return (this->_id <= rhs._id); }
      constexpr inline bool operator>(const Type& rhs) const noexcept { return (this->_id > rhs._id); }
      constexpr inline bool operator>=(const Type& rhs) const noexcept { return (this->_id >= rhs._id); }

    protected:
      void _generateId() noexcept {
        static std::atomic<Id> totalTicketNumber{ static_cast<Id>(0uLL) };
        this->_id = ++totalTicketNumber;
      }

    private:
      Id _id{ 0LL };
      _ContentType _data;
    };

  }
}

namespace std {
  template <typename _ContentType, typename _AssignedValueType, _AssignedValueType _InvalidState>
  struct hash<pandora::pattern::Ticket<_ContentType,_AssignedValueType,_InvalidState> > {
    std::size_t operator()(const pandora::pattern::Ticket<_ContentType,_AssignedValueType,_InvalidState>& key) const {
      return static_cast<std::size_t>(key.id());
    }
  };
}
