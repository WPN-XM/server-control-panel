# CMake version required. 
# This must be the first line! It sets default policies, affecting everything else.
cmake_minimum_required(VERSION 3.8)

# Project name and version
project(wpn-xm VERSION 0.8.6)

#------------------------------------------------------------------------------
# CMake Includes
#------------------------------------------------------------------------------

include(FeatureSummary)

#------------------------------------------------------------------------------
# Version
#------------------------------------------------------------------------------

# todo? add APP_VERSION_PRERELEASE =  alpha, beta, rc

# Git revision
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
  find_package(Git QUIET)
  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_REVISION 
    )
    string(REGEX REPLACE "\n" "" GIT_REVISION "${GIT_REVISION}")
    set(PROJECT_GIT_REVISION "${GIT_REVISION}")
  else()
    message(STATUS "Git revision could not be determined")
  endif()
endif()

# Define version tokens used in /src/main/version.h.cmake 
set(APP_VERSION ${PROJECT_VERSION})
set(APP_VERSION_GITREV "${PROJECT_VERSION}.${PROJECT_GIT_REVISION}")
# this needs to be comma separated format 1,2,3,0
set(APP_VERSION_RC "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
string(TIMESTAMP YEAR "%Y")

# Generate "version.h" (replace tokens in version.h.cmake template file)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/version.h.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/src/main/version.h
)

#------------------------------------------------------------------------------
# Policies
#------------------------------------------------------------------------------

if(POLICY CMP0084)
    cmake_policy(SET CMP0084 NEW) # FindQt
endif()

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # if LIBNAME_ROOT is set, use it for finding LIBNAME
endif()

#------------------------------------------------------------------------------
# Options
#------------------------------------------------------------------------------

option(OPTION_ENABLE_LTO   "Enable link-time optimizations, if supported by the compiler"   OFF)

#------------------------------------------------------------------------------
# Configure
#------------------------------------------------------------------------------

# Reduce build types to debug and release
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 

# Do not allow an in-source-tree build, request an out-of-source-tree build.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  get_filename_component(ProjectDirName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  message(FATAL_ERROR "Please build outside of the source tree!\n                        
        Make an out-of-source-tree folder for building and run CMake there:\n
        mkdir ../_build; cd ../_build; cmake ../${ProjectDirName}}\n
        Before that, cleanup:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Disable in-source builds and modifications to the source tree.
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Set path to our custom CMAKE scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(FindPackageHandleStandardArgs)

# Output dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set Plugin Dir (used by CMakeLists.txt of plugins)
set(APP_INSTALL_PLUGINDIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins")
set(PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/${APP_INSTALL_PLUGINDIR}")

# Qt Code-Generators
set(CMAKE_AUTOMOC ON) # compile metaobject files
#set(CMAKE_AUTORCC ON) # compile qrc resource files
set(CMAKE_AUTOUIC ON) # compile ui files

# Visual Studio to find it's own shit...
#set(CMAKE_MAKE_PROGRAM "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe")

#------------------------------------------------------------------------------
#                                 Defines                                        
#------------------------------------------------------------------------------

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DAPP_DEBUG)
else("Release")
    add_definitions(-DAPP_RELEASE)
endif()

if(MSVC) 
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Qt specific defines
add_definitions(-DQT_USE_QSTRINGBUILDER) # string concat + is turned to %

# emit warnings if you use deprecated features
add_definitions(-DQT_DEPRECATED_WARNINGS)

# disable all deprecated APIs before the following version
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x051100)

# shut the fuck up, you unmaintained "yaml-cpp"
add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

#------------------------------------------------------------------------------
# OS specific
#------------------------------------------------------------------------------

# taken from https://stackoverflow.com/a/40217291
macro(get_WIN32_WINNT version)
    if (CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
        string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
        # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        if ("${verMajor}" MATCHES "10")
            set(verMajor "A")
            string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
        endif ("${verMajor}" MATCHES "10")
        # Remove all remaining '.' characters.
        string(REPLACE "." "" ver ${ver})
        # Prepend each digit with a zero.
        string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
        set(${version} "0x${ver}")
    endif(CMAKE_SYSTEM_VERSION)
endmacro(get_WIN32_WINNT)

get_WIN32_WINNT(winnt_version)
add_definitions(-D_WIN32_WINNT=${winnt_version})

#------------------------------------------------------------------------------
# Compiler flags
#------------------------------------------------------------------------------

# Force C++ standard, do not fall back, do not use (gcc-specific) compiler extensions
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)  
set(CMAKE_CXX_EXTENSIONS OFF)

# Define C++ Standard to use, be quiet
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MP /nologo")

#------------------------------------------------------------------------------
# Linker flags
#------------------------------------------------------------------------------

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:5000000 /machine:x64")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:console")

add_link_options("/nologo")

# ==================================================================================================
# Link time optimizations (LTO)
# ==================================================================================================
if (OPTION_ENABLE_LTO)
    include(CheckIPOSupported)

    check_ipo_supported(RESULT IPO_SUPPORT)

    if (IPO_SUPPORT)
        message(STATUS "LTO support is enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

#------------------------------------------------------------------------------
# Setup Dependencies
#------------------------------------------------------------------------------

message(STATUS "\n== Dependencies:\n")

#
# Mandatory: Qt5
#
set(QT_PATH             "D:\\Qt\\5.13.2\\")
set(CMAKE_PREFIX_PATH   "${QT_PATH}\\msvc2017_64")
set(Qt5_DIR             "${QT_PATH}\\lib\\cmake\\Qt5") 

set(QT_MIN_VERSION "5.12.0")
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Network Widgets)

