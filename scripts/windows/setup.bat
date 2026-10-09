@echo off
setlocal
cd /d "%~dp0\..\.."

if not defined XCLANG_ROOT (
  cmake "-DXCLANG_INSTALL_DIR=%CD%\build\xclang-sdk" -P scripts\ci\install-xclang.cmake
  if errorlevel 1 exit /b 1
  set "XCLANG_ROOT=%CD%\build\xclang-sdk\xclang"
)
rem Qt5 needs only the MSYS2 make package, not a second compiler/runtime.
if not defined MSYS2_ROOT (
  echo Set MSYS2_ROOT to an MSYS2 installation with the mingw-w64-ucrt-x86_64-make package.
  exit /b 1
)
set "PATH=%MSYS2_ROOT%\ucrt64\bin;%PATH%"
set "PATH=%XCLANG_ROOT%\bin;%PATH%"
where mingw32-make >nul 2>nul
if errorlevel 1 (
  echo Install the MSYS2 mingw-w64-ucrt-x86_64-make package and add its ucrt64\bin to PATH.
  exit /b 1
)
if not exist dist mkdir dist
copy /Y lib\windows\*.dll dist\
copy /Y scripts\windows\sbinit.config dist\
pushd source
cmake --preset windows-release
set "result=%ERRORLEVEL%"
popd
exit /b %result%
