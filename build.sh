#!/bin/bash

# 跨平台构建脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检测操作系统
detect_platform() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# 打印信息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    local platform=$1
    
    print_info "检查构建依赖..."
    
    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake未安装。请安装CMake 3.15或更高版本。"
        return 1
    fi
    
    # 检查编译器
    case $platform in
        "linux"|"macos")
            if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
                print_error "未找到C++编译器。请安装g++或clang++。"
                return 1
            fi
            ;;
        "windows")
            if ! command -v cl &> /dev/null && ! command -v g++ &> /dev/null; then
                print_warning "推荐使用Visual Studio或MinGW-w64进行Windows编译。"
            fi
            ;;
    esac
    
    print_info "依赖检查完成。"
}

# 构建函数
build_project() {
    local build_type=${1:-Release}
    local platform=$(detect_platform)
    
    print_info "检测到平台: $platform"
    print_info "构建类型: $build_type"
    
    # 检查依赖
    if ! check_dependencies $platform; then
        exit 1
    fi
    
    # 创建构建目录
    BUILD_DIR="build-$platform"
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    
    print_info "开始CMake配置..."
    
    # 平台特定的CMake配置
    case $platform in
        "windows")
            # Windows特定配置
            cmake .. -DCMAKE_BUILD_TYPE=$build_type \
                     -DCMAKE_TOOLCHAIN_FILE=cmake/windows.cmake 2>/dev/null || \
            cmake .. -DCMAKE_BUILD_TYPE=$build_type
            ;;
        "macos")
            # macOS特定配置
            cmake .. -DCMAKE_BUILD_TYPE=$build_type \
                     -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
            ;;
        *)
            # Linux和其他平台
            cmake .. -DCMAKE_BUILD_TYPE=$build_type
            ;;
    esac
    
    print_info "开始编译..."
    
    # 构建项目
    cmake --build . --config $build_type
    
    if [ $? -eq 0 ]; then
        print_info "构建成功！"
        print_info "可执行文件位置: $(pwd)/bin/mysh"
    else
        print_error "构建失败！"
        exit 1
    fi
    
    cd ..
}

# 清理函数
clean_build() {
    print_info "清理构建文件..."
    rm -rf build-*
    print_info "清理完成。"
}

# 安装函数
install_project() {
    local platform=$(detect_platform)
    local build_dir="build-$platform"
    
    if [ ! -d "$build_dir" ]; then
        print_error "请先构建项目。"
        exit 1
    fi
    
    cd $build_dir
    
    case $platform in
        "windows")
            print_info "Windows安装需要管理员权限..."
            cmake --install . --config Release
            ;;
        *)
            print_info "安装到系统目录（需要sudo权限）..."
            sudo cmake --install .
            ;;
    esac
    
    cd ..
    print_info "安装完成。"
}

# 测试函数
run_tests() {
    local platform=$(detect_platform)
    local build_dir="build-$platform"
    
    if [ ! -d "$build_dir" ]; then
        print_error "请先构建项目。"
        exit 1
    fi
    
    print_info "运行测试..."
    
    cd $build_dir
    ctest --output-on-failure
    cd ..
    
    # 运行自定义测试脚本
    if [ -f "test_shell.sh" ] && [ "$platform" != "windows" ]; then
        print_info "运行功能测试..."
        # 修改测试脚本以使用正确的可执行文件路径
        sed "s|./build/linux/x86_64/release/mysh|./$build_dir/bin/mysh|g" test_shell.sh > temp_test.sh
        chmod +x temp_test.sh
        ./temp_test.sh
        rm temp_test.sh
    fi
}

# 打包函数
package_project() {
    local platform=$(detect_platform)
    local build_dir="build-$platform"
    
    if [ ! -d "$build_dir" ]; then
        print_error "请先构建项目。"
        exit 1
    fi
    
    print_info "创建发布包..."
    
    cd $build_dir
    cpack
    cd ..
    
    print_info "发布包创建完成，位置: $build_dir/"
}

# 帮助信息
show_help() {
    echo "MyShell 跨平台构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  build [Debug|Release]  构建项目（默认Release）"
    echo "  clean                  清理构建文件"
    echo "  install               安装到系统"
    echo "  test                  运行测试"
    echo "  package               创建发布包"
    echo "  help                  显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 build              # 构建Release版本"
    echo "  $0 build Debug        # 构建Debug版本"
    echo "  $0 clean              # 清理构建文件"
    echo "  $0 test               # 运行测试"
}

# 主逻辑
case ${1:-help} in
    "build")
        build_project $2
        ;;
    "clean")
        clean_build
        ;;
    "install")
        install_project
        ;;
    "test")
        run_tests
        ;;
    "package")
        package_project
        ;;
    "help"|*)
        show_help
        ;;
esac