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
--------------------------------------------------------------------------------
Macros : __align_type(size, <simple_type_declared>)
         __align_prefix(size) <type_declared_with_methods> __align_suffix(size)
*******************************************************************************/
#pragma once

# if defined(_MSC_VER)
#   define __align_prefix(alignment)  __declspec(align(alignment))
#   define __align_suffix(alignment)  
#   define __align_type(alignment, typeDeclaration)  __declspec(align(alignment)) typeDeclaration

# elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__MINGW32__)
#   define __align_prefix(alignment)
#   define __align_suffix(alignment)  __attribute__((aligned(alignment)))
#   define __align_type(alignment, typeDeclaration)  typeDeclaration __attribute__((aligned(alignment)))

# else
#   define __align_prefix(alignment)
#   define __align_suffix(alignment) 
#   define __align_type(alignment, typeDeclaration)  typeDeclaration
# endif
