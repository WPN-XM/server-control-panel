find_package(Qt5Core REQUIRED)


# 1. set WINDEPLOYQT_BUILD_TYPE_ARG
# this is passed as argument to windeployqt so that release/debug libs are deployed

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  set(WINDEPLOYQT_BUILD_TYPE_ARG "--debug")
elseif( CMAKE_BUILD_TYPE STREQUAL "Release")
   set(WINDEPLOYQT_BUILD_TYPE_ARG "--release")
elseif( CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(WINDEPLOYQT_BUILD_TYPE_ARG "--release-with-debug-info")
endif()

# 2. get absolute path to qmake, then use it to find windeployqt executable

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# 3. windeploy qt function

function(windeployqt target)

    if(NOT target)
        message(FATAL_ERROR "Please specify a build TARGET for windeployqt.")
    endif()

    # POST_BUILD step
    # - after build, we have a bin/lib for analyzing qt dependencies
    # - we run windeployqt on target and deploy Qt libs

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
        COMMAND set PATH="${_qt_bin_dir}"
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"          
                --verbose 1
                ${WINDEPLOYQT_BUILD_TYPE_ARG}
                --no-svg
                --no-angle
                --no-opengl
                --no-opengl-sw
                --no-compiler-runtime
                --no-system-d3d-compiler
                \"$<TARGET_FILE:${target}>\"
        COMMENT "Deploying Qt libraries using windeployqt for compilation target '${target}' ..."
    )

    # INSTALL step
    # copy windeployqt deployment directory from build directory to installation folder
    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

endfunction()

mark_as_advanced(WINDEPLOYQT_EXECUTABLE)
