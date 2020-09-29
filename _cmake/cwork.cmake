# ┌──────────────────────────────────────────────────────────────────────┐
# │  CWORK Project Tools                                                 │
# │  -- Make Cmake work again ! --                                       │
# └──────────────────────────────────────────────────────────────────────┘
if(NOT DEFINED _CWORK_PROJECT_TOOLS_FOUND)
    set(_CWORK_PROJECT_TOOLS_FOUND ON)
    
    # includes
    include(${CMAKE_CURRENT_LIST_DIR}/cwork_utils_sources.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cwork_utils_symbols.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cwork_utils_packages.cmake)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Solutions                                                       │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief: Create a solution for common IDEs, containing multiple projects
    #params: - root_path: root path of the solution directory containing subdirectories
    #        - cwork_path: path of the directory containing cwork utilities 
    #        - ARGN: list of project subdirectories (by order of dependencies, from independant lib to most dependant or app)
    macro(cwork_create_solution root_path cwork_path)
        # configure solution
        set(CWORK_SOLUTION_NAME ${PROJECT_NAME} CACHE STRING "CWORK_SOLUTION_NAME")
        set(CWORK_SOLUTION_PATH ${root_path} CACHE STRING "CWORK_SOLUTION_PATH")
        include(${cwork_path}/cwork_options.cmake)
        include(${cwork_path}/cwork_arch_flags.cmake)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
        
        # build message
        if(DEFINED INSTALL_REV AND INSTALL_REV)
            message("##teamcity[buildNumber '${PROJECT_VERSION}${INSTALL_REV}']")
        else()
            message("\n——————————————————————————————————————————————————————————————————————————————")
            message("  Solution: ${CWORK_SOLUTION_NAME} - build ${PROJECT_VERSION}")
            message("  Platform: ${CMAKE_SYSTEM_NAME}-${CWORK_SYSTEM_ARCH_BITS}")
            message("  Build type: C++ ${CWORK_CPP_REVISION}")
            message("——————————————————————————————————————————————————————————————————————————————")
        endif()
        
        if (CWORK_TESTS)
            enable_testing()
        endif()
        
        # project subdirectories
        set(_all_projects "${ARGN}")
        if (_all_projects)
            foreach(_subdir ${_all_projects})
                add_subdirectory(${_subdir})
            endforeach()
        endif()
    endmacro()
    
    #brief: Add multiple projects to an existing solution
    #params: ARGN: list of project subdirectories (by order of dependencies, from independant lib to most dependant or app)
    macro(cwork_set_solution_projects)
        message("\n——————————————————————————————————————————————————————————————————————————————")
        message("  SOLUTION PROJECTS (${CWORK_SOLUTION_NAME} - build ${PROJECT_VERSION})")
        message("——————————————————————————————————————————————————————————————————————————————")
    
        # project subdirectories
        set(_all_projects "${ARGN}")
        if (_all_projects)
            foreach(_subdir ${_all_projects})
                add_subdirectory(${_subdir})
            endforeach()
        endif()
    endmacro()
    
    #brief:   Set default solution name for project, if no parent solution is already defined
    #warning: Must be called before any other function (project creation, dependencies, ...)
    #params: - title: default solution name
    #        - root_path: root path of the parent solution directory
    macro(cwork_set_default_solution title root_path)
        if(NOT DEFINED CWORK_SOLUTION_NAME OR NOT CWORK_SOLUTION_NAME)
            set(CWORK_SOLUTION_NAME ${title} CACHE STRING "CWORK_SOLUTION_NAME")
        endif()
        if(NOT DEFINED CWORK_SOLUTION_PATH OR NOT CWORK_SOLUTION_PATH)
            set(CWORK_SOLUTION_PATH ${root_path} CACHE STRING "CWORK_SOLUTION_PATH")
        endif()
    endmacro()
    
    #brief:   Set project version in CWORK_BUILD_VERSION, based on the contents of a file (if multi-line, each non-empty line is appended after a dot)
    #warning: Must be called before any other function (project creation, dependencies, ...)
    #params: - file_path: path of the file to read
    #        - overwrite_if_defined: if CWORK_BUILD_VERSION already exists, replace it (ON) or keep it (OFF)
    macro(cwork_read_version_from_file file_path overwrite_if_defined)
        if(CWORK_BUILD_VERSION AND overwrite_if_defined)
            unset(CWORK_BUILD_VERSION)
        endif()
        
        if(NOT CWORK_BUILD_VERSION) # if no overwrite allowed, don't replace value
            if(EXISTS ${file_path})
                file (STRINGS ${file_path} _TMP_VERSION_PARTS)
                if(_TMP_VERSION_PARTS)
                    foreach(part IN LISTS _TMP_VERSION_PARTS)
                        if(NOT part STREQUAL "")
                            if(DEFINED _CWORK_BUILD_VERSION)
                                set(_CWORK_BUILD_VERSION "${_CWORK_BUILD_VERSION}.${part}")
                            else()
                                set(_CWORK_BUILD_VERSION "${part}")
                            endif()
                        endif()
                    endforeach()
                    string(REPLACE " " "" CWORK_BUILD_VERSION ${_CWORK_BUILD_VERSION})
                    unset(_CWORK_BUILD_VERSION)
                endif()
                unset(_TMP_VERSION_PARTS)
            else()
                message(FATAL_ERROR "cwork_read_version_from_file: version file not found (${file_path})")
            endif()
        endif()
    endmacro()
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Source files                                                    │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:   Manually specify source include files, instead of auto-detecting them
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    #params: - ARGN: list of include files (.h/.hpp/.hxx/...)
    macro(cwork_set_include_files)
        set(_file_list "${ARGN}")
        if (_file_list)
            set(${PROJECT_NAME}_INCLUDE_FILES "${_file_list}")
        endif()
        unset(_file_list)
    endmacro()
    
    #brief:   Manually specify source code files, instead of auto-detecting them
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    #params: - ARGN: list of source code files (.c/.cpp/.cxx/...)
    macro(cwork_set_source_files)
        set(_file_list "${ARGN}")
        if (_file_list)
            set(${PROJECT_NAME}_SOURCE_FILES "${_file_list}")
        endif()
        unset(_file_list)
    endmacro()
    
    #brief:   Manually specify test files, instead of auto-detecting them
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    #params: - ARGN: list of test files
    macro(cwork_set_test_files)
        set(_file_list "${ARGN}")
        if (_file_list)
            set(${PROJECT_NAME}_TEST_FILES "${_file_list}")
        endif()
        unset(_file_list)
    endmacro()
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Dependencies                                                    │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:   Set external dependencies for current project
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    #params: - scope: scope of external package: "public", "private"
    #        - ARGN: list of external packages to include in current project
    macro(cwork_set_external_libs scope)
        set(_ext_list "${ARGN}")
        if (_ext_list)
            string(TOUPPER ${scope} ${PROJECT_NAME}_EXTERN_LIBS_SCOPE)
            set(${PROJECT_NAME}_EXTERN_LIBS "${_ext_list}")
        endif()
        unset(_ext_list)
    endmacro()
    
    #brief:   Set external frameworks for current project
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    #params: - scope: scope of external frameworks: "public", "private"
    #        - ARGN: list of external frameworks to include in current project
    macro(cwork_set_external_frameworks scope)
        set(_ext_list "${ARGN}")
        if (_ext_list)
            string(TOUPPER ${scope} ${PROJECT_NAME}_EXTERN_FRAMEWORKS_SCOPE)
            set(${PROJECT_NAME}_EXTERN_FRAMEWORKS "${_ext_list}")
        endif()
        unset(_ext_list)
    endmacro()
    
    #brief:   Set internal dependencies for current project (projects in the same solution)
    #warning: Must be called BEFORE cwork_create_project and cwork_set_default_solution
    #params: - ARGN: list of internal projects to include in current project
    macro(cwork_set_internal_libs)
        set(_internal_list "${ARGN}")
        if (_internal_list)
            set(${PROJECT_NAME}_INTERNAL_LIBS "${_internal_list}")
        endif()
        unset(_internal_list)
    endmacro()
    
    #brief:   Set custom library dependencies
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_create_solution
    #params: - root_path: path to directory containing custom library
    #        - custom_solution_name: custom library name
    #        - ARGN: list of packages to include in custom library
    macro(cwork_set_custom_libs root_path custom_solution_name)
        set(__ORIG_CWORK_SOLUTION_NAME ${CWORK_SOLUTION_NAME})
        set(__ORIG_CWORK_SOLUTION_PATH ${CWORK_SOLUTION_PATH})
        unset(CWORK_SOLUTION_NAME CACHE)
        unset(CWORK_SOLUTION_PATH CACHE)
        set(CWORK_SOLUTION_NAME ${custom_solution_name} CACHE STRING "CWORK_SOLUTION_NAME")
        set(CWORK_SOLUTION_PATH "${root_path}/${custom_solution_name}" CACHE STRING "CWORK_SOLUTION_PATH")
        
        if(CWORK_BUILD_VERSION)
          set(__ORIG_CWORK_BUILD_VERSION ${CWORK_BUILD_VERSION})
          unset(CWORK_BUILD_VERSION)
        else()
          set(__ORIG_CWORK_BUILD_VERSION ${PROJECT_VERSION})
        endif()
        cwork_read_version_from_file("${root_path}/${custom_solution_name}/build_version.txt" ON)
        
        message("\n——————————————————————————————————————————————————————————————————————————————")
        message("  CUSTOM SOLUTION (${custom_solution_name} - build ${CWORK_BUILD_VERSION})")
        message("——————————————————————————————————————————————————————————————————————————————")
        
        # project subdirectories
        set(_all_projects "${ARGN}")
        if (_all_projects)
            foreach(_subdir ${_all_projects})
                add_subdirectory("${root_path}/${custom_solution_name}/${_subdir}")
                if(NOT _CWORK_CUSTOM_LIBS)
                    set(_CWORK_CUSTOM_LIBS "${custom_solution_name}.${_subdir}")
                else()
                    set(_CWORK_CUSTOM_LIBS ${_CWORK_CUSTOM_LIBS} "${custom_solution_name}.${_subdir}")
                endif()
            endforeach()
        endif()
        
        set(CWORK_CUSTOM_LIBS ${_CWORK_CUSTOM_LIBS} CACHE STRING "CWORK_CUSTOM_LIBS")
        unset(_CWORK_CUSTOM_LIBS)
        
        unset(CWORK_SOLUTION_NAME CACHE)
        unset(CWORK_SOLUTION_PATH CACHE)
        unset(CWORK_BUILD_VERSION)
        set(CWORK_SOLUTION_NAME ${__ORIG_CWORK_SOLUTION_NAME} CACHE STRING "CWORK_SOLUTION_NAME")
        set(CWORK_SOLUTION_PATH ${__ORIG_CWORK_SOLUTION_PATH} CACHE STRING "CWORK_SOLUTION_PATH")
        set(CWORK_BUILD_VERSION ${__ORIG_CWORK_BUILD_VERSION})
        unset(__ORIG_CWORK_SOLUTION_NAME)
        unset(__ORIG_CWORK_SOLUTION_PATH)
        unset(__ORIG_CWORK_BUILD_VERSION)
    endmacro()
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Projects                                                        │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:   Generate header file with version information
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    macro(cwork_generate_version_header)
        set(${PROJECT_NAME}_VERSION_HEADERS ON)
    endmacro()
    
    #brief:   Set type of sub-project (tools, tests, perfs, ...)
    #warning: Must be called BEFORE cwork_create_project and AFTER cwork_set_default_solution
    macro(cwork_set_subproject_type type_name)
        set(${PROJECT_NAME}_SUBPROJECT_TYPE ${type_name})
    endmacro()
    
    #brief:   Create a solution for common IDEs, containing multiple projects
    #warning: Dependencies/libraries must be set BEFORE calling this, with cwork_set_external_libs and cwork_set_internal_libs
    #params: - build_type: type of project:
    #                      "static" = static link library (embedded in executable / MT)
    #                      "dynamic" or "library" = dynamic link library (shared library / MD)
    #                      "app" or "executable" = executable application
    #        - cwork_path: path of the directory containing cwork utilities 
    #        - module_path: path of the directory containing Cmake modules and library finders 
    #                       (if empty, CMAKE_MODULE_PATH must be set as a Cmake param)
    #        - include_dir: directory containing source headers (.h/.hpp/.hxx) - required
    #        - source_dir: directory containing source code - can be empty or non-existing for header-only libraries
    #        - test_dir: directory containing unit tests - can be empty or non-existing if no tests are available
    #        - ARGN: optional list of subdirectories for sub-projects, tools, ...
    macro(cwork_create_project build_type cwork_path module_path include_dir source_dir test_dir)
        # configure project
        set(CWORK_PROJECT_NAME "${PROJECT_NAME}")
        if (CMAKE_MODULE_PATH)
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${module_path})
        else()
            set(CMAKE_MODULE_PATH ${module_path})
        endif()
        
        string(TOLOWER ${build_type} CWORK_BUILD_TYPE)
        if(${CWORK_BUILD_TYPE} STREQUAL "library")
            set(CWORK_BUILD_TYPE "dynamic")
        elseif(${CWORK_BUILD_TYPE} STREQUAL "app")
            set(CWORK_BUILD_TYPE "executable")
        endif()
        
        include(${cwork_path}/cwork_options.cmake)
        include(${cwork_path}/cwork_arch_flags.cmake)
        include(${cwork_path}/cwork_config_flags.cmake)
        
        set(CWORK_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}")
        if(${CWORK_PROJECT_NAME}_INTERNAL_LIBS)
          set(CWORK_INTERNAL_LIBS ${${CWORK_PROJECT_NAME}_INTERNAL_LIBS})
          set(CWORK_INTERNAL_LIBS_SCOPE PUBLIC)
          unset(${CWORK_PROJECT_NAME}_INTERNAL_LIBS)
        endif()
        if(${CWORK_PROJECT_NAME}_EXTERN_LIBS)
          set(CWORK_EXTERN_LIBS ${${CWORK_PROJECT_NAME}_EXTERN_LIBS})
          set(CWORK_EXTERN_LIBS_SCOPE ${${CWORK_PROJECT_NAME}_EXTERN_LIBS_SCOPE})
          unset(${CWORK_PROJECT_NAME}_EXTERN_LIBS)
          unset(${CWORK_PROJECT_NAME}_EXTERN_LIBS_SCOPE)
        endif()
        if(${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS)
          set(CWORK_EXTERN_FRAMEWORKS ${${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS})
          set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS_SCOPE})
          unset(${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS)
          unset(${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS_SCOPE)
        endif()
        if(CWORK_LINKED_LIBRARIES)
            unset(CWORK_LINKED_LIBRARIES)
        endif()
        
        # build message
        if(DEFINED INSTALL_REV AND INSTALL_REV)
            message("##teamcity[buildNumber '${PROJECT_VERSION}${INSTALL_REV}']")
        elseif(${PROJECT_NAME}_SUBPROJECT_TYPE)
            message("\n  --------------------------------------------------")
            message("> ${CWORK_PROJECT_NAME} - build ${PROJECT_VERSION}")
            message("  Build type: ${CWORK_BUILD_TYPE} - C++ ${CWORK_CPP_REVISION}")
            message("  --------------------------------------------------")
        else()
            message("\n------------------------------------------------------------------------------")
            message("> ${CWORK_PROJECT_NAME} - build ${PROJECT_VERSION}")
            message("  Platform: ${CMAKE_SYSTEM_NAME}-${CWORK_SYSTEM_ARCH_BITS}")
            message("  Build type: ${CWORK_BUILD_TYPE} - C++ ${CWORK_CPP_REVISION}")
            message("------------------------------------------------------------------------------")
        endif()
        
        # -- PROJECT SOURCE FILES --
        
        # generate version header file
        if(DEFINED ${CWORK_PROJECT_NAME}_VERSION_HEADERS AND ${CWORK_PROJECT_NAME}_VERSION_HEADERS)
            message("> version header generated...")
            file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}/version.h")
            configure_file("${cwork_path}/templates/version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}/version.h")
        endif()
        
        # auto-detect source files
        if(NOT ${CWORK_PROJECT_NAME}_INCLUDE_FILES)
            message("> auto-detection of include files...")
            autodetect_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}" "*.h*")
            set(${CWORK_PROJECT_NAME}_INCLUDE_FILES ${CWORK_AUTODETECTED_FILES})
            unset(CWORK_AUTODETECTED_FILES)
        endif()
        if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
            message("> auto-detection of source files...")
            autodetect_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}" "*.c*")
            set(${CWORK_PROJECT_NAME}_SOURCE_FILES ${CWORK_AUTODETECTED_FILES})
            unset(CWORK_AUTODETECTED_FILES)
            
            # generate dummy source file, to force compilation of interface libraries
            if(CWORK_DUMMY_SOURCES AND NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                message("> no source files: dummy source file generated...")
                file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/_build/dummy_src.cpp")
                set(PROJECT_GENERATED_INCLUDES "/* library headers */")
                foreach(_hpp_file IN ITEMS ${${CWORK_PROJECT_NAME}_INCLUDE_FILES})
                    set(PROJECT_GENERATED_INCLUDES "${PROJECT_GENERATED_INCLUDES}\n#include <${_hpp_file}>")
                endforeach()
                configure_file("${cwork_path}/templates/dummy_src.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/_build/dummy_src.cpp")
                set(${CWORK_PROJECT_NAME}_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/_build/dummy_src.cpp")
            endif()
        endif()
        # count source files
        list(LENGTH ${CWORK_PROJECT_NAME}_INCLUDE_FILES CWORK_INCLUDE_FILES_LENGTH)
        message("> ${CWORK_INCLUDE_FILES_LENGTH} include files found in '${include_dir}'.")
        list(LENGTH ${CWORK_PROJECT_NAME}_SOURCE_FILES CWORK_SOURCE_FILES_LENGTH)
        message("> ${CWORK_SOURCE_FILES_LENGTH} source files found in '${source_dir}'.")
        unset(CWORK_INCLUDE_FILES_LENGTH)
        unset(CWORK_SOURCE_FILES_LENGTH)
        
        # -- LOAD DEPENDENCIES --
        
        # internal dependencies
        if(CWORK_INTERNAL_LIBS)
            if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                set(CWORK_INTERNAL_LIBS_SCOPE INTERFACE)
            endif()
            foreach(_internal_lib IN ITEMS ${CWORK_INTERNAL_LIBS})
                # include project
                if(CWORK_SOLUTION_PATH)
                    set(_TMP_LIB_NAME "${CWORK_SOLUTION_NAME}.${_internal_lib}")
                    if(NOT ${_TMP_LIB_NAME}_FOUND)
                        add_subdirectory(${CWORK_SOLUTION_PATH}/${_internal_lib} ${CMAKE_BINARY_DIR}/${_TMP_LIB_NAME})
                    endif()
                else()
                    set(_TMP_LIB_NAME ${_internal_lib})
                    if(NOT ${_TMP_LIB_NAME}_FOUND)
                        add_subdirectory(${_internal_lib})
                    endif()
                endif()
                # link
                if(${_TMP_LIB_NAME}_FOUND)
                    if(NOT ${_TMP_LIB_NAME}_INTERFACE)
                        set(_internal_lib_scope ${CWORK_INTERNAL_LIBS_SCOPE})
                    else()
                        set(_internal_lib_scope INTERFACE)
                    endif()
                    if(NOT DEFINED CWORK_LINKED_LIBRARIES OR NOT CWORK_LINKED_LIBRARIES)
                        set(CWORK_LINKED_LIBRARIES ${_internal_lib_scope} ${_TMP_LIB_NAME})
                    else()
                        set(CWORK_LINKED_LIBRARIES ${CWORK_LINKED_LIBRARIES} ${_internal_lib_scope} ${_TMP_LIB_NAME})
                    endif()
                    unset(_internal_lib_scope)
                endif()

                # include directories
                if(${_TMP_LIB_NAME}_INCLUDE_DIRS)
                    set(CWORK_INCLUDE_DIRS ${CWORK_INCLUDE_DIRS} ${${_TMP_LIB_NAME}_INCLUDE_DIRS})
                    foreach(_include_dir IN ITEMS ${${_TMP_LIB_NAME}_INCLUDE_DIRS})
                        include_directories(${_include_dir} ${CMAKE_BINARY_DIR}/${_TMP_LIB_NAME})
                    endforeach()
                endif()
                # public external frameworks for subdirectory
                if(${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    if(CWORK_EXTERN_FRAMEWORKS)
                        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_LIBS} ${${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS})
                    else()
                        set(CWORK_EXTERN_FRAMEWORKS ${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    endif()
                endif()
                
                unset(_TMP_LIB_NAME)
            endforeach()
        endif()
        # custom dependencies
        if(CWORK_CUSTOM_LIBS)
            if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                set(CWORK_INTERNAL_LIBS_SCOPE INTERFACE)
            endif()
            foreach(_internal_lib IN ITEMS ${CWORK_CUSTOM_LIBS})
                set(_TMP_LIB_NAME ${_internal_lib})
                
                # link
                if(${_TMP_LIB_NAME}_FOUND)
                    if(NOT ${_TMP_LIB_NAME}_INTERFACE)
                        set(_internal_lib_scope ${CWORK_INTERNAL_LIBS_SCOPE})
                    else()
                        set(_internal_lib_scope INTERFACE)
                    endif()
                    if(NOT DEFINED CWORK_LINKED_LIBRARIES OR NOT CWORK_LINKED_LIBRARIES)
                        set(CWORK_LINKED_LIBRARIES ${_internal_lib_scope} ${_TMP_LIB_NAME})
                    else()
                        set(CWORK_LINKED_LIBRARIES ${CWORK_LINKED_LIBRARIES} ${_internal_lib_scope} ${_TMP_LIB_NAME})
                    endif()
                    unset(_internal_lib_scope)
                endif()

                # include directories
                if(${_TMP_LIB_NAME}_INCLUDE_DIRS)
                    set(CWORK_INCLUDE_DIRS ${CWORK_INCLUDE_DIRS} ${${_TMP_LIB_NAME}_INCLUDE_DIRS})
                    foreach(_include_dir IN ITEMS ${${_TMP_LIB_NAME}_INCLUDE_DIRS})
                        include_directories(${_include_dir} ${CMAKE_BINARY_DIR}/${_TMP_LIB_NAME})
                    endforeach()
                endif()
                # public external frameworks for subdirectory
                if(${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    if(CWORK_EXTERN_FRAMEWORKS)
                        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_LIBS} ${${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS})
                    else()
                        set(CWORK_EXTERN_FRAMEWORKS ${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    endif()
                endif()
                unset(_TMP_LIB_NAME)
            endforeach()
        endif()
        # expose include directories
        set(${CWORK_PROJECT_NAME}_INCLUDE_DIRS ${CWORK_INCLUDE_DIRS} CACHE INTERNAL "${CWORK_PROJECT_NAME}_INCLUDE_DIRS")
        
        # external dependencies
        if(CWORK_EXTERN_LIBS)
            # find packages + link
            if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                set(CWORK_EXTERN_LIBS_SCOPE INTERFACE)
            endif()
            foreach(_external_lib IN ITEMS ${CWORK_EXTERN_LIBS})
              find_package(${_external_lib} REQUIRED)
              if(${_external_lib}__FOUND AND ${_external_lib}__LINKED)
                  if(NOT DEFINED CWORK_LINKED_LIBRARIES OR NOT CWORK_LINKED_LIBRARIES)
                      set(CWORK_LINKED_LIBRARIES ${CWORK_EXTERN_LIBS_SCOPE} ${${_external_lib}__LINKED})
                  else()
                      set(CWORK_LINKED_LIBRARIES ${CWORK_LINKED_LIBRARIES} ${CWORK_EXTERN_LIBS_SCOPE} ${${_external_lib}__LINKED})
                  endif()
              endif()
            endforeach()
        endif()
        
        
        # external frameworks (some may be set by find_package of external libraries)
        if(CWORK_EXTERN_FRAMEWORKS)
            # expose public external dependencies
            if(${CWORK_EXTERN_FRAMEWORKS_SCOPE} STREQUAL "PUBLIC")
                set(${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} CACHE INTERNAL "${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS")
            endif()
            # link
            if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                set(CWORK_EXTERN_FRAMEWORKS_SCOPE INTERFACE)
            endif()
            foreach(_external_framework IN ITEMS ${CWORK_EXTERN_FRAMEWORKS})
                if(NOT DEFINED CWORK_LINKED_LIBRARIES OR NOT CWORK_LINKED_LIBRARIES)
                    set(CWORK_LINKED_LIBRARIES ${CWORK_EXTERN_FRAMEWORKS_SCOPE} "-framework ${_external_framework}")
                else()
                    set(CWORK_LINKED_LIBRARIES ${CWORK_LINKED_LIBRARIES} ${CWORK_EXTERN_FRAMEWORKS_SCOPE} "-framework ${_external_framework}")
                endif()
            endforeach()
        endif()
        
        # -- PROJECT TARGET DEFINITION --
        
        set(${CWORK_PROJECT_NAME}_FOUND ON CACHE STRING "${CWORK_PROJECT_NAME}_FOUND")
        
        if(${CWORK_PROJECT_NAME}_SOURCE_FILES)
            set(CWORK_PROJECT_SCOPE PUBLIC)
            
            if(${CWORK_BUILD_TYPE} STREQUAL "executable") # executable
                if(${CWORK_PROJECT_NAME}_INCLUDE_FILES)
                    add_executable(${CWORK_PROJECT_NAME} ${${CWORK_PROJECT_NAME}_SOURCE_FILES} ${${CWORK_PROJECT_NAME}_INCLUDE_FILES})
                else()
                    add_executable(${CWORK_PROJECT_NAME} ${${CWORK_PROJECT_NAME}_SOURCE_FILES})
                endif()
            else() # library
                if(${CWORK_BUILD_TYPE} STREQUAL "dynamic")
                    add_library(${CWORK_PROJECT_NAME} SHARED ${${CWORK_PROJECT_NAME}_SOURCE_FILES} ${${CWORK_PROJECT_NAME}_INCLUDE_FILES})
                else()
                    add_library(${CWORK_PROJECT_NAME} STATIC ${${CWORK_PROJECT_NAME}_SOURCE_FILES} ${${CWORK_PROJECT_NAME}_INCLUDE_FILES})
                endif()
            endif()
        else() # header-only library
            set(${CWORK_PROJECT_NAME}_INTERFACE ON CACHE STRING "${CWORK_PROJECT_NAME}_INTERFACE")
            set(CWORK_PROJECT_SCOPE INTERFACE)
            
            if(${CWORK_BUILD_TYPE} STREQUAL "executable")
                message(FATAL_ERROR "Error: Executable projects must contain source files. Cmake will exit.")
            else()
                add_library(${CWORK_PROJECT_NAME} INTERFACE)
            endif()
        endif()

        # link libraries + set include directory
        if(CWORK_LINKED_LIBRARIES)
            message("> linking libraries: ${CWORK_LINKED_LIBRARIES}")
            target_link_libraries(${CWORK_PROJECT_NAME} ${CWORK_LINKED_LIBRARIES})
        endif()
        target_include_directories(${CWORK_PROJECT_NAME} ${CWORK_PROJECT_SCOPE} 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}>
            $<INSTALL_INTERFACE:include>
        )
        # symbols
        if(CWORK_EXPORT_SYMBOLS AND ${CWORK_PROJECT_NAME}_SOURCE_FILES)
            set_target_properties(${CWORK_PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
        endif()
        if(CWORK_BUILD_SYMBOL_FILES)
            create_symbol_files(${CWORK_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/)
        endif()
        
        # -- IDE CLASSIFICATION PROPERTIES --
        
        if(${CWORK_PROJECT_NAME}_SOURCE_FILES)
            if(CWORK_SOLUTION_NAME)
                if(${PROJECT_NAME}_SUBPROJECT_TYPE)
                    set_target_properties(${CWORK_PROJECT_NAME} PROPERTIES FOLDER ${CWORK_SOLUTION_NAME}/${${PROJECT_NAME}_SUBPROJECT_TYPE})
                else()
                    set_target_properties(${CWORK_PROJECT_NAME} PROPERTIES FOLDER ${CWORK_SOLUTION_NAME})
                endif()
            endif()
            set_ide_source_groups(${CMAKE_CURRENT_SOURCE_DIR} ${${CWORK_PROJECT_NAME}_INCLUDE_FILES})
            set_ide_source_groups(${CMAKE_CURRENT_SOURCE_DIR} ${${CWORK_PROJECT_NAME}_SOURCE_FILES})
        else() # interface
            add_custom_target(${CWORK_PROJECT_NAME}__i ALL SOURCES ${${CWORK_PROJECT_NAME}_INCLUDE_FILES} )
            if(CWORK_SOLUTION_NAME)
                set_target_properties(${CWORK_PROJECT_NAME}__i PROPERTIES FOLDER ${CWORK_SOLUTION_NAME})
            endif()
            set_ide_source_groups(${CMAKE_CURRENT_SOURCE_DIR} ${${CWORK_PROJECT_NAME}_INCLUDE_FILES})
        endif()
        
        # -- DOCUMENTATION --
        
        if(CWORK_DOCS AND EXISTS "${cwork_path}/templates/Doxyfile.in" AND NOT TARGET ${CWORK_PROJECT_NAME}.docs)
            find_package(Doxygen)
            if(DOXYGEN_FOUND)
                message("> ${CWORK_PROJECT_NAME}.docs - build ${PROJECT_VERSION} -")
                configure_file("${cwork_path}/templates/doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
                add_custom_target(${CWORK_PROJECT_NAME}.docs 
                    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                    COMMENT "Generating documentation with Doxygen" VERBATIM
                )
                set_target_properties(${CWORK_PROJECT_NAME}.docs PROPERTIES FOLDER docs)
            endif()
        endif()
        
        # -- TESTS --
        
        if(CWORK_TESTS)
            if(NOT ${CWORK_PROJECT_NAME}_TEST_FILES)
                message("> auto-detection of test files...")
                autodetect_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}" "*.[hc]*")
                set(${CWORK_PROJECT_NAME}_TEST_FILES ${CWORK_AUTODETECTED_FILES})
                unset(CWORK_AUTODETECTED_FILES)
            endif()
            if(${CWORK_PROJECT_NAME}_TEST_FILES)
                if(NOT TARGET gtest)
                    find_package(gtest REQUIRED)
                endif()

                # test project
                message("> ${CWORK_PROJECT_NAME}.test - build ${PROJECT_VERSION} -")
                enable_testing()
                add_executable(${CWORK_PROJECT_NAME}.test ${${CWORK_PROJECT_NAME}_TEST_FILES})
                
                # link
                if(NOT ${CWORK_PROJECT_NAME}_INTERFACE)
                    set(CWORK_TESTED_LIB_SCOPE PRIVATE)
                else()
                    set(CWORK_TESTED_LIB_SCOPE INTERFACE)
                endif()
                if(CWORK_LINKED_LIBRARIES)
                    target_link_libraries(${CWORK_PROJECT_NAME}.test 
                        ${CWORK_TESTED_LIB_SCOPE} ${CWORK_PROJECT_NAME}
                        PRIVATE gtest gtest_main
                        ${CWORK_LINKED_LIBRARIES}
                    )
                else()
                    target_link_libraries(${CWORK_PROJECT_NAME}.test 
                        ${CWORK_TESTED_LIB_SCOPE} ${CWORK_PROJECT_NAME}
                        PRIVATE gtest gtest_main
                    )
                endif()
                target_include_directories(${CWORK_PROJECT_NAME}.test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}" "${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}")
                if(${CWORK_BUILD_TYPE} STREQUAL "dynamic")
                    copy_shared_lib(${CWORK_PROJECT_NAME} ${CWORK_PROJECT_NAME}.test)
                endif()

                # IDE properties
                if(CWORK_SOLUTION_NAME)
                    set_target_properties(${CWORK_PROJECT_NAME}.test PROPERTIES FOLDER "${CWORK_SOLUTION_NAME}/tests")
                    set_ide_source_groups(${CMAKE_CURRENT_SOURCE_DIR} ${${CWORK_PROJECT_NAME}_TEST_FILES})
                endif()
                
                add_test(${CWORK_PROJECT_NAME} ${CWORK_PROJECT_NAME}.test)
            endif()
        endif()
        
        # -- INSTALL PROJECT --
        
        if(NOT CMAKE_INSTALL_PREFIX)
            set(CMAKE_INSTALL_PREFIX bin)
        endif()
        set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_SYSTEM_NAME}${CWORK_SYSTEM_ARCH_BITS}/${CWORK_PROJECT_NAME}/${PROJECT_VERSION}")
        
        if(${CWORK_BUILD_TYPE} STREQUAL "executable")
            if(CWORK_BUILD_SYMBOL_FILES)
                install_symbol_files(${CWORK_PROJECT_NAME} ${CMAKE_INSTALL_PREFIX}/bin)
            endif()
        else()
            install(DIRECTORY "${include_dir}/" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
            if(CWORK_BUILD_SYMBOL_FILES)
                install_symbol_files(${CWORK_PROJECT_NAME} ${CMAKE_INSTALL_PREFIX}/lib)
            endif()
        endif()
        
        if(CWORK_DOCS AND DOXYGEN_FOUND)
            install(DIRECTORY "docs/" DESTINATION "${CMAKE_INSTALL_PREFIX}/docs")
        endif()
        
        # -- SUB-PROJECTS --
        
        if (CWORK_TOOLS)
            set(_tools_list "${ARGN}")
            if (_tools_list)
                foreach(_subdir ${_tools_list})
                    message("> ${CWORK_PROJECT_NAME} tools - ${_subdir} -")
                endforeach()
                foreach(_subdir ${_tools_list})
                    add_subdirectory(${_subdir})
                endforeach()
            endif()
            unset(_tools_list)
        endif()
    endmacro()

endif()
