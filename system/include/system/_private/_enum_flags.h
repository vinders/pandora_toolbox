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
