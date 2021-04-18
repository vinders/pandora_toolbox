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

endif()