message(STATUS "    Found Qt version: ${Qt5Core_VERSION_STRING}")
message(STATUS "    Qt Path:          ${QT_PATH}")
message(STATUS "    Qt Include dirs:  ${Qt5Core_INCLUDE_DIRS}")
message(STATUS "    Qt Core libs:     ${Qt5Core_LIBRARIES}")

#
# YAML
#
set(YAMLCPP_DIR "${PROJECT_SOURCE_DIR}/libs/yaml-cpp")
find_package(YAMLCPP REQUIRED) 
#message(STATUS "    YAML-CPP found: ${YAMLCPP_INCLUDE_DIR} ${YAMLCPP_LIBRARY}") 

#
# zlib
#
set(ZLIB_ROOT "${PROJECT_SOURCE_DIR}/libs/zlib")
find_package(ZLIB REQUIRED)

#
# QuaZip
#
set(QUAZIP_DIR "${PROJECT_SOURCE_DIR}/libs/quazip/include")
find_package(Quazip REQUIRED)

#------------------------------------------------------------------------------
# Define the standard link libraries
#------------------------------------------------------------------------------
# Notes:
# 1. "uuid ole32 shell32" - needed for "createShellLink" (see ConfigurationDialog)
# 2. "ws2_32 wsock32"     - needed for "process and port detection" (see ProcessViewerDialog)

#set(CMAKE_CXX_STANDARD_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib wsock32.lib ws2_32.lib")

#------------------------------------------------------------------------------
#                                 Display Status                                         
#------------------------------------------------------------------------------

message("\n== Building ${PROJECT_NAME} version ${PROJECT_VERSION} using build type '${CMAKE_BUILD_TYPE}'.")
message("")
message(STATUS "    Source directory is '${PROJECT_SOURCE_DIR}'.")
message(STATUS "    Build  directory is '${PROJECT_BINARY_DIR}'.")
message(STATUS "    Installation path is '${CMAKE_INSTALL_PREFIX}' (set -DCMAKE_INSTALL_PREFIX=<path> to change).")
message(STATUS "    Dependencies folder: '${PROJECT_SOURCE_DIR}/libs'.")
message("")
message(STATUS "    Generated for config types: '${CMAKE_CONFIGURATION_TYPES}'.")
message("")
message(STATUS "    CMake version is '${CMAKE_VERSION}'.")
message(STATUS "    CXX Compiler ID: '${CMAKE_CXX_COMPILER_ID}'.")
message(STATUS "    CXX Compiler Flags: '${CMAKE_CXX_FLAGS}'.")
message("")

#------------------------------------------------------------------------------
# Build Executable
#------------------------------------------------------------------------------

#add_library(AppCore SHARED 
#    # generated from src/app/version.h.cmake
#    ${PROJECT_BINARY_DIR}/src/main/version.h
#    ${SOURCES} 
#    ${USERINTERFACES}      
#    ${RESOURCES}
#    ${QRC} 
#)

#target_include_directories(AppCore  
#  PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
#          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#          ${Qt5_INCLUDE_DIRS}
#          ${Qt5Core_INCLUDE_DIRS}
#          ${YAMLCPP_INCLUDE_DIR}
#          ${ZLIB_INCLUDE_DIR}  
#          ${QUAZIP_INCLUDE_DIR}
#  PUBLIC ${CMAKE_SOURCE_DIR}/src/app
#         # we need to add the build target folder to include /src/app/version.h
#         # it's generated from src/app/version.h.cmake
#         ${PROJECT_BINARY_DIR}/src/app 
#)
#

#------------------------------------------------------------------------------
# Build executable with main entry point and link AppCore
#------------------------------------------------------------------------------

#add_executable(wpn-xm  
#    #WIN32 # entrypoint for qtwinmain  
#    #Qt5::WinMain   
#    #AppCore 
#)

#target_link_libraries(wpn-xm AppCore)  

#------------------------------------------------------------------------------
# Finally, scan subdirectories for CMakeLists
#------------------------------------------------------------------------------

add_subdirectory(src/core)
add_subdirectory(src/main)
add_subdirectory(src/plugins)

#------------------------------------------------------------------------------
# Add Source Groups for IDE source explorers (e.g. Visual Studio)
#------------------------------------------------------------------------------

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ALL_SOURCES ${SOURCES} ${USERINTERFACES} ${RESOURCES})

foreach(FILE ${ALL_SOURCES}) 
    # Group into "Source Files", "Header Files", "UI", "Resources"
    if ("${FILE}" MATCHES ".*\\.cpp")
       source_group(
           TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
           PREFIX "Source Files - app"
           FILES "${FILE}"
       )
    elseif("${FILE}" MATCHES ".*\\.h")
       source_group(
           TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
           PREFIX "Header Files - app"
           FILES "${FILE}"
       )
    elseif("${FILE}" MATCHES ".*\\.ui")
       source_group(
           TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
           PREFIX "UI - app"
           FILES "${FILE}"
       )
    elseif("${FILE}" MATCHES ".*\\.rc|.*\\.qrc")
       source_group(
           TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
           PREFIX "Resources - app"
           FILES "${FILE}"
       )
    endif()       
endforeach()

# add a group for auto-generated files, too
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "AutoGenerated Files")

#------------------------------------------------------------------------------
# feature summary
#------------------------------------------------------------------------------

feature_summary(INCLUDE_QUIET_PACKAGES WHAT
  PACKAGES_FOUND PACKAGES_NOT_FOUND
  ENABLED_FEATURES DISABLED_FEATURES
)