/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Macros : _P_EXPAND,
         _P_STRINGIFY, _P_STRINGIFY_COMMA, _P_STRINGIFY_SEMICOLON,
         _P_GET_ARG_COUNT, _P_GET_FIRST_ARG, _P_DROP_FIRST_ARG, 
         _P_FOREACH, _P_PARAM_FOREACH, 
         _P_DUPLICATE_<#>X, _P_DUPLICATE_<#>X_COMMA, _P_DUPLICATE_<#>X_SEMICOLON
Code generators : _P_SERIALIZABLE_ENUM, 
                  _P_LIST_ENUM_VALUES,
                  _P_FLAGS_OPERATORS
*******************************************************************************/
#pragma once

#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstddef>
#include <cstring>
#include <string>
#include <array>
#include "./_private/_preprocessor_tools.h"


// -- expand arguments --

#define _P_EXPAND(x)           x    // expand argument

// -- convert arguments to strings --

#define _P_STRINGIFY(x)            #x               // stringify argument
#define _P_STRINGIFY_COMMA(x)      _P_STRINGIFY(x), // stringify + append comma after string
#define _P_STRINGIFY_SEMICOLON(x)  _P_STRINGIFY(x); // stringify + append semi-colon after string


// -- variadic arguments --

// count number of variadic arguments (up to 100)
//_P_GET_ARG_COUNT(...) : defined in private file

// get specific argument in variadic arguments
#define _P_GET_FIRST_ARG(x,...)   x           // get first of variadic arguments
#define _P_DROP_FIRST_ARG(x,...)  __VA_ARGS__ // drop first of variadic arguments


// -- ranged loop for variadic arguments --

// apply a macro to each argument (up to 100)
#define _P_FOREACH(macro, ...)              __P_FOREACH_RANGE(_P_GET_ARG_COUNT(__VA_ARGS__), macro, __VA_ARGS__)
// apply a macro to each argument (up to 100) - comma separators
#define _P_FOREACH_COMMA(macro, ...)        __P_FOREACH_RANGE_COMMA(_P_GET_ARG_COUNT(__VA_ARGS__), macro, __VA_ARGS__)
// apply a macro to each argument (up to 100) - semi-colon separators
#define _P_FOREACH_SEMICOLON(macro, ...)    __P_FOREACH_RANGE_SEMICOLON(_P_GET_ARG_COUNT(__VA_ARGS__), macro, __VA_ARGS__)

// apply a macro with parameter to each argument (up to 100)
#define _P_PARAM_FOREACH(macro, param, ...) __P_PARAM_FOREACH_RANGE(_P_GET_ARG_COUNT(__VA_ARGS__), macro, param, __VA_ARGS__)


// -- enums --

#define __P_CONCAT_ENUM(type, value)            type :: value
#define __P_SERIALIZE_ENUM_CASE(type, value)    case (__P_CONCAT_ENUM(type,value)): { return _P_STRINGIFY_SEMICOLON(value) }
#define __P_DESERIALIZE_ENUM_CASE(type, value)  else if (_val == _P_STRINGIFY(value)) { _out=(__P_CONCAT_ENUM(type,value)); return true; }
#define __P_SERIALIZE_ENUM_CASE_BUFFER(type, value)    case (__P_CONCAT_ENUM(type,value)): { strncpy(_out, _P_STRINGIFY(value), _bufferSize); _out[_bufferSize - 1u] = '\0'; break; }
#define __P_DESERIALIZE_ENUM_CASE_BUFFER(type, value)  else if (strncmp(_val, _P_STRINGIFY(value), _length) == 0) { _out=(__P_CONCAT_ENUM(type,value)); return true; }
#define __P_EXPAND_ENUM_VALUE(type, value)      , __P_CONCAT_ENUM(type,value)

