include(../../../common.pri)

TEMPLATE = lib

CONFIG += plugin

QT += core gui widgets

TARGET = $$qtLibraryTarget(HelloWorldPlugin)

SOURCES += \
        HelloWorldPlugin.cpp 

HEADERS += \
        HelloWorldPlugin.h
        
# Build Folder

DESTDIR = $${PLUGINS_DIR}

# Deploy Plugin Description

# target path = place in the plugins folder
json.path = $${PLUGINS_DIR}
json.files += HelloWorldPlugin.json

INSTALLS += json
COPIES += json
