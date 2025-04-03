@echo off
setlocal

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake is not installed or not in PATH.
    pause
    exit /b 1
)

where git >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Git is not installed or not in PATH.
    pause
    exit /b 1
)

echo Running build.cmake...
cmake -P build.cmake

if %errorlevel% neq 0 (
    pause
)
exit /b %errorlevel%