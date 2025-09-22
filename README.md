# MyShell - 跨平台Shell工具

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-green.svg)
![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)
![Build](https://img.shields.io/badge/build-CMake-orange.svg)

<div align="center">
  <img src="https://github.com/user-attachments/assets/shell-demo.gif" alt="MyShell Demo" width="600">
</div>

## 项目简介

MyShell是一个用C++开发的跨平台shell工具，支持Linux、Windows和macOS，提供了完整的命令行交互环境，支持内置命令、外部程序执行、输入输出重定向、管道操作等功能。

## 平台支持

| 平台 | 状态 | 构建工具 |
|------|------|----------|
| Linux | ✅ 完全支持 | CMake, Make, XMake |
| macOS | ✅ 完全支持 | CMake, Make |
| Windows | ✅ 完全支持 | CMake, Visual Studio, MinGW |

### 平台特性对比

| 功能 | Linux | macOS | Windows |
|------|-------|-------|----------|
| 进程管理 | fork/exec | fork/exec | CreateProcess |
| 管道支持 | pipe() | pipe() | CreatePipe |
| 重定向 | dup2() | dup2() | SetStdHandle |
| 信号处理 | signal() | signal() | SetConsoleCtrlHandler |
| 路径分隔符 | / | / | \\ |
| 可执行文件 | 任意 | 任意 | .exe, .bat, .cmd |

## 功能特性

### 基础功能
- ✅ 命令行解析和词法分析
- ✅ 内置命令实现（cd、pwd、exit、help、echo等）
- ✅ 外部程序执行
- ✅ 输入/输出重定向（`>`, `>>`, `<`）
- ✅ 管道支持（`|`）
- ✅ 环境变量管理
- ✅ 命令历史记录
- ✅ 后台进程执行（`&`）

### 内置命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `help` | 显示帮助信息 | `help` |
| `exit [n]` | 退出shell，可选择退出码 | `exit 0` |
| `pwd` | 显示当前工作目录 | `pwd` |
| `cd [dir]` | 切换目录，无参数时切换到HOME | `cd /tmp` |
| `echo [-n] text` | 显示文本，-n选项不换行 | `echo "Hello World"` |
| `export var=value` | 设置环境变量 | `export PATH=/usr/bin` |
| `env` | 显示所有环境变量 | `env` |
| `unset var` | 删除环境变量 | `unset PATH` |
| `history` | 显示命令历史 | `history` |
| `clear` | 清屏 | `clear` |
| `which cmd` | 查找命令位置 | `which ls` |

### 特殊功能

- **输出重定向**: `command > file` 或 `command >> file`
- **输入重定向**: `command < file`
- **管道**: `command1 | command2`
- **后台运行**: `command &`
- **环境变量替换**: `echo $HOME`
- **支持引号**: `echo "hello world"`

## 架构设计

```
mysh/
├── src/
│   ├── main.cpp           # 主程序入口
│   ├── shell.h/.cpp       # Shell主类
│   ├── parser.h/.cpp      # 命令解析器
│   ├── executor.h/.cpp    # 命令执行器
│   ├── builtin.h/.cpp     # 内置命令
│   └── history.h/.cpp     # 命令历史
└── xmake.lua             # 构建配置
```

### 核心组件

1. **Shell类**: 主控制器，负责用户交互和组件协调
2. **Parser类**: 词法分析和语法解析，支持复杂命令解析
3. **Executor类**: 执行外部程序，处理进程管理和信号
4. **BuiltinCommands类**: 实现所有内置命令
5. **History类**: 命令历史记录和持久化

## 编译运行

### 前置条件

**所有平台**:
- CMake 3.15+
- C++17兼容的编译器

**Linux/macOS**:
- GCC 7+ 或 Clang 5+
- Make (可选)

**Windows**:
- Visual Studio 2017+ 或 MinGW-w64
- Windows 10+ (推荐)

### 快速开始

#### 使用跨平台构建脚本 (推荐)

**Linux/macOS**:
```bash
# 赋予执行权限
chmod +x build.sh

# 构建项目
./build.sh build

# 运行
./build-linux/bin/mysh  # Linux
./build-macos/bin/mysh  # macOS
```

**Windows**:
```cmd
REM 构建项目
build.bat build

REM 运行
build-windows\bin\mysh.exe
```

#### 使用CMake (所有平台)

```bash
# 创建构建目录
mkdir build && cd build

# 配置
cmake ..

# 编译
cmake --build .

# 运行
./bin/mysh          # Linux/macOS
.\bin\mysh.exe      # Windows
```

#### 传统方式 (Linux/macOS)

```bash
# 使用Make
make
./mysh

# 使用XMake
xmake
./build/linux/x86_64/release/mysh
```

## 使用示例

### 基本操作
```bash
# 查看当前目录
snow@mysh:~$ pwd
/home/snow

# 切换目录
snow@mysh:~$ cd /tmp
snow@mysh:/tmp$ 

# 列出文件
snow@mysh:/tmp$ ls -la

# 使用环境变量
snow@mysh:/tmp$ echo $HOME
/home/snow
```

### 重定向和管道
```bash
# 输出重定向
snow@mysh:~$ echo "Hello World" > hello.txt

# 追加输出
snow@mysh:~$ echo "Second line" >> hello.txt

# 输入重定向
snow@mysh:~$ wc -l < hello.txt

# 管道操作
snow@mysh:~$ ls -la | grep ".txt"
snow@mysh:~$ ps aux | grep mysh | wc -l
```

### 后台进程
```bash
# 后台运行程序
snow@mysh:~$ sleep 60 &
[Background] Process 12345 started

# 继续使用shell
snow@mysh:~$ pwd
```

### 命令历史
```bash
# 查看历史记录
snow@mysh:~$ history

# 查看最后10条
snow@mysh:~$ history -10

# 搜索历史
snow@mysh:~$ history grep

# 清空历史
snow@mysh:~$ history -c
```

## 技术实现要点

### 1. 命令解析
- 支持引号处理
- 环境变量展开
- 特殊字符识别
- 管道符解析

### 2. 进程管理
- fork/exec模型
- 信号处理
- 作业控制
- 僵尸进程清理

### 3. 重定向实现
- 文件描述符操作
- dup2系统调用
- 标准输入输出管理

### 4. 管道实现
- pipe系统调用
- 多进程协调
- 文件描述符传递

## 扩展功能建议

1. **Tab补全**: 文件名和命令自动补全
2. **作业控制**: 前台/后台作业管理（jobs, fg, bg）
3. **通配符**: 文件名通配符展开（*, ?）
4. **别名系统**: 命令别名定义和使用
5. **配置文件**: 启动配置文件支持
6. **颜色输出**: 彩色提示符和语法高亮
7. **脚本执行**: Shell脚本文件执行

## 开发收获

通过这个项目，你将学习到：

- Linux系统编程基础
- 进程管理和信号处理
- 文件描述符和I/O重定向
- 管道通信机制
- C++面向对象设计
- 词法分析和语法解析
- 用户界面设计
- 软件架构设计

这是一个很好的系统编程练习项目，涵盖了操作系统的多个重要概念！