// define function to get a list of enum values (_P_SERIALIZABLE_ENUM required)
#define _P_LIST_ENUM_VALUES(type, suffix, ...) \
        constexpr inline std::array<type, _P_GET_ARG_COUNT(__VA_ARGS__)> __P_CONCAT3(type,_,suffix)() noexcept { \
          return std::array<type, _P_GET_ARG_COUNT(__VA_ARGS__)>{ __P_CONCAT_ENUM(type,_P_GET_FIRST_ARG(__VA_ARGS__)) _P_PARAM_FOREACH(__P_EXPAND_ENUM_VALUE, type, _P_DROP_FIRST_ARG(__VA_ARGS__)) }; \
        }

// make an enum serializable/deserializable
#define _P_SERIALIZABLE_ENUM(type, ...) \
        inline std::string toString(type _val) { \
          switch (_val) { \
            _P_PARAM_FOREACH(__P_SERIALIZE_ENUM_CASE, type, __VA_ARGS__) \
            default: return ""; \
          } \
        } \
        template < type _val> \
        inline constexpr const char* toString() noexcept { \
          switch (_val) { \
            _P_PARAM_FOREACH(__P_SERIALIZE_ENUM_CASE, type, __VA_ARGS__) \
            default: return ""; \
          } \
        } \
        inline bool fromString(const std::string& _val, type& _out) noexcept { \
          if (_val.empty()) { return false; } \
          _P_PARAM_FOREACH(__P_DESERIALIZE_ENUM_CASE, type, __VA_ARGS__) \
          return false; \
        } \
        constexpr inline size_t __P_CONCAT(type,_size)() { return _P_GET_ARG_COUNT(__VA_ARGS__); }

// make an enum serializable/deserializable - external buffer
#define _P_SERIALIZABLE_ENUM_BUFFER(type, ...) \
        inline char* toString(char* _out, size_t _bufferSize, type _val) noexcept { \
          switch (_val) { \
            _P_PARAM_FOREACH(__P_SERIALIZE_ENUM_CASE_BUFFER, type, __VA_ARGS__) \
            default: if (_bufferSize > size_t{ 0 }) { *_out = '\0'; } break; \
          } \
          return _out; \
        } \
        inline bool fromString(const char* _val, size_t _length, type& _out) noexcept { \
          if (_length == 0) { return false; } \
          _P_PARAM_FOREACH(__P_DESERIALIZE_ENUM_CASE_BUFFER, type, __VA_ARGS__) \
          return false; \
        }


// -- flags operations --

#define _P_FLAGS_OPERATORS(typeName, valueType) \
        inline constexpr bool operator==(typeName lhs, bool rhs) noexcept { return ((static_cast<valueType>(lhs) != 0) == rhs); } \
        inline constexpr bool operator!=(typeName lhs, bool rhs) noexcept { return ((static_cast<valueType>(lhs) != 0) != rhs); } \
        inline constexpr typeName operator&(typeName lhs, typeName rhs) noexcept { return static_cast<typeName>( static_cast<valueType>(lhs) & static_cast<valueType>(rhs) ); } \
        inline constexpr typeName operator|(typeName lhs, typeName rhs) noexcept { return static_cast<typeName>( static_cast<valueType>(lhs) | static_cast<valueType>(rhs) ); } \
        inline constexpr typeName operator^(typeName lhs, typeName rhs) noexcept { return static_cast<typeName>( static_cast<valueType>(lhs) ^ static_cast<valueType>(rhs) ); } \
        inline constexpr typeName operator~(typeName rhs) noexcept { return static_cast<typeName>(~static_cast<valueType>(rhs)); } \
        inline typeName& operator&=(typeName& lhs, typeName rhs) noexcept { lhs = static_cast<typeName>( static_cast<valueType>(lhs) & static_cast<valueType>(rhs) ); return lhs; } \
        inline typeName& operator|=(typeName& lhs, typeName rhs) noexcept { lhs = static_cast<typeName>( static_cast<valueType>(lhs) | static_cast<valueType>(rhs) ); return lhs; } \
        inline typeName& operator^=(typeName& lhs, typeName rhs) noexcept { lhs = static_cast<typeName>( static_cast<valueType>(lhs) ^ static_cast<valueType>(rhs) ); return lhs; } \
        inline constexpr bool operator<(typeName lhs, typeName rhs) noexcept  { return (static_cast<valueType>(lhs) < static_cast<valueType>(rhs)); } \
        inline constexpr bool operator<=(typeName lhs, typeName rhs) noexcept { return (static_cast<valueType>(lhs) <= static_cast<valueType>(rhs)); } \
        inline constexpr bool operator>(typeName lhs, typeName rhs) noexcept  { return (static_cast<valueType>(lhs) > static_cast<valueType>(rhs)); } \
        inline constexpr bool operator>=(typeName lhs, typeName rhs) noexcept { return (static_cast<valueType>(lhs) >= static_cast<valueType>(rhs)); } \
        inline typeName& addFlag(typeName& lhs, typeName flag) noexcept    { lhs = static_cast<typeName>( static_cast<valueType>(lhs) | static_cast<valueType>(flag) ); return lhs; } \
        inline typeName& removeFlag(typeName& lhs, typeName flag) noexcept { lhs = static_cast<typeName>( static_cast<valueType>(lhs) & (~static_cast<valueType>(flag)) ); return lhs; }


