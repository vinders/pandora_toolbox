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
        option(CWORK_EXPORT_SYMBOLS "auto-export all symbols" ON) # should always be ON for static libs
    endif()
    if(NOT DEFINED CWORK_WINDOWS_VERSION)
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(CWORK_WINDOWS_VERSION "7" CACHE STRING "minimum Windows version (vista.SP1, 7, 8.1, 10.RS2)") # support versions of Windows older than Windows 10
            set_property(CACHE CWORK_WINDOWS_VERSION PROPERTY STRINGS "10" "8" "7" "6") # possible values for GUI
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
    if(NOT DEFINED CWORK_OPENGL4_VERSION)
        if(APPLE)
            set(CWORK_OPENGL4_VERSION "41" CACHE STRING "minimum OpenGL4 revision support") # mac OS only supports OpenGL 4.1
        else()
            set(CWORK_OPENGL4_VERSION "45" CACHE STRING "minimum OpenGL4 revision support")
            set_property(CACHE CWORK_OPENGL4_VERSION PROPERTY STRINGS "46" "45" "43" "41") # possible values for GUI
        endif()
    endif()
    if(NOT DEFINED CWORK_OPENGLES3_VERSION)
        set(CWORK_OPENGLES3_VERSION "32" CACHE STRING "minimum OpenGL ES3 revision support")
        set_property(CACHE CWORK_OPENGLES3_VERSION PROPERTY STRINGS "32" "31" "30") # possible values for GUI
    endif()

    if((NOT WIN32 AND NOT WIN64 AND NOT _WIN32 AND NOT _WIN64) OR NOT MSVC)
        set(CWORK_VIDEO_D3D11 OFF CACHE STRING "enable Direct3D 11 features")
        set(_VIDEO_D3D11_NOT_SUPPORTED ON CACHE STRING "")
    else()
        if(NOT DEFINED CWORK_VIDEO_D3D11)
            option(CWORK_VIDEO_D3D11 "enable Direct3D 11 features" ON)
        endif()
        if(NOT DEFINED CWORK_D3D11_VERSION)
            set(CWORK_D3D11_VERSION "111" CACHE STRING "minimum Direct3D 11 revision support")
            set_property(CACHE CWORK_D3D11_VERSION PROPERTY STRINGS "114" "113" "111" "110") # possible values for GUI
        endif()
    endif()

    if(IOS OR ("$ENV{VULKAN_SDK}" STREQUAL "" AND NOT ANDROID))
        set(CWORK_VIDEO_VULKAN OFF CACHE STRING "enable Vulkan 1.2 features")
        set(_VIDEO_VULKAN_NOT_SUPPORTED ON CACHE STRING "")
    else()
        if(NOT DEFINED CWORK_VIDEO_VULKAN)
            option(CWORK_VIDEO_VULKAN "enable Vulkan 1.2 features" ON)
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
            message("> Minimum Windows version: ${CWORK_WINDOWS_VERSION}")
        endif()

        if(CWORK_VIDEO_OPENGL4)
            if(NOT IOS AND NOT ANDROID)
                message("> OpenGL: ${CWORK_OPENGL4_VERSION}")
            else()
                message("> OpenGLES: ${CWORK_OPENGLES3_VERSION}")
            endif()
        else()
            message("> OpenGL: OFF")
        endif()
        if(NOT DEFINED _VIDEO_D3D11_NOT_SUPPORTED)
            if(CWORK_VIDEO_D3D11)
                message("> Direct3D: ${CWORK_D3D11_VERSION}")
            else()
                message("> Direct3D: OFF")
            endif()
        endif()
        if(NOT DEFINED _VIDEO_VULKAN_NOT_SUPPORTED)
            if(CWORK_VIDEO_VULKAN)
                message("> Vulkan: 1.2")
            else()
                message("> Vulkan: OFF")
            endif()
        endif()
    endmacro()

endif()

