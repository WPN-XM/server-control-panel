message("You are running qmake on scp.pro file.")

TEMPLATE = subdirs

SUBDIRS += src/app
SUBDIRS += src/plugins/hello_world_plugin

# location of the suprojects
app.subdir              = src/app
helloworldplugin.subdir = src/plugins/hello_world_plugin

# what subproject depends on others
app.depends = helloworldplugin

# build the project sequentially as listed in SUBDIRS
#CONFIG += ordered