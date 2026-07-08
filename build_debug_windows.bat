@echo off

echo Building TritonEngine... (Windows OS, MSVC compiler)
call build_engine_debug_win32.bat
echo TritonEngine built.

echo.
echo Building TritonEditor... (Windows OS, MSVC compiler)
call build_editor_debug_win32.bat
echo TritonEditor built.

echo.
echo Build completed.