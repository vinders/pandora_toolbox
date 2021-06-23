/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
