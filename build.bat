@echo off
setlocal enabledelayedexpansion

echo MyShell Windows Build Script
echo ============================

REM 检查参数
if "%1"=="clean" (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
    echo Clean completed.
    goto :eof
)

REM 创建构建目录
if not exist build mkdir build

REM 尝试找到Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

REM 如果找到Visual Studio，设置环境
if defined VS_PATH (
    echo Found Visual Studio at: !VS_PATH!
    call "!VS_PATH!\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul 2>&1
    if !errorlevel! equ 0 (
        echo Using Visual Studio compiler
        cmake -G "Visual Studio 17 2022" -A x64 -S . -B build
        if !errorlevel! equ 0 (
            cmake --build build --config Release
            if !errorlevel! equ 0 (
                echo Build successful! Executable: build\Release\mysh.exe
                goto :eof
            )
        )
    )
)

REM 尝试MinGW
where gcc >nul 2>&1
if !errorlevel! equ 0 (
    echo Using MinGW compiler
    cmake -G "MinGW Makefiles" -S . -B build
    if !errorlevel! equ 0 (
        cmake --build build
        if !errorlevel! equ 0 (
            echo Build successful! Executable: build\bin\mysh.exe
            goto :eof
        )
    )
)

REM 手动编译（最后的选择）
echo No suitable compiler found. Attempting manual compilation...
echo Please install Visual Studio 2017+ or MinGW-w64

echo.
echo Available options:
echo 1. Install Visual Studio Community (free): https://visualstudio.microsoft.com/vs/community/
echo 2. Install MinGW-w64: https://www.mingw-w64.org/downloads/
echo 3. Install MSYS2: https://www.msys2.org/
echo.
echo After installation, run this script again.

pause