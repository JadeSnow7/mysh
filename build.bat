@echo off
REM Windows跨平台构建脚本

setlocal enabledelayedexpansion

set "BUILD_TYPE=Release"
set "BUILD_DIR=build-windows"

REM 颜色定义（Windows 10+）
set "GREEN=[92m"
set "RED=[91m"
set "YELLOW=[93m"
set "NC=[0m"

:print_info
echo %GREEN%[INFO]%NC% %~1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %~1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %~1
goto :eof

:check_dependencies
call :print_info "检查构建依赖..."

REM 检查CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    call :print_error "CMake未安装。请安装CMake 3.15或更高版本。"
    exit /b 1
)

REM 检查Visual Studio或MinGW
cl >nul 2>&1
if not errorlevel 1 (
    set "COMPILER=MSVC"
    call :print_info "检测到Visual Studio编译器"
    goto :dep_check_done
)

g++ --version >nul 2>&1
if not errorlevel 1 (
    set "COMPILER=MinGW"
    call :print_info "检测到MinGW编译器"
    goto :dep_check_done
)

call :print_warning "未检测到编译器。请安装Visual Studio或MinGW-w64。"

:dep_check_done
call :print_info "依赖检查完成。"
goto :eof

:build_project
if not "%~1"=="" set "BUILD_TYPE=%~1"

call :print_info "构建类型: %BUILD_TYPE%"

call :check_dependencies
if errorlevel 1 exit /b 1

REM 创建构建目录
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

call :print_info "开始CMake配置..."

REM 根据编译器选择生成器
if "%COMPILER%"=="MSVC" (
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else (
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
)

if errorlevel 1 (
    call :print_error "CMake配置失败！"
    cd ..
    exit /b 1
)

call :print_info "开始编译..."

cmake --build . --config %BUILD_TYPE%

if errorlevel 1 (
    call :print_error "构建失败！"
    cd ..
    exit /b 1
)

call :print_info "构建成功！"
call :print_info "可执行文件位置: %CD%\bin\mysh.exe"

cd ..
goto :eof

:clean_build
call :print_info "清理构建文件..."
if exist "build-windows" rmdir /s /q "build-windows"
call :print_info "清理完成。"
goto :eof

:install_project
if not exist "%BUILD_DIR%" (
    call :print_error "请先构建项目。"
    exit /b 1
)

cd "%BUILD_DIR%"

call :print_info "安装到系统目录（需要管理员权限）..."
cmake --install . --config %BUILD_TYPE%

if errorlevel 1 (
    call :print_error "安装失败！请以管理员身份运行。"
    cd ..
    exit /b 1
)

cd ..
call :print_info "安装完成。"
goto :eof

:run_tests
if not exist "%BUILD_DIR%" (
    call :print_error "请先构建项目。"
    exit /b 1
)

call :print_info "运行测试..."

cd "%BUILD_DIR%"
ctest --output-on-failure
cd ..

REM Windows下运行PowerShell测试脚本
if exist "test_shell.ps1" (
    call :print_info "运行功能测试..."
    powershell -ExecutionPolicy Bypass -File test_shell.ps1
)

goto :eof

:package_project
if not exist "%BUILD_DIR%" (
    call :print_error "请先构建项目。"
    exit /b 1
)

call :print_info "创建发布包..."

cd "%BUILD_DIR%"
cpack
cd ..

call :print_info "发布包创建完成，位置: %BUILD_DIR%\"
goto :eof

:show_help
echo MyShell Windows构建脚本
echo.
echo 用法: %~n0 [选项]
echo.
echo 选项:
echo   build [Debug^|Release]  构建项目（默认Release）
echo   clean                  清理构建文件
echo   install               安装到系统
echo   test                  运行测试
echo   package               创建发布包
echo   help                  显示此帮助信息
echo.
echo 示例:
echo   %~n0 build              # 构建Release版本
echo   %~n0 build Debug        # 构建Debug版本
echo   %~n0 clean              # 清理构建文件
echo   %~n0 test               # 运行测试
goto :eof

REM 主逻辑
set "COMMAND=%~1"
if "%COMMAND%"=="" set "COMMAND=help"

if "%COMMAND%"=="build" (
    call :build_project %2
) else if "%COMMAND%"=="clean" (
    call :clean_build
) else if "%COMMAND%"=="install" (
    call :install_project
) else if "%COMMAND%"=="test" (
    call :run_tests
) else if "%COMMAND%"=="package" (
    call :package_project
) else (
    call :show_help
)