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
