# MIT License
# Copyright (c) 2023 Romain Vinders

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
# 
# Source: github.com/vinders/pandora_toolbox

if(NOT TARGET freetype)
    set(freetype__FOUND ON)
    set(FREETYPE__PATH ${CWORK_MODULE_DESTINATION}/freetype)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${FREETYPE__PATH} OR NOT EXISTS "${FREETYPE__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/freetype/freetype.git)
        set(_GIT_EXT_TAG "VER-2-13-0")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/freetype)
        set(_GIT_EXT_DIR ${FREETYPE__PATH})
        configure_file("${CMAKE_CURRENT_LIST_DIR}/git_external.cmake" "${_GIT_EXT_CACHE}/CMakeLists.txt")
        execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" ${_GIT_EXT_CACHE} WORKING_DIRECTORY ${_GIT_EXT_CACHE})
        execute_process(COMMAND "${CMAKE_COMMAND}" --build ${_GIT_EXT_CACHE} WORKING_DIRECTORY ${_GIT_EXT_CACHE})
        unset(_GIT_EXT_REPOSITORY)
        unset(_GIT_EXT_TAG)
        unset(_GIT_EXT_CACHE)
        unset(_GIT_EXT_DIR)
    endif()

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Include project                                                 │
    # └──────────────────────────────────────────────────────────────────┘
    if(DEFINED BUILD_SHARED_LIBS)
        set(__OLD_BUILD_SHARED_LIBS BUILD_SHARED_LIBS)
    endif()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SKIP_INSTALL_ALL ON)
    
    add_subdirectory(${FREETYPE__PATH} ${CMAKE_BINARY_DIR}/freetype)
    set(freetype_INCLUDE_DIRS ${FREETYPE__PATH}/include ${FREETYPE__PATH}/src)
    set(freetype_INCLUDE_DIR ${FREETYPE__PATH}/include ${FREETYPE__PATH}/src)
    set(freetype__INCLUDE   ${FREETYPE__PATH}/include ${FREETYPE__PATH}/src)
    
    if(TARGET freetype)
        set_target_properties(freetype PROPERTIES FOLDER libs)
        set(freetype_LIBRARIES freetype)
        set(freetype__LINKED freetype)
        message("-- Found Freetype: ${freetype__LINKED}")
    endif()
    
    if(DEFINED __OLD_BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS __OLD_BUILD_SHARED_LIBS)
        unset(__OLD_BUILD_SHARED_LIBS)
    else()
        unset(BUILD_SHARED_LIBS)
    endif()
    unset(SKIP_INSTALL_ALL)
endif()
