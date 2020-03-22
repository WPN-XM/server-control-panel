add_definitions(-DAPP_CORE_SHAREDLIBRARY)

#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(SOURCES ${SOURCES} ${CMAKE_SOURCE_DIR}/tests/test.cpp)
#    include_directories(${CMAKE_SOURCE_DIR}/tests)
#endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    config
    file
    hostmanager
    plugins
    processviewer
    registry
    updater
    widgets
)

set(SOURCES ${SOURCES}
    # application core library files
    mainwindow.cpp
    mainwindow.h
    common.h
    cli.cpp
    cli.h
    servers.cpp
    servers.h
    settings.cpp
    settings.h
    splashscreen.cpp
    splashscreen.h
    processes.cpp
    processes.h
    networkutils.cpp
    networkutils.h
    tray.cpp
    tray.h
    windowsapi.h
    windowsapi.cpp
    selfupdater.cpp
    selfupdater.h

    config/configurationdialog.cpp
    config/configurationdialog.h  
    config/nginxaddserverdialog.cpp  
    config/nginxaddserverdialog.h    
    config/nginxaddupstreamdialog.cpp  
    config/nginxaddupstreamdialog.h    
    config/pluginlistdelegate.cpp      
    config/pluginlistdelegate.h
    config/pluginsmanager.h  
    config/pluginsmanager.cpp 

    file/csv.cpp
    file/csv.h
    file/filehandling.cpp
    file/filehandling.h
    file/ini.cpp
    file/ini.h
    file/json.cpp
    file/json.h
    file/yml.cpp
    file/yml.h

    hostmanager/adddialog.cpp
    hostmanager/adddialog.h
    hostmanager/host.cpp
    hostmanager/host.h
    hostmanager/hostmanagerdialog.cpp
    hostmanager/hostmanagerdialog.h
    hostmanager/hosttablemodel.cpp
    hostmanager/hosttablemodel.h

    plugins/global_plugin_interface.h
    plugins/plugininterface.h
    plugins/plugins.cpp
    plugins/plugins.h

    processviewer/AlreadyRunningProcessesDialog.cpp
    processviewer/AlreadyRunningProcessesDialog.h
    processviewer/processviewerdialog.cpp
    processviewer/processviewerdialog.h

    registry/registrymanager.cpp
    registry/registrymanager.h

    updater/actioncolumnitemdelegate.cpp
    updater/actioncolumnitemdelegate.h
    updater/downloadmanager.cpp
    updater/downloadmanager.h
    updater/package.cpp
    updater/package.h
    updater/softwarecolumnitemdelegate.cpp
    updater/softwarecolumnitemdelegate.h
    updater/transferitem.cpp
    updater/updaterdialog.cpp
    updater/updaterdialog.h 

    widgets/dockwidgets/consoledockwidget.cpp
    widgets/dockwidgets/consoledockwidget.h
    widgets/dockwidgets/qclosedockwidget.cpp
    widgets/dockwidgets/qclosedockwidget.h

    widgets/tooltips/BalloonTip.cpp
    widgets/tooltips/BalloonTip.h
    widgets/tooltips/LabelWithHoverTooltip.cpp
    widgets/tooltips/LabelWithHoverTooltip.h
    widgets/tooltips/TrayToolTip.cpp
    widgets/tooltips/TrayToolTip.h
)

set(USERINTERFACE_FILES
    config/configurationdialog.ui
    config/nginxaddserverdialog.ui
    config/nginxaddupstreamdialog.ui
    config/pluginsmanager.ui
    processviewer/processviewerdialog.ui 
    updater/updaterdialog.ui
    mainwindow.ui
)
qt5_wrap_ui(USERINTERFACES ${USERINTERFACE_FILES})

set(RESOURCE_FILES
    data/resources.qrc
)
qt5_add_resources(RESOURCES ${RESOURCE_FILES})

#------------------------------------------------------------------------------
# Build Application Core Library
#------------------------------------------------------------------------------

add_library(AppCoreLib SHARED ${SOURCES} ${USERINTERFACES} ${RESOURCES})

target_link_libraries(AppCoreLib    
    Qt5::Core 
    Qt5::Gui 
    Qt5::Network 
    Qt5::Widgets
    ${YAMLCPP_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${QUAZIP_LIBRARY} # NOT LIBRARIES...
)

target_include_directories(AppCoreLib
   PUBLIC
        ${Qt5_INCLUDE_DIRS}
        ${Qt5Core_INCLUDE_DIRS}
        ${YAMLCPP_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${QUAZIP_INCLUDE_DIR} 

        # we need to add the build target folder to include the file "/src/main/version.h",
        # which is generated from "src/main/version.h.cmake"
        ${PROJECT_BINARY_DIR}/src/main 
)

#------------------------------------------------------------------------------
# Source Groups
#------------------------------------------------------------------------------

source_group("UI" FILES ${USERINTERFACE_FILES})
source_group("Resources" FILES ${RESOURCE_FILES})
