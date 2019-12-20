# CMake version required. 
# This must be the first line! It sets default policies, affecting everything else.
cmake_minimum_required(VERSION 3.8)

# Project name and version
project(wpn-xm VERSION 0.8.6)

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

# Define version tokens used in /src/app/version.h.cmake 
set(APP_VERSION ${PROJECT_VERSION})
set(APP_VERSION_GITREV "${PROJECT_VERSION}.${PROJECT_GIT_REVISION}")
# this needs to be comma separated format 1,2,3,0
set(APP_VERSION_RC "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
string(TIMESTAMP YEAR "%Y")

# Generate "version.h" (replace tokens in version.h.cmake template file)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/version.h.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/src/app/version.h
)

#------------------------------------------------------------------------------
# Policies
#------------------------------------------------------------------------------

cmake_policy(SET CMP0084 NEW) # FindQt

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

# Qt Code-Generators
set(CMAKE_AUTOMOC ON)   # compile metaobject files
set(CMAKE_AUTOUIC ON)   # compile ui files
# set(CMAKE_AUTORCC ON) # compile qrc resource files

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

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# Define C++ Standard to use
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MP")

# Be quiet
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /nologo")

#------------------------------------------------------------------------------
# Linker flags
#------------------------------------------------------------------------------

set(CMAKE_EXE_LINKER_FLAGS "/STACK:5000000 /machine:x64")

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
find_package(zlib REQUIRED)

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

set(CMAKE_CXX_STANDARD_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib wsock32.lib ws2_32.lib")

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

set(SOURCES 
    # application files
    src/app/main/main.cpp
    src/app/main/main.h

    src/app/cli.cpp
    src/app/cli.h
    src/app/servers.cpp
    src/app/servers.h
    src/app/settings.cpp
    src/app/settings.h
    src/app/splashscreen.cpp
    src/app/splashscreen.h
    src/app/networkutils.cpp
    src/app/networkutils.h
    src/app/tray.cpp
    src/app/tray.h
    src/app/windowsapi.h
    src/app/windowsapi.cpp
    src/app/selfupdater.cpp
    src/app/selfupdater.h

    src/app/config/configurationdialog.cpp
    src/app/config/configurationdialog.h  
    src/app/config/nginxaddserverdialog.cpp  
    src/app/config/nginxaddserverdialog.h    
    src/app/config/nginxaddupstreamdialog.cpp  
    src/app/config/nginxaddupstreamdialog.h    
    src/app/config/pluginlistdelegate.cpp      
    src/app/config/pluginlistdelegate.h
    src/app/config/pluginsmanager.h  
    src/app/config/pluginsmanager.cpp 

    src/app/file/csv.cpp
    src/app/file/csv.h
    src/app/file/filehandling.cpp
    src/app/file/filehandling.h
    src/app/file/ini.cpp
    src/app/file/ini.h
    src/app/file/json.cpp
    src/app/file/json.h
    src/app/file/yml.cpp
    src/app/file/yml.h

    src/app/hostmanager/adddialog.cpp
    src/app/hostmanager/adddialog.h
    src/app/hostmanager/host.cpp
    src/app/hostmanager/host.h
    src/app/hostmanager/hostmanagerdialog.cpp
    src/app/hostmanager/hostmanagerdialog.h
    src/app/hostmanager/hosttablemodel.cpp
    src/app/hostmanager/hosttablemodel.h

    src/app/mainwindow.cpp
    src/app/mainwindow.h

    src/app/plugins/global_plugin_interface.h
    src/app/plugins/plugininterface.h
    src/app/plugins/plugins.cpp
    src/app/plugins/plugins.h

    src/app/processviewer/AlreadyRunningProcessesDialog.cpp
    src/app/processviewer/AlreadyRunningProcessesDialog.h
    src/app/processviewer/processes.cpp
    src/app/processviewer/processes.h
    src/app/processviewer/processviewerdialog.cpp
    src/app/processviewer/processviewerdialog.h

    src/app/registry/registrymanager.cpp
    src/app/registry/registrymanager.h

    src/app/updater/actioncolumnitemdelegate.cpp
    src/app/updater/actioncolumnitemdelegate.h
    src/app/updater/downloadmanager.cpp
    src/app/updater/downloadmanager.h
    src/app/updater/package.cpp
    src/app/updater/package.h
    src/app/updater/softwarecolumnitemdelegate.cpp
    src/app/updater/softwarecolumnitemdelegate.h
    src/app/updater/transferitem.cpp
    src/app/updater/updaterdialog.cpp
    src/app/updater/updaterdialog.h 

    src/app/widgets/dockwidgets/consoledockwidget.cpp
    src/app/widgets/dockwidgets/consoledockwidget.h
    src/app/widgets/dockwidgets/qclosedockwidget.cpp
    src/app/widgets/dockwidgets/qclosedockwidget.h

    src/app/widgets/tooltips/BalloonTip.cpp
    src/app/widgets/tooltips/BalloonTip.h
    src/app/widgets/tooltips/LabelWithHoverTooltip.cpp
    src/app/widgets/tooltips/LabelWithHoverTooltip.h
    src/app/widgets/tooltips/TrayToolTip.cpp
    src/app/widgets/tooltips/TrayToolTip.h
)

set(USERINTERFACES
    src/app/config/configurationdialog.ui
    src/app/config/nginxaddserverdialog.ui
    src/app/config/nginxaddupstreamdialog.ui
    src/app/config/pluginsmanager.ui    
    src/app/mainwindow.ui
    src/app/processviewer/processviewerdialog.ui 
    src/app/updater/updaterdialog.ui
)

set(RESOURCES
    src/app/resources/application.rc    
)

qt5_add_resources(QRC "src/app/resources/resources.qrc")

add_executable(wpn-xm
    WIN32 # entrypoint for qtwinmain
    # generated from src/app/version.h.cmake
    ${PROJECT_BINARY_DIR}/src/app/version.h
    ${SOURCES}  
    ${USERINTERFACES}      
    ${RESOURCES}
    ${QRC}    
)

target_include_directories(wpn-xm  
  PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
          ${Qt5_INCLUDE_DIRS}
          ${Qt5Core_INCLUDE_DIRS}
          ${YAMLCPP_INCLUDE_DIR}
          ${ZLIB_INCLUDE_DIR}  
          ${QUAZIP_INCLUDE_DIR}
  PUBLIC ${CMAKE_SOURCE_DIR}/src/app
         # we need to add the build target folder to include /src/app/version.h
         # it's generated from src/app/version.h.cmake
         ${PROJECT_BINARY_DIR}/src/app 
)

target_link_libraries(wpn-xm
    Qt5::WinMain 
    Qt5::Core 
    Qt5::Gui 
    Qt5::Network 
    Qt5::Widgets
    ${YAMLCPP_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${QUAZIP_LIBRARY} # NOT LIBRARIES...
)

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

