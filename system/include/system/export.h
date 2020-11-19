/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Dynamic library import/export utility (_P_EXPORT)
Usage : - cwork (cmake framework) automatically defines _P_DYNAMIC_LIBRARY_BUILD for dynamic libs.
        - before including a file of another (external) dynamic library:
          -> define _P_DYNAMIC_LIBRARY_IMPORT above the include directive.
          -> undef _P_DYNAMIC_LIBRARY_IMPORT below the include directive.
          -> that way, you can safely import features of another dynamic lib from a dynamic lib project.
          -> Example :
              #define _P_DYNAMIC_LIBRARY_IMPORT
              #include <file_of_other_dll_also_built_with_cwork.h>
              #undef _P_DYNAMIC_LIBRARY_IMPORT
*******************************************************************************/
#pragma once

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(_WINDOWS) || defined(_WIN32) || defined(_WIN64) || defined(_WINNT)
# if defined(_P_DYNAMIC_LIBRARY_BUILD) && !defined(_P_DYNAMIC_LIBRARY_IMPORT)
#   define _P_EXPORT __declspec(dllexport)
# else
#   define _P_EXPORT __declspec(dllimport)
# endif

#elif defined(__GNUC__) || defined(__GNUG__) || defined(__clang__) || (defined(__INTEL_COMPILER) && (defined(__linux__) || defined(__APPLE)) )
# if defined(_P_DYNAMIC_LIBRARY_BUILD) && !defined(_P_DYNAMIC_LIBRARY_IMPORT)
#   define _P_EXPORT __attribute__((visibility("default")))
# else
#   define _P_EXPORT
# endif

#else
# define _P_EXPORT
#endif
