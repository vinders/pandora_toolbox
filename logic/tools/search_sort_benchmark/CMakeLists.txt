#*******************************************************************************
# Author  :     Romain Vinders
# License :     MIT
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../_cmake/cwork.cmake")
cwork_set_default_solution("pandora" "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
cwork_read_version_from_file("${CMAKE_CURRENT_SOURCE_DIR}/../../../build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.search_sort_benchmark" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Dependencies                                                    │
# └──────────────────────────────────────────────────────────────────┘
if(ANDROID)
    cwork_set_external_libs("private" android_glue)
    cwork_set_internal_libs(system logic)
else()
    cwork_set_internal_libs(logic)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cwork_set_subproject_type("tools")
cwork_create_project("console" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     "include" "src" "test")
