@echo off

::set QTDIR="C:\Qt\Qt5.5.0\5.5\mingw492_32"

pushd "%~dp0"
call "%QTDIR%\bin\qtenv2.bat"
popd

:: -- zlib
pushd "%~dp0"
cd third-party\zlib
:: 1) configure
qmake.exe zlib.pro -r -spec win32-g++ "DESTDIR = $$PWD/../../../libs"
:: 2) make
mingw32-make
popd

:: -- quazip
pushd "%~dp0"
cd third-party\quazip\quazip
:: 1) configure
qmake.exe quazip.pro -r -spec win32-g++ "INCLUDEPATH += $$PWD/../../zlib" "LIBS +=-L$$PWD/../../../libs -lz" "DESTDIR = $$PWD/../../../libs"
:: 2) make
mingw32-make
popd