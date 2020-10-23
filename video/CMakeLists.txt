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
# │  Options                                                         │
# └──────────────────────────────────────────────────────────────────┘
if(NOT DEFINED USE_PANDORA_VIDEO_OPENGL)
    option(USE_PANDORA_VIDEO_OPENGL "pandora.video: enable OpenGL4/ES features" ON)
endif()

if((NOT WIN32 AND NOT WIN64 AND NOT _WIN32 AND NOT _WIN64) OR NOT MSVC)
    set(USE_PANDORA_VIDEO_D3D11 OFF)
    set(_DIRECT3D_NOT_SUPPORTED_FLAG ON)
elseif(NOT DEFINED USE_PANDORA_VIDEO_D3D11)
    option(USE_PANDORA_VIDEO_D3D11 "pandora.video: enable Direct3D 11 features" ON)
endif()

if(IOS OR ("$ENV{VULKAN_SDK}" STREQUAL "" AND NOT ANDROID))
    set(USE_PANDORA_VIDEO_VULKAN OFF)
    set(_VULKAN_NOT_SUPPORTED_FLAG ON)
elseif(NOT DEFINED USE_PANDORA_VIDEO_VULKAN)
    option(USE_PANDORA_VIDEO_VULKAN "pandora.video: enable Vulkan1.2 features" ON)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Dependencies                                                    │
# └──────────────────────────────────────────────────────────────────┘
set(_EXTERN_LIBS)
if(USE_PANDORA_VIDEO_OPENGL) # openGL 4.1+/ES
    set(_EXTERN_LIBS opengl_libs libglew_static)
endif()
if(USE_PANDORA_VIDEO_D3D11) # direct3D 11+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} direct3d11)
endif()
if(USE_PANDORA_VIDEO_VULKAN) # vulkan 1.2+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} vulkan_libs)
endif()
if(USE_PANDORA_VIDEO_OPENGL OR USE_PANDORA_VIDEO_VULKAN) # common
    set(_EXTERN_LIBS ${_EXTERN_LIBS} glm)
endif()

if(_EXTERN_LIBS)
    cwork_set_external_libs("private" ${_EXTERN_LIBS})
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

if(USE_PANDORA_VIDEO_OPENGL)
    set(_INCLUDE_FILES ${_INCLUDE_FILES} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${_INCLUDE_DIR}/video/api/opengl.h
    )
endif()
if(USE_PANDORA_VIDEO_VULKAN)
    set(_INCLUDE_FILES ${_INCLUDE_FILES} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${_INCLUDE_DIR}/video/api/vulkan.h
    )
endif()

cwork_set_include_files(${_INCLUDE_FILES})
#cwork_set_source_files(${_SOURCE_FILES})
cwork_set_test_files(${_TEST_FILES})

cwork_create_project("static" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     ${_INCLUDE_DIR} ${_SOURCE_DIR} ${_TEST_DIR})
if((WIN32 OR WIN64 OR _WIN32 OR _WIN64) AND DEFINED _DIRECT3D_NOT_SUPPORTED_FLAG AND _DIRECT3D_NOT_SUPPORTED_FLAG)
    message("> Windows SDK not installed or not supported by compiler -> direct3D disabled")
endif()
if(DEFINED _VULKAN_NOT_SUPPORTED_FLAG AND _VULKAN_NOT_SUPPORTED_FLAG)
    message("> Vulkan SDK not installed or not supported on current system -> disabled")
endif()
