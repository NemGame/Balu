@echo off
setlocal
pushd "%~dp0"
g++ -o main.exe main.cpp -std=c++17
echo Build complete.
popd