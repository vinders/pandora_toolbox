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

endif()
