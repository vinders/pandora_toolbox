/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

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
