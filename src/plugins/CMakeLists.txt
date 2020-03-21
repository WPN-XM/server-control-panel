set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins")

add_subdirectory(HelloWorldPlugin)

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    add_subdirectory(TestPlugin)
#endif()
