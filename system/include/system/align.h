/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Macros : __align_type
*******************************************************************************/
#pragma once

#ifndef __align_type
# if defined(_MSC_VER)
#   define __align_type(alignment, typeDeclaration)  __declspec(align(alignment)) typeDeclaration

# elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__MINGW32__)
#   define __align_type(alignment, typeDeclaration)  typeDeclaration __attribute__((aligned(alignment)))

# else
#   define __align_type(alignment, typeDeclaration)  typeDeclaration
# endif
#endif
