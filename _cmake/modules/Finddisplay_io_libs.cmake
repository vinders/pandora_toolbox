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

set(display_io_libs__FOUND ON)

# Android
if(ANDROID)
    if(ANDROID_NDK)
        set(_PATH_PREFIX ${ANDROID_NDK})
    elseif(ANDROID_NDK_ROOT)
        set(_PATH_PREFIX ${ANDROID_NDK_ROOT})
    elseif(DEFINED ENV{ANDROID_NDK_ROOT})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_ROOT})
    elseif(ANDROID_NDK_HOME)
        set(_PATH_PREFIX ${ANDROID_NDK_HOME})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_HOME})
    else()
        set(_PATH_PREFIX /usr/local/android-ndk)
    endif()
    
    include(${CMAKE_CURRENT_LIST_DIR}/path_utils.cmake)

    cwork_find_arch_label()
    cwork_unknown_path_to_absolute(${_PATH_PREFIX} "/sources/android/native_app_glue/android_native_app_glue.c")
    set(display_io_libs__LINKED jnigraphics android log)
    set(display_io_libs__INCLUDE 
        ${CWORK_VALID_ABSOLUTE_PATH}/sources/android/native_app_glue 
        ${CWORK_VALID_ABSOLUTE_PATH}/sources
        ${CWORK_VALID_ABSOLUTE_PATH}/platforms/${ANDROID_PLATFORM}/arch-${CWORK_ARCH_LABEL}/usr/include
    )
    unset(CWORK_VALID_ABSOLUTE_PATH)
    unset(_PATH_PREFIX)

# iOS
elseif(IOS)
    set(_EXTERNAL_FRAMEWORKS
        Foundation
        UIKit
    )
    if(CWORK_EXTERN_FRAMEWORKS)
        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} ${_EXTERNAL_FRAMEWORKS})
    else()
        set(CWORK_EXTERN_FRAMEWORKS ${_EXTERNAL_FRAMEWORKS})
    endif()
    unset(_EXTERNAL_FRAMEWORKS)

# MacOS
elseif(APPLE)
    set(_EXTERNAL_FRAMEWORKS
        Cocoa
        IOKit
        CoreFoundation
    )
    if(CWORK_EXTERN_FRAMEWORKS)
        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} ${_EXTERNAL_FRAMEWORKS})
    else()
        set(CWORK_EXTERN_FRAMEWORKS ${_EXTERNAL_FRAMEWORKS})
    endif()
    unset(_EXTERNAL_FRAMEWORKS)

# Windows
elseif(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CWORK_WINDOWS_VERSION AND (CWORK_WINDOWS_VERSION STREQUAL "10" OR CWORK_WINDOWS_VERSION STREQUAL "8"))
        set(display_io_libs__LINKED user32 shcore)
    else()
        set(display_io_libs__LINKED user32)
    endif()   

# Linux
elseif(CWORK_LINUX_WAYLAND)
    message("-- checking wayland packages... --")
    message("note: libwayland-dev, wayland-protocols, libxkbcommon-dev must be installed")
    include(GNUInstallDirs)
    include(FindPkgConfig)
    pkg_check_modules(Wayland REQUIRED
        wayland-client>=0.2.7
        xkbcommon
    )
    include(CheckIncludeFiles)
    include(CheckFunctionExists)
    check_include_files(xkbcommon/xkbcommon-compose.h _P_ENABLE_LINUX_XKBCOMMON_COMPOSE)
    check_function_exists(memfd_create _P_ENABLE_LINUX_MEMFD_CREATE)
    
    set(_LINUX_WAYLAND_LINKED ON)
    set(display_io_libs__INCLUDE ${Wayland_INCLUDE_DIRS})
    set(display_io_libs__LINKED ${Wayland_LINK_LIBRARIES} ${CMAKE_DL_LIBS})
else()
    set(display_io_libs__LINKED X11 ${CMAKE_DL_LIBS})
endif()

if(CWORK_EXTERN_FRAMEWORKS AND NOT CWORK_EXTERN_FRAMEWORKS_SCOPE)
    if(CWORK_EXTERN_LIBS_SCOPE)
        set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${CWORK_EXTERN_LIBS_SCOPE})
    else()
        set(CWORK_EXTERN_FRAMEWORKS_SCOPE PUBLIC)
    endif()
endif()
