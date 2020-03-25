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
# Build + Link
#------------------------------------------------------------------------------

add_executable(wpn-xm ${SOURCES})

target_link_libraries(wpn-xm AppCore)

#------------------------------------------------------------------------------
# Install
#------------------------------------------------------------------------------

install(TARGETS wpn-xm RUNTIME DESTINATION "${APP_INSTALL_DIRECTORY}")

#------------------------------------------------------------------------------
# Install Additional Files
#------------------------------------------------------------------------------

windeployqt(wpn-xm)

# Copy third-party dependencies into build dir

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
    add_custom_command(TARGET wpn-xm
        COMMAND "${CMAKE_COMMAND}" -E
            copy_if_different "${file}" \"$<TARGET_FILE_DIR:wpn-xm>\"
        COMMAND ${CMAKE_COMMAND} -E echo "Copying library: ${filename}"
    )
endforeach()

#------------------------------------------------------------------------------
# Source Groups
#------------------------------------------------------------------------------

source_group("Resources" FILES ${RES_FILES})

