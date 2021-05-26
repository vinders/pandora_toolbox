/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Constants : __forceinline
*******************************************************************************/
#pragma once

#ifndef __forceinline
# if defined(_MSC_VER)
#   if _MSC_VER < 1900
#     define __forceinline inline
#   endif

# elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__MINGW32__)
#   define __forceinline __attribute__((always_inline)) inline

# else
#   define __forceinline inline
# endif
#endif
