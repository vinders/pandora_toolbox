if(NOT TARGET glfw3 AND NOT TARGET glfw AND NOT IOS AND NOT ANDROID)
    set(glfw3__FOUND ON)
    set(GLFW3__PATH ${CWORK_MODULE_DESTINATION}/glfw3)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${GLFW3__PATH} OR NOT EXISTS "${GLFW3__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/glfw/glfw.git)
        set(_GIT_EXT_TAG "3.3.2")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/glfw3)
        set(_GIT_EXT_DIR ${GLFW3__PATH})
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
    if(NOT DEFINED GLFW_STATIC OR NOT GLFW_STATIC)
        set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    else()
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    endif()
    
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    
    add_subdirectory(${GLFW3__PATH} ${CMAKE_BINARY_DIR}/glfw3)
    set(glfw3__INCLUDE ${GLFW3__PATH}/include ${GLFW3__PATH}/src)
    
    if(TARGET glfw)
        set_target_properties(glfw PROPERTIES FOLDER libs)
        set(glfw3__LINKED glfw)
    endif()
    if(TARGET glfw3)
        set_target_properties(glfw3 PROPERTIES FOLDER libs)
        set(glfw3__LINKED glfw3)
    endif()
    message("-- Found GLFW3: ${glfw3__LINKED}")
    
    if(DEFINED __OLD_BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS __OLD_BUILD_SHARED_LIBS)
        unset(__OLD_BUILD_SHARED_LIBS)
    else()
        unset(BUILD_SHARED_LIBS)
    endif()
endif()
