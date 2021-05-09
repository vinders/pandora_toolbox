#*******************************************************************************
# Example - window creation + main loop (solution + project)
#*******************************************************************************
# ┌──────────────────────────────────────────────────────────────────┐
# │  Solution settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.14)

# --> replace with your path to toolbox (usually in subdirectory or same parent directory)
set(PANDORA_TOOLBOX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
cwork_read_version_from_file("${PANDORA_TOOLBOX_DIR}/build_version.txt" ON)

project("example" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)
cwork_create_solution("${CMAKE_CURRENT_SOURCE_DIR}" "${PANDORA_TOOLBOX_DIR}/_cmake")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
# --> uncomment if project is placed in separate CMakeLists file
#cmake_minimum_required(VERSION 3.14)
#include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
#cwork_read_version_from_file("${PANDORA_TOOLBOX_DIR}/build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.window_main" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

cwork_set_custom_libs("${PANDORA_TOOLBOX_DIR}" pandora ON OFF
    system
    thread
    hardware
    video
)

if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    cwork_set_embedded_resource_dir("${PANDORA_TOOLBOX_DIR}/_img/test_win32")
else()
    cwork_set_resource_dir("${PANDORA_TOOLBOX_DIR}/_img/test_common")
endif()

cwork_create_project("executable" "${PANDORA_TOOLBOX_DIR}/_cmake" 
                     "${PANDORA_TOOLBOX_DIR}/_cmake/modules"
                     "include" "src" "test")
