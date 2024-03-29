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

if(NOT DEFINED CWORK_SUBPROJECTS_FOUND)
    set(CWORK_SUBPROJECTS_FOUND ON)
    
    include(${CMAKE_CURRENT_LIST_DIR}/cwork_utils_packages.cmake)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Sub-project detection and including (macros)                    │
    # └──────────────────────────────────────────────────────────────────┘
    # values required: - CWORK_PROJECT_NAME
    #                  - CWORK_SOLUTION_NAME
    #                  - CWORK_SOLUTION_PATH
    #                  - CMAKE_CURRENT_SOURCE_DIR
    #                  - CMAKE_BINARY_DIR
    #                  - PROJECT_BINARY_DIR
    # optional values: - CWORK_CUSTOM_SOLUTION_EMBEDDED
    #                  - CWORK_LINUX_WAYLAND
    #                  - ${CWORK_PROJECT_NAME}_INTERNAL_LIBS
    #                  - ${CWORK_PROJECT_NAME}_CUSTOM_LIBS
    #                  - ${CWORK_PROJECT_NAME}_EXTERN_LIBS
    #                  - ${CWORK_PROJECT_NAME}_EXTERN_LIBS_SCOPE
    #                  - ${CWORK_PROJECT_NAME}_TESTS_EXTERN_LIBS
    #                  - ${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS
    #                  - ${CWORK_PROJECT_NAME}_EXTERN_FRAMEWORKS_SCOPE
    #                  - ${CWORK_PROJECT_NAME}_LANG_JAVA_FILES
    #                  - ${CWORK_PROJECT_NAME}_RESOURCE_DIRS
    
    #brief:  Set variables required for 'cwork_include_...' macros
    #params: - include_dir: directory with include files for current project
    macro(cwork_set_variables_for_includes include_dir)
        set(CWORK_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}")
        
        if(${CWORK_PROJECT_NAME}_INTERNAL_LIBS)
            set(CWORK_INTERNAL_LIBS ${${CWORK_PROJECT_NAME}_INTERNAL_LIBS})
            set(CWORK_INTERNAL_LIBS_SCOPE PUBLIC)
            unset(${CWORK_PROJECT_NAME}_INTERNAL_LIBS)
        endif()
        if(${CWORK_PROJECT_NAME}_CUSTOM_LIBS)
            set(CWORK_CUSTOM_LIBS ${${CWORK_PROJECT_NAME}_CUSTOM_LIBS})
            set(CWORK_CUSTOM_LIBS_SCOPE PUBLIC)
            unset(${CWORK_PROJECT_NAME}_CUSTOM_LIBS)
        endif()
        if(${CWORK_PROJECT_NAME}_EXTERN_LIBS)
            set(CWORK_EXTERN_LIBS ${${CWORK_PROJECT_NAME}_EXTERN_LIBS})
            set(CWORK_EXTERN_LIBS_SCOPE ${${CWORK_PROJECT_NAME}_EXTERN_LIBS_SCOPE})
            unset(${CWORK_PROJECT_NAME}_EXTERN_LIBS)
            unset(${CWORK_PROJECT_NAME}_EXTERN_LIBS_SCOPE)
        endif()
        if(CWORK_TESTS AND ${CWORK_PROJECT_NAME}_TESTS_EXTERN_LIBS AND NOT CWORK_CUSTOM_SOLUTION_EMBEDDED)
            set(CWORK_TESTS_EXTERN_LIBS ${${CWORK_PROJECT_NAME}_TESTS_EXTERN_LIBS})
            set(CWORK_TESTS_EXTERN_LIBS_SCOPE PUBLIC)
            unset(${CWORK_PROJECT_NAME}_TESTS_EXTERN_LIBS)
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
        if(CWORK_INCLUDED_LIBRARIES)
            unset(CWORK_INCLUDED_LIBRARIES)
        endif()
    endmacro()
    
    #brief:  Set variable for target_include_directories + expose value for parent projects
    #remarks: should be called AFTER calling all macros 'cwork_include_...'
    #returns: - CWORK_INCLUDED_LIBRARIES
    #         - CWORK_LINKED_LIBRARIES
    macro(cwork_define_target_include_directories)
        set(${CWORK_PROJECT_NAME}_INCLUDE_DIRS ${CWORK_INCLUDE_DIRS} CACHE INTERNAL "${CWORK_PROJECT_NAME}_INCLUDE_DIRS")
        if(NOT DEFINED CWORK_INCLUDED_LIBRARIES OR NOT CWORK_INCLUDED_LIBRARIES)
            set(CWORK_INCLUDED_LIBRARIES ${CWORK_INCLUDE_DIRS})
        else()
            set(CWORK_INCLUDED_LIBRARIES ${CWORK_INCLUDED_LIBRARIES} ${CWORK_INCLUDE_DIRS})
        endif()
    endmacro()
    
    #brief: Add internal libraries (same solution) to current project
    macro(cwork_include_internal_libs)
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
                endif()
                # public external frameworks for subdirectory
                if(${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    if(CWORK_EXTERN_FRAMEWORKS)
                        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} ${${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS})
                    else()
                        set(CWORK_EXTERN_FRAMEWORKS ${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    endif()
                endif()
                # java modules
                if(${_TMP_LIB_NAME}_LANG_JAVA_FILES)
                    if(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES)
                        set(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES ${${CWORK_PROJECT_NAME}_LANG_JAVA_FILES} ${${_TMP_LIB_NAME}_LANG_JAVA_FILES})
                    else()
                        set(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES ${${_TMP_LIB_NAME}_LANG_JAVA_FILES})
                    endif()
                endif()
                # resources
                if(${_TMP_LIB_NAME}_RESOURCE_DIRS)
                    if(${CWORK_PROJECT_NAME}_RESOURCE_DIRS)
                        set(${CWORK_PROJECT_NAME}_RESOURCE_DIRS ${${_TMP_LIB_NAME}_RESOURCE_DIRS} ${${CWORK_PROJECT_NAME}_RESOURCE_DIRS})
                    else()
                        set(${CWORK_PROJECT_NAME}_RESOURCE_DIRS ${${_TMP_LIB_NAME}_RESOURCE_DIRS})
                    endif()
                endif()
                # embedded resources (only on Windows -> added to each project sources, unless header-only project
                if((WIN32 OR WIN64 OR _WIN32 OR _WIN64) AND ${_TMP_LIB_NAME}_INTERFACE)
                    if(${_TMP_LIB_NAME}_EMBED_RESOURCE_DIRS)
                        if(${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS)
                            set(${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS ${${_TMP_LIB_NAME}_EMBED_RESOURCE_DIRS} ${${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS})
                        else()
                            set(${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS ${${_TMP_LIB_NAME}_EMBED_RESOURCE_DIRS})
                        endif()
                    endif()
                endif()
                unset(_TMP_LIB_NAME)
            endforeach()
        endif()
    endmacro()
    
    #brief: Add custom libraries (other solution, same repository) to current project
    macro(cwork_include_custom_libs)
        if(CWORK_CUSTOM_LIBS)
            if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                set(CWORK_CUSTOM_LIBS_SCOPE INTERFACE)
            endif()
            foreach(_internal_lib IN ITEMS ${CWORK_CUSTOM_LIBS})
                set(_TMP_LIB_NAME ${_internal_lib})
                message("> custom dependency: ${_TMP_LIB_NAME}")
                
                # link
                if(${_TMP_LIB_NAME}_FOUND)
                    if(NOT ${_TMP_LIB_NAME}_INTERFACE)
                        set(_custom_lib_scope ${CWORK_CUSTOM_LIBS_SCOPE})
                    else()
                        set(_custom_lib_scope INTERFACE)
                    endif()
                    if(NOT DEFINED CWORK_LINKED_LIBRARIES OR NOT CWORK_LINKED_LIBRARIES)
                        set(CWORK_LINKED_LIBRARIES ${_custom_lib_scope} ${_TMP_LIB_NAME})
                    else()
                        set(CWORK_LINKED_LIBRARIES ${CWORK_LINKED_LIBRARIES} ${_custom_lib_scope} ${_TMP_LIB_NAME})
                    endif()
                    unset(_custom_lib_scope)
                endif()

                # include directories
                if(${_TMP_LIB_NAME}_INCLUDE_DIRS)
                    set(CWORK_INCLUDE_DIRS ${CWORK_INCLUDE_DIRS} ${${_TMP_LIB_NAME}_INCLUDE_DIRS})
                endif()
                # public external frameworks for subdirectory
                if(${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    if(CWORK_EXTERN_FRAMEWORKS)
                        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} ${${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS})
                    else()
                        set(CWORK_EXTERN_FRAMEWORKS ${_TMP_LIB_NAME}_EXTERN_FRAMEWORKS)
                    endif()
                endif()
                # java modules
                if(${_TMP_LIB_NAME}_LANG_JAVA_FILES)
                    if(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES)
                        set(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES ${${CWORK_PROJECT_NAME}_LANG_JAVA_FILES} ${${_TMP_LIB_NAME}_LANG_JAVA_FILES})
                    else()
                        set(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES ${${_TMP_LIB_NAME}_LANG_JAVA_FILES})
                    endif()
                endif()
                # resources
                if(${_TMP_LIB_NAME}_RESOURCE_DIRS)
                    if(${CWORK_PROJECT_NAME}_RESOURCE_DIRS)
                        set(${CWORK_PROJECT_NAME}_RESOURCE_DIRS ${${_TMP_LIB_NAME}_RESOURCE_DIRS} ${${CWORK_PROJECT_NAME}_RESOURCE_DIRS})
                    else()
                        set(${CWORK_PROJECT_NAME}_RESOURCE_DIRS ${${_TMP_LIB_NAME}_RESOURCE_DIRS})
                    endif()
                endif()
                # embedded resources (only on Windows -> added to each project sources, unless header-only project
                if((WIN32 OR WIN64 OR _WIN32 OR _WIN64) AND ${_TMP_LIB_NAME}_INTERFACE)
                    if(${_TMP_LIB_NAME}_EMBED_RESOURCE_DIRS)
                        if(${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS)
                            set(${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS ${${_TMP_LIB_NAME}_EMBED_RESOURCE_DIRS} ${${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS})
                        else()
                            set(${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS ${${_TMP_LIB_NAME}_EMBED_RESOURCE_DIRS})
                        endif()
                    endif()
                endif()
                unset(_TMP_LIB_NAME)
            endforeach()
        endif()
    endmacro()
    
    #brief: Add extern libraries (linked) to current project
    macro(cwork_include_extern_libs)
        if(CWORK_EXTERN_LIBS)
            # find packages + link
            if(NOT ${CWORK_PROJECT_NAME}_SOURCE_FILES)
                set(CWORK_EXTERN_LIBS_SCOPE INTERFACE)
            endif()
            foreach(_external_lib IN ITEMS ${CWORK_EXTERN_LIBS})
                if(_external_lib STREQUAL "public")
                    if(${CWORK_PROJECT_NAME}_SOURCE_FILES)
                        set(CWORK_EXTERN_LIBS_SCOPE PUBLIC)
                    endif()
                elseif(_external_lib STREQUAL "private")
                    if(${CWORK_PROJECT_NAME}_SOURCE_FILES)
                        set(CWORK_EXTERN_LIBS_SCOPE PRIVATE)
                    endif()
                else()
                    find_package(${_external_lib} REQUIRED)
                    if(${_external_lib}__FOUND)
                        if(${_external_lib}__LINKED)
                            if(NOT DEFINED CWORK_LINKED_LIBRARIES OR NOT CWORK_LINKED_LIBRARIES)
                                set(CWORK_LINKED_LIBRARIES ${CWORK_EXTERN_LIBS_SCOPE} ${${_external_lib}__LINKED})
                            else()
                                set(CWORK_LINKED_LIBRARIES ${CWORK_LINKED_LIBRARIES} ${CWORK_EXTERN_LIBS_SCOPE} ${${_external_lib}__LINKED})
                            endif()
                        endif()
                        if(${_external_lib}__INCLUDE)
                            if(NOT DEFINED CWORK_INCLUDED_LIBRARIES OR NOT CWORK_INCLUDED_LIBRARIES)
                                set(CWORK_INCLUDED_LIBRARIES ${${_external_lib}__INCLUDE})
                            else()
                                set(CWORK_INCLUDED_LIBRARIES ${CWORK_INCLUDED_LIBRARIES} ${${_external_lib}__INCLUDE})
                            endif()
                        endif()
                        if(${_external_lib}__SOURCE)
                            set(${CWORK_PROJECT_NAME}_SOURCE_FILES ${${CWORK_PROJECT_NAME}_SOURCE_FILES} ${${_external_lib}__SOURCE})
                        endif()
                    endif()
                endif()
            endforeach()
        endif()
    endmacro()
    
    #brief: Add extern libraries (linked), only for unit tests
    macro(cwork_include_extern_test_libs)
        if(CWORK_TESTS_EXTERN_LIBS)
            foreach(_external_lib IN ITEMS ${CWORK_TESTS_EXTERN_LIBS})
                find_package(${_external_lib} REQUIRED)
                if(${_external_lib}__FOUND AND ${_external_lib}__LINKED)
                    if(NOT DEFINED CWORK_TESTS_LINKED_LIBRARIES OR NOT CWORK_TESTS_LINKED_LIBRARIES)
                        set(CWORK_TESTS_LINKED_LIBRARIES ${CWORK_TESTS_EXTERN_LIBS_SCOPE} ${${_external_lib}__LINKED})
                    else()
                        set(CWORK_TESTS_LINKED_LIBRARIES ${CWORK_TESTS_LINKED_LIBRARIES} ${CWORK_TESTS_EXTERN_LIBS_SCOPE} ${${_external_lib}__LINKED})
                    endif()
                endif()
                if(${_external_lib}__FOUND AND ${_external_lib}__INCLUDE)
                    if(NOT DEFINED CWORK_TESTS_INCLUDED_LIBRARIES OR NOT CWORK_TESTS_INCLUDED_LIBRARIES)
                        set(CWORK_TESTS_INCLUDED_LIBRARIES ${${_external_lib}__INCLUDE})
                    else()
                        set(CWORK_TESTS_INCLUDED_LIBRARIES ${CWORK_TESTS_INCLUDED_LIBRARIES} ${${_external_lib}__INCLUDE})
                    endif()
                endif()
            endforeach()
        endif()
    endmacro()
    
    #brief: Add extern frameworks (linked) to current project (some may be set by find_package of external libraries -> call this one after it)
    macro(cwork_include_extern_frameworks)
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
    endmacro()
    
    #brief: Add project resources (copy to build directory, to app directory, or add sources, depending on platform)
    macro(cwork_insert_resources cwork_path build_type)
        # windows: add embedded resources to source files (if not header-only library)
        if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
            if(${CWORK_PROJECT_NAME}_SOURCE_FILES AND ${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS)
                message("> including project resources...")
                foreach(_dir IN ITEMS ${${CWORK_PROJECT_NAME}_EMBED_RESOURCE_DIRS})
                    autodetect_source_files(${_dir} "*")
                    if(CWORK_AUTODETECTED_FILES)
                        set(${CWORK_PROJECT_NAME}_SOURCE_FILES ${${CWORK_PROJECT_NAME}_SOURCE_FILES} ${CWORK_AUTODETECTED_FILES})
                        unset(CWORK_AUTODETECTED_FILES)
                    endif()
                endforeach()
            endif()
        endif()
        # windows/linux/unix/bsd/mac/ios: copy resources near build target (if executable/shared-lib)
        # android: copy resources in app tree (if executable/shared-lib)
        if(${build_type} STREQUAL "dynamic" OR ${build_type} STREQUAL "executable" OR ${build_type} STREQUAL "console")
            if(${CWORK_PROJECT_NAME}_RESOURCE_DIRS)
                message("> copying project resources...")
                cwork_copy_resource_files("${PROJECT_BINARY_DIR}" ${${CWORK_PROJECT_NAME}_RESOURCE_DIRS})
            endif()
            
            # android app tree: manifest + build script + java modules
            if(ANDROID)
                # create manifest + build.gradle script
                if(${CWORK_PROJECT_NAME}_IS_ANDROID_APP)
                    set(__ANDROID_MANIFEST_TYPE executable)
                else()
                    set(__ANDROID_MANIFEST_TYPE dynamic)
                endif()
                if(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES)
                    set(__ANDROID_MANIFEST_JAVA_LIBS "true")
                else()
                    set(__ANDROID_MANIFEST_JAVA_LIBS "false")
                endif()
                if(${CWORK_PROJECT_NAME}_RESOURCE_DIRS)
                    set(__ANDROID_MANIFEST_RES "true")
                else()
                    set(__ANDROID_MANIFEST_RES "false")
                endif()
                cwork_android_create_manifest(${cwork_path} ${CWORK_PROJECT_NAME} ${PROJECT_VERSION} "${PROJECT_BINARY_DIR}" 
                                              ${__ANDROID_MANIFEST_TYPE} ${__ANDROID_MANIFEST_JAVA_LIBS} ${__ANDROID_MANIFEST_RES})
                unset(__ANDROID_MANIFEST_JAVA_LIBS)
                unset(__ANDROID_MANIFEST_RES)
                unset(__ANDROID_MANIFEST_TYPE)
                
                # copy java modules to java directory
                if(${CWORK_PROJECT_NAME}_LANG_JAVA_FILES)
                    cwork_android_copy_java_files(${PROJECT_BINARY_DIR} ${${CWORK_PROJECT_NAME}_LANG_JAVA_FILES})
                endif()
            endif()
        endif()
    endmacro()
    
endif()
