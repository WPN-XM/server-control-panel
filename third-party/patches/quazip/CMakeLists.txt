project(QuaZip)
cmake_minimum_required(VERSION 2.8)

# CMP0042: Explicitly acknowledge MACOSX_RPATH
cmake_policy(SET CMP0042 NEW)

find_package(Qt5Core)

set(CMAKE_CXX_STANDARD 11)
set(QTCORE_LIBRARIES ${Qt5Core_LIBRARIES})
set(QUAZIP_LIB_VERSION_SUFFIX 5)
# if there is no QT_ROOT, try to deduce it from Qt QtCore include
if("${QT_ROOT}" STREQUAL "")
    set(QT_ROOT ${QT_QTCORE_INCLUDE_DIR}/../..)
endif()
include_directories(${Qt5Core_INCLUDE_DIRS})

macro(qt_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
endmacro()

find_package(ZLIB)

# All build libraries are moved to this directory
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)

set(QUAZIP_LIB_TARGET_NAME quazip${QUAZIP_LIB_VERSION_SUFFIX} CACHE
    INTERNAL "Target name of libquazip" FORCE)

add_subdirectory(quazip)

install(FILES FindQuaZip.cmake RENAME FindQuaZip${QUAZIP_LIB_VERSION_SUFFIX}.cmake DESTINATION ${CMAKE_ROOT}/Modules)