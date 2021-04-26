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

set(_EXTERN_LIBS window_libs)
if(CWORK_VIDEO_OPENGL4) # openGL 4.1+/ES3+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" opengl_libs libglew_static)
endif()
if(CWORK_VIDEO_D3D11) # direct3D 11+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" direct3d11)
endif()
if(CWORK_VIDEO_VULKAN) # vulkan 1.2+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" vulkan_libs)
endif()
if(CWORK_VIDEO_OPENGL4 OR CWORK_VIDEO_VULKAN) # common
    set(_EXTERN_LIBS ${_EXTERN_LIBS} glm)
endif()

if(_EXTERN_LIBS)
    cwork_set_external_libs("private" ${_EXTERN_LIBS})
endif()
cwork_set_internal_libs(system thread hardware)


# ┌──────────────────────────────────────────────────────────────────┐
# │  Helpers                                                         │
# └──────────────────────────────────────────────────────────────────┘
set(_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/video")
set(_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/shaders")
set(_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")

# autodetect files related to a specific category
macro(pandora_video_autodetect_files category_id)
    message("> auto-detection of ${category_id} files...")
    autodetect_source_files("${_INCLUDE_DIR}/${category_id}" "*.h*")
    if(CWORK_AUTODETECTED_FILES)
        set(_INCLUDE_FILES ${_INCLUDE_FILES} ${CWORK_AUTODETECTED_FILES})
    endif()
    unset(CWORK_AUTODETECTED_FILES)
    
    autodetect_source_files("${_SOURCE_DIR}/${category_id}" "*.c*")
    if(CWORK_AUTODETECTED_FILES)
        if(_SOURCE_FILES)
            set(_SOURCE_FILES ${_SOURCE_FILES} ${CWORK_AUTODETECTED_FILES})
        else()
            set(_SOURCE_FILES ${CWORK_AUTODETECTED_FILES})
        endif()
    endif()
    unset(CWORK_AUTODETECTED_FILES)
    
    autodetect_source_files("${_SHADER_DIR}/${category_id}" "*.*")
    if(CWORK_AUTODETECTED_FILES)
        set(_SHADER_FILES ${_SHADER_FILES} ${CWORK_AUTODETECTED_FILES})
    endif()
    unset(CWORK_AUTODETECTED_FILES)
    
    autodetect_source_files("${_TEST_DIR}/${category_id}" "*.[hc]*")
    if(CWORK_AUTODETECTED_FILES)
        set(_TEST_FILES ${_TEST_FILES} ${CWORK_AUTODETECTED_FILES})
    endif()
    unset(CWORK_AUTODETECTED_FILES)
endmacro()


# ┌──────────────────────────────────────────────────────────────────┐
# │  File detection - common files                                   │
# └──────────────────────────────────────────────────────────────────┘
if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows") # win32 (windows)
    set(_VIDEO_IGNORED_EXT "_x11.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m" "_andr.cpp")
elseif(IOS) # uikit (iOS)
    set(_VIDEO_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_andr.cpp")
elseif(ANDROID) # jni (android)
    set(_VIDEO_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m")
elseif(APPLE) # cocoa (macOS)
    set(_VIDEO_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_wln.cpp" "_ios.cpp" "_ios.m" "_andr.cpp")
elseif(CWORK_LINUX_WAYLAND) # wayland (linux)
    set(_VIDEO_IGNORED_EXT "_win32.cpp" "_x11.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m" "_andr.cpp")
else() # x11 (linux/unix)
    set(_VIDEO_IGNORED_EXT "_win32.cpp" "_wln.cpp" "_cocoa.cpp" "_cocoa.m" "_ios.cpp" "_ios.m" "_andr.cpp")
endif()

autodetect_source_files_no_recurse(${_INCLUDE_DIR} "*.h*") # include
set(_INCLUDE_FILES ${CWORK_AUTODETECTED_FILES})
unset(CWORK_AUTODETECTED_FILES)

autodetect_source_files_no_recurse(${_TEST_DIR} "*.[hc]*") # test
set(_TEST_FILES ${CWORK_AUTODETECTED_FILES})
unset(CWORK_AUTODETECTED_FILES)

set(_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/build_shaders.py) # shaders

autodetect_source_files_no_recurse(${_SOURCE_DIR} "*.[hcm]*") # src
if(CWORK_AUTODETECTED_FILES) # filter: remove unwanted system-specific files
    set(_SOURCE_FILES)
    set(_OBJC_SOURCE_FILES "")
    foreach(_file ${CWORK_AUTODETECTED_FILES})
        foreach(_ignored ${_VIDEO_IGNORED_EXT})
            if(_file MATCHES ".*${_ignored}$")
                set(_IGNORED_FILE ON)
                break()
            endif()
        endforeach()
        if(NOT _IGNORED_FILE)
            set(_SOURCE_FILES ${_SOURCE_FILES} ${_file})
            if((IOS OR APPLE) AND _file MATCHES ".*\\.m$")
                set(_OBJC_SOURCE_FILES ${_OBJC_SOURCE_FILES} ${_file})
            endif()
        else()
            unset(_IGNORED_FILE)
        endif()
    endforeach()
    unset(CWORK_AUTODETECTED_FILES)
    
    if((IOS OR APPLE) AND NOT _OBJC_SOURCE_FILES STREQUAL "") # tag objective-C files
        cwork_set_source_language(C, ${_OBJC_SOURCE_FILES})
    endif()
endif()


# ┌──────────────────────────────────────────────────────────────────┐
# │  File detection - renderer-specific files                        │
# └──────────────────────────────────────────────────────────────────┘
if(CWORK_VIDEO_OPENGL4)
    set(_INCLUDE_FILES ${_INCLUDE_FILES} ${_INCLUDE_DIR}/api/opengl.h)
    pandora_video_autodetect_files(openGL4)
endif()
if(CWORK_VIDEO_D3D11)
    pandora_video_autodetect_files(d3d11)
endif()
if(CWORK_VIDEO_VULKAN)
    set(_INCLUDE_FILES ${_INCLUDE_FILES} ${_INCLUDE_DIR}/api/vulkan.h)
    pandora_video_autodetect_files(vulkan)
endif()


# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cwork_set_include_files(${_INCLUDE_FILES})
if(_SOURCE_FILES)
    cwork_set_source_files(${_SOURCE_FILES})
endif()
cwork_set_custom_files(${_SHADER_FILES})
cwork_set_test_files(${_TEST_FILES})

cwork_create_project("static" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     "include" "src" "test")

# rendering api config - feedback messages
if(CWORK_VIDEO_OPENGL4)
    if(_VIDEO_OPENGL_ES3_VERSION_STRING)
        message("-- OpenGL ES${_VIDEO_OPENGL_ES3_VERSION_STRING} features level --")
    else()
        message("-- OpenGL ${_VIDEO_OPENGL4_VERSION_STRING} feature level --")
    endif()
endif()
if((WIN32 OR WIN64 OR _WIN32 OR _WIN64) AND DEFINED _VIDEO_D3D11_NOT_SUPPORTED AND _VIDEO_D3D11_NOT_SUPPORTED)
    message("> Windows SDK not installed or not supported by compiler -> direct3D disabled")
elseif(CWORK_VIDEO_D3D11)
    message("-- Direct3D ${_VIDEO_D3D11_VERSION_STRING} feature level --")
endif()
if(DEFINED _VIDEO_VULKAN_NOT_SUPPORTED AND _VIDEO_VULKAN_NOT_SUPPORTED)
    message("> Vulkan SDK not installed or not supported on current system -> disabled")
elseif(CWORK_VIDEO_VULKAN)
    message("-- Vulkan ${_VIDEO_VULKAN_VERSION_STRING} feature level --")
endif()

# build shaders from shader modules
add_custom_target(${PROJECT_NAME}.build_shaders ALL
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/build_shaders.py "${_SHADER_DIR}" 
                                                                              "${CMAKE_CURRENT_SOURCE_DIR}/_generated" 
                                                                              "${_INCLUDE_DIR}/_embed"
    COMMENT "Building shader files..."
)
set_target_properties(${PROJECT_NAME}.build_shaders PROPERTIES FOLDER ${CWORK_SOLUTION_NAME}/tools)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.build_shaders)
