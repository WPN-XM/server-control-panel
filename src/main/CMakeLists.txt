include_directories(
  ../core
  ../core/plugins
  ../core/processviewer
)

set(SOURCES 
    # main application files
    main.cpp
)

if (WIN32)
    set(RES_FILES application.rc)
    configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
    set(SOURCES ${SOURCES} version.h ${RES_FILES})
endif()

#------------------------------------------------------------------------------
# Build and Link Executable
#------------------------------------------------------------------------------

add_executable(wpn-xm ${SOURCES})

target_link_libraries(wpn-xm AppCoreLib)

#------------------------------------------------------------------------------
# Source Groups
#------------------------------------------------------------------------------

source_group("Resources" FILES ${RES_FILES})

#------------------------------------------------------------------------------
# Additional Files to Deploy
#------------------------------------------------------------------------------

# 1. Automatically determine and copy Qt dependencies to build folder using windeployqt

find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QTDIR} ENV QTDIR PATH_SUFFIXES bin)

add_custom_command(TARGET wpn-xm POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${CMAKE_PREFIX_PATH}/bin" "${WINDEPLOYQT_EXECUTABLE}"
            #--verbose 0
            --no-svg
            --no-system-d3d-compiler
            --no-opengl
            --no-compiler-runtime
            --no-angle
            --no-opengl-sw
            \"$<TARGET_FILE:wpn-xm>\"
    COMMENT "Running windeployqt..."
)

# 2. Copy system runtime libraries

set(CMAKE_INSTALL_UCRT_LIBRARIES FALSE)

include(InstallRequiredSystemLibraries)
foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
get_filename_component(filename "${lib}" NAME)
add_custom_command(TARGET wpn-xm POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        copy_if_different "${lib}" \"$<TARGET_FILE_DIR:wpn-xm>\"
    COMMAND ${CMAKE_COMMAND} -E echo "Copying system-lib: ${filename}..."
)
endforeach()

# 3. Copy third-party dependencies into build dir

set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(FILES_TO_DEPLOY_DEBUG
        #"${LIBS_DIR}/curl/bin/libcurl.dll"
        "${LIBS_DIR}/quazip/bin/quazipd.dll"
        "${LIBS_DIR}/zlib/bin/zlibd.dll"
        #"${LIBS_DIR}/yaml-cpp/lib/yaml-cppmd.dll" # no copy, static .lib
        #"${LIBS_DIR}/openssl/bin/libeay32.dll"    # no debug build
        #"${LIBS_DIR}/openssl/bin/ssleay32.dll"    # no debug build
    )
else("Release")
    set(FILES_TO_DEPLOY_RELEASE
        #add_lib_to_deploy("${LIBS_DIR}/curl/bin/libcurl.dll"
        "${LIBS_DIR}/quazip/bin/quazip.dll"
        "${LIBS_DIR}/zlib/bin/zlib.dll"
        #"${LIBS_DIR}/yaml-cpp/lib/yaml-cpp.dll" # no copy, static .lib
        #"${LIBS_DIR}/openssl/bin/libeay32.dll"
        #"${LIBS_DIR}/openssl/bin/ssleay32.dll"
    )
endif()

foreach(file ${FILES_TO_DEPLOY_DEBUG} ${FILES_TO_DEPLOY_RELEASE})
    get_filename_component(filename "${file}" NAME)
    add_custom_command(TARGET wpn-xm POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            copy_if_different "${file}" \"$<TARGET_FILE_DIR:wpn-xm>\"
        COMMAND ${CMAKE_COMMAND} -E echo "Copying: ${filename}..."
    )
endforeach()
