/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if (!defined(_CPP_REVISION) || _CPP_REVISION != 14) && ( (defined(_MSC_VER) && _MSC_VER >= 1914) || (defined(__GNUC__) && __GNUC__ >= 8) || (defined(__clang_major__) && __clang_major__ >= 11) )
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
