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

if(NOT TARGET SDL2 AND NOT TARGET SDL2-static)
    set(sdl2__FOUND ON)
    set(SDL2__PATH ${CWORK_MODULE_DESTINATION}/sdl2)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${SDL2__PATH} OR NOT EXISTS "${SDL2__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/spurious/SDL-mirror.git)
        set(_GIT_EXT_TAG "release-2.0.12")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/sdl2)
        set(_GIT_EXT_DIR ${SDL2__PATH})
        configure_file("${CMAKE_CURRENT_LIST_DIR}/git_external.cmake" "${_GIT_EXT_CACHE}/CMakeLists.txt")
        execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" ${_GIT_EXT_CACHE} WORKING_DIRECTORY ${_GIT_EXT_CACHE})
        execute_process(COMMAND "${CMAKE_COMMAND}" --build ${_GIT_EXT_CACHE} WORKING_DIRECTORY ${_GIT_EXT_CACHE})
        configure_file("${CMAKE_CURRENT_LIST_DIR}/sdl2_config.cmake" "${SDL2__PATH}/sdl2-config.cmake")
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
    set(SDL2_DIR ${SDL2__PATH} CACHE PATH "SDL2 directory")
    if(NOT DEFINED SDL_STATIC_ENABLED_BY_DEFAULT OR NOT SDL_STATIC_ENABLED_BY_DEFAULT)
        set(SDL_STATIC_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
        set(SDL_SHARED_ENABLED_BY_DEFAULT ON CACHE BOOL "")
        set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    else()
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    endif()
    
    add_subdirectory(${SDL2__PATH} ${CMAKE_BINARY_DIR}/sdl2)
    set(sdl2__INCLUDE ${SDL2_INCLUDE_DIRS})
    
    if(TARGET SDL2)
        set_target_properties(SDL2 PROPERTIES FOLDER libs)
    endif()
    if(TARGET SDL2-static)
        set_target_properties(SDL2-static PROPERTIES FOLDER libs)
    endif()
    if(TARGET SDL2main)
        set_target_properties(SDL2main PROPERTIES FOLDER libs)
    endif()
    if(TARGET uninstall)
        set_target_properties(uninstall PROPERTIES FOLDER "libs/tools")
    endif()
    
    set(sdl2__LINKED ${SDL2_LIBRARIES})
    message("-- Found SDL2: ${sdl2__LINKED}")
    
    if(DEFINED __OLD_BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS __OLD_BUILD_SHARED_LIBS)
        unset(__OLD_BUILD_SHARED_LIBS)
    else()
        unset(BUILD_SHARED_LIBS)
    endif()
endif()
