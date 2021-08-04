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
        if(NOT CWORK_SUBMODULE_PERMISSIVE AND NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
    
    if(NOT CWORK_SUBMODULE_PERMISSIVE AND NOT EXISTS "${CWORK_SUBMODULE_PATH}/CMakeLists.txt")
        message(FATAL_ERROR "Error: Submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Cmake will exit.")
    endif()
endif()
