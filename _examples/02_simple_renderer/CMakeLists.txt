#*******************************************************************************
# Pandora Toolbox examples by Romain Vinders

# To the extent possible under law, the person who associated CC0 with
# Pandora Toolbox examples has waived all copyright and related or neighboring
# rights to Pandora Toolbox examples.
# CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
# ------------------------------------------------------------------------------
# Example - window creation + simple 3D renderer (solution + project)
#*******************************************************************************
# ┌──────────────────────────────────────────────────────────────────┐
# │  Solution settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.14)

# --> replace with your path to toolbox (usually in subdirectory or same parent directory)
# --> note: if the Pandora toolbox libs are compiled and installed in a local repository,
#           they can be referenced from Cmake files that do not use CWork.
set(PANDORA_TOOLBOX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
cwork_read_version_from_file("${PANDORA_TOOLBOX_DIR}/build_version.txt" ON)

# --> solution params (replace "example" with your solution name)
project("example" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)
cwork_create_solution("${CMAKE_CURRENT_SOURCE_DIR}" "${PANDORA_TOOLBOX_DIR}/_cmake")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
# --> uncomment if project is placed in separate CMakeLists file (in child directory)
#cmake_minimum_required(VERSION 3.14)
#include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
#cwork_read_version_from_file("${PANDORA_TOOLBOX_DIR}/build_version.txt" OFF)

# --> project params (replace "window_renderer" with your project name)
project("${CWORK_SOLUTION_NAME}.simple_renderer" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

# --> internal toolbox dependencies
# --> try to keep order of (sub)dependencies when linking libs, to improve build time (and avoid Cmake failures)
cwork_set_custom_libs("${PANDORA_TOOLBOX_DIR}" pandora ON OFF # ON OFF for solution, OFF OFF for projects
    system
    thread
    time
    memory
    pattern
    io
    hardware
    video
)

# --> extern libs
if(CWORK_VIDEO_D3D11)
    cwork_set_external_libs("private" directxtk)
else()
    cwork_set_external_libs("private" glm gli)
endif()

# --> replace with your actual icons/resources/...
if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    cwork_set_embedded_resource_dir("${PANDORA_TOOLBOX_DIR}/_img/test_win32") # embedded: icons, cursors...
endif()
cwork_set_resource_dir("${CWORK_SOLUTION_PATH}/resources") # resource files: shaders, textures (+ icons/cursors on linux/mac)

# --> detect shader sources
if(CWORK_VIDEO_D3D11)
    autodetect_source_files("${CWORK_SOLUTION_PATH}/resources/shaders" "*.hlsl*")
    foreach(_shaderFile IN LISTS CWORK_AUTODETECTED_FILES)
        set_source_files_properties(${_shaderFile} PROPERTIES VS_TOOL_OVERRIDE "None") # no IDE shader compilation
        # to analyze/debug shaders in Visual Studio, comment line above + set properties for each shader file (shader type + model 5_0)
    endforeach()
    cwork_set_custom_files(${CWORK_AUTODETECTED_FILES})
    unset(CWORK_AUTODETECTED_FILES)
else()
    autodetect_source_files("${CWORK_SOLUTION_PATH}/resources/shaders" "*.glsl*")
    cwork_set_custom_files(${CWORK_AUTODETECTED_FILES})
    unset(CWORK_AUTODETECTED_FILES)
endif()

# --> configure project & sources
cwork_create_project("executable" "${PANDORA_TOOLBOX_DIR}/_cmake" 
                     "${PANDORA_TOOLBOX_DIR}/_cmake/modules"
                     "include" "src" "test")
