@echo off

echo.
echo Building dependencies using MSVC
echo.

echo -- Building zlib
echo.
pushd "%~dp0"
cd third-party\zlib
:: 1) configure
cmake -G "Visual Studio 14 2015" CMakeLists.txt -B./build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../libs
:: 2) make
msbuild .\build\zlib.vcxproj /p:Configuration=Release /p:OutDir=..\..\..\libs\ /nologo /verbosity:minimal /maxcpucount
msbuild .\build\zlibstatic.vcxproj /p:Configuration=Release /p:OutDir=..\..\..\libs\ /nologo /verbosity:minimal /maxcpucount
popd
echo Done.
echo.

echo -- Building QuaZip
echo.
pushd "%~dp0"
echo C:\Qt\%qt%\bin
cd third-party\quazip\quazip
:: 1) configure
mkdir build 
cd build
cmake .. -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../../libs -DCMAKE_PREFIX_PATH="C:\Qt\%qt%\bin" -DCONFIG+=statliclib  -DINCLUDEPATH="$$PWD/../../zlib" -DLIBS+="-L$$PWD/../../../libs -lz" -DDESTDIR="$$PWD/../../../libs"
:: 2) make
cmake --build . --config %CONFIGURATION%
popd
echo Done.
echo.