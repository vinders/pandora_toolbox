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

if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    set(direct3d12__FOUND ON)
    
    # find the windows SDK path
    if("$ENV{WIN10_SDK_PATH}$ENV{WIN10_SDK_VERSION}" STREQUAL "")
        get_filename_component(_WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0;InstallationFolder]" ABSOLUTE CACHE)
        get_filename_component(_TMP_WIN10_SDK_VERSION "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0;ProductVersion]" ABSOLUTE CACHE)
        get_filename_component(_WIN10_SDK_VERSION ${_TMP_WIN10_SDK_VERSION} NAME)
    else()
        set(_WIN10_SDK_PATH $ENV{WIN10_SDK_PATH})
        set(_WIN10_SDK_VERSION $ENV{WIN10_SDK_VERSION})
    endif()
    if(IS_DIRECTORY "${_WIN10_SDK_PATH}/Include/${_WIN10_SDK_VERSION}.0") # version directory may be suffixed by '.0'
        set(_WIN10_SDK_VERSION "${_WIN10_SDK_VERSION}.0")
    endif()

    if(_WIN10_SDK_PATH AND _WIN10_SDK_VERSION)
        set(_D3D_INCLUDE_PATH "${_WIN10_SDK_PATH}/Include/${_WIN10_SDK_VERSION}")
        set(_D3D_LIB_PATH "${_WIN10_SDK_PATH}/Lib/${_WIN10_SDK_VERSION}")
    endif()
    unset(_WIN10_SDK_PATH)
    unset(_WIN10_SDK_VERSION)
    
    
    if(_D3D_INCLUDE_PATH)
        # find D3D/DXGI include path
        find_path(D3D12_INCLUDE_DIR d3d12.h HINTS "${_D3D_INCLUDE_PATH}/um" DOC "Windows SDK include directory" HINTS)
        find_path(DXGI_INCLUDE_DIR dxgi.h HINTS "${_D3D_INCLUDE_PATH}/shared" DOC "Windows SDK shared include directory" HINTS)
        set(D3D12_INCLUDE_DIRS ${D3D12_INCLUDE_DIR} ${DXGI_INCLUDE_DIR} CACHE STRING "D3D12_INCLUDE_DIRS")
        
        # find D3D/DXGI library path
        if(CMAKE_GENERATOR MATCHES "Visual Studio.*ARM")
            set(_D3D12_ARCH arm)
        elseif(CMAKE_GENERATOR MATCHES "Visual Studio.*ARM64")
            set(_D3D12_ARCH arm64)
        elseif((DEFINED CMAKE_SIZEOF_VOID_P AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8) OR CMAKE_CL_64)
            set(_D3D12_ARCH x64)
        else()
            set(_D3D12_ARCH x86)
        endif()
        find_library(D3D12_LIBRARY NAMES d3d12.lib HINTS "${_D3D_LIB_PATH}/um/${_D3D12_ARCH}")
        find_library(DXGI_LIBRARY  NAMES dxgi.lib  HINTS "${_D3D_LIB_PATH}/um/${_D3D12_ARCH}")
        
        if(DEFINED CWORK_INCLUDE_SHADER_COMPILERS AND CWORK_INCLUDE_SHADER_COMPILERS)
            find_library(D3DCOMPILER_LIBRARY NAMES d3dcompiler.lib HINTS "${_D3D_LIB_PATH}/um/${_D3D12_ARCH}")  
            set(D3D12_LIBRARIES ${D3D12_LIBRARY} ${DXGI_LIBRARY} ${D3DCOMPILER_LIBRARY} CACHE STRING "D3D12_LIBRARIES")
        else()
            set(D3D12_LIBRARIES ${D3D12_LIBRARY} ${DXGI_LIBRARY} CACHE STRING "D3D12_LIBRARIES")
        endif()

        unset(_D3D_INCLUDE_PATH)
        unset(_D3D_LIB_PATH)
        unset(_D3D12_ARCH)

        # set results
        set(direct3d12__INCLUDE ${D3D12_INCLUDE_DIRS})
        set(direct3d12__LINKED ${D3D12_LIBRARIES})
        message("-- Found Direct3D12: ${direct3d12__LINKED}")
        
    elseif(MSVC)
        set(direct3d12__LINKED d3d12 dxgi d3dcompiler) # not found -> rely on MSVC API includes
    else()
        message(FATAL_ERROR "-- Finddirect3d12: Windows SDK was not found on current system.")
    endif()
else()
  message(FATAL_ERROR "-- Finddirect3d12: the DirectX API is not supported on current system.")
endif()
