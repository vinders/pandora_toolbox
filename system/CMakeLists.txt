#*******************************************************************************
# Author  :     Romain Vinders
# License :     MIT
# ------------------------------------------------------------------------------
# System tools, system APIs, system information, intrinsics, ...
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)
include("${CMAKE_CURRENT_SOURCE_DIR}/../_cmake/cwork.cmake")
cwork_set_default_solution("pandora" "${CMAKE_CURRENT_SOURCE_DIR}/..")
cwork_read_version_from_file("${CMAKE_CURRENT_SOURCE_DIR}/../build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.system" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

# ┌──────────────────────────────────────────────────────────────────┐
# │  System APIs                                                     │
# └──────────────────────────────────────────────────────────────────┘
if (ANDROID AND NOT WIN32 AND NOT APPLE)
    cwork_set_external_libs("private" android_libs)
    set(_ANDROID_SOURCE_FILES ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
    
    autodetect_source_files("${CMAKE_CURRENT_SOURCE_DIR}/src" "*.[hcm]*")
    if(CWORK_AUTODETECTED_FILES)
        set(_ANDROID_SOURCE_FILES ${_ANDROID_SOURCE_FILES} ${CWORK_AUTODETECTED_FILES})
        unset(CWORK_AUTODETECTED_FILES)
    endif()
    
    cwork_set_source_files(${_ANDROID_SOURCE_FILES})
endif()


# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cwork_create_project("static" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     "include" "src" "test")
