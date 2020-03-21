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
    configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
    set(SOURCES ${SOURCES} version.h application.rc) 
endif()

add_executable(wpn-xm ${SOURCES})

target_link_libraries(wpn-xm AppCoreLib)
