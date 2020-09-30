/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
