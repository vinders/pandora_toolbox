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

# ┌──────────────────────────────────────────────────────────────────┐
# │  Path finding utilities                                          │
# └──────────────────────────────────────────────────────────────────┘

#brief: Find architecture label for linux/unix/android libraries
#returns: CWORK_ARCH_LABEL
macro(cwork_find_arch_label)
    if(CWORK_ARCH_LABEL)
        unset(CWORK_ARCH_LABEL)
    endif()

    if(ANDROID AND CMAKE_ANDROID_ARCH_ABI)
        if(CMAKE_ANDROID_ARCH_ABI MATCHES "^arm64")
            set(CWORK_ARCH_LABEL arm64)
        elseif(CMAKE_ANDROID_ARCH_ABI MATCHES "^arm")
            set(CWORK_ARCH_LABEL arm)
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL x86_64 OR CMAKE_ANDROID_ARCH_ABI STREQUAL amd64)
            set(CWORK_ARCH_LABEL x86_64)
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL x86 OR CMAKE_ANDROID_ARCH_ABI STREQUAL mips64 OR CMAKE_ANDROID_ARCH_ABI STREQUAL mips64)
            set(CWORK_ARCH_LABEL ${CMAKE_ANDROID_ARCH_ABI})
        endif()
    endif()

    if(NOT CWORK_ARCH_LABEL AND CMAKE_SYSTEM_PROCESSOR)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL arm64 
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL AARCH64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARM64 
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv8 OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv8a)
            set(CWORK_ARCH_LABEL arm64)
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL arm OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARM OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv7 OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv6)
            set(CWORK_ARCH_LABEL arm)

        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL x64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL amd64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64)
            set(CWORK_ARCH_LABEL x86_64)
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL x86 OR CMAKE_SYSTEM_PROCESSOR STREQUAL Win32 OR CMAKE_SYSTEM_PROCESSOR STREQUAL i386 OR CMAKE_SYSTEM_PROCESSOR STREQUAL i686)
            set(CWORK_ARCH_LABEL x86)
        endif()
    endif()

    if(NOT CWORK_ARCH_LABEL)
        if(ANDROID OR IOS)
            if((CMAKE_SIZEOF_VOID_P AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8) OR CMAKE_CL_64)
                set(CWORK_ARCH_LABEL arm64)
            else()
                set(CWORK_ARCH_LABEL arm)
            endif()
        else()
            if((CMAKE_SIZEOF_VOID_P AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8) OR CMAKE_CL_64)
                set(CWORK_ARCH_LABEL x86_64)
            else()
                set(CWORK_ARCH_LABEL x86)
            endif()
        endif()
    endif()
endmacro()

#brief: Convert relative path with unknown reference to absolute path
#params: - prefix: relative or absolute path to directory (unknown reference)
#        - suffix: expected path to a valid file that should be appendable after prefix
#returns: CWORK_VALID_ABSOLUTE_PATH
macro(cwork_unknown_path_to_absolute prefix suffix)
    if(NOT IS_ABSOLUTE ${prefix})
        if(NOT EXISTS ${prefix}${suffix})
            if(EXISTS ../${prefix}${suffix})
                set(_VALID_PATH_PREFIX ../${prefix})
            elseif(EXISTS ../../${prefix}${suffix})
                set(_VALID_PATH_PREFIX ../../${prefix})
            elseif(EXISTS ../../../${prefix}${suffix})
                set(_VALID_PATH_PREFIX ../../../${prefix})
            elseif(EXISTS ../../../../${prefix}${suffix})
                set(_VALID_PATH_PREFIX ../../../../${prefix})
            elseif(EXISTS ../../../../../${prefix}${suffix})
                set(_VALID_PATH_PREFIX ../../../../../${prefix})
            elseif(EXISTS ../../../../../../${prefix}${suffix})
                set(_VALID_PATH_PREFIX ../../../../../../${prefix})
            else()
                STRING(REGEX REPLACE "^\\.\\./" "" _STRIP_PATH_PREFIX ${prefix})
                if(EXISTS ${_STRIP_PATH_PREFIX}${suffix})
                    set(_VALID_PATH_PREFIX ${_STRIP_PATH_PREFIX})
                else()
                    STRING(REGEX REPLACE "^\\.\\./" "" _STRIP_PATH_PREFIX2 ${_STRIP_PATH_PREFIX})
                    if(EXISTS ${_STRIP_PATH_PREFIX2}${suffix})
                        set(_VALID_PATH_PREFIX ${_STRIP_PATH_PREFIX2})
                    else()
                        STRING(REGEX REPLACE "^\\.\\./" "" _STRIP_PATH_PREFIX3 ${_STRIP_PATH_PREFIX2})
                        if(EXISTS ${_STRIP_PATH_PREFIX3}${suffix})
                            set(_VALID_PATH_PREFIX ${_STRIP_PATH_PREFIX3})
                        else()
                            set(_VALID_PATH_PREFIX ${prefix}) # file may not exist
                        endif()
                        unset(_STRIP_PATH_PREFIX3)
                    endif()
                    unset(_STRIP_PATH_PREFIX2)
                endif()
                unset(_STRIP_PATH_PREFIX)
            endif()
        else()
            set(_VALID_PATH_PREFIX ${prefix})
        endif()
        
        get_filename_component(CWORK_VALID_ABSOLUTE_PATH "${_VALID_PATH_PREFIX}" ABSOLUTE)
    else()
        set(CWORK_VALID_ABSOLUTE_PATH ${prefix})
    endif()
endmacro()
