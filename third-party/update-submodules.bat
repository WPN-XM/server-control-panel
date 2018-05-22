@echo off

cd zlib
git checkout v1.2.11
cd ..
git add zlib
git commit -m "updated submodule zlib to v1.2.11"

cd yaml-cpp
rem git checkout yaml-cpp-0.6.2
git checkout 4fb1c4b92bf8d94b32ebccdd890407d45b3bc794
cd ..
git add yaml-cpp
rem git commit -m "updated submodule yaml-cpp to v0.5.2"
git commit -m "updated submodule yaml-cpp to 4fb1c4"


cd quazip 
git checkout d59996137f4fb2796e64224b2387759191babf13
cd ..
git add quazip
git commit -m "updated submodule quazip to d599961"

git push
