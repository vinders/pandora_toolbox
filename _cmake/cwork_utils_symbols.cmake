if(NOT DEFINED CWORK_UTILS_SYMBOLS_FOUND)
    set(CWORK_UTILS_SYMBOLS_FOUND ON)
    
    # ┌──────────────────────────────────────────────────────────────────┐
    # │  Symbol files                                                    │
    # └──────────────────────────────────────────────────────────────────┘

    #brief: Set compile flags to generate symbol files (generate PDB in Visual Studio, extract symbols from libraries in GCC)
    #params: - target_name: project name of the target
    #        - ARGN: deployment directory (optional)
    function(create_symbol_files target_name)
        if(MSVC)
            set(_DEBUG_SYMBOLS_PDB   "$<TARGET_FILE_DIR:${target_name}>/${target_name}${CMAKE_DEBUG_POSTFIX}.pdb")
            set(_RELEASE_SYMBOLS_PDB "$<TARGET_FILE_DIR:${target_name}>/${target_name}.pdb")
            target_compile_options(${target_name} PRIVATE "$<$<CONFIG:Debug>:/Fd${_DEBUG_SYMBOLS_PDB}>")
            target_compile_options(${target_name} PRIVATE "$<$<CONFIG:Release>:/Fd${_RELEASE_SYMBOLS_PDB}>")
            unset(_DEBUG_SYMBOLS_PDB)
            unset(_RELEASE_SYMBOLS_PDB)
        
        elseif(CMAKE_COMPILER_IS_GNUCXX)
            set(_SYMBOL_FILE "$<TARGET_FILE:${target_name}>")
            set(_SYMBOL_FILE_NAME "$<TARGET_FILE_NAME:${target_name}>")
            set(_SYMBOL_FILE_DIR "$<TARGET_FILE_DIR:${target_name}>")
            set(_DEBUG_SYMBOLS_FILE  "${_SYMBOL_FILE_NAME}$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>.symbols")

            add_custom_command(TARGET ${target_name} POST_BUILD 
                COMMAND objcopy --only-keep-debug ${_SYMBOL_FILE} "${_SYMBOL_FILE_DIR}/${_DEBUG_SYMBOLS_FILE}"
            )
            add_custom_command(TARGET ${target_name} POST_BUILD 
                COMMAND strip -g ${_SYMBOL_FILE}
            )
            add_custom_command(TARGET ${target_name} POST_BUILD 
                COMMAND cd ${_SYMBOL_FILE_DIR} && objcopy --add-gnu-debuglink=${_DEBUG_SYMBOLS_FILE} ${_SYMBOL_FILE}
            )
            if(${ARGC} GREATER 1) # deployed to other directory
              add_custom_command(TARGET ${target_name} POST_BUILD    
                COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGV1}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_SYMBOL_FILE_DIR}/${_DEBUG_SYMBOLS_FILE}" ${ARGV1}
              )
            endif()
        endif()
    endfunction()
    
    #brief: Install symbol files (PDB in Visual Studio, extracted symbol files in GCC)
    #params: - target_name: project name of the target
    #        - destination_path: destination directory
    function(install_symbol_files target_name destination_path)
        if(MSVC)
            install(FILES "$<TARGET_FILE_DIR:${target_name}>/${target_name}${CMAKE_DEBUG_POSTFIX}.pdb" DESTINATION ${destination_path} CONFIGURATIONS Debug OPTIONAL)
            install(FILES "$<TARGET_FILE_DIR:${target_name}>/${target_name}.pdb" DESTINATION ${destination_path} CONFIGURATIONS Release OPTIONAL)
        
        elseif(CMAKE_COMPILER_IS_GNUCXX)
            install(FILES "$<TARGET_FILE:${target_name}>${CMAKE_DEBUG_POSTFIX}.symbols" DESTINATION ${destination_path} CONFIGURATIONS Debug)
            install(FILES "$<TARGET_FILE:${target_name}>.symbols" DESTINATION ${destination_path} CONFIGURATIONS Release)
        endif()
    endfunction()

endif()
