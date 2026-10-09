@echo off
setlocal
cd /d "%~dp0\..\.."
if not defined XCLANG_ROOT set "XCLANG_ROOT=%CD%\build\xclang-sdk\xclang"
set "PATH=%XCLANG_ROOT%\bin;%PATH%"
pushd source
cmake --build --preset windows-release %*
set "result=%ERRORLEVEL%"
popd
if not "%result%" == "0" exit /b %result%
call scripts\ci\windows\post_build.bat
