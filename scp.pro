message(Loading scp.pro)

TEMPLATE = subdirs

SUBDIRS += src/app
SUBDIRS += src/plugins/hello_world_plugin

# location of the suprojects
helloworldplugin.subdir = src/plugins/hello_world_plugin
app.subdir              = src/app

# what subproject depends on others
app.depends = helloworldplugin

# build the project sequentially as listed in SUBDIRS
#CONFIG += ordered

# disable the creation of debug/release folders in shadow build folders
# https://bugreports.qt.io/browse/QTCREATORBUG-13807
CONFIG -= debug_and_release

# not sure, if this works from a pro file
# if not, diable qml_debug in the project configuration dialog
CONFIG -= qml_debug

# enable file copy feature
# this exists, because INSTALLS doesn't work cross-platform
CONFIG *= file_copies
