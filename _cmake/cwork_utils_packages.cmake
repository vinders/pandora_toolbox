if(NOT DEFINED CWORK_UTILS_PACKAGES_FOUND)
    set(CWORK_UTILS_PACKAGES_FOUND ON)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Shared libraries                                                │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief: Copy shared library in the directory of the target.
    #params: - lib_name: name of the library to copy
    #        - target_name: name of the destination target
    function(copy_shared_lib lib_name target_name)
        get_target_property(_source_imported ${lib_name} IMPORTED)
        
        if(_source_imported)
          get_target_property(_source_debug ${lib_name} IMPORTED_LOCATION_DEBUG)
          get_target_property(_source_release ${lib_name} IMPORTED_LOCATION_RELEASE)
          get_filename_component(_source_debug_path ${_source_debug} DIRECTORY)
          get_filename_component(_source_release_path ${_source_release} DIRECTORY)
          
          if(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
            get_filename_component(_source_debug_name ${_source_debug} NAME_WE)
            set(_source_debug_name ${_source_debug_name}.dll)
            get_filename_component(_source_release_name ${_source_release} NAME_WE)
            set(_source_release_name ${_source_release_name}.dll)
          else()
            get_filename_component(_source_debug_name ${_source_debug} NAME)
            get_filename_component(_source_release_name ${_source_release} NAME)
          endif()
              
          add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Debug>:${_source_debug_path}/${_source_debug_name}>$<$<CONFIG:Release>:${_source_release_path}/${_source_release_name}> $<TARGET_FILE_DIR:${target_name}>				
          )
          
        else()						
          add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${lib_name}> $<TARGET_FILE_DIR:${target_name}> 			
          )
        endif()
    endfunction() 
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  File generators                                                 │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:  Generate include file with project version
    #params: - cwork_path: path of the directory containing cwork utilities 
    #        - include_dir: path of the directory with include files
    #        - mode: type of version namespaces ("solution" / "project")
    macro(cwork_create_version_header_file cwork_path include_dir project_name mode)
        string(REPLACE "." ";" __PROJECT_NAME_PARTS ${project_name})
        list(GET __PROJECT_NAME_PARTS -1 _LIB_NAME)
        set(__OUTPUT_VERSION_FILE "${include_dir}/${_LIB_NAME}/version.h")
        unset(__PROJECT_NAME_PARTS)
        
        file(REMOVE ${__OUTPUT_VERSION_FILE})
        if(CWORK_SOLUTION_NAME AND ${mode} STREQUAL "solution")
            configure_file("${cwork_path}/templates/version_solution_lib.h.in" ${__OUTPUT_VERSION_FILE})
        else()
            configure_file("${cwork_path}/templates/version_lib.h.in" ${__OUTPUT_VERSION_FILE})
        endif()
        unset(__OUTPUT_VERSION_FILE)
        unset(_LIB_NAME)
    endmacro()
    
    #brief:  Generate dummy source files, to force compilation of header-only library
    #params: - cwork_path: path of the directory containing cwork utilities 
    #        - project_name: name of current project
    #        - use_solution_name: use solution as namespace in generated file (ON/OFF)
    macro(cwork_generate_dummy_sources cwork_path project_name)
        file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/_build/dummy_src.cpp")
        
        set(PROJECT_GENERATED_INCLUDES "/* library headers */")
        foreach(_hpp_file IN ITEMS ${${project_name}_INCLUDE_FILES})
            set(PROJECT_GENERATED_INCLUDES "${PROJECT_GENERATED_INCLUDES}\n#include <${_hpp_file}>")
        endforeach()
        
        configure_file("${cwork_path}/templates/dummy_src.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/_build/dummy_src.cpp")
        set(${project_name}_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/_build/dummy_src.cpp")
    endmacro()
    
    #brief:  Generate protocol files for Wayland display server
    #params: - target_name: name of the project (add_library/executable must already have been called)
    #        - include_dir: project include directory
    function(cwork_wayland_set_protocol_files target_name include_dir)
        find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
        pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.15)
        pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)
        
        macro(cwork_wayland_generate_file target_name src_file output_file)
            add_custom_command(OUTPUT "${output_file}.h"
                COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${src_file}" "${output_file}.h"
                DEPENDS "${src_file}" VERBATIM)
            add_custom_command(OUTPUT "${output_file}.c"
                COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${src_file}" "${output_file}.c"
                DEPENDS "${src_file}" VERBATIM)
            target_sources(${target_name} PRIVATE "${output_file}.h" "${output_file}.c")
        endmacro()

        file(MAKE_DIRECTORY ${include_dir})
        cwork_wayland_generate_file(${target_name} "${WAYLAND_PROTOCOLS_BASE}/stable/xdg-shell/xdg-shell.xml"
                                    "${include_dir}/wayland-xdg-shell-client-protocol")
        cwork_wayland_generate_file(${target_name} "${WAYLAND_PROTOCOLS_BASE}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
                                    "${include_dir}/wayland-xdg-decoration-client-protocol")
        cwork_wayland_generate_file(${target_name} "${WAYLAND_PROTOCOLS_BASE}/stable/viewporter/viewporter.xml"
                                    "${include_dir}/wayland-viewporter-client-protocol")
        cwork_wayland_generate_file(${target_name} "${WAYLAND_PROTOCOLS_BASE}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
                                    "${include_dir}/wayland-relative-pointer-unstable-v1-client-protocol")
        cwork_wayland_generate_file(${target_name} "${WAYLAND_PROTOCOLS_BASE}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
                                    "${include_dir}/wayland-pointer-constraints-unstable-v1-client-protocol")
        cwork_wayland_generate_file(${target_name} "${WAYLAND_PROTOCOLS_BASE}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml"
                                    "${include_dir}/wayland-idle-inhibit-unstable-v1-client-protocol")
    endfunction()
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Resource packages                                               │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:  Generate manifest file for Android app
    #params: - cwork_path: path to cwork tools
    #        - project_name: full project name (solution.project_name)
    #        - project_version: full project version (#.#.#.REV)
    #        - output_dir: output directory
    #        - build_type: executable / dynamic
    #        - has_java: project contains java modules: "true" / "false"
    #        - has_resources: project contains drawable resources: "true" / "false"
    function(cwork_android_create_manifest cwork_path project_name project_version output_dir build_type has_java has_resources)
        if(ANDROID)
            string(REPLACE "." ";" __NAME_PARTS ${project_name})
            list(GET __NAME_PARTS -1 __SUBPROJECT_NAME)
            string(REPLACE "_" " " __PROJECT_LABEL ${__SUBPROJECT_NAME}) # __PROJECT_LABEL (ex: "hardware an@lyzer")
      
            # valid format: A-Za-z0-9_. (with a letter after each dot)
            string(REGEX REPLACE "[ +=*%{}:!?,;'°^\(\)&@#€$£µ\t\r\n/\\-]+" "_" __STRIPPED_NAME "${project_name}")
            string(REGEX REPLACE "[.][0-9]" "\.N" __PROJECT_PACKAGE ${__STRIPPED_NAME}) # __PROJECT_PACKAGE (ex: "pandora.hardware_an_lyzer")
            
            # valid revision: 0-9
            string(REPLACE "." ";" __VERSION_PARTS ${project_version})
            list(GET __VERSION_PARTS -1 __VERSION_LAST_PART)
            string(REGEX REPLACE "[ \t\r\n]+" "" __VERSION_REV ${__VERSION_LAST_PART}) # __VERSION_REV (ex: 138)
            
            # "friendly" version (without revision)
            list(REMOVE_AT __VERSION_PARTS -1)
            string(REPLACE ";" "." __VERSION_NAME "${__VERSION_PARTS}") # __VERSION_NAME (ex: 1.2.1)

            # valid file name
            string(REGEX REPLACE "[:\*?<>/\\]+" "" __LIB_NAME "${project_name}")
            
            # SDK min/target versions
            set(__MIN_SDK_VERSION 24)
            if(ANDROID_PLATFORM)
                string(REGEX MATCH "([0-9]+)" __ANDROID_PLATFORM ${ANDROID_PLATFORM})
                if(NOT CMAKE_MATCH_1)
                    set(__SDK_VERSION 30)
                else()
                    set(__SDK_VERSION ${CMAKE_MATCH_1})
                    if(CMAKE_MATCH_1 LESS ${__MIN_SDK_VERSION})
                        set(__MIN_SDK_VERSION ${__SDK_VERSION})
                    endif()
                endif()
            else()
                set(__SDK_VERSION 30)
            endif()
            
            set(__HAS_CODE ${has_java})
            if(${has_resources} STREQUAL "true")
                set(__APPV7_VERSION ${__SDK_VERSION})
                if(${__APPV7_VERSION} LESS 23)
                    set(__APPV7_VERSION 23)
                elseif(${__APPV7_VERSION} GREATER 28)
                    set(__APPV7_VERSION 28)
                endif()
            endif()
            
            # cleanup
            file(REMOVE "${output_dir}/app/build.gradle")
            file(REMOVE "${output_dir}/app/src/main/AndroidManifest.xml")
            # generate
            if(${has_resources} STREQUAL "true")
                configure_file("${cwork_path}/templates/build.gradle.resources.in" "${output_dir}/app/build.gradle")
            else()
                configure_file("${cwork_path}/templates/build.gradle.in" "${output_dir}/app/build.gradle")
            endif()
            if(${build_type} STREQUAL executable)
                configure_file("${cwork_path}/templates/AndroidManifest.xml.in" "${output_dir}/app/src/main/AndroidManifest.xml")
            else()
                configure_file("${cwork_path}/templates/AndroidManifest.library.xml.in" "${output_dir}/app/src/main/AndroidManifest.xml")
            endif()
        endif()
    endfunction()
    
    #brief:  Copy java modules to Android app directory
    #params: - output_dir: output directory
    #        - ARGN: list of java modules by quads: source_file1 output_dir1 source_name1 output_name1 ...
    function(cwork_android_copy_java_files output_dir)
        foreach(_file ${ARGN})
            if(NOT _SOURCE_PATH)
                set(_SOURCE_PATH "${_file}")
            elseif(NOT _OUTPUT_PATH)
                set(_OUTPUT_PATH "${output_dir}/app/src/main/java/${_file}")
            elseif(NOT _SOURCE_NAME)
                set(_SOURCE_NAME "${_file}")
            else()
                set(_OUTPUT_NAME "${_file}")
                file(COPY "${_SOURCE_PATH}" DESTINATION "${_OUTPUT_PATH}")
                if(NOT ${_SOURCE_NAME} STREQUAL ${_OUTPUT_NAME})
                    file(RENAME "${_OUTPUT_PATH}/${_SOURCE_NAME}" "${_OUTPUT_PATH}/${_OUTPUT_NAME}")
                endif()
                unset(_SOURCE_PATH)
                unset(_OUTPUT_PATH)
                unset(_SOURCE_NAME)
                unset(_OUTPUT_NAME)
            endif()
        endforeach()
    endfunction()
    
    #brief:  Set list of java modules by quads (source_file1 output_dir1 source_name1 output_name1 ...)
    #params: - ARGN: list of java source files
    macro(cwork_set_java_source_quads)
        set(_file_list "${ARGN}")
        if(_file_list)
            string(REGEX REPLACE "[ +=*%{}:!?,;'°^\(\)&@#€$£µ\t\r\n/\\-]+" "_" __STRIPPED_NAME "${PROJECT_NAME}")
            string(REPLACE "." "/" __OUTPUT_PATH ${__STRIPPED_NAME})
            unset(__STRIPPED_NAME)
        
            foreach(_file IN ITEMS ${_file_list})
                string(REGEX REPLACE "[/\\]+" ";" __FILE_PARTS ${_file})
                list(GET __FILE_PARTS -1 __FILE_NAME)
                unset(__FILE_PARTS)
                string(REGEX REPLACE "_impl_andr" "" __OUTPUT_FILE_NAME0 ${__FILE_NAME})
                string(REGEX REPLACE "_impl" "" __OUTPUT_FILE_NAME ${__OUTPUT_FILE_NAME0})
                unset(__OUTPUT_FILE_NAME0)

                set(${PROJECT_NAME}_LANG_${_LANG}_FILES ${${PROJECT_NAME}_LANG_${_LANG}_FILES} "${_file}" "${__OUTPUT_PATH}" "${__FILE_NAME}" "${__OUTPUT_FILE_NAME}")
                unset(__OUTPUT_FILE_NAME)
                unset(__FILE_NAME)
            endforeach()
            unset(__OUTPUT_PATH)
        endif()
    endmacro()
    
    #brief:  Copy resources to build directory (and keep internal directory tree)
    #params: - output_dir: build directory
    #        - ARGN: list of source directories
    function(cwork_copy_resource_files output_dir)
        if(ANDROID)
            set(_RES_OUTPUT_DIR ${output_dir}/app/src/main/res)
        else()
            set(_RES_OUTPUT_DIR ${output_dir})
        endif()
    
        foreach(_dir IN ITEMS ${ARGN})
            autodetect_source_files(${_dir} "*")
            if(CWORK_AUTODETECTED_FILES)
                foreach(_file IN ITEMS ${CWORK_AUTODETECTED_FILES})
                    file(RELATIVE_PATH __RELATIVE_RES_PATH ${_dir} ${_file})
                    string(REPLACE "/" ";" __RELATIVE_RES_PATH_LIST ${__RELATIVE_RES_PATH})
                    list(REMOVE_AT __RELATIVE_RES_PATH_LIST -1)
                    string(REPLACE ";" "/" __RELATIVE_RES_DIR ${__RELATIVE_RES_PATH_LIST})
                    
                    file(COPY ${_file} DESTINATION ${_RES_OUTPUT_DIR}/${__RELATIVE_RES_DIR})
                    unset(__RELATIVE_RES_DIR)
                    unset(__RELATIVE_RES_PATH)
                    unset(__RELATIVE_RES_PATH_LIST)
                endforeach()
                unset(CWORK_AUTODETECTED_FILES)
            endif()
        endforeach()
    endfunction()

endif()
