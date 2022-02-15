#*******************************************************************************
# Pandora Toolbox examples by Romain Vinders

# To the extent possible under law, the person who associated CC0 with
# Pandora Toolbox examples has waived all copyright and related or neighboring
# rights to Pandora Toolbox examples.
# CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
# ------------------------------------------------------------------------------
# Example - window creation + main loop (solution + project)
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)

# --> set relative path to pandora_toolbox sources (usually in subdirectory or same parent directory)
set(PANDORA_TOOLBOX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# --> this solution doesn't use any rendering API -> disable them
set(CWORK_VIDEO_D3D11 OFF CACHE INTERNAL "" FORCE)
set(CWORK_VIDEO_VULKAN OFF CACHE INTERNAL "" FORCE)
set(CWORK_VIDEO_OPENGL4 OFF CACHE INTERNAL "" FORCE)
set(CWORK_SHADER_COMPILERS OFF CACHE INTERNAL "" FORCE)

include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
cwork_read_version_from_file("${PANDORA_TOOLBOX_DIR}/build_version.txt" ON)

# -- Solution settings (version read from file) --
project("example" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)
cwork_create_solution("${CMAKE_CURRENT_SOURCE_DIR}" "${PANDORA_TOOLBOX_DIR}/_cmake")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
# --> uncomment if project is placed in separate CMakeLists file
#cmake_minimum_required(VERSION 3.14)
#include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
#cwork_read_version_from_file("${PANDORA_TOOLBOX_DIR}/build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.base_window" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

cwork_set_custom_libs("${PANDORA_TOOLBOX_DIR}" pandora ON OFF
    system
    hardware
    video
)

# -- project resources (icon + cursor)
if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    cwork_set_embedded_resource_dir("${PANDORA_TOOLBOX_DIR}/_img/test_win32")
else()
    cwork_set_resource_dir("${PANDORA_TOOLBOX_DIR}/_img/test_common")
endif()

cwork_create_project("executable" "${PANDORA_TOOLBOX_DIR}/_cmake" 
                     "${PANDORA_TOOLBOX_DIR}/_cmake/modules"
                     "include" "src" "test")
