@echo off
echo Setting up MSYS2 for POSIX compatibility on Windows
echo ====================================================

REM 检查是否已安装MSYS2
if exist "C:\msys64\usr\bin\bash.exe" (
    echo MSYS2 already installed at C:\msys64
    goto :setup_env
)

echo MSYS2 not found. Please install MSYS2 first:
echo 1. Download from: https://www.msys2.org/
echo 2. Install to default location (C:\msys64)
echo 3. Run this script again
pause
exit /b 1

:setup_env
echo Setting up MSYS2 environment...

REM 添加MSYS2到PATH
set "MSYS2_PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin"
echo Adding MSYS2 to PATH: %MSYS2_PATH%

REM 设置环境变量
setx PATH "%PATH%;%MSYS2_PATH%" /M 2>nul
if %errorlevel% neq 0 (
    echo Warning: Could not set system PATH. Setting for current session only.
    set "PATH=%PATH%;%MSYS2_PATH%"
)

echo.
echo MSYS2 setup complete!
echo You can now use POSIX commands and compile with GCC.
echo.
echo To install required packages, run:
echo   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake make
echo.
pause