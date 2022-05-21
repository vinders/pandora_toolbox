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

set(CWORK_SHADER_COMPILERS ON CACHE INTERNAL "" FORCE) # shaders compiled at runtime (for big projects, disable it to precompile them)
set(CWORK_GEOM_STAGE_OFF ON CACHE INTERNAL "" FORCE)   # no geometry shader used by project -> disable unused features
set(CWORK_TESS_STAGE_OFF ON CACHE INTERNAL "" FORCE)   # no tessellation shader used by project -> disable unused features
include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
include("${PANDORA_TOOLBOX_DIR}/Version.cmake") # version read from file

# --> solution params (replace "example" with your solution name)
project("example" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)
cwork_create_solution("${CMAKE_CURRENT_SOURCE_DIR}" "${PANDORA_TOOLBOX_DIR}/_cmake")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
# --> uncomment if project is placed in separate CMakeLists file (in child directory)
#cmake_minimum_required(VERSION 3.14)
#include("${PANDORA_TOOLBOX_DIR}/_cmake/cwork.cmake")
#include("${PANDORA_TOOLBOX_DIR}/Version.cmake")

# --> project params (replace with your project name)
project("${CWORK_SOLUTION_NAME}.simple_renderer_3D_cube" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

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
cwork_set_resource_dir("${CWORK_SOLUTION_PATH}/resources") # resource files: sprites/textures (+ icons/cursors on linux/mac)

# --> detect shader sources
if(CWORK_VIDEO_D3D11)
    autodetect_source_files("${CWORK_SOLUTION_PATH}/shaders/d3d11" "*.hlsl*")
    foreach(_shaderFile IN LISTS CWORK_AUTODETECTED_FILES)
        set_source_files_properties(${_shaderFile} PROPERTIES VS_TOOL_OVERRIDE "None") # no IDE shader compilation
        # to analyze/debug shaders in Visual Studio, comment line above + set properties for each shader file (shader type + model 5_0)
    endforeach()
elseif(CWORK_VIDEO_VULKAN)
    autodetect_source_files("${CWORK_SOLUTION_PATH}/shaders/vulkan" "*")
else()
    autodetect_source_files("${CWORK_SOLUTION_PATH}/shaders/openGL4" "*")
endif()
cwork_set_custom_files(${CWORK_AUTODETECTED_FILES})
unset(CWORK_AUTODETECTED_FILES)

if (NOT CWORK_SHADER_COMPILERS)
    add_definitions(-D_P_SHADER_PRECOMPILED=1) # inform code to use pre-compiled shaders
endif()

# --> configure project & sources
cwork_create_project("executable" "${PANDORA_TOOLBOX_DIR}/_cmake" 
                     "${PANDORA_TOOLBOX_DIR}/_cmake/modules"
                     "include" "src" "test")

# --> shader build
#     -> dependency to 
#     -> enable CWORK_SHADER_COMPILERS if you only use text shaders (compiled at runtime)
#     -> disable CWORK_SHADER_COMPILERS to pre-compile shaders
if (NOT CWORK_SHADER_COMPILERS)
    # --> custom target to pre-compile shader sources (only useful if shaders aren't compiled at runtime)
    set(_BUILD_SHADERS_COMMAND "${PYTHON_EXECUTABLE}" "${PANDORA_TOOLBOX_DIR}/video/build_shaders.py"
                                                      "${CWORK_SOLUTION_PATH}/shaders"         # source shaders
                                                      "${PROJECT_BINARY_DIR}/shaders"          # minified sources + compiled binaries
                                                      "${PROJECT_BINARY_DIR}/include/shaders") # stringified version (usable in CPP files)
    add_custom_target(${CWORK_SOLUTION_NAME}.build_shaders ALL
                      COMMAND ${_BUILD_SHADERS_COMMAND} COMMENT "Building shader files...")
    set_target_properties(${CWORK_SOLUTION_NAME}.build_shaders PROPERTIES FOLDER ${CWORK_SOLUTION_NAME})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${_BUILD_SHADERS_COMMAND}) # force compilation when building project
else()
    # --> custom target to deploy shader sources (for compilation at runtime)
    set(_DEPLOY_SHADERS_COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CWORK_SOLUTION_PATH}/shaders/" # source shaders
                                                                     "${PROJECT_BINARY_DIR}/shaders/") # destination directory
    add_custom_target(${CWORK_SOLUTION_NAME}.deploy_shaders ALL
                      COMMAND ${_DEPLOY_SHADERS_COMMAND} COMMENT "Copying shader files...")
    set_target_properties(${CWORK_SOLUTION_NAME}.deploy_shaders PROPERTIES FOLDER ${CWORK_SOLUTION_NAME})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${_DEPLOY_SHADERS_COMMAND}) # force deployment when building project
endif()
