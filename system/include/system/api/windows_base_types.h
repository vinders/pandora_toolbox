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
