/*******************************************************************************
Description : Microsoft Windows base type definitions
*******************************************************************************/
#pragma once

#ifdef _WINDOWS
  // disable unwanted features
# ifndef NOMINMAX // no min/max macros
#   define NOMINMAX
# endif
# ifndef WIN32_LEAN_AND_MEAN // exclude rare MFC libraries
#   define WIN32_LEAN_AND_MEAN
# endif

# include <cstddef>
# include "./windows_version.h"

# if !defined(_AMD64_) && !defined(_IA64_) && !defined(_X86_) && !defined(_ARM64_) && !defined(_ARM_)
#   if defined(__aarch64__) || defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__arm) || defined(_ARM) || defined(_M_ARMT) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB)
#     if defined(__aarch64__) || defined(_M_ARM64)
#       define _ARM64_
#     else
#       define _ARM_
#     endif

#   elif defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__)
#     define _AMD64_
#   else
#     define _X86_
#   endif
# endif

# include <windef.h>

#endif
