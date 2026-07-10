@echo off

set PRESET_NAME=debug_win64
set SOURCE_DIR=.
set BUILD_DIR=build\%PRESET_NAME%
set GENERATOR="Visual Studio 17 2022"

git submodule update --init --recursive
rem rmdir /S /Q %BUILD_DIR%
cmake ^
    -S %SOURCE_DIR% ^
    -B %BUILD_DIR% ^
    -G %GENERATOR%
cmake --build %BUILD_DIR% --preset %PRESET_NAME%

pause