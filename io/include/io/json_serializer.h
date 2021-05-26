/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <io/key_value_serializer.h>

namespace pandora {
  namespace io {
    /// @class JsonSerializer
    /// @brief Reader/writer for JSON key-value pairs (to/from serialized string)
    /// @remarks Typically used to read/write *.json files.
    class JsonSerializer final : public KeyValueSerializer {
    public:
      /// @brief Configure serializer
      /// @param indentSize Row count per indentation level (only used for 'toString')
      JsonSerializer(size_t indentSize = size_t{ 2u }) noexcept : KeyValueSerializer(), _indentSize(indentSize) {}
      
      /// @brief Serialize list of key-value pairs to JSON-formatted data string
      /// @details - line example:      "myKey": myInt,
      ///          - line with comment: "myKey": "myText", // myComment
      ///          - object level:      "myKey": {
      ///                                 "mySubKey1": mySubInt,
      ///                                 "mySubKey2": "mySubText"
      ///                               },
      ///          - array level:      "myKey": [
      ///                                 myValue1,
      ///                                 myValue2
      ///                               ],
      /// @warning Keys and text values should be ASCII or UTF-8 strings.
      /// @throws exception if allocation failure
      std::string toString(const SerializableValue::Object& keyValueList) const override;
      
      /// @brief Deserialize list of key-value pairs from JSON-formatted data string
      /// @remarks - Not permissive for syntax errors: any invalid syntax will result in an exception.
      ///          - Comments are not parsed (SerializableValue.comment() is only meant for 'toString').
      ///          - 'serialized' should be an ASCII or UTF-8 string.
      /// @returns Deserialized objet, or empty object if empty string
      /// @throws exception if allocation failure / if invalid JSON string
      SerializableValue::Object fromString(const char* serialized) const override;
      
    private:
      static void _toArray(const SerializableValue::Array*, const char*, size_t, std::string&, std::string&);
      static void _toObject(const SerializableValue::Object*, const char*, size_t, std::string&, std::string&);
      static const char* _readArray(const char*, SerializableValue::Array&);
      static const char* _readObject(const char*, SerializableValue::Object&);
      
      size_t _indentSize{ 2u };
    };
  }
}
