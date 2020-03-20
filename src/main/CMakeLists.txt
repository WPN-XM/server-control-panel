include_directories(
  ../core
)

set(SOURCES 
    # main application files
    main.cpp
)

if (WIN32)
    set(SOURCES ${SOURCES} application.rc)
    configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
    set(SOURCES ${SOURCES} version.h)
endif()

add_executable(wpn-xm ${SOURCES})

target_link_libraries(wpn-xm AppCoreLib)