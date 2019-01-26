@echo off

git submodule status

cd zlib
git pull
git checkout v1.2.11
cd ..
git add zlib
git commit -m "updated submodule zlib to v1.2.11"

cd yaml-cpp
git pull
rem git checkout yaml-cpp-0.6.2
git checkout b87c76a2ef4f974e2739a2d410ee5e36122cfc00
cd ..
git add yaml-cpp
rem git commit -m "updated submodule yaml-cpp to v0.6.2"
git commit -m "updated submodule yaml-cpp to b87c76a2"

cd quazip 
git pull
git checkout 0.7.6
cd ..
git add quazip
git commit -m "updated submodule quazip to 0.7.6"

git push
