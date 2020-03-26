function(install_thirdparty_libs target files_to_deploy_release files_to_deploy_debug)

    foreach(file ${FILES_TO_DEPLOY_DEBUG} ${FILES_TO_DEPLOY_RELEASE})
        
        get_filename_component(filename "${file}" NAME)
        
        # POST_BUILD step
        add_custom_command(TARGET ${target}
            COMMAND "${CMAKE_COMMAND}" -E
                copy_if_different "${file}" \"$<TARGET_FILE_DIR:${target}>\"
            COMMAND ${CMAKE_COMMAND} -E echo "Copying library: ${filename}"
        )
    
        # install step
        install(FILES ${file} DESTINATION ${CMAKE_INSTALL_PREFIX})
        
    endforeach()

endfunction()