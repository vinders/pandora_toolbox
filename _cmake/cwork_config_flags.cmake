#no include protection here - allow re-includes in subdirectories
#params: - CWORK_BUILD_TYPE

#includes
include(${CMAKE_CURRENT_LIST_DIR}/cwork_options.cmake)
if(NOT DEFINED CWORK_SYSTEM_ARCH_BITS)
    include(${CMAKE_CURRENT_LIST_DIR}/cwork_arch_flags.cmake)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Language flags                                                  │
# └──────────────────────────────────────────────────────────────────┘
#set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(CWORK_CPP_REVISION AND ${CWORK_CPP_REVISION} STREQUAL "14")
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build configurations                                            │
# └──────────────────────────────────────────────────────────────────┘
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
if(CWORK_BUILD_TYPE AND ${CWORK_BUILD_TYPE} STREQUAL "dynamic")
    set(BUILD_SHARED_LIBS TRUE)
endif()
if(NOT CMAKE_BUILD_TYPE)
    if(CWORK_COVERAGE)
        set(CMAKE_BUILD_TYPE Debug)
    else()
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4103 /wd4505 /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /Zi /Ob0 /Od /RTC1 /DDEBUG")    
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /O2 /Ob2 /Zi /DRELEASE")
    
    if(CWORK_EXPORT_SYMBOLS)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif()
    if(CWORK_BUILD_TYPE AND ${CWORK_BUILD_TYPE} STREQUAL "dynamic")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    endif()

else() # GCC / Clang
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(-Wall -Wno-missing-braces -Wmissing-field-initializers)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    else()
        add_compile_options(-Wall)
    endif()
    if(MINGW)
        if(CWORK_EXPORT_SYMBOLS)
            set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        endif()
        set(CMAKE_SH "CMAKE_SH-NOTFOUND")
    endif()
    
    if(CWORK_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  System definitions                                              │
# └──────────────────────────────────────────────────────────────────┘
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
    if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(${CWORK_SYSTEM_ARCH_BITS} EQUAL 64)
            add_definitions(-D_WIN64=1)
        else()
            add_definitions(-D_WIN32=1)
        endif()
    endif()

else() # GCC / Clang
    if(MINGW)
        add_definitions(-D_WINDOWS=1)
        add_definitions(-D_WIN32_WINNT=0x0601)
        if(${CWORK_SYSTEM_ARCH_BITS} EQUAL 64)
            add_definitions(-D_WIN64=1)
        else()
            add_definitions(-D_WIN32=1)
        endif()
    elseif(IOS)
        add_definitions(-D__APPLE_IOS__=1)
    endif()
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Language features                                               │
# └──────────────────────────────────────────────────────────────────┘
if(CWORK_CPP_REVISION AND ${CWORK_CPP_REVISION} STREQUAL "14")
    if(MSVC)
        add_compile_options(/std:c++14)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-std=c++14)
    endif()
    add_definitions(-D_CPP_REVISION=14)
else()
    if(MSVC)
        add_compile_options(/std:c++17)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-std=c++17)
    endif()
endif()
