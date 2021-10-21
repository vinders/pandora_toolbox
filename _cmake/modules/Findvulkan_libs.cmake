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
            if(WIN32)
                if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                    find_library(VULKANCOMPILER_LIBRARY NAMES glslang
                                 PATHS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/Bin")
                elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
                    find_library(VULKANCOMPILER_LIBRARY NAMES glslang
                                 PATHS "$ENV{VULKAN_SDK}/Lib32" "$ENV{VULKAN_SDK}/Bin32" NO_SYSTEM_ENVIRONMENT_PATH)
                endif()
            else()
                find_library(VULKANCOMPILER_LIBRARY NAMES glslang PATHS "$ENV{VULKAN_SDK}/lib")
            endif()
            set(vulkan_libs__LINKED ${vulkan_libs__LINKED} ${VULKANCOMPILER_LIBRARY})
            
            find_path(_VULKANCOMPILER_INCLUDES_TEST NAMES glslang/SPIRV/SpvTools.h PATHS "$ENV{VULKAN_SDK}/Include" NO_CACHE)
            if(_VULKANCOMPILER_INCLUDES_TEST STREQUAL "_VULKANCOMPILER_INCLUDES_TEST-NOTFOUND")
                add_definitions(-D_P_VULKAN_GLSLANG_OLD_INCLUDES=1)
            endif()
            unset(_VULKANCOMPILER_INCLUDES_TEST)
        endif()
    endif()
endif()
