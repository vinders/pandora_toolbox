/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace pandora {
  namespace io {
    class KeyValueSerializer;
    
    // ---
    
    /// @brief Generic container for serializable value (multi-type) - immutable
    class SerializableValue {
    public:
      /// @brief Serializable value types
      enum class Type : uint32_t {
        integer = 0u, ///< integer number value
        number  = 1u, ///< floating-point number value
        boolean = 2u, ///< boolean value
        text    = 3u, ///< string value (text, character, int64, ...)
        arrays  = 4u, ///< array of values
        object  = 5u  ///< object value (map containing sub-values)
      };
      using Array = std::vector<SerializableValue>; ///< Array value type
      using Object = std::unordered_map<std::string, SerializableValue>; ///< Object value type
      
      SerializableValue() noexcept { _value.integer = 0; }
      SerializableValue(const SerializableValue& rhs) { _copy(rhs); }
      SerializableValue(SerializableValue&& rhs) noexcept;
      SerializableValue& operator=(const SerializableValue& rhs) { _destroy(); _copy(rhs); return *this; }
      SerializableValue& operator=(SerializableValue&&) noexcept;
      ~SerializableValue() noexcept { _destroy(); }

      bool operator==(const SerializableValue& rhs) const noexcept;
      bool operator!=(const SerializableValue& rhs) const noexcept { return !operator==(rhs); }
      
      // -- create immutable value --
      
      /// @brief Create integer number
      SerializableValue(int32_t value) noexcept : _length(1), _valueType(Type::integer) { this->_value.integer = value; }
      /// @brief Create floating-point number (double)
      explicit SerializableValue(double value) noexcept : _length(1), _valueType(Type::number) { this->_value.number = value; }
      /// @brief Create floating-point number (float)
      explicit SerializableValue(float value) noexcept : _length(1), _valueType(Type::number) { this->_value.number = (double)value; }
      /// @brief Create boolean value
      explicit SerializableValue(bool value) noexcept : _length(1), _valueType(Type::boolean) { this->_value.boolean = value; }
      /// @brief Create string value (text, character, int64, ...)
      /// @remarks The value should already be encoded for the output (UTF-8)
      /// @throw exception if allocation failure
      SerializableValue(const char* value);
      /// @brief Create string value (text, character, int64, ...)
      /// @remarks The value should already be encoded for the output (UTF-8)
      /// @throw exception if allocation failure
      explicit SerializableValue(const std::string& value);
      /// @brief Create array of values (immutable: must be filled before creating SerializableValue)
      /// @throw exception if allocation failure or vector error
      SerializableValue(Array&& value);
      /// @brief Create object with sub-values (immutable: must be filled before creating SerializableValue)
      /// @throw exception if allocation failure or map error
      SerializableValue(Object&& value);

      // -- accessors --
      
      inline Type type() const noexcept { return this->_valueType; } ///< Get inner value type
      inline size_t size() const noexcept { return this->_length; }  ///< Get size: number of sub-values / text length / array size
      inline bool empty() const noexcept { return (this->_length == size_t{0}); } ///< Verify if value is an empty text/array/object (NULL value)
      inline operator bool() const noexcept { return (this->_length != size_t{0}); }
      
      inline const char* comment() const noexcept { return (const char*)this->_comment; } ///< Get comment currently associated with the value
      void setComment(const char* comment); ///< Add a comment to associate with the value (for serialization)
      
      // -- read value --
      
      /// @brief Get integer value, or convert from number/string value
      /// @returns Integer value, or 0 if not convertible (array/map/non-number string)
      int32_t getInteger() const noexcept;
      /// @brief Get floating-point number value, or convert from integer/string value
      /// @returns Floating-point value, or 0.0 if not convertible (array/map/non-number string)
      double getNumber() const noexcept;
      /// @brief Get boolean value, or convert any other value type
      /// @returns Boolean value, or number!=0, or text/array/map not empty
      bool getBoolean() const noexcept;
      /// @brief Get string value, or stringify bool value
      /// @warning Do NOT free/delete this value!
      /// @returns String value, or nullptr if empty string or if int/number/array/map
      const char* getText() const;
      /// @brief Get array of values
      /// @returns Array of values, or nullptr if not an array or if the array is empty
      const Array* getArray() const noexcept { 
        return (this->_valueType == Type::arrays) ? (const std::vector<SerializableValue>*)this->_value.arrayRef : nullptr; 
      }
      /// @brief Get object with sub-values
      /// @returns Object map, or nullptr if not an object or if the object is empty
      const Object* getObject() const noexcept { 
        return (this->_valueType == Type::object) ? (const Object*)this->_value.objectRef : nullptr; 
      }
      
    private:
      SerializableValue(size_t, char*) noexcept;
      Array* _getArray();
      void _pushArrayItem(SerializableValue&&);
      void _copy(const SerializableValue&);
      void _destroy() noexcept;
      friend class KeyValueSerializer;
      
    private:
      char* _comment = nullptr; // description comment to add next to entry (can be multi-line)
      union {
        int32_t integer;
        double number;
        bool boolean;
        char* text;
        Array* arrayRef;
        Object* objectRef;
      } _value;                        // mixed values
      size_t _length = 0;              // text length / array size / object map size
      Type _valueType = Type::integer; // type of '_value'
    };
    
    
    // -- Key-value serializer interface --
    
    /// @class KeyValueSerializer
    /// @brief Reader/writer for lists of key-value pairs (to/from serialized string) - interface
    /// @remarks Typically used to read/write *.ini/*.json/... files.
    class KeyValueSerializer {
    public:
      /// @brief Serialize list of key-value pairs to formatted data string
      virtual std::string toString(const SerializableValue::Object& keyValueList) const = 0;
      /// @brief Deserialize list of key-value pairs from formatted data string
      virtual SerializableValue::Object fromString(const char* serialized) const = 0;
    
      // ---
    
    protected:
      KeyValueSerializer() = default;

      static inline SerializableValue _valueFromMovedText(size_t length, char* movedValue) noexcept { return SerializableValue(length, movedValue); }
      static inline SerializableValue::Array* _getArray(SerializableValue& arrayContainer) { return arrayContainer._getArray(); }
      static inline void _pushArrayItem(SerializableValue& arrayRef, SerializableValue&& value) { arrayRef._pushArrayItem(std::move(value)); }
    };
  }
}
