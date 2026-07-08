@echo off

set TOOLCHAIN_NAME=MSVC
set TOOLCHAIN_FILE=cmake\msvc_toolchain.cmake
set SOURCE_DIR=.
set BUILD_DIR=build\%TOOLCHAIN_NAME%
set GENERATOR="Visual Studio 17 2022"

rem rmdir /S /Q %BUILD_DIR%
cmake ^
    -S %SOURCE_DIR% ^
    -B %BUILD_DIR% ^
    -G %GENERATOR% ^
    -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
cmake --build %BUILD_DIR% --config Debug