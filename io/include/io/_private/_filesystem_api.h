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

#if (!defined(_CPP_REVISION) || _CPP_REVISION != 14) && ( (defined(_MSC_VER) && _MSC_VER >= 1914) || (!defined(__MINGW32__) && defined(__GNUC__) && __GNUC__ >= 8) || (defined(__MINGW32__) && defined(__GNUC__) && __GNUC__ > 9) || (defined(__clang_major__) && __clang_major__ >= 11) )
# include <filesystem>
#else
# define _USE_NATIVE_FILESYSTEM 1
#endif

#ifdef _WINDOWS
  // windows
# include <io.h>
# include <tchar.h>

# ifndef NOMINMAX
#   define NOMINMAX // no min/max macros
#   define WIN32_LEAN_AND_MEAN // exclude rare MFC libraries
# endif
# include <Windows.h>
# include <shlwapi.h>
# include <direct.h>

# define _MAX_PATH_SIZE MAX_PATH

#else 
  // linux/unix
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <pwd.h>
# ifdef _USE_NATIVE_FILESYSTEM
#   include <dirent.h>
# endif

# define _MAX_PATH_SIZE 10240
#endif
