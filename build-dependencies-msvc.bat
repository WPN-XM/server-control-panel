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
msbuild .\build\zlib.vcxproj /p:Configuration=Release /verbosity:minimal /p:OutDir=..\..\..\libs\
msbuild .\build\zlibstatic.vcxproj /p:Configuration=Release /verbosity:minimal /p:OutDir=..\..\..\libs\
popd
echo Done.
echo.

echo -- Building QuaZip
echo.
pushd "%~dp0"
cd third-party\quazip\quazip
call qtenv2.bat
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
:: 1) configure
mkdir build && cd build
cmake .. -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../../libs -DCONFIG+=statliclib -DINCLUDEPATH="$$PWD/../../zlib" -DLIBS+="-L$$PWD/../../../libs -lz" -DDESTDIR="$$PWD/../../../libs"
:: 2) make
cmake --build . --config %CONFIGURATION%
popd
echo Done.
echo.