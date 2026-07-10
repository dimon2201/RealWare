@echo off

set PRESET_NAME=debug_win64

git submodule update --init --recursive

rem rmdir /S /Q build
cmake --preset %PRESET_NAME%
cmake --build --preset %PRESET_NAME%

pause