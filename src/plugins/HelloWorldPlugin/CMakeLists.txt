#------------------------------------------------------------------------------
# Define Sources
#------------------------------------------------------------------------------

set(SOURCES
    HelloWorldPlugin.cpp
    ConfigDialog.cpp
)

set(UI_FILES
    ConfigDialog.ui
)
qt5_wrap_ui(UI ${UI_FILES})

#set(RES_FILES
#    HelloWorldPlugin.qrc
#)
#qt5_add_resources(RES ${RES_FILES})

#------------------------------------------------------------------------------
# Create Library: build, link, install
#------------------------------------------------------------------------------

add_library(HelloWorldPlugin MODULE ${SOURCES} ${UI}) #${RES}

target_link_libraries(HelloWorldPlugin AppCoreLib)

install(TARGETS HelloWorldPlugin DESTINATION ${APP_INSTALL_PLUGINDIR})

#------------------------------------------------------------------------------
# Install additional files
#------------------------------------------------------------------------------

# TODO deploy/copy plugin description json

#------------------------------------------------------------------------------
# Source Groups
#------------------------------------------------------------------------------

source_group("UI" FILES ${UI_FILES})
source_group("Resource" FILES ${RES_FILES})
