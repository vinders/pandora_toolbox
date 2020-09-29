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

