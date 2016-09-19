@echo off

echo "Building dependencies using MSVC"

:: configure QT
pushd "%~dp0"
call "%QTDIR%\bin\qtenv2.bat"
popd

:: configure MSVC
SET VSVER=14.0
pushd "%~dp0"
call "%ProgramFiles(x86)%\Microsoft Visual Studio %VSVER%\VC\vcvarsall.bat"
popd

:: -- zlib
echo "-- Building zlib"
pushd "%~dp0"
cd third-party\zlib
:: 1) configure
cmake -G"Visual Studio 14 2015" CMakeLists.txt -B./build -DCMAKE_INSTALL_PREFIX=../../libs
:: 2) make
msbuild .\build\zlib.vcxproj /p:Configuration=Release /verbosity:minimal /p:OutDir=..\..\..\libs\
msbuild .\build\zlibstatic.vcxproj /p:Configuration=Release /verbosity:minimal /p:OutDir=..\..\..\libs\
popd
echo "Done."

:: -- quazip
echo "-- Building QuaZip"
pushd "%~dp0"
cd third-party\quazip\quazip
:: 1) configure
mkdir build && cd build
cmake.exe .. -DCMAKE_INSTALL_PREFIX=../../../libs -DINCLUDEPATH="$$PWD/../../zlib" -DLIBS+="-L$$PWD/../../../libs -lz" -DDESTDIR="$$PWD/../../../libs"
:: 2) make
popd
echo "Done."