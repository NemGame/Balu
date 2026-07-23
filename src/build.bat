@echo off
setlocal
pushd "%~dp0"
g++ -o main.exe main.cpp -std=c++17
if %errorlevel% neq 0 (
    echo Build failed.
    goto exit
)
echo Build complete.
:exit
popd
exit /b %errorlevel%