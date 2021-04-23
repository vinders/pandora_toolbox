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
    # │  Wayland packages                                                │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:  Generate protocol files for Wayland display server
    #params: - target_name: name of the project
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
    # │  Android packages                                                │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief:  Generate manifest file for Android app
    #params: - cwork_path: path to cwork tools
    #        - project_name: full project name (solution.project_name)
    #        - project_version: full project version (#.#.#.REV)
    #        - output_dir: output directory
    function(cwork_android_create_manifest cwork_path project_name project_version output_dir)
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
            
            file(REMOVE "${output_dir}/AndroidManifest.xml")
            configure_file("${cwork_path}/templates/AndroidManifest.xml.in" "${output_dir}/AndroidManifest.xml")
        endif()
    endfunction()

endif()
