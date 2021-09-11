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

#ifndef _P_VK_STATIC_LINK
# define VK_NO_PROTOTYPES 1 // dynamic linking
#endif

#if defined(_WINDOWS)
# ifndef VK_USE_PLATFORM_WIN32_KHR
#   define VK_USE_PLATFORM_WIN32_KHR
# endif
#elif defined(__ANDROID__)
# ifndef VK_USE_PLATFORM_ANDROID_KHR
#   define VK_USE_PLATFORM_ANDROID_KHR
# endif
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# ifndef VK_USE_PLATFORM_METAL_EXT
#   define VK_USE_PLATFORM_METAL_EXT
# endif
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   ifndef VK_USE_PLATFORM_IOS_MVK
#     define VK_USE_PLATFORM_IOS_MVK
#   endif
# else
#   ifndef VK_USE_PLATFORM_MACOS_MVK
#     define VK_USE_PLATFORM_MACOS_MVK
#   endif
# endif
#elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
# if defined(_P_ENABLE_LINUX_WAYLAND)
#   ifndef VK_USE_PLATFORM_WAYLAND_KHR
#     define VK_USE_PLATFORM_WAYLAND_KHR
#   endif
# else
#   ifndef VK_USE_PLATFORM_XCB_KHR
#     define VK_USE_PLATFORM_XCB_KHR
#   endif
#   ifndef VK_USE_PLATFORM_XLIB_KHR
#     define VK_USE_PLATFORM_XLIB_KHR
#   endif
# endif
#endif

#include <cstdint>
#if !defined(_WINDOWS) && defined(__APPLE__)
# include <MoltenVK/vulkan/vulkan.h> // MacOS / iOS
#else
# include <vulkan/vulkan.h> // Windows / Linux / Android
#endif
