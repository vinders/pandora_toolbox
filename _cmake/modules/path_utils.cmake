# Path finding utilities

#brief: Find architecture label for linux/unix/android libraries
macro(cwork_find_arch_label)
    if(CMAKE_SYSTEM_PROCESSOR)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL arm64 
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL AARCH64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARM64 
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv8 OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv9)
            set(CWORK_ARCH_LABEL arm64)
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL arm OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARM OR CMAKE_SYSTEM_PROCESSOR STREQUAL ARMv7)
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
