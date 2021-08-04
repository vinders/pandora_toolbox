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

# ---

#no include protection here - allow re-includes in subdirectories

# ┌──────────────────────────────────────────────────────────────────┐
# │  Architecture information                                        │
# └──────────────────────────────────────────────────────────────────┘
if (DEFINED CMAKE_SIZEOF_VOID_P)
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 16)
        set(CWORK_SYSTEM_ARCH_BITS 128)
    elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8 OR CMAKE_CL_64)
        set(CWORK_SYSTEM_ARCH_BITS 64)
    elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 2)
        set(CWORK_SYSTEM_ARCH_BITS 16)
    else()
        set(CWORK_SYSTEM_ARCH_BITS 32)
    endif()
else()
    if (CMAKE_CL_64)
        set(CWORK_SYSTEM_ARCH_BITS 64)
    else()
        set(CWORK_SYSTEM_ARCH_BITS 32)
    endif()
endif()

if(MSVC AND (WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        AND NOT ${CMAKE_GENERATOR} STREQUAL "Ninja" 
        AND NOT ${CMAKE_GENERATOR} STREQUAL "MinGW Makefiles" 
        AND NOT ${CMAKE_GENERATOR} STREQUAL "NMake Makefiles"
        AND NOT ${CMAKE_GENERATOR} STREQUAL "NMake Makefiles JOM"
        AND NOT ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
    if(${CWORK_SYSTEM_ARCH_BITS} EQUAL 64)
        if (NOT CMAKE_GENERATOR_PLATFORM)
            set(CMAKE_GENERATOR_PLATFORM x64)
        endif()
        if (NOT CMAKE_VS_PLATFORM_NAME)
            set(CMAKE_VS_PLATFORM_NAME x64)
        endif()
    else()
        if (NOT CMAKE_GENERATOR_PLATFORM)
            set(CMAKE_GENERATOR_PLATFORM x86)
        endif()
        if (NOT CMAKE_VS_PLATFORM_NAME)
            set(CMAKE_VS_PLATFORM_NAME x86)
        endif()
    endif()
endif()
