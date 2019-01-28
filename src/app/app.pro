#
#    WPN-XM Server Control Panel
#
#    Copyright (c) Jens-Andre Koch <jakoch@web.de>
#
#    WPN-XM SCP is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    WPN-XM SCP is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with WPN-XM SCP. If not, see <http://www.gnu.org/licenses/>.
#

message("You are running qmake on app.pro file.")

if(!equals(QT_MAJOR_VERSION, 5)) {
    error("This program can only be compiled with Qt 5.")
}


# emit warnings if you use deprecated features
DEFINES += QT_DEPRECATED_WARNINGS

# disable all deprecated APIs before the following version
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051100

include($${PROJECT_DIR}/project-version.pri)

DEPLOYMENT.display_name = WPN-XM Server Control Panel

CONFIG += qt console c++14

QT += core network widgets

# needed for "createShellLink", see ConfigurationDialog
LIBS += -luuid -lole32 -lshell32
# needed for process and port detection, see ProcessViewerDialog
#LIBS += -liphlpapi -lws2_32 -lwsock32

# ZLIB
INCLUDEPATH += $${LIBS_DIR}/zlib/include
LIBS += -L$${LIBS_DIR}/zlib/lib -lzlib$${DEBUG_EXT}

# QuaZIP
INCLUDEPATH += $${LIBS_DIR}/quazip/include
LIBS += -L$${LIBS_DIR}/quazip/lib -lquazip$${DEBUG_EXT}

# YAML-CPP (static linkage!)
INCLUDEPATH += $${LIBS_DIR}/yaml-cpp/include
LIBS += -L$${LIBS_DIR}/yaml-cpp/lib -llibyaml-cppmd$${DEBUG_EXT}

QMAKE_CXXFLAGS -= -fno-keep-inline-dllexport

HEADERS += \
    main/main.h \
    cli.h \
    config/configurationdialog.h \
    config/nginxaddserverdialog.h \
    config/nginxaddupstreamdialog.h \
    file/filehandling.h \
    file/csv.h \
    file/ini.h \
    file/json.h \
    file/yml.h \
    hostmanager/adddialog.h \
    hostmanager/host.h \
    hostmanager/hostmanagerdialog.h \
    hostmanager/hosttablemodel.h \
    jobscheduler.h \
    mainwindow.h \
    networkutils.h \
    processviewer/AlreadyRunningProcessesDialog.h \
    processviewer/processes.h \
    processviewer/processviewerdialog.h \
    registry/registrymanager.h \
    selfupdater.h \
    servers.h \
    services.h \
    settings.h \
    splashscreen.h \
    tooltips/BalloonTip.h \
    tooltips/LabelWithHoverTooltip.h \
    tooltips/TrayTooltip.h \
    tray.h \
    updater/actioncolumnitemdelegate.h \
    updater/downloadmanager.h \
    updater/package.h \
    updater/softwarecolumnitemdelegate.h \
    updater/updaterdialog.h \
    version.h \
    windowsapi.h

SOURCES += \
    main/main.cpp \
    cli.cpp \   
    config/configurationdialog.cpp \
    config/nginxaddserverdialog.cpp \
    config/nginxaddupstreamdialog.cpp \
    file/csv.cpp \    
    file/filehandling.cpp \
    file/ini.cpp \
    file/json.cpp \
    file/yml.cpp \
    hostmanager/adddialog.cpp \
    hostmanager/host.cpp \
    hostmanager/hostmanagerdialog.cpp \
    hostmanager/hosttablemodel.cpp \
    jobscheduler.cpp \
    mainwindow.cpp \
    networkutils.cpp \
    processviewer/AlreadyRunningProcessesDialog.cpp \
    processviewer/processes.cpp \
    processviewer/processviewerdialog.cpp \
    registry/registrymanager.cpp \
    selfupdater.cpp \
    servers.cpp \
    services.cpp \
    settings.cpp \
    splashscreen.cpp \
    tooltips/BalloonTip.cpp \
    tooltips/LabelWithHoverTooltip.cpp \
    tooltips/TrayTooltip.cpp \
    tray.cpp \
    updater/actioncolumnitemdelegate.cpp \
    updater/downloadmanager.cpp \
    updater/package.cpp \
    updater/softwarecolumnitemdelegate.cpp \
    updater/transferitem.cpp \
    updater/updaterdialog.cpp \
    windowsapi.cpp

