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
