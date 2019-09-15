@echo off

echo.
echo Building dependencies using MSVC
echo. 

echo -- Set Generator
echo. 
SET "GENERATOR=Visual Studio 14 2015 Win64"
echo    Using CMake Generator: %GENERATOR%
echo.

echo -- Set Qt folder
echo.
SET "QTVERSION=Qt5.10.1\5.10.1\msvc2017_64"
SET QTDIR=D:\Qt\%QTVERSION%
SET PATH=%QTDIR%\bin;%QTDIR%;%PATH%
echo    Using Qt: %QTDIR%\bin
echo.

echo -- Setup Qt environment
call %QTDIR%\bin\qtenv2.bat
qmake -v
echo.

echo -- Setup MSVC environment
SET "PLATFORM=amd64"
SET "MSBUILD_FLAGS=/verbosity:minimal /maxcpucount /nologo"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %PLATFORM%
echo Done.
echo.

echo -- Building zlib
echo.
pushd "%~dp0"
cd third-party\zlib
:: 0) cleanup
rm -rf build
:: 1) configure
cmake -G %GENERATOR% CMakeLists.txt -B./build -DCMAKE_INSTALL_PREFIX=../../libs
:: 2) make
msbuild .\build\zlib.vcxproj /p:Configuration=Release %MSBUILD_FLAGS%
msbuild .\build\zlibstatic.vcxproj /p:Configuration=Release /p:Platform=%PLATFORM% /p:OutDir=..\..\..\libs\ %MSBUILD_FLAGS%
:: 3) install 
msbuild .\build\INSTALL.vcxproj %MSBUILD_FLAGS% 
popd
echo Done.
echo.

echo -- Building QuaZip
echo.
pushd "%~dp0"
cd third-party\quazip
:: 0) cleanup
rm -rf build
:: 1) configure
mkdir build 
cd build
cmake .. -G %GENERATOR% -DCMAKE_INSTALL_PREFIX=../../../libs -DLIB_DESTINATION=../../../libs/quazip/bin -DZLIB_INCLUDE_DIRS=../../../libs/zlib/include -DZLIB_LIBRARIES=../../../libs/zdll.lib
:: 2) make
msbuild ..\quazip.vcxproj /p:Configuration=Release /p:Platform=%PLATFORM% /p:OutDir=..\..\..\libs\ %MSBUILD_FLAGS%
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
cmake .. -G %GENERATOR% -DCMAKE_INSTALL_PREFIX=../../../libs -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF
:: 2) make
msbuild yaml-cpp.vcxproj /p:Configuration=Release /p:Platform=%PLATFORM% /p:OutDir=..\..\..\libs %MSBUILD_FLAGS%
:: 3) install 
msbuild INSTALL.vcxproj %MSBUILD_FLAGS% 
popd
echo Done.
echo.

pause