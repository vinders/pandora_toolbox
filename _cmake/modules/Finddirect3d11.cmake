if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    set(direct3d11__FOUND ON)
    
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
    elseif(NOT "$ENV{DXSDK_DIR}" STREQUAL "" AND IS_DIRECTORY "$ENV{DXSDK_DIR}/Include") # if not found, find the directX SDK path
        set(_D3D_INCLUDE_PATH "$ENV{DXSDK_DIR}/Include")
        set(_D3D_LIB_PATH "$ENV{DXSDK_DIR}/Lib")
    elseif(IS_DIRECTORY "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK (June 2010)/Include")
        set(_D3D_INCLUDE_PATH "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK (June 2010)/Include")
        set(_D3D_LIB_PATH "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK (June 2010)/Lib")
    elseif(IS_DIRECTORY "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK/Include")
        set(_D3D_INCLUDE_PATH "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK/Include")
        set(_D3D_LIB_PATH "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK/Lib")
    elseif(IS_DIRECTORY "$ENV{ProgramFiles}/Microsoft DirectX SDK/Include")
        set(_D3D_INCLUDE_PATH "$ENV{ProgramFiles}/Microsoft DirectX SDK/Include")
        set(_D3D_LIB_PATH "$ENV{ProgramFiles}/Microsoft DirectX SDK/Lib")
    endif()
    unset(_WIN10_SDK_PATH)
    unset(_WIN10_SDK_VERSION)
    
    
    if (_D3D_INCLUDE_PATH)
        # find D3D/DXGI include path
        find_path(D3D11_INCLUDE_DIR d3d11.h HINTS "${_D3D_INCLUDE_PATH}/um" "${_D3D_INCLUDE_PATH}" DOC "Windows SDK include directory" HINTS)
        if (IS_DIRECTORY "${_D3D_INCLUDE_PATH}/shared")
            find_path(DXGI_INCLUDE_DIR dxgi.h HINTS "${_D3D_INCLUDE_PATH}/shared" "${_D3D_INCLUDE_PATH}" DOC "Windows SDK shared include directory" HINTS)
            set(D3D11_INCLUDE_DIRS ${D3D11_INCLUDE_DIR} ${DXGI_INCLUDE_DIR} CACHE STRING "D3D11_INCLUDE_DIRS")
        else()
            set(D3D11_INCLUDE_DIRS ${D3D11_INCLUDE_DIR} CACHE STRING "D3D11_INCLUDE_DIRS")
        endif()
        
        # find D3D/DXGI library path
        if (CMAKE_GENERATOR MATCHES "Visual Studio.*ARM")
            set(_D3D11_ARCH arm)
        elseif (CMAKE_GENERATOR MATCHES "Visual Studio.*ARM64")
            set(_D3D11_ARCH arm64)
        elseif ((DEFINED CMAKE_SIZEOF_VOID_P AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8) OR CMAKE_CL_64)
            set(_D3D11_ARCH x64)
        else()
            set(_D3D11_ARCH x86)
        endif()
        find_library(D3D11_LIBRARY NAMES d3d11.lib HINTS "${_D3D_LIB_PATH}/um/${_D3D11_ARCH}" "${_D3D_LIB_PATH}/${_D3D11_ARCH}")  
        find_library(DXGI_LIBRARY  NAMES dxgi.lib  HINTS "${_D3D_LIB_PATH}/um/${_D3D11_ARCH}" "${_D3D_LIB_PATH}/${_D3D11_ARCH}")  
        find_library(D3DCOMPILER_LIBRARY NAMES d3dcompiler.lib HINTS "${_D3D_LIB_PATH}/um/${_D3D11_ARCH}" "${_D3D_LIB_PATH}/${_D3D11_ARCH}")  
        set(D3D11_LIBRARIES ${D3D11_LIBRARY} ${DXGI_LIBRARY} ${D3DCOMPILER_LIBRARY} CACHE STRING "D3D11_LIBRARIES")

        unset(_D3D_INCLUDE_PATH)
        unset(_D3D_LIB_PATH)
        unset(_D3D11_ARCH)

        # set results
        include_directories(${D3D11_INCLUDE_DIRS})
        set(direct3d11__LINKED ${D3D11_LIBRARIES})
        message("-- Found Direct3D11: ${direct3d11__LINKED}")
        
    elseif(MSVC)
        set(direct3d11__LINKED d3d11 dxgi d3dcompiler) # not found -> rely on MSVC API includes
    else()
        message(FATAL_ERROR "-- Finddirect3d11: Windows SDK was not found on current system.")
    endif()
else()
  message(FATAL_ERROR "-- Finddirect3d11: the DirectX API is not supported on current system.")
endif()
