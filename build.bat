@echo off
pushd src
call build.bat
popd
exit /b %errorlevel%