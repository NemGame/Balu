@echo off
setlocal
pushd "%~dp0"
g++ -o main.exe main.cpp
echo Build complete.
popd