/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <io/key_value_serializer.h>

namespace pandora {
  namespace io {
    /// @class IniSerializer
    /// @brief Reader/writer for INI/config key-value pairs (to/from serialized string)
    /// @remarks Typically used to read/write *.ini files.
    class IniSerializer final : public KeyValueSerializer {
    public:
      /// @brief Configure serializer
      IniSerializer() noexcept = default;
      
      /// @brief Serialize list of key-value pairs to INI-formatted data string
      /// @details - line example:      myKey=myInt
      ///          - line with comment: myKey="myText" ;myComment
      ///          - object section:    [myKey]
      ///                               mySubKey1=mySubInt
      ///                               mySubKey2="mySubText"
      ///          - array level:       myKey[]=myValue1
      ///                               myKey[]=myValue2
      /// @warning - INI format does not allow nested objets (objects in sections, objects in arrays) nor arrays in arrays!
      ///            To serialize a hierarchy of objects, consider using JSON/XML/YAML instead.
      ///          - INI format does not allow empty keys (or only made of spaces)
      ///          - Keys and text values should be ASCII or UTF-8 strings.
      /// @throws exception if allocation failure / if contains nested objects
      std::string toString(const SerializableValue::Object& keyValueList) const override;
      
      /// @brief Deserialize list of key-value pairs from INI-formatted data string
      /// @remarks - Permissive parsing: invalid lines are just ignored.
      ///          - Comments are not parsed (SerializableValue.comment() is only meant for 'toString').
      ///          - 'serialized' should be an ASCII or UTF-8 string.
      /// @warning - Section IDs (such as [myKey]) must be unique.
      ///          - Sections will be added as object properties of the top-level object (root object).
      /// @returns Deserialized objet, or empty object if invalid or empty string
      /// @throws exception if allocation failure
      SerializableValue::Object fromString(const char* serialized) const override;
      
    private:
      static void _toArray(const SerializableValue::Array*, const std::string&, std::string&);
      static void _toObject(const SerializableValue::Object*, bool, std::string&);
      static void _insertProperty(const std::string&, bool, SerializableValue::Object&, SerializableValue&&);
      static const char* _readObject(const char*, bool, SerializableValue::Object&);
    };
  }
}
