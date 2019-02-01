include(../../../common.pri)

TEMPLATE = lib

CONFIG += plugin

QT += core gui widgets

TARGET = $$qtLibraryTarget(HelloWorldPlugin)

SOURCES += \
        HelloWorldPlugin.cpp 

HEADERS += \
        HelloWorldPlugin.h
        
DISTFILES += HelloWorldPlugin.json

# Build Folder

DESTDIR     = $${PLUGINS_DIR}
