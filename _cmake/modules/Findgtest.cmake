if(NOT TARGET gtest AND NOT IOS AND NOT ANDROID)
    set(gtest__FOUND ON)
    set(GTEST__PATH ${CWORK_MODULE_DESTINATION}/gtest)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Git submodules                                                  │
    # └──────────────────────────────────────────────────────────────────┘
    set(CWORK_SUBMODULE_PERMISSIVE ON)
    set(CWORK_SUBMODULE_PATH ${GTEST__PATH})
    include(${CMAKE_CURRENT_LIST_DIR}/git_submodules.cmake)
    unset(CWORK_SUBMODULE_PATH)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Manual project download                                         │
    # └──────────────────────────────────────────────────────────────────┘
    if(NOT EXISTS ${GTEST__PATH} OR NOT EXISTS "${GTEST__PATH}/CMakeLists.txt")
        set(_GIT_EXT_REPOSITORY https://github.com/google/googletest.git)
        set(_GIT_EXT_TAG "release-1.10.0")
        set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/gtest)
        set(_GIT_EXT_DIR ${GTEST__PATH})
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
    add_subdirectory(${GTEST__PATH} ${CMAKE_BINARY_DIR}/gtest)
    set(gtest__INCLUDE ${GTEST__PATH}/include ${GTEST__PATH}/src)
    
    if(TARGET gtest)
        set_target_properties(gtest PROPERTIES FOLDER "libs/test")
    endif()
    if(TARGET gtest_main)
        set_target_properties(gtest_main PROPERTIES FOLDER "libs/test")
    endif()
    if(TARGET gmock)
        set_target_properties(gmock PROPERTIES FOLDER "libs/test")
    endif()
    if(TARGET gmock_main)
        set_target_properties(gmock_main PROPERTIES FOLDER "libs/test")
    endif()
    
    set(gtest__LINKED gtest gtest_main)
    set(gmock__LINKED gmock gmock_main)
endif()