// -- duplicate arguments --

// duplicate arguments, separated by spaces
#define _P_DUPLICATE_2X(arg)   arg arg
#define _P_DUPLICATE_3X(arg)   arg arg arg
#define _P_DUPLICATE_4X(arg)   arg arg arg arg
#define _P_DUPLICATE_5X(arg)   _P_DUPLICATE_3X(arg) _P_DUPLICATE_2X(arg)
#define _P_DUPLICATE_6X(arg)   _P_DUPLICATE_3X(arg) _P_DUPLICATE_3X(arg)
#define _P_DUPLICATE_7X(arg)   _P_DUPLICATE_4X(arg) _P_DUPLICATE_3X(arg)
#define _P_DUPLICATE_8X(arg)   _P_DUPLICATE_4X(arg) _P_DUPLICATE_4X(arg)
#define _P_DUPLICATE_9X(arg)   _P_DUPLICATE_5X(arg) _P_DUPLICATE_4X(arg)
#define _P_DUPLICATE_10X(arg)  _P_DUPLICATE_5X(arg) _P_DUPLICATE_5X(arg)
#define _P_DUPLICATE_12X(arg)  _P_DUPLICATE_6X(arg) _P_DUPLICATE_6X(arg)
#define _P_DUPLICATE_16X(arg)  _P_DUPLICATE_8X(arg) _P_DUPLICATE_8X(arg)
#define _P_DUPLICATE_24X(arg)  _P_DUPLICATE_16X(arg) _P_DUPLICATE_8X(arg)
#define _P_DUPLICATE_32X(arg)  _P_DUPLICATE_16X(arg) _P_DUPLICATE_16X(arg)
#define _P_DUPLICATE_48X(arg)  _P_DUPLICATE_32X(arg) _P_DUPLICATE_16X(arg)
#define _P_DUPLICATE_64X(arg)  _P_DUPLICATE_32X(arg) _P_DUPLICATE_32X(arg)

