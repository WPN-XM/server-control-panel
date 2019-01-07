# WPN-XM Server Control Panel 

[![Build status](https://ci.appveyor.com/api/projects/status/sil30bww9uj6x7id/branch/master?svg=true)](https://ci.appveyor.com/project/jakoch/server-control-panel/branch/master)
[![Twitter Follow](https://img.shields.io/twitter/follow/wpnxm.svg?style=social&label=Follow&maxAge=2592000)](https://twitter.com/wpnxm)

## About

The WPN-XM Server Control Panel enables you to control and configure servers on Windows easily.

The main window of the control panel provides start and stop, next to quick access buttons for logs and config files and indicates the process state, the used port and the software version for each server. 

The control panel provides a tight integration with additionally installed tools, a self-updater, an updater for already installed software, a configuration dialog and a process viewer.

The following servers are supported: PHP, Nginx, MariaDb, MongoDb, Redis, Memcached, PostgreSQL.

It's written in C++ using Qt. It's free and open-source under GNU/GPL v3 license.

## Documentation

You find the usage documentation in the [WPN-XM User Manual](http://wpn-xm.github.io/docs/user-manual/en/#_the_server_control_panel).

Breaking changes, new features, deprecations and bug fixes are tracked in the [CHANGELOG](https://github.com/WPN-XM/server-control-panel/blob/master/CHANGELOG.md).

## Screenshots

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

* [Qt >= 5.2](http://download.qt.io/official_releases/)
* [QuaZIP](https://github.com/stachenov/quazip) - LGPL v2.1
  * [zlib](https://github.com/madler/zlib) - ZLIB License
* [yaml-cpp](https://github.com/jbeder/yaml-cpp) - MIT License

### Downloading Qt

Install Qt binaries from either the Qt SDK or standalone binary package or just use the Qt Online Installer.

You should be able to find everything at http://download.qt.io/official_releases/

* Qt Online Installer

  http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe

* Qt 5.12

  http://download.qt.io/official_releases/qt/5.12/5.12.0/qt-opensource-windows-x86-5.12.0.exe

* Qt Creator

  http://download.qt.io/official_releases/qtcreator/4.8/4.8.0/qt-creator-opensource-windows-x86_64-4.8.0.exe

### Installation Steps and Requirements for Qt

-  http://doc.qt.io/qt-5/windows-support.html
-  http://doc.qt.io/qt-5/windows-requirements.html
-  http://doc.qt.io/qt-5/index.html

## Bugs

If you find a bug in the software, please let us know about it.

Please post the issue to the main project via https://github.com/WPN-XM/WPN-XM/issues/new
