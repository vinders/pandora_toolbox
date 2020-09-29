# ┌──────────────────────────────────────────────────────────────────┐
# │  Manage git submodules                                           │
# └──────────────────────────────────────────────────────────────────┘
#params: - CWORK_SOLUTION_PATH
#        - CWORK_SUBMODULE_PATH
if(NOT EXISTS "${CWORK_SUBMODULE_PATH}/CMakeLists.txt")
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CWORK_SOLUTION_PATH}/.git")
        message(STATUS "> git submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote
                        WORKING_DIRECTORY ${CWORK_SOLUTION_PATH}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
    
    if(NOT EXISTS "${CWORK_SUBMODULE_PATH}/CMakeLists.txt")
        message(FATAL_ERROR "Error: Submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Cmake will exit.")
    endif()
endif()