#define _P_DUPLICATE_2X_COMMA(arg)   arg,arg
#define _P_DUPLICATE_3X_COMMA(arg)   arg,arg,arg
#define _P_DUPLICATE_4X_COMMA(arg)   arg,arg,arg,arg
#define _P_DUPLICATE_5X_COMMA(arg)   _P_DUPLICATE_3X_COMMA(arg),_P_DUPLICATE_2X_COMMA(arg)
#define _P_DUPLICATE_6X_COMMA(arg)   _P_DUPLICATE_3X_COMMA(arg),_P_DUPLICATE_3X_COMMA(arg)
#define _P_DUPLICATE_7X_COMMA(arg)   _P_DUPLICATE_4X_COMMA(arg),_P_DUPLICATE_3X_COMMA(arg)
#define _P_DUPLICATE_8X_COMMA(arg)   _P_DUPLICATE_4X_COMMA(arg),_P_DUPLICATE_4X_COMMA(arg)
#define _P_DUPLICATE_9X_COMMA(arg)   _P_DUPLICATE_5X_COMMA(arg),_P_DUPLICATE_4X_COMMA(arg)
#define _P_DUPLICATE_10X_COMMA(arg)  _P_DUPLICATE_5X_COMMA(arg),_P_DUPLICATE_5X_COMMA(arg)
#define _P_DUPLICATE_12X_COMMA(arg)  _P_DUPLICATE_6X_COMMA(arg),_P_DUPLICATE_6X_COMMA(arg)
#define _P_DUPLICATE_16X_COMMA(arg)  _P_DUPLICATE_8X_COMMA(arg),_P_DUPLICATE_8X_COMMA(arg)
#define _P_DUPLICATE_24X_COMMA(arg)  _P_DUPLICATE_16X_COMMA(arg),_P_DUPLICATE_8X_COMMA(arg)
#define _P_DUPLICATE_32X_COMMA(arg)  _P_DUPLICATE_16X_COMMA(arg),_P_DUPLICATE_16X_COMMA(arg)
#define _P_DUPLICATE_48X_COMMA(arg)  _P_DUPLICATE_32X_COMMA(arg),_P_DUPLICATE_16X_COMMA(arg)
#define _P_DUPLICATE_64X_COMMA(arg)  _P_DUPLICATE_32X_COMMA(arg),_P_DUPLICATE_32X_COMMA(arg)

#define _P_DUPLICATE_2X_SEMICOLON(arg)   (arg);(arg)
#define _P_DUPLICATE_3X_SEMICOLON(arg)   (arg);(arg);(arg)
#define _P_DUPLICATE_4X_SEMICOLON(arg)   (arg);(arg);(arg);(arg)
#define _P_DUPLICATE_5X_SEMICOLON(arg)   _P_DUPLICATE_3X_SEMICOLON(arg);_P_DUPLICATE_2X_SEMICOLON(arg)
#define _P_DUPLICATE_6X_SEMICOLON(arg)   _P_DUPLICATE_3X_SEMICOLON(arg);_P_DUPLICATE_3X_SEMICOLON(arg)
#define _P_DUPLICATE_7X_SEMICOLON(arg)   _P_DUPLICATE_4X_SEMICOLON(arg);_P_DUPLICATE_3X_SEMICOLON(arg)
#define _P_DUPLICATE_8X_SEMICOLON(arg)   _P_DUPLICATE_4X_SEMICOLON(arg);_P_DUPLICATE_4X_SEMICOLON(arg)
#define _P_DUPLICATE_9X_SEMICOLON(arg)   _P_DUPLICATE_5X_SEMICOLON(arg);_P_DUPLICATE_4X_SEMICOLON(arg)
#define _P_DUPLICATE_10X_SEMICOLON(arg)  _P_DUPLICATE_5X_SEMICOLON(arg);_P_DUPLICATE_5X_SEMICOLON(arg)
#define _P_DUPLICATE_12X_SEMICOLON(arg)  _P_DUPLICATE_6X_SEMICOLON(arg);_P_DUPLICATE_6X_SEMICOLON(arg)
#define _P_DUPLICATE_16X_SEMICOLON(arg)  _P_DUPLICATE_8X_SEMICOLON(arg);_P_DUPLICATE_8X_SEMICOLON(arg)
#define _P_DUPLICATE_24X_SEMICOLON(arg)  _P_DUPLICATE_16X_SEMICOLON(arg);_P_DUPLICATE_8X_SEMICOLON(arg)
#define _P_DUPLICATE_32X_SEMICOLON(arg)  _P_DUPLICATE_16X_SEMICOLON(arg);_P_DUPLICATE_16X_SEMICOLON(arg)
#define _P_DUPLICATE_48X_SEMICOLON(arg)  _P_DUPLICATE_32X_SEMICOLON(arg);_P_DUPLICATE_16X_SEMICOLON(arg)
#define _P_DUPLICATE_64X_SEMICOLON(arg)  _P_DUPLICATE_32X_SEMICOLON(arg);_P_DUPLICATE_32X_SEMICOLON(arg)
