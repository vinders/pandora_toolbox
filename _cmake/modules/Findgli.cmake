if(NOT TARGET gli)
    set(gli__FOUND ON)
    set(GLI__PATH ${CWORK_MODULE_DESTINATION}/gli)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Git submodules                                                  │
    # └──────────────────────────────────────────────────────────────────┘
    #set(CWORK_SUBMODULE_PATH ${GLI__PATH})
    #include(${CMAKE_CURRENT_LIST_DIR}/git_submodules.cmake)
    #unset(CWORK_SUBMODULE_PATH)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${GLI__PATH} OR NOT EXISTS "${GLI__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/g-truc/gli.git)
        set(_GIT_EXT_TAG "779b99ac6656e4d30c3b24e96e0136a59649a869")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/gli)
        set(_GIT_EXT_DIR ${GLI__PATH})
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
    #add_subdirectory(${GLI__PATH} ${CMAKE_BINARY_DIR}/gli)  # useless (header-only)
    set(gli__INCLUDE ${GLI__PATH} ${CMAKE_BINARY_DIR}/gli)
    
    if (TARGET gli)
        set_target_properties(gli PROPERTIES FOLDER libs)
    endif()
endif()
