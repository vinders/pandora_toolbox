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

if(NOT DEFINED _CWORK_OPTIONS_FOUND)
    set(_CWORK_OPTIONS_FOUND ON)

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Options                                                         │
    # └──────────────────────────────────────────────────────────────────┘
    
    # -- compiler settings --
    
    if(NOT DEFINED CWORK_CPP_REVISION)
        if( (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
         OR (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.14)
         OR ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0) )
            set(CWORK_CPP_REVISION "14" CACHE STRING "C++ revision") # compiler too old for C++17 -> only C++14
        else()
            set(CWORK_CPP_REVISION AUTO CACHE STRING "C++ revision") # modern compiler -> customizable C++ revision
            set_property(CACHE CWORK_CPP_REVISION PROPERTY STRINGS AUTO "20" "17" "14") # possible values for GUI
        endif()
    endif()
    
    if(NOT DEFINED CWORK_BUILD_SYMBOL_FILES)
        option(CWORK_BUILD_SYMBOL_FILES "symbol files" OFF) # extract symbol files
    endif()
    if(NOT DEFINED CWORK_EXPORT_SYMBOLS)
        option(CWORK_EXPORT_SYMBOLS "auto-export all symbols" ON) # should always be ON for static libs
    endif()
    if(NOT DEFINED CWORK_WINDOWS_VERSION)
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
            if("${CMAKE_SYSTEM_VERSION}" EQUAL 6.1 OR "${CMAKE_SYSTEM_VERSION}" EQUAL 6.2 OR MINGW)
                set(_DEFAULT_MIN_WINDOWS_VERSION "7")
            else()
                set(_DEFAULT_MIN_WINDOWS_VERSION "8")
            endif()
        
            set(CWORK_WINDOWS_VERSION ${_DEFAULT_MIN_WINDOWS_VERSION} CACHE STRING "minimum Windows version (7, 8(8.1), 10(10.RS2))") # support versions of Windows older than Windows 10
            set_property(CACHE CWORK_WINDOWS_VERSION PROPERTY STRINGS "10" "8" "7") # possible values for GUI
        endif()
    endif()
    if(NOT DEFINED CWORK_LINUX_WAYLAND)
        if(NOT WIN32 AND NOT WIN64 AND NOT _WIN32 AND NOT _WIN64 AND NOT APPLE AND NOT ANDROID AND NOT IOS)
            option(CWORK_LINUX_WAYLAND "Wayland UI (instead of X.org)" OFF) # use Wayland display server instead of X11
        endif()
    endif()

    # -- features to include --
    
    if(NOT DEFINED CWORK_DOCS AND NOT CMAKE_CROSSCOMPILING)
        option(CWORK_DOCS "docs" OFF) # doxygen docs generation
    endif()
    if(NOT DEFINED CWORK_TESTS AND NOT CMAKE_CROSSCOMPILING AND NOT IOS AND NOT ANDROID)
        option(CWORK_TESTS "tests" ON) # unit tests + integration tests
    endif()
    if(NOT DEFINED CWORK_TOOLS)
        option(CWORK_TOOLS "tools" ON) # additional tools (sub-projects, perfs tests, graphical tests, editors...)
    endif()
    
    # -- video rendering --
    
    if(NOT IOS AND NOT ANDROID AND NOT DEFINED CWORK_SHADER_COMPILERS)
        option(CWORK_SHADER_COMPILERS "include D3D/Vulkan shader compilation at runtime" ON)
    endif()
    
    if(IOS OR ANDROID)
        set(CWORK_VIDEO_OPENGL4 OFF CACHE BOOL "not supported (desktop only)")
        set(_VIDEO_OPENGL4_NOT_SUPPORTED ON CACHE INTERNAL "" FORCE)
    else()
        if(NOT DEFINED CWORK_VIDEO_OPENGL4)
            option(CWORK_VIDEO_OPENGL4 "enable OpenGL 4 features" OFF)
        endif()
        if(NOT DEFINED CWORK_OPENGL4_VERSION)
            if(APPLE)
                set(CWORK_OPENGL4_VERSION "41" CACHE STRING "maximum OpenGL 4 revision support") # mac OS only supports OpenGL 4.1
            else()
                set(CWORK_OPENGL4_VERSION "45" CACHE STRING "maximum OpenGL 4 revision support")
                set_property(CACHE CWORK_OPENGL4_VERSION PROPERTY STRINGS "46" "45" "43" "41") # possible values for GUI
            endif()
        endif()
    endif()

    if((NOT WIN32 AND NOT WIN64 AND NOT _WIN32 AND NOT _WIN64) OR NOT MSVC)
        set(CWORK_VIDEO_D3D11 OFF CACHE BOOL "not supported (Windows only)")
        set(_VIDEO_D3D11_NOT_SUPPORTED ON CACHE INTERNAL "" FORCE)
    else()
        if(NOT DEFINED CWORK_VIDEO_D3D11)
            option(CWORK_VIDEO_D3D11 "enable Direct3D 11 features" ON)
        endif()
        if(NOT DEFINED CWORK_D3D11_VERSION)
            if(CWORK_WINDOWS_VERSION AND CWORK_WINDOWS_VERSION STREQUAL "7")
                set(_DEFAULT_D3D11_MAX_VERSION "110")
            else()
                set(_DEFAULT_D3D11_MAX_VERSION "114")
            endif()
            set(CWORK_D3D11_VERSION ${_DEFAULT_D3D11_MAX_VERSION} CACHE STRING "maximum Direct3D 11 revision support")
            set_property(CACHE CWORK_D3D11_VERSION PROPERTY STRINGS "114" "113" "111" "110") # possible values for GUI
        endif()
    endif()

    if("$ENV{VULKAN_SDK}" STREQUAL "")
        set(CWORK_VIDEO_VULKAN OFF CACHE BOOL "not supported (SDK not found)")
        set(_VIDEO_VULKAN_NOT_SUPPORTED ON CACHE INTERNAL "" FORCE)
    else()
        if(NOT DEFINED CWORK_VIDEO_VULKAN)
            option(CWORK_VIDEO_VULKAN "enable Vulkan features" ON)
        endif()
    endif()
    
    # -- CI / debugging --
    
    if(NOT DEFINED CWORK_COVERAGE 
       AND NOT DEFINED WIN32 AND NOT DEFINED WIN64 AND NOT DEFINED _WIN32 AND NOT DEFINED _WIN64 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" 
       AND NOT CMAKE_CROSSCOMPILING AND NOT IOS AND NOT ANDROID)
        option(CWORK_COVERAGE "coverage" OFF) # code coverage (only on Linux)
    endif()
    if(NOT DEFINED CWORK_DUMMY_SOURCES)
        option(CWORK_DUMMY_SOURCES "compile header-only libs (generate dummy sources)" OFF) # generate source files for header-only libraries (to force compilation)
    endif()
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Helpers                                                         │
    # └──────────────────────────────────────────────────────────────────┘
    
    # C++ revision label for messages
    if(CWORK_CPP_REVISION AND NOT CWORK_CPP_REVISION STREQUAL "AUTO")
        set(CWORK_CPP_REVISION_NAME ${CWORK_CPP_REVISION})
    else()
        set(CWORK_CPP_REVISION_NAME "17")
    endif()
    
    #brief: Print message with active option values
    macro(cwork_print_options)
        message("> Build docs: ${CWORK_DOCS}")
        message("> Build tests: ${CWORK_TESTS}")
        message("> Build tools: ${CWORK_TOOLS}")
        
        if(NOT DEFINED WIN32 AND NOT DEFINED WIN64 AND NOT DEFINED _WIN32 AND NOT DEFINED _WIN64 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_CROSSCOMPILING AND NOT IOS AND NOT ANDROID)
          message("> Coverage: ${CWORK_COVERAGE}")
        endif()
        if(CWORK_BUILD_SYMBOL_FILES)
            message("> Symbol files: ON")
        endif()
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
            if(CWORK_WINDOWS_VERSION STREQUAL "10")
                message("> Minimum Windows: 10 RS2 (2017)")
            elseif(CWORK_WINDOWS_VERSION STREQUAL "8")
                message("> Minimum Windows: 8.1 (2013)")
            else()
                message("> Minimum Windows: 7 (2009)")
            endif()
        endif()
        if(NOT WIN32 AND NOT WIN64 AND NOT _WIN32 AND NOT _WIN64 AND NOT APPLE AND NOT ANDROID AND NOT IOS)
            if(CWORK_LINUX_WAYLAND)
                message("> Display server: Wayland")
            else()
                message("> Display server: X11")
            endif()
        endif()

        if(NOT DEFINED _VIDEO_OPENGL4_NOT_SUPPORTED)
            if(CWORK_VIDEO_OPENGL4)
                string(SUBSTRING ${CWORK_OPENGL4_VERSION} 1 1 __CWORK_OPENGL4_VERSION_MINOR)
                message("> OpenGL: 4.${__CWORK_OPENGL4_VERSION_MINOR}")
                unset(__CWORK_OPENGL4_VERSION_MINOR)
            else()
                message("> OpenGL: OFF")
            endif()
        endif()
        if(NOT DEFINED _VIDEO_D3D11_NOT_SUPPORTED)
            if(CWORK_VIDEO_D3D11)
                string(SUBSTRING ${CWORK_D3D11_VERSION} 2 1 __CWORK_D3D11_VERSION_MINOR)
                message("> Direct3D: 11.${__CWORK_D3D11_VERSION_MINOR}")
                unset(__CWORK_D3D11_VERSION_MINOR)
            else()
                message("> Direct3D: OFF")
            endif()
        endif()
        if(NOT DEFINED _VIDEO_VULKAN_NOT_SUPPORTED)
            if(CWORK_VIDEO_VULKAN)
                message("> Vulkan: 1.2+")
            else()
                message("> Vulkan: OFF")
            endif()
        endif()
    endmacro()

endif()