RESOURCES += \
    resources/resources.qrc

FORMS += \
    config/configurationdialog.ui \
    config/nginxaddserverdialog.ui \
    config/nginxaddupstreamdialog.ui \
    mainwindow.ui \
    processviewer/processviewerdialog.ui \   
    updater/updaterdialog.ui

# WINDOWS RC-FILE (sets the executable attributes)
exists(C:\Windows\System32\cmd.exe) {
    message("Running on Windows")
    HEADERS += version_localdev.h
    RC_FILE = resources/application_localdev.rc
} else {
    RC_FILE = resources/application.rc
}

# set binary name
CONFIG(release, debug|release): TARGET = wpn-xm
CONFIG(debug, debug|release):   TARGET = wpn-xm-debug

message($$QMAKESPEC) # Determine the platform we are on

!isEmpty($$(TRAVIS)) {
    message("The project is build on Travis: $$(TRAVIS)")
}

linux-g++ {
    message("using linux g++")
}

win32-g++ {
    message("using win32 g++")
}

win32-msvc* {
    message("using win32 msvc")
    #QMAKE_CXXFLAGS += /MP
    QMAKE_CXXFLAGS += /D_CRT_SECURE_NO_WARNINGS
}

*-g++-32 {
    message("using g++-32, adding -msse2 flag")

    QMAKE_CXXFLAGS += -msse2
    QMAKE_CFLAGS += -msse2
}

message("QMAKE_CXXFLAGS is $${QMAKE_CXXFLAGS}")

# Deployment - Automatically Determine AND Copy Dependencies to Build Folder

win32 {
    DEPLOY_COMMAND = $$shell_quote($$shell_path($$[QT_INSTALL_BINS]/windeployqt))

    DEPLOY_OPTIONS = "--no-svg --no-system-d3d-compiler --no-opengl --no-angle --no-opengl-sw"

    CONFIG(debug, debug|release) {
        DEPLOY_TARGET = $$shell_quote($$shell_path($${RELEASE_DIR}/$${TARGET}.exe))
        DEPLOY_OPTIONS += "--debug"
    } else {
        DEPLOY_TARGET = $$shell_quote($$shell_path($${RELEASE_DIR}/$${TARGET}.exe))
        DEPLOY_OPTIONS += "--release"
    }

    message($${DEPLOY_COMMAND} $${DEPLOY_OPTIONS} $${DEPLOY_TARGET})
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_OPTIONS} $${DEPLOY_TARGET}
}

# Build Folder

DESTDIR = $${RELEASE_DIR}

# Deployment - Copy Dependencies

# target path for libraries = place in the same folder as executable
libs.path = $${RELEASE_DIR}

message(Deploying libraries to $${libs.path})

CONFIG(debug, debug|release) { # copy Debug libraries

    libs.files += $${LIBS_DIR}/quazip/bin/quazipd.dll
    libs.files += $${LIBS_DIR}/zlib/bin/zlibd.dll
    #libs.files += $${LIBS_DIR}/yaml-cpp/bin/yaml-cppmd.dll
    libs.files += $${LIBS_DIR}/openssl/bin/libeay32.dll # not av
    libs.files += $${LIBS_DIR}/openssl/bin/ssleay32.dll # not av

} else {  # copy Release libraries

    libs.files += $${LIBS_DIR}/quazip/bin/quazip.dll
    libs.files += $${LIBS_DIR}/zlib/bin/zlib.dll
    #libs.files += $${LIBS_DIR}/yaml-cpp/bin/yaml-cpp.dll
    libs.files += $${LIBS_DIR}/openssl/bin/libeay32.dll
    libs.files += $${LIBS_DIR}/openssl/bin/ssleay32.dll
}

INSTALLS  += libs

# enable file copy feature
# this exists, because INSTALLS doesn't work on Windows
# https://stackoverflow.com/a/54162789/1163786
CONFIG += file_copies
COPIES += libs
