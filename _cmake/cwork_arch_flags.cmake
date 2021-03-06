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
