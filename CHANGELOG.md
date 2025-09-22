# Changelog

本文件记录了MyShell项目的所有重要变更。

格式基于[Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
本项目遵循[语义化版本控制](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 新增
- 跨平台支持（Linux、macOS、Windows）
- CMake构建系统
- GitHub Actions自动化构建
- 平台抽象层设计
- 跨平台构建脚本

### 修改
- 重构代码以支持跨平台
- 更新文档以反映跨平台特性

## [1.0.0] - 2025-01-22

### 新增
- 基本shell功能
- 命令行解析器
- 内置命令支持
  - `cd` - 切换目录
  - `pwd` - 显示当前目录
  - `echo` - 输出文本
  - `exit` - 退出shell
  - `help` - 显示帮助
  - `export` - 设置环境变量
  - `env` - 显示环境变量
  - `unset` - 删除环境变量
  - `history` - 命令历史
  - `clear` - 清屏
  - `which` - 查找命令
- 外部程序执行
- 输入输出重定向（`>`, `>>`, `<`）
- 管道支持（`|`）
- 后台进程执行（`&`）
- 环境变量替换（`$VAR`）
- 命令历史记录持久化
- 模块化架构设计
- Makefile和XMake构建支持
- 基础测试脚本

### 技术特性
- C++17标准
- POSIX系统调用
- 模块化设计模式
- RAII资源管理
- 异常安全设计

## [0.1.0] - 初始版本

### 新增
- 项目初始化
- 基础项目结构
- Hello World示例