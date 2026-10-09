@echo off

for %%f in (lib\windows\*.dll) do (
    xcopy "%%f" "dist\" /Y
)

for %%f in (build\windows-release\vcpkg_installed\x64-windows\bin\*.dll) do (
    xcopy "%%f" "dist\" /Y
)
