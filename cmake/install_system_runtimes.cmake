# 2. Copy system runtime libraries

macro(install_system_runtimes target)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_INSTALL_DEBUG_LIBRARIES true)
        set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    endif()

    set(CMAKE_INSTALL_UCRT_LIBRARIES FALSE)

    include(InstallRequiredSystemLibraries)

    foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})    
    get_filename_component(filename "${lib}" NAME)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            copy_if_different "${lib}" \"$<TARGET_FILE_DIR:${target}>\"
        COMMAND ${CMAKE_COMMAND} -E echo "Copying system-lib: ${filename}..."
    )
    endforeach()

endmacro()
