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
