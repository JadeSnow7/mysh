# MyShell 项目目录重构方案

## 🎯 重构目标

1. **模块化分离**: 按功能模块组织代码
2. **文档集中**: 将文档文件统一管理
3. **构建系统**: 分离构建配置文件
4. **测试独立**: 创建独立的测试目录
5. **跨平台支持**: 分离平台特定代码

## 📁 新目录结构

```
mysh/
├── README.md                    # 项目主要说明
├── LICENSE                      # 开源协议
├── CHANGELOG.md                 # 变更日志
├── .gitignore                  # Git忽略文件
│
├── docs/                       # 📚 文档目录
│   ├── DEVELOPMENT.md          # 开发指南
│   ├── CONTRIBUTING.md         # 贡献指南
│   ├── CROSS_PLATFORM.md      # 跨平台指南
│   ├── BUILD_VERIFICATION.md  # 构建验证报告
│   ├── GITHUB_ACTIONS_FIX.md  # GitHub Actions修复说明
│   ├── WINDOWS_CI_FIX.md      # Windows CI修复说明
│   ├── GITHUB_UPLOAD.md       # GitHub上传指南
│   └── UPLOAD_NOW.md          # 立即上传指南
│
├── src/                        # 💻 源代码目录
│   ├── core/                   # 核心模块
│   │   ├── shell.h
│   │   ├── shell.cpp
│   │   ├── parser.h
│   │   ├── parser.cpp
│   │   ├── executor.h
│   │   ├── executor.cpp
│   │   ├── builtin.h
│   │   ├── builtin.cpp
│   │   ├── history.h
│   │   └── history.cpp
│   │
│   ├── platform/               # 跨平台抽象层
│   │   ├── platform.h
│   │   ├── platform.cpp
│   │   ├── linux/
│   │   │   └── platform_linux.cpp
│   │   ├── macos/
│   │   │   └── platform_macos.cpp
│   │   └── windows/
│   │       └── platform_windows.cpp
│   │
│   └── main.cpp               # 程序入口
│
├── tests/                      # 🧪 测试目录
│   ├── unit/                   # 单元测试
│   │   ├── test_parser.cpp
│   │   ├── test_builtin.cpp
│   │   └── test_history.cpp
│   ├── integration/            # 集成测试
│   │   └── test_shell.sh
│   └── data/                   # 测试数据
│       ├── input_samples/
│       └── expected_outputs/
│
├── build/                      # 🔨 构建系统
│   ├── cmake/                  # CMake模块
│   │   ├── FindThreads.cmake
│   │   └── Platform.cmake
│   ├── scripts/               # 构建脚本
│   │   ├── build.sh
│   │   └── build.bat
│   └── configs/               # 构建配置
│       ├── CMakeLists.txt
│       ├── Makefile
│       └── xmake.lua
│
├── .github/                    # 🤖 GitHub配置
│   ├── workflows/             # CI/CD工作流
│   │   ├── build.yml
│   │   └── release.yml
│   ├── ISSUE_TEMPLATE/        # Issue模板
│   └── PULL_REQUEST_TEMPLATE.md
│
└── assets/                     # 📦 资源文件
    ├── icons/                 # 图标文件
    ├── screenshots/           # 截图
    └── demo/                  # 演示文件
```

## 🔄 重构优势

### 1. 模块化清晰
- **core/**: 核心Shell功能模块
- **platform/**: 跨平台抽象层分离
- **tests/**: 测试代码独立组织

### 2. 文档管理
- **docs/**: 统一文档管理
- 便于维护和查找
- 支持文档网站生成

### 3. 构建系统
- **build/**: 构建相关文件集中
- 支持多种构建工具
- 配置文件分离

### 4. 开发友好
- 模块间依赖关系清晰
- 便于新功能开发
- 支持并行开发

## 📋 重构步骤

1. 创建新目录结构
2. 移动现有文件到对应目录
3. 更新构建配置文件中的路径
4. 更新包含头文件的路径
5. 测试构建和功能
6. 更新文档中的路径引用

## ⚠️ 注意事项

1. **保持Git历史**: 使用`git mv`移动文件
2. **更新引用**: 修改所有路径引用
3. **构建配置**: 更新CMakeLists.txt等配置
4. **CI/CD**: 确保GitHub Actions路径正确
5. **向后兼容**: 考虑现有用户的使用习惯