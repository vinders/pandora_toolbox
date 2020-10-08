set(filesystem__FOUND ON)

if(MINGW)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL 9.0)
        set(filesystem__LINKED user32.lib Shlwapi.lib)
    else()
        set(filesystem__LINKED Shlwapi.lib)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL 9.0 
AND (NOT CWORK_CPP_REVISION OR NOT CWORK_CPP_REVISION STREQUAL "14"))
    set(filesystem__LINKED "stdc++fs")
elseif(MSVC AND (WIN32 OR WIN64 OR _WIN32 OR _WIN64))
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.14)
        set(filesystem__LINKED User32.lib Shlwapi.lib)
    else()
        set(filesystem__LINKED Shlwapi.lib)
    endif()
endif()
