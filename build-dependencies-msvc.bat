@echo off

echo.
echo Building dependencies using MSVC
echo. 

echo -- Set Qt folder
echo.
SET "QTVERSION=5.13.2\msvc2017_64"
SET QTDIR=D:\Qt\%QTVERSION%
SET PATH=%QTDIR%\bin;%QTDIR%;%PATH%
echo    Using Qt: %QTDIR%\bin
echo.

echo -- Setup Qt environment
call %QTDIR%\bin\qtenv2.bat
qmake -v
echo.
echo -- Setup MSVC environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo Done.
echo.

echo -- Building zlib
echo.
pushd "%~dp0"
cd third-party\zlib
rm -rf build
mkdir build 
cd build
:: 1) configure
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_INSTALL_PREFIX=../../../libs/zlib
:: 2) make
cmake --build . --target install --config Release
popd
echo Done.
echo.

echo -- Building QuaZip
echo.
pushd "%~dp0"
cd third-party\quazip
rm -rf build
mkdir build 
cd build
:: 1) configure
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_INSTALL_PREFIX=../../../libs/quazip -DLIB_DESTINATION=../../../libs/quazip/bin -DZLIB_ROOT=../../../libs/zlib
:: 2) make
cmake --build . --target install --config Release
popd
echo Done.
echo.

echo -- Building yaml-cpp
echo.
pushd "%~dp0"
cd third-party\yaml-cpp
rm -rf build
mkdir build 
cd build
:: 1) configure
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_INSTALL_PREFIX=../../../libs/yaml-cpp -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF
:: 2) make
cmake --build . --target install --config Release
popd
echo Done.
echo.

pause