@echo off

git submodule status

:: https://github.com/madler/zlib/releases
cd zlib
git checkout v1.2.11
cd ..
git add zlib
git commit -m "updated submodule zlib to v1.2.11"

:: https://github.com/jbeder/yaml-cpp/releases
cd yaml-cpp
rem git checkout yaml-cpp-0.6.2
git checkout 968e0c1f0281fdaf66d0983bd8bd694263400810
cd ..
git add yaml-cpp
rem git commit -m "updated submodule yaml-cpp to v0.6.2"
git commit -m "updated submodule yaml-cpp to 968e0c"

:: https://github.com/stachenov/quazip/releases
cd quazip 
git checkout 0.8.1
cd ..
git add quazip
git commit -m "updated submodule quazip to 0.8.1"

git push
