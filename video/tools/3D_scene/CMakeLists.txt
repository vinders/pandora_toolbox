#*******************************************************************************
# Author  :     Romain Vinders
# License :     MIT
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../_cmake/cwork.cmake")
cwork_set_default_solution("pandora" "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
cwork_read_version_from_file("${CMAKE_CURRENT_SOURCE_DIR}/../../../build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.3D_scene" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Dependencies                                                    │
# └──────────────────────────────────────────────────────────────────┘
cwork_set_internal_libs(video)
if(ANDROID)
    cwork_set_external_libs("private" android_glue)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    cwork_set_embedded_resource_dir("${CWORK_SOLUTION_PATH}/_img/test_win32")
else()
    cwork_set_resource_dir("${CWORK_SOLUTION_PATH}/_img/test_common")
endif()

cwork_set_subproject_type("tools")
cwork_create_project("executable" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     "include" "src" "test")
