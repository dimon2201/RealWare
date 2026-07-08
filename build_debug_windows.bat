@echo off

echo Building TritonEngine... (Windows OS, MSVC compiler)
call build_debug_engine_windows.bat
echo TritonEngine built.

echo.
echo Building TritonEditor... (Windows OS, MSVC compiler)
call build_debug_editor_windows.bat
echo TritonEditor built.

echo.
echo Build completed.

pause