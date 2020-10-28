#*******************************************************************************
# Author  :     Romain Vinders
# License :     MIT
# ------------------------------------------------------------------------------
# Video tools, graphics rendering, video filters, ...
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)
include("${CMAKE_CURRENT_SOURCE_DIR}/../_cmake/cwork.cmake")
cwork_set_default_solution("pandora" "${CMAKE_CURRENT_SOURCE_DIR}/..")
cwork_read_version_from_file("${CMAKE_CURRENT_SOURCE_DIR}/../build_version.txt" OFF)
project("${CWORK_SOLUTION_NAME}.video" VERSION ${CWORK_BUILD_VERSION} LANGUAGES C CXX)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Dependencies                                                    │
# └──────────────────────────────────────────────────────────────────┘
if(PYTHON_EXECUTABLE)
    unset(PYTHON_EXECUTABLE CACHE)
endif()
find_package(Python3 COMPONENTS Interpreter Development)

set(_EXTERN_LIBS)
if(CWORK_VIDEO_OPENGL4) # openGL 4.1+/ES3+
    set(_EXTERN_LIBS opengl_libs libglew_static)
endif()
if(CWORK_VIDEO_D3D11) # direct3D 11+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} direct3d11)
endif()
if(CWORK_VIDEO_VULKAN) # vulkan 1.2+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} vulkan_libs)
endif()
if(CWORK_VIDEO_OPENGL4 OR CWORK_VIDEO_VULKAN) # common
    set(_EXTERN_LIBS ${_EXTERN_LIBS} glm)
endif()

if(_EXTERN_LIBS)
    cwork_set_external_libs("public" ${_EXTERN_LIBS})
endif()
cwork_set_internal_libs(system thread hardware)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
set(_INCLUDE_DIR "include")
set(_SOURCE_DIR "src")
set(_TEST_DIR "test")

set(_INCLUDE_FILES)
set(_SOURCE_FILES)
set(_TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_TEST_DIR}/main.cpp)

if(CWORK_VIDEO_OPENGL4)
    set(_INCLUDE_FILES ${_INCLUDE_FILES} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${_INCLUDE_DIR}/video/api/opengl.h
    )
endif()
if(CWORK_VIDEO_VULKAN)
    set(_INCLUDE_FILES ${_INCLUDE_FILES} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${_INCLUDE_DIR}/video/api/vulkan.h
    )
endif()

cwork_set_include_files(${_INCLUDE_FILES})
#cwork_set_source_files(${_SOURCE_FILES})
cwork_set_test_files(${_TEST_FILES})

cwork_create_project("static" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     ${_INCLUDE_DIR} ${_SOURCE_DIR} ${_TEST_DIR})
if((WIN32 OR WIN64 OR _WIN32 OR _WIN64) AND DEFINED _VIDEO_D3D11_NOT_SUPPORTED AND _VIDEO_D3D11_NOT_SUPPORTED)
    message("> Windows SDK not installed or not supported by compiler -> direct3D disabled")
endif()
if(DEFINED _VIDEO_VULKAN_NOT_SUPPORTED AND _VIDEO_VULKAN_NOT_SUPPORTED)
    message("> Vulkan SDK not installed or not supported on current system -> disabled")
endif()

# build shaders from shader modules
add_custom_target(${PROJECT_NAME}.build_shaders ALL
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/build_shaders.py "${CMAKE_CURRENT_SOURCE_DIR}/${_INCLUDE_DIR}/video/shaders" 
                                                                              "${CMAKE_CURRENT_SOURCE_DIR}/_generated" 
                                                                              "${CMAKE_CURRENT_SOURCE_DIR}/${_INCLUDE_DIR}/_embed"
    COMMENT "Building shader files..."
)
set_target_properties(${PROJECT_NAME}.build_shaders PROPERTIES FOLDER ${CWORK_SOLUTION_NAME}/tools)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.build_shaders)
