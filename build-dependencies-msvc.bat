@echo off

echo.
echo Building dependencies using MSVC
echo. 
echo Using CMake Generator: Visual Studio 14 2015 Win64 =  (x86_amd64)
echo.

echo -- Set Qt folder
echo.
SET "QTVERSION=5.9.2\msvc2017_64"
SET QTDIR=C:\Qt\%QTVERSION%
rem SET PATH=%QTDIR%\bin;%QTDIR%;%PATH%
echo %QTDIR%\bin
call %QTDIR%\bin\qtenv2.bat

SET "platform=amd64"
SET "MSBUILD_FLAGS=/verbosity:minimal /maxcpucount /nologo"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %platform%
echo Done.
echo.

echo -- Building zlib
echo.
pushd "%~dp0"
cd third-party\zlib
:: 0) cleanup
rm -rf build
:: 1) configure
cmake -G "Visual Studio 14 2015 Win64" CMakeLists.txt -B./build -DCMAKE_INSTALL_PREFIX=../../libs
:: 2) make
msbuild .\build\zlib.vcxproj /p:Configuration=Release %MSBUILD_FLAGS%
msbuild .\build\zlibstatic.vcxproj /p:Configuration=Release /p:Platform=%platform% /p:OutDir=..\..\..\libs\ %MSBUILD_FLAGS%
:: 3) install 
msbuild .\build\INSTALL.vcxproj %MSBUILD_FLAGS% 
popd
echo Done.
echo.

echo -- Building QuaZip
echo.
pushd "%~dp0"
cd third-party\quazip\quazip
:: 0) cleanup
rm -rf build
:: 1) configure
mkdir build 
cd build
cmake .. -G "Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX=../../../libs -DLIB_DESTINATION=../../../libs/quazip/bin -DZLIB_INCLUDE_DIRS=../../../libs/zlib/include -DZLIB_LIBRARIES=../../../libs/zdll.lib
:: 2) make
msbuild ..\quazip.vcxproj /p:Configuration=Release /p:Platform=%platform% /p:OutDir=..\..\..\libs\ %MSBUILD_FLAGS%
:: 3) install 
msbuild ..\INSTALL.vcxproj %MSBUILD_FLAGS% 
popd
echo Done.
echo.

echo -- Building yaml-cpp
echo.
pushd "%~dp0"
cd third-party\yaml-cpp
:: 0) cleanup
rm -rf build
:: 1) configure
mkdir build 
cd build
cmake .. -G "Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX=../../../libs -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF
:: 2) make
msbuild yaml-cpp.vcxproj /p:Configuration=Release /p:Platform=%platform% /p:OutDir=..\..\..\libs %MSBUILD_FLAGS%
:: 3) install 
msbuild INSTALL.vcxproj %MSBUILD_FLAGS% 
popd
echo Done.
echo.