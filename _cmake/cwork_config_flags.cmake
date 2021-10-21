# MIT License
# Copyright (c) 2021 Romain Vinders

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
# IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# ---

#no include protection here - allow re-includes in subdirectories
#params: - CWORK_BUILD_TYPE

#includes
include(${CMAKE_CURRENT_LIST_DIR}/cwork_options.cmake)
if(NOT DEFINED CWORK_SYSTEM_ARCH_BITS)
    include(${CMAKE_CURRENT_LIST_DIR}/cwork_arch_flags.cmake)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Language flags                                                  │
# └──────────────────────────────────────────────────────────────────┘
#set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(CWORK_CPP_REVISION AND ${CWORK_CPP_REVISION} STREQUAL "14")
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
elseif(CWORK_CPP_REVISION AND ${CWORK_CPP_REVISION} STREQUAL "20")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build configurations                                            │
# └──────────────────────────────────────────────────────────────────┘
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
if(CWORK_BUILD_TYPE AND ${CWORK_BUILD_TYPE} STREQUAL "dynamic")
    set(BUILD_SHARED_LIBS TRUE)
endif()
if(NOT CMAKE_BUILD_TYPE)
    if(CWORK_COVERAGE)
        set(CMAKE_BUILD_TYPE Debug)
    else()
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4505 /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /Zi /Ob0 /Od /RTC1 /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /O2 /Ob2 /Zi /DRELEASE")
    add_definitions(-DUNICODE -D_UNICODE)
    
    if(CWORK_EXPORT_SYMBOLS)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")

else() # GCC / Clang
    if(CWORK_BUILD_TYPE AND (${CWORK_BUILD_TYPE} STREQUAL "executable" OR ${CWORK_BUILD_TYPE} STREQUAL "console"))
        set(__CWORK_EXECUTABLE_SCOPE ON) # current lib and subdependencies
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(-Wall -Wno-missing-braces -Wmissing-field-initializers)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    else()
        add_compile_options(-Wall)
    endif()
    if(MINGW)
        if(CWORK_EXPORT_SYMBOLS)
            set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        endif()
        set(CMAKE_SH "CMAKE_SH-NOTFOUND")
    endif()
    
    if(NOT DEFINED __CWORK_EXECUTABLE_SCOPE AND NOT MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif()
    if(CWORK_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  System definitions                                              │
# └──────────────────────────────────────────────────────────────────┘
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CWORK_WINDOWS_VERSION)
    add_definitions(-D_P_MIN_WINDOWS_VERSION=${CWORK_WINDOWS_VERSION})
endif()
if(CWORK_LINUX_WAYLAND)
    add_definitions(-D_P_ENABLE_LINUX_WAYLAND=1)
endif()
if(MSVC)
    if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(${CWORK_SYSTEM_ARCH_BITS} EQUAL 64)
            add_definitions(-D_WIN64=1)
        else()
            add_definitions(-D_WIN32=1)
        endif()
    endif()

else() # GCC / Clang
    if(MINGW)
        add_definitions(-D_WINDOWS=1)
        add_definitions(-D_WIN32_WINNT=0x0601)
        if(${CWORK_SYSTEM_ARCH_BITS} EQUAL 64)
            add_definitions(-D_WIN64=1)
        else()
            add_definitions(-D_WIN32=1)
        endif()
    elseif(IOS)
        add_definitions(-D__APPLE_IOS__=1)
    endif()
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Language features                                               │
# └──────────────────────────────────────────────────────────────────┘
if(CWORK_CPP_REVISION AND ${CWORK_CPP_REVISION} STREQUAL "14")
    if(MSVC)
        add_compile_options(/std:c++14 /wd4127)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-std=c++14)
    endif()
    add_definitions(-D_CPP_REVISION=14)
elseif(CWORK_CPP_REVISION AND ${CWORK_CPP_REVISION} STREQUAL "20")
    if(MSVC)
        add_compile_options(/std:c++20)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-std=c++20)
    endif()
else()
    if(MSVC)
        add_compile_options(/std:c++17)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-std=c++17)
    endif()
endif()

if(DEFINED CWORK_CI_DISABLE_SLOW_TESTS AND CWORK_CI_DISABLE_SLOW_TESTS)
    add_definitions(-D_P_CI_DISABLE_SLOW_TESTS=1)
endif()
if(DEFINED CWORK_SHADER_COMPILERS AND CWORK_SHADER_COMPILERS)
    add_definitions(-D_P_VIDEO_SHADER_COMPILERS=1)
endif()

if(CWORK_VIDEO_OPENGL4)
    add_definitions(-D_VIDEO_OPENGL4_SUPPORT=1)
    if(APPLE)
        add_definitions(-D_VIDEO_OPENGL_VERSION=41)
        set(_VIDEO_OPENGL4_VERSION_STRING "4.1" CACHE INTERNAL "" FORCE)
    else()
        if(CWORK_OPENGL4_VERSION AND ${CWORK_OPENGL4_VERSION} STREQUAL "46")
            add_definitions(-D_VIDEO_OPENGL_VERSION=46)
            set(_VIDEO_OPENGL4_VERSION_STRING "4.6" CACHE INTERNAL "" FORCE)
        elseif(CWORK_OPENGL4_VERSION AND ${CWORK_OPENGL4_VERSION} STREQUAL "45")
            add_definitions(-D_VIDEO_OPENGL_VERSION=45)
            set(_VIDEO_OPENGL4_VERSION_STRING "4.5" CACHE INTERNAL "" FORCE)
        elseif(CWORK_OPENGL4_VERSION AND ${CWORK_OPENGL4_VERSION} STREQUAL "43")
            add_definitions(-D_VIDEO_OPENGL_VERSION=43)
            set(_VIDEO_OPENGL4_VERSION_STRING "4.3" CACHE INTERNAL "" FORCE)
        else()
            add_definitions(-D_VIDEO_OPENGL_VERSION=41)
            set(_VIDEO_OPENGL4_VERSION_STRING "4.1" CACHE INTERNAL "" FORCE)
        endif()
    endif()
endif()

if(CWORK_VIDEO_D3D11)
    add_definitions(-D_VIDEO_D3D11_SUPPORT=1)
    if(CWORK_D3D11_VERSION AND ${CWORK_D3D11_VERSION} STREQUAL "114")
        add_definitions(-D_VIDEO_D3D11_VERSION=114)
        set(_VIDEO_D3D11_VERSION_STRING "11.4" CACHE INTERNAL "" FORCE)
    elseif(CWORK_D3D11_VERSION AND ${CWORK_D3D11_VERSION} STREQUAL "113")
        add_definitions(-D_VIDEO_D3D11_VERSION=113)
        set(_VIDEO_D3D11_VERSION_STRING "11.3" CACHE INTERNAL "" FORCE)
    elseif(CWORK_D3D11_VERSION AND ${CWORK_D3D11_VERSION} STREQUAL "111")
        add_definitions(-D_VIDEO_D3D11_VERSION=111)
        set(_VIDEO_D3D11_VERSION_STRING "11.1" CACHE INTERNAL "" FORCE)
    else()
        add_definitions(-D_VIDEO_D3D11_VERSION=110)
        set(_VIDEO_D3D11_VERSION_STRING "11.0" CACHE INTERNAL "" FORCE)
    endif()
endif()

if(CWORK_VIDEO_VULKAN)
    add_definitions(-D_VIDEO_VULKAN_SUPPORT=1)
    add_definitions(-D_VIDEO_VULKAN_VERSION=12)
    set(_VIDEO_VULKAN_VERSION_STRING "1.2" CACHE INTERNAL "" FORCE)
endif()
