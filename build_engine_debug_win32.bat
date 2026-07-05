@echo off

set TOOLCHAIN=msvc
set TOOLCHAIN_FILE=..\cmake\msvc_toolchain.cmake
set SOURCE_DIR=engine
set BUILD_DIR=build\%TOOLCHAIN%\engine
set GENERATOR="Visual Studio 17 2022"

rmdir /S /Q %BUILD_DIR%
cmake ^
    -S %SOURCE_DIR% ^
    -B %BUILD_DIR% ^
    -G %GENERATOR% ^
    -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
cmake --build %BUILD_DIR% --config Debug