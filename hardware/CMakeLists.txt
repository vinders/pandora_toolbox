#*******************************************************************************
# Author  :     Romain Vinders
# License :     MIT
# ------------------------------------------------------------------------------
# Hardware and device tools, hardware information, CPU specs, ...
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)
include("${CMAKE_CURRENT_SOURCE_DIR}/../_cmake/cwork.cmake")
cwork_set_default_solution("pandora" "${CMAKE_CURRENT_SOURCE_DIR}/..")
cwork_read_version_from_file("${CMAKE_CURRENT_SOURCE_DIR}/../build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.hardware" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Dependencies                                                    │
# └──────────────────────────────────────────────────────────────────┘
cwork_set_external_libs("private" display_io_libs)
cwork_set_internal_libs(system)
if(MINGW)
    cwork_set_compile_options(-mfxsr -mxsave -mxsaveopt)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  File detection                                                  │
# └──────────────────────────────────────────────────────────────────┘
if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows") # win32 (windows)
    set(_HARDWARE_IGNORED_EXT "_x11.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m" "_andr.cpp")
elseif(IOS) # ios (iOS)
    set(_HARDWARE_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_andr.cpp")
elseif(ANDROID) # android
    set(_HARDWARE_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m")
elseif(APPLE) # cocoa (macOS)
    set(_HARDWARE_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_wln.cpp" "_ios.cpp" "_ios.m" "_andr.cpp")
elseif(CWORK_LINUX_WAYLAND) # wayland (linux)
    set(_HARDWARE_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m" "_andr.cpp")
else() # x11 (linux/unix)
    set(_HARDWARE_IGNORED_EXT "_win32.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m" "_andr.cpp")
endif()

autodetect_source_files("${CMAKE_CURRENT_SOURCE_DIR}/src" "*.[hcm]*")
if(CWORK_AUTODETECTED_FILES)
    set(_SOURCE_FILES)
    set(_OBJC_SOURCE_FILES "")
    foreach(_file ${CWORK_AUTODETECTED_FILES})
        foreach(_ignored ${_HARDWARE_IGNORED_EXT})
            if(_file MATCHES ".*${_ignored}$")
                set(_IGNORED_FILE ON)
                break()
            endif()
        endforeach()
        if(NOT _IGNORED_FILE)
            set(_SOURCE_FILES ${_SOURCE_FILES} ${_file})
            if((IOS OR APPLE) AND _file MATCHES ".*\\.m$")
                set(_OBJC_SOURCE_FILES ${_OBJC_SOURCE_FILES} ${_file})
            endif()
        else()
            unset(_IGNORED_FILE)
        endif()
    endforeach()
    unset(CWORK_AUTODETECTED_FILES)
    
    cwork_set_source_files(${_SOURCE_FILES})
    if((IOS OR APPLE) AND NOT _OBJC_SOURCE_FILES STREQUAL "")
        cwork_set_source_language(C, ${_OBJC_SOURCE_FILES})
    endif()
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cwork_create_project("static" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     "include" "src" "test" "tools/hardware_analyzer")
