set(display_io_libs__FOUND ON)

# Android
if(ANDROID)
    if(ANDROID_NDK)
        set(_PATH_PREFIX ${ANDROID_NDK})
    elseif(ANDROID_NDK_HOME)
        set(_PATH_PREFIX ${ANDROID_NDK_HOME})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_HOME})
    else()
        set(_PATH_PREFIX /usr/local/android-ndk)
    endif()
    if(NOT IS_ABSOLUTE ${_PATH_PREFIX})
        get_filename_component(_ABS_PATH_PREFIX "${_PATH_PREFIX}" ABSOLUTE)
        set(_PATH_PREFIX ${_ABS_PATH_PREFIX})
        unset(_ABS_PATH_PREFIX)
    endif()

    include(${CMAKE_CURRENT_LIST_DIR}/arch_detect.cmake)
    set(display_io_libs__LINKED jnigraphics android)
    set(display_io_libs__INCLUDE 
        ${_PATH_PREFIX}/sources/android/native_app_glue 
        ${_PATH_PREFIX}/sources
        ${_PATH_PREFIX}/platforms/${ANDROID_PLATFORM}/arch-${CWORK_ARCH_DETECT}/usr/include
    )
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
