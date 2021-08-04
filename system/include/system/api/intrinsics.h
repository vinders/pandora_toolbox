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
System-dependant includes of intrinsics
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cassert>

// -- windows includes --

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(_WINDOWS) || defined(_WIN32) || defined(WIN32) || defined(_WIN64)
# if defined(M_ARM64)
#   include <arm64intr.h>
#   include <arm64_neon.h>
# elif defined(M_ARM)
#   include <armintr.h>
#   include <arm_neon.h>
# else
#   include <intrin.h>
# endif
  
// -- linux / unix includes --

#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__) || defined(__APPLE__)
# if !defined(__clang__)
#   include <fcntl.h>
#   include <unistd.h>
#   if defined(__linux__)
#     include <elf.h>
#     include <linux/auxvec.h>
#   endif
# endif
# if defined(__APPLE__)
#   include <TargetConditionals.h>
# endif

# if defined(__aarch64__)
#   if (!defined(__ANDROID__) && (!defined(__APPLE__) || !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE))
#     include <arm64intr.h>  // NEON support by android-NDK and iOS depends heavily on the toolchain
#     include <arm64_neon.h> // -> considered as not supported until the situation improves
#   endif
# elif defined(__ARM_NEON__)
#   if (!defined(__ANDROID__) && (!defined(__APPLE__) || !defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE))
#     include <armintr.h>  // NEON support by android-NDK and iOS depends heavily on the toolchain
#     include <arm_neon.h> // -> considered as not supported until the situation improves
#   endif
# elif defined(__IWMMXT__)
#   include <mmintrin.h>
# elif defined(__VEC__) || defined(__ALTIVEC__)
#   include <altivec.h>
# elif defined(__SPE__)
#   include <spe.h>
# else
#   include <x86intrin.h>
# endif
#endif
