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

if((WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR MSVC) AND NOT TARGET DirectXTK)
    set(directxtk__FOUND ON)
    set(DIRECTXTK__PATH ${CWORK_MODULE_DESTINATION}/DirectXTK)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${DIRECTXTK__PATH} OR NOT EXISTS "${DIRECTXTK__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/Microsoft/DirectXTK.git)
        set(_GIT_EXT_TAG "sept2020")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/DirectXTK)
        set(_GIT_EXT_DIR ${DIRECTXTK__PATH})
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
    
    add_subdirectory(${DIRECTXTK__PATH} ${CMAKE_BINARY_DIR}/DirectXTK)
    set(directxtk__INCLUDE ${DIRECTXTK__PATH}/Inc)
    
    if(TARGET DirectXTK)
        set_target_properties(DirectXTK PROPERTIES FOLDER libs)
    endif()
    if(TARGET xwbtool)
        set_target_properties(xwbtool PROPERTIES FOLDER libs)
        set_target_properties(xwbtool PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
    
    set(directxtk__LINKED DirectXTK)
    message("-- Found DirectXTK: ${directxtk__LINKED}")
    
    if(DEFINED __OLD_BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS __OLD_BUILD_SHARED_LIBS)
        unset(__OLD_BUILD_SHARED_LIBS)
    else()
        unset(BUILD_SHARED_LIBS)
    endif()
endif()
