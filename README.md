WPN-XM Server Control Panel 
===========================

[![Build status](https://ci.appveyor.com/api/projects/status/sil30bww9uj6x7id/branch/master?svg=true)](https://ci.appveyor.com/project/jakoch/server-control-panel/branch/master)
[![Twitter Follow](https://img.shields.io/twitter/follow/wpnxm.svg?style=social&label=Follow&maxAge=2592000)](https://twitter.com/wpnxm)

The WPN-XM Server Control Panel enables you to control and configure servers on Windows easily.
The main window of the control panel provides start and stop, next to quick access buttons for logs and config files and indicates the process state, the used port and the software version for each server. The control panel provides a tight integration with additionally installed tools, a self-updater, updater for the installed software, a configuration dialog and a process viewer.
The following servers are supported: PHP, Nginx, MariaDb, MongoDb, Redis, Memcached, PostgreSQL.
It's written in C++ using Qt 5.2+, free and open-source under GNU/GPL v3 license.

![WPN-XM Server Control Panel v0.8.0](https://cloud.githubusercontent.com/assets/85608/4353472/9dfe4d10-4233-11e4-96bd-939f82b82869.jpg)

![WPN-XM Server Control Panel v0.8.0 - Main Application Window](https://cloud.githubusercontent.com/assets/85608/4353466/85a395c2-4233-11e4-9ff3-5d7d975e7396.jpg)

## Features

- hybrid Windows application (uh, like, is this possible? YES, apparently)
  - CLI mode
  - GUI mode with MainWindow and TrayMenu
- Startup Checks
  - check for already running processes 
    - with the option to "indicate the processes as running" or "select the processes to shutdown"
  - check for used/blocked ports
    - with the option to "ignore port block" or "shut the processes using the port down"
- Server Control 
  - Nginx      - start, stop, restart, reload
  - PHP        - start, stop, restart
  - MariaDB    - start, stop, restart
  - MongoDB    - start, stop, restart
  - PostgreSQL - start, stop, restart
  - Memcached  - start, stop, restart
  - Redis      - start, stop, restart
- Log File Access
  - one-click "Open Log File"
  - automatically clear log file on start of a server (do not append to existing log file)
- Configuration File Access
  - one-click "Edit Config"
  - select "Editor"
- Server Process Monitoring 
  - indicates the process state and the used port(s)
- Configuration
- Self-Updater
  - the SCP is able to check for a new version and update itself 
- Updater
  - the Updater uses our software registry to "download and install new software" or "update already installed software"
- ProcessViewer
  - lists running processes with icon, name, path, port, pid, ppid
  - allows to search a process by name, pid, port
  - allows to kill a process or process tree
- Supported Configuration File formats:
  - CSV
  - INI
  - JSON  
- Developed on Github
- Continously Integrated using Travis-CI
- Released to Github Releases
  - releases to Github Releases, when pushing a new "git tag"
- Published Build Artifacts
  - one-file Qt application (Engima packaged)
  - multi-file Qt application
  - dependencies

## Build Dependencies

* Qt >= 5.2
* QuaZIP
  * zlib

### Downloading Qt

Install Qt binaries from either the Qt SDK or standalone binary package or just use the Qt Online Installer.

You should be able to find everything at http://www.qt.io/download-open-source/

* Qt Online Installer

  http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe

* Qt 5.7.x - MSVC 2015

  http://download.qt.io/official_releases/qt/5.7/5.7.0/qt-opensource-windows-x86-msvc2015-5.7.0.exe
  http://download.qt.io/official_releases/qt/5.7/5.7.0/qt-opensource-windows-x86-msvc2015_64-5.7.0.exe

* Qt Creator

  http://download.qt.io/official_releases/qtcreator/4.1/4.1.0/qt-creator-opensource-windows-x86-4.1.0.exe

### Installation Steps and Requirements for Qt

-  http://doc.qt.io/qt-5/windows-support.html
-  http://doc.qt.io/qt-5/windows-requirements.html
-  http://doc.qt.io/qt-5/index.html

#### Add Qt folder to PATH

In order to build and use Qt, the PATH environment variable needs to be extended
by adding the Qt installation folder, e.g. "c:\Qt".

* Control Panel -> System -> Advanced -> Environment variables

#### Build the Qt Library

Go to the installation folder, e.g. "c:\Qt" and run "configure" followed by "mingw32-make".

Type configure -help to get a list of all available options.
The Configuration Options for Qt page gives a brief overview of these.

See http://qt-project.org/doc/qt-4.8/configure-options.html

#### Build Qt Debug Libraries

You have to build the libraries to be able to link successfully.

* Start -> Programs -> Qt -> Qt Build Debug Libraries

## Build and Deployment Instructions

### 1 The "/build" directory must contain the Qt dependencies.

We are using the Qt command `windeployqt` to automatically detect and copy the dependencies to the debug/release folder.
You find the list of the files to copy at the end of the `.pro` file, in case you need to copy dependencies manually. 

For deployment we use our [Qt Minimal Deployment Kit](https://github.com/WPN-XM/qt-mini-deploy/). This is a packaged bundle of dependencies and allows a basic drop-in of dependencies, next to the executable.

### 2 Run environment

The run environment should include paths to Qt libraries.

### 3 Build

The easiest way to build the "WPN-XM SCP" is to build it from Qt Creator.

Launch the following commands to build it from command line:
* `qmake wpnxm-servercontrolpanel.pro`
* `mingw32-make -j4`

### How to push a new tag?

	git commit -m "This is the commit message. Tagging v1.2.3"
	git tag 1.2.3
	git push --tags
	git push

## Bugs

If you find a bug in the software, please let us know about it.

Please post the issue to the main project via https://github.com/WPN-XM/WPN-XM/issues/new
