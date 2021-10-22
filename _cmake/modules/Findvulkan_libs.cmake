# MIT License
# Copyright (c) 2021 Romain Vinders

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
# IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

if(ANDROID)
    set(vulkan_libs__FOUND ON)
    if(ANDROID_STANDALONE_TOOLCHAIN)
        set(vulkan_libs__INCLUDE "${ANDROID_STANDALONE_TOOLCHAIN}/usr/include")
    endif()
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)
    set(vulkan_libs__LINKED android vulkan)
    
else()
    find_package(Vulkan REQUIRED)
    if (Vulkan_FOUND)
        set(vulkan_libs__FOUND ON)
        
        set(vulkan_libs__INCLUDE ${Vulkan_INCLUDE_DIRS})
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
            set(vulkan_libs__LINKED ${Vulkan_LIBRARIES})
            add_definitions(-DVK_USE_PLATFORM_WIN32_KHR=1)
        else()
            set(vulkan_libs__LINKED ${Vulkan_LIBRARIES})
        endif()
        
        add_definitions(-DDISABLE_VK_LAYER_VALVE_steam_overlay_1=1)
        
        if(DEFINED CWORK_SHADER_COMPILERS AND CWORK_SHADER_COMPILERS)
            set(LIBGLSLANG__PATH ${CWORK_MODULE_DESTINATION}/glslang)
            # ┌──────────────────────────────────────────────────────────────────┐
            # │  Manual project download                                         │
            # └──────────────────────────────────────────────────────────────────┘
            if(NOT EXISTS ${LIBGLSLANG__PATH} OR NOT EXISTS "${LIBGLSLANG__PATH}/CMakeLists.txt")
                set(_GIT_EXT_REPOSITORY https://github.com/KhronosGroup/glslang.git)
                set(_GIT_EXT_TAG "11.6.0")
                set(_GIT_EXT_CACHE ${CWORK_MODULE_DESTINATION}/.cache/glslang)
                set(_GIT_EXT_DIR ${LIBGLSLANG__PATH})
                configure_file("${CMAKE_CURRENT_LIST_DIR}/git_external.cmake" "${_GIT_EXT_CACHE}/CMakeLists.txt")
                execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" ${_GIT_EXT_CACHE} WORKING_DIRECTORY ${_GIT_EXT_CACHE})
                execute_process(COMMAND "${CMAKE_COMMAND}" --build ${_GIT_EXT_CACHE} WORKING_DIRECTORY ${_GIT_EXT_CACHE})
                unset(_GIT_EXT_REPOSITORY)
                unset(_GIT_EXT_TAG)
                unset(_GIT_EXT_CACHE)
                unset(_GIT_EXT_DIR)
            endif()

            # ┌──────────────────────────────────────────────────────────────────┐
            # │  Include project                                                 │
            # └──────────────────────────────────────────────────────────────────┘
            if(NOT libglslang__FOUND)
                set(libglslang__FOUND ON)
                
                set(SKIP_GLSLANG_INSTALL ON CACHE INTERNAL "" FORCE)
                set(ENABLE_SPVREMAPPER OFF CACHE INTERNAL "" FORCE)
                set(ENABLE_GLSLANG_JS OFF CACHE INTERNAL "" FORCE)
                set(ENABLE_HLSL OFF CACHE INTERNAL "" FORCE)
                set(ENABLE_PCH OFF CACHE INTERNAL "" FORCE)
                set(ENABLE_CTEST OFF CACHE INTERNAL "" FORCE)
                set(OVERRIDE_MSVCCRT OFF CACHE INTERNAL "" FORCE)
                set(BUILD_SHARED_LIBS OFF)
                set(_CMAKE_C_FLAGS_ORIG ${CMAKE_C_FLAGS})
                set(_CMAKE_CXX_FLAGS_ORIG ${CMAKE_CXX_FLAGS})
                
                set(CMAKE_C_FLAGS "") # disable the countless warnings in glslang
                set(CMAKE_CXX_FLAGS "")
                add_subdirectory(${LIBGLSLANG__PATH} ${CMAKE_BINARY_DIR}/libglslang)
                set(CMAKE_C_FLAGS ${_CMAKE_C_FLAGS_ORIG})
                set(CMAKE_CXX_FLAGS ${_CMAKE_CXX_FLAGS_ORIG})
                unset(_CMAKE_C_FLAGS_ORIG)
                unset(_CMAKE_CXX_FLAGS_ORIG)
                
                set(libglslang__INCLUDE ${LIBGLSLANG__PATH}/glslang/Include ${LIBGLSLANG__PATH})
                set(libglslang__LINKED glslang-default-resource-limits SPIRV OGLCompiler)
                
                if(TARGET glslang)
                    set_target_properties(glslang PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET glslang-default-resource-limits)
                    set_target_properties(glslang-default-resource-limits PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET GenericCodeGen)
                    set_target_properties(GenericCodeGen PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET MachineIndependent)
                    set_target_properties(MachineIndependent PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET OGLCompiler)
                    set_target_properties(OGLCompiler PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET OSDependent)
                    set_target_properties(OSDependent PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET SPIRV)
                    set_target_properties(SPIRV PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET SPVRemapper)
                    set_target_properties(SPVRemapper PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET HLSL)
                    set_target_properties(HLSL PROPERTIES FOLDER libs/glslang)
                endif()
                if(TARGET glslangValidator)
                    set_target_properties(glslangValidator PROPERTIES FOLDER libs/tools)
                endif()
                if(TARGET spirv-remap)
                    set_target_properties(spirv-remap PROPERTIES FOLDER libs/tools)
                endif()
                message("-- Found glslang: ${libglslang__LINKED}")
            
                set(vulkan_libs__INCLUDE ${vulkan_libs__INCLUDE} ${libglslang__INCLUDE})
                set(vulkan_libs__LINKED ${vulkan_libs__LINKED} ${libglslang__LINKED})
            endif()
        endif()
    endif()
endif()
