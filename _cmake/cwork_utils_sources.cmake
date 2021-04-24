if(NOT DEFINED CWORK_UTILS_SOURCES_FOUND)
    set(CWORK_UTILS_SOURCES_FOUND ON)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Source files                                                    │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief: Automatically detect source files in directories
    #params: - lib_path: library path containing include/sources/tests directories
    #        - source_dir: path to directory containing files
    #        - ARGN: optional filter for source dir (ex: "*.c*" or "*.h*" or "*.[hc]*")
    #returns: CWORK_AUTODETECTED_FILES
    function(autodetect_source_files source_dir)
        if(EXISTS ${source_dir})
            if(${ARGC} GREATER 1 AND NOT ${ARGV1} STREQUAL "")
                set(_filter ${ARGV1})
            else()
                set(_filter "*.[hc]*")
            endif()
            
            file(GLOB_RECURSE _files "${source_dir}/${_filter}")
            set(CWORK_AUTODETECTED_FILES ${_files} PARENT_SCOPE)
        endif()
    endfunction()
    
    #brief: Automatically detect files in one directory (no recursion in subdirectories)
    #params: - lib_path: library path containing directory
    #        - source_dir: path to directory containing files
    #        - ARGN: optional filter for source dir (ex: "*.c*" or "*.h*" or "*.[hc]*")
    #returns: CWORK_AUTODETECTED_FILES
    function(autodetect_source_files_no_recurse source_dir)
        if(EXISTS ${source_dir})
            if(${ARGC} GREATER 1 AND NOT ${ARGV1} STREQUAL "")
                set(_filter ${ARGV1})
            else()
                set(_filter "*.[hc]*")
            endif()
            
            file(GLOB _files "${source_dir}/${_filter}")
            set(CWORK_AUTODETECTED_FILES ${_files} PARENT_SCOPE)
        endif()
    endfunction()

    # ┌──────────────────────────────────────────────────────────────────┐
    # │  IDE file group properties                                       │
    # └──────────────────────────────────────────────────────────────────┘
    
    #brief: Set source groups for IDE such as Visual Studio
    #params: - root_path: root path of the project (that contains include/src/test/...)
    #        - ARGN: files to add to source groups (variable number)
    function(set_ide_source_groups root_path)
        foreach(_file IN ITEMS ${ARGN})
            get_filename_component(_source_path "${_file}" PATH)
            file(RELATIVE_PATH _source_path_rel "${root_path}" "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
            source_group("${_group_path}" FILES "${_file}")
        endforeach()
    endfunction()

endif()
