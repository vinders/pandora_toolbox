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
            set_property(CACHE CWORK_CPP_REVISION PROPERTY STRINGS AUTO "17" "14") # possible values for GUI
        endif()
    endif()
    
    if(NOT DEFINED CWORK_BUILD_SYMBOL_FILES)
        option(CWORK_BUILD_SYMBOL_FILES "symbol files" OFF) # extract symbol files
    endif()
    if(NOT DEFINED CWORK_EXPORT_SYMBOLS)
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
            option(CWORK_EXPORT_SYMBOLS "auto-export all symbols" ON)
        endif()
    endif()
    if(NOT DEFINED CWORK_WINDOWS_SUPPORT)
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(CWORK_WINDOWS_SUPPORT "7" CACHE STRING "minimum Windows version") # support versions of Windows older than Windows 10
            set_property(CACHE CWORK_WINDOWS_SUPPORT PROPERTY STRINGS "10" "8" "7" "6") # possible values for GUI
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
    
    if(NOT DEFINED CWORK_VIDEO_OPENGL4)
        option(CWORK_VIDEO_OPENGL4 "enable OpenGL4/ES3 features" ON)
    endif()

    if((NOT WIN32 AND NOT WIN64 AND NOT _WIN32 AND NOT _WIN64) OR NOT MSVC)
        set(CWORK_VIDEO_D3D11 OFF CACHE STRING "enable Direct3D 11 features")
        set(_VIDEO_D3D11_NOT_SUPPORTED ON CACHE STRING "")
    elseif(NOT DEFINED CWORK_VIDEO_D3D11)
        option(CWORK_VIDEO_D3D11 "enable Direct3D 11 features" ON)
    endif()

    if(IOS OR ("$ENV{VULKAN_SDK}" STREQUAL "" AND NOT ANDROID))
        set(CWORK_VIDEO_VULKAN OFF CACHE STRING "enable Vulkan 1.2 features")
        set(_VIDEO_VULKAN_NOT_SUPPORTED ON CACHE STRING "")
    elseif(NOT DEFINED CWORK_VIDEO_VULKAN)
        option(CWORK_VIDEO_VULKAN "enable Vulkan 1.2 features" ON)
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

endif()

