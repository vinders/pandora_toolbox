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
Macros : TRACE(<format>,...)             : write message in standard console output (in debug mode only)
         ERROR_TRACE(<format>,...)       : write message in standard error output (in debug mode only)
         FILE_TRACE(<file>,<format>,...) : write message in a file (in debug mode only)
*******************************************************************************/
#pragma once

#include <cstddef>

// -- DEBUG MODE --
#if defined(_DEBUG) || !defined(NDEBUG)

# include <cstdio>
# include <cstring>

# ifdef _MSC_VER
#   define __PRETTY_FUNCTION__ __FUNCSIG__
# endif
# ifndef __FILE_NAME__
#   ifdef _WINDOWS
#     define __FILE_NAME__ strrchr("\\" __FILE__, '\\') + 1
#   else
#     define __FILE_NAME__ strrchr("/" __FILE__, '/') + 1
#   endif
# endif
# define __DEBUG_ARG__(arg) arg

# define TRACE(format)       fprintf(stdout, "%s(%d): " format "\n", __FILE_NAME__, __LINE__)
# define TRACE_N(format,...) fprintf(stdout, "%s(%d): " format "\n", __FILE_NAME__, __LINE__, __VA_ARGS__)

# define ERROR_TRACE(format)       fprintf(stderr, "%s(%d): " format "\n", __FILE_NAME__, __LINE__)
# define ERROR_TRACE_N(format,...) fprintf(stderr, "%s(%d): " format "\n", __FILE_NAME__, __LINE__, __VA_ARGS__)

# define FILE_TRACE(fileHandle,format)       fprintf(fileHandle, "%s(%d): " format "\n", __FILE_NAME__, __LINE__)
# define FILE_TRACE_N(fileHandle,format,...) fprintf(fileHandle, "%s(%d): " format "\n", __FILE_NAME__, __LINE__, __VA_ARGS__)

// -- RELEASE MODE --
#else
# ifndef __noop
#   define __noop ((void)0)
# endif
# ifndef __FILE_NAME__
#   define __FILE_NAME__
# endif
# define __DEBUG_ARG__(arg)

# define TRACE(format)        __noop  // no operation
# define TRACE_N(format,...)  __noop  // no operation
# define ERROR_TRACE(format)        __noop  // no operation
# define ERROR_TRACE_N(format,...)  __noop  // no operation
# define FILE_TRACE(fileHandle, format)        __noop // no operation
# define FILE_TRACE_N(fileHandle, format,...)  __noop // no operation
#endif
