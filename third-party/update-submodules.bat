@echo off

cd zlib
git checkout v1.2.11
cd ..
git add zlib
git commit -m "updated submodule zlib to v1.2.11"

cd yaml-cpp
git checkout yaml-cpp-0.6.2
cd ..
git add yaml-cpp
git commit -m "updated submodule yaml-cpp to v0.5.2"


cd quazip 
git checkout d59996137f4fb2796e64224b2387759191babf13
cd ..
git add quazip
git commit -m "updated submodule quazip to d599961"

git push
