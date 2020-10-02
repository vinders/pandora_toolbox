if(NOT TARGET glm)
    set(glm__FOUND ON)
    set(GLM__PATH ${CWORK_MODULE_DESTINATION}/glm)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Git submodules                                                  │
    # └──────────────────────────────────────────────────────────────────┘
    #set(CWORK_SUBMODULE_PATH ${GLM__PATH})
    #include(${CMAKE_CURRENT_LIST_DIR}/git_submodules.cmake)
    #unset(CWORK_SUBMODULE_PATH)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${GLM__PATH} OR NOT EXISTS "${GLM__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/g-truc/glm.git)
        set(_GIT_EXT_TAG "0.9.9.8")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/glm)
        set(_GIT_EXT_DIR ${GLM__PATH})
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
    #add_subdirectory(${GLM__PATH} ${CMAKE_BINARY_DIR}/glm)  # useless (header-only)
    include_directories(${GLM__PATH} ${CMAKE_BINARY_DIR}/glm)
    
    if (TARGET glm)
        set_target_properties(glm PROPERTIES FOLDER libs)
    endif()
endif()
