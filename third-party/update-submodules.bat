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
git checkout 587b24e2eedea1afa21d79419008ca5f7bda3bf4
cd ..
git add yaml-cpp
rem git commit -m "updated submodule yaml-cpp to v0.6.2"
git commit -m "updated submodule yaml-cpp to 587b24"

:: https://github.com/stachenov/quazip/releases
cd quazip 
git checkout 0.8.1
cd ..
git add quazip
git commit -m "updated submodule quazip to 0.8.1"

git push
