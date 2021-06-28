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
if(CWORK_VIDEO_OPENGL4) # openGL 4.1+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" opengl_libs libglew_static)
endif()
if(CWORK_VIDEO_OPENGLES3) # openGLES 3+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" opengles_libs)
endif()
if(CWORK_VIDEO_D3D11) # direct3D 11+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" direct3d11)
endif()
if(CWORK_VIDEO_VULKAN) # vulkan 1.2+
    set(_EXTERN_LIBS ${_EXTERN_LIBS} "public" vulkan_libs)
endif()
if(CWORK_VIDEO_OPENGL4 OR CWORK_VIDEO_OPENGLES3 OR CWORK_VIDEO_VULKAN) # common
    set(_EXTERN_LIBS ${_EXTERN_LIBS} glm)
endif()

if(_EXTERN_LIBS)
    cwork_set_external_libs("private" ${_EXTERN_LIBS})
endif()
cwork_set_internal_libs(system thread memory hardware)


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
    
    autodetect_source_files("${_SOURCE_DIR}/${category_id}" "*.[hc]*")
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
    set(_SYSTEM_SOURCE_SUFFIX "win32")
else()
    if(IOS) # uikit (iOS)
        set(_SYSTEM_SOURCE_SUFFIX "uikit")
    elseif(ANDROID) # jni (android)
        set(_SYSTEM_SOURCE_SUFFIX "andr")
    elseif(APPLE) # cocoa (macOS)
        set(_SYSTEM_SOURCE_SUFFIX "cocoa")
    elseif(CWORK_LINUX_WAYLAND) # wayland (linux)
        set(_SYSTEM_SOURCE_SUFFIX "wln")
    else() # x11 (linux/unix)
        set(_SYSTEM_SOURCE_SUFFIX "x11")
    endif()
endif()

autodetect_source_files_no_recurse(${_INCLUDE_DIR} "*.h*") # include
set(_INCLUDE_FILES ${CWORK_AUTODETECTED_FILES})
unset(CWORK_AUTODETECTED_FILES)
autodetect_source_files("${_INCLUDE_DIR}/_private" "*.h*") # include/_private
set(_INCLUDE_FILES ${_INCLUDE_FILES} ${CWORK_AUTODETECTED_FILES})
unset(CWORK_AUTODETECTED_FILES)

autodetect_source_files_no_recurse(${_TEST_DIR} "*.[hc]*") # test
set(_TEST_FILES ${CWORK_AUTODETECTED_FILES})
unset(CWORK_AUTODETECTED_FILES)

set(_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/build_shaders.py) # shaders

# base source files
autodetect_source_files_no_recurse(${_SOURCE_DIR} "*.[hc]*") # src
if(CWORK_AUTODETECTED_FILES)
    if(NOT IOS AND NOT APPLE)
        set(_SOURCE_FILES ${CWORK_AUTODETECTED_FILES})
    else()
        foreach(_file ${CWORK_AUTODETECTED_FILES})
            if(NOT _file MATCHES ".*_noapple.cpp")
                set(_SOURCE_FILES ${_SOURCE_FILES} "${_file}")
            endif()
        endforeach()
    endif()
    unset(CWORK_AUTODETECTED_FILES)
endif()
if(IOS OR APPLE)
    autodetect_source_files_no_recurse(${_SOURCE_DIR} "*.m*") # src
    set(_SOURCE_FILES ${_SOURCE_FILES} ${CWORK_AUTODETECTED_FILES})
    cwork_set_source_language(CXX ${CWORK_AUTODETECTED_FILES}) # tag objective-C++ files
    unset(CWORK_AUTODETECTED_FILES)
endif()

# window management source files
autodetect_source_files("${_SOURCE_DIR}/window/${_SYSTEM_SOURCE_SUFFIX}" "*.[hcm]*") # src/window
if(CWORK_AUTODETECTED_FILES)
    if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
        set(_SOURCE_FILES ${_SOURCE_FILES} ${CWORK_AUTODETECTED_FILES})
    else()
        set(_SOURCE_FILES ${_SOURCE_FILES} ${CWORK_AUTODETECTED_FILES} "${_SOURCE_DIR}/window/message_box_common.cpp")
        if(IOS OR APPLE)
            set(_OBJC_SOURCE_FILES "")
            foreach(_file ${CWORK_AUTODETECTED_FILES})
                if(_file MATCHES ".*\\.m$")
                    set(_OBJC_SOURCE_FILES ${_OBJC_SOURCE_FILES} "${_file}")
                endif()
            endforeach()
            if(NOT _OBJC_SOURCE_FILES STREQUAL "") # tag objective-C files
                cwork_set_source_language(C ${_OBJC_SOURCE_FILES})
            endif()
        endif()
    endif()
    unset(CWORK_AUTODETECTED_FILES)
endif()
if(ANDROID)
    autodetect_source_files("${_SOURCE_DIR}/window${_SYSTEM_SOURCE_SUFFIX}" "*.java*")
    if(CWORK_AUTODETECTED_FILES)
        cwork_set_source_language(JAVA ${CWORK_AUTODETECTED_FILES}) # tag java files
        unset(CWORK_AUTODETECTED_FILES)
    endif()
endif()


# ┌──────────────────────────────────────────────────────────────────┐
# │  File detection - renderer-specific files                        │
# └──────────────────────────────────────────────────────────────────┘
if(CWORK_VIDEO_OPENGL4)
    pandora_video_autodetect_files(openGL4)
endif()
if(CWORK_VIDEO_OPENGLES3)
    pandora_video_autodetect_files(openGLES3)
endif()
if(CWORK_VIDEO_D3D11)
    pandora_video_autodetect_files(d3d11)
endif()
if(CWORK_VIDEO_VULKAN)
    pandora_video_autodetect_files(vulkan)
endif()


# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cwork_set_include_files(${_INCLUDE_FILES})
cwork_set_source_files(${_SOURCE_FILES})
cwork_set_custom_files(${_SHADER_FILES})
cwork_set_test_files(${_TEST_FILES})
if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    cwork_set_test_embedded_resource_dir("${CWORK_SOLUTION_PATH}/_img/test_win32")
endif()
cwork_set_test_resource_dir("${CWORK_SOLUTION_PATH}/_img/test_common")

set(_VIDEO_TOOLS "tools/window_viewer")
if(NOT IOS AND NOT ANDROID)
    set(_VIDEO_TOOLS ${_VIDEO_TOOLS} "tools/3D_scene")
endif()

cwork_create_project("static" "${CWORK_SOLUTION_PATH}/_cmake" "${CWORK_SOLUTION_PATH}/_cmake/modules"
                     "include" "src" "test" ${_VIDEO_TOOLS})

# rendering api config - feedback messages
message(" ") # new line
if(CWORK_VIDEO_OPENGL4)
    message("-- OpenGL ${_VIDEO_OPENGL4_VERSION_STRING} feature level --")
endif()
if(CWORK_VIDEO_OPENGLES3)
    message("-- OpenGLES ${_VIDEO_OPENGLES3_VERSION_STRING} feature level --")
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
