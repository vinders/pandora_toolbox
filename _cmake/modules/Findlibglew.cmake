if(NOT TARGET libglew AND NOT TARGET libglew_static AND NOT IOS AND NOT APPLE AND NOT ANDROID AND NOT CWORK_LINUX_WAYLAND)
    set(libglew__FOUND ON)
    set(LIBGLEW__PATH ${CWORK_MODULE_DESTINATION}/glew)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Git submodules                                                  │
    # └──────────────────────────────────────────────────────────────────┘
    #set(CWORK_SUBMODULE_PATH ${LIBGLEW__PATH})
    #include(${CMAKE_CURRENT_LIST_DIR}/git_submodules.cmake)
    #unset(CWORK_SUBMODULE_PATH)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${LIBGLEW__PATH} OR NOT EXISTS "${LIBGLEW__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/Perlmint/glew-cmake.git)
        set(_GIT_EXT_TAG "glew-cmake-2.2.0")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/glew)
        set(_GIT_EXT_DIR ${LIBGLEW__PATH})
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
    if(NOT DEFINED GLEW_STATIC OR NOT GLEW_STATIC)
        set(glew-cmake_BUILD_SHARED ON CACHE STRING "" FORCE)
    else()
        add_definitions(-DGLEW_STATIC)
        set(glew-cmake_BUILD_SHARED OFF CACHE STRING "" FORCE)
    endif()
    
    add_subdirectory(${LIBGLEW__PATH} ${CMAKE_BINARY_DIR}/libglew)
    set(libglew__INCLUDE ${LIBGLEW__PATH}/include ${LIBGLEW__PATH}/src)
    
    if(TARGET libglew)
        set_target_properties(libglew PROPERTIES FOLDER libs)
    endif()
    if(TARGET libglew_shared)
        set_target_properties(libglew_shared PROPERTIES FOLDER libs)
    endif()
    if(TARGET libglewmx_shared)
        set_target_properties(libglewmx_shared PROPERTIES FOLDER libs)
    endif()
    if(TARGET libglew_static)
        set_target_properties(libglew_static PROPERTIES FOLDER libs)
    endif()
    if(TARGET libglewmx_static)
        set_target_properties(libglewmx_static PROPERTIES FOLDER libs)
    endif()
    
    if(NOT DEFINED GLEW_STATIC OR NOT GLEW_STATIC)
        set(libglew__LINKED libglew libglew_shared)
    else()
        set(libglew__LINKED libglew_static)
    endif()
    message("-- Found GLEW: ${libglew__LINKED}")
endif()
