# 贡献指南

感谢你对MyShell项目的关注！我们欢迎各种形式的贡献。

## 如何贡献

### 报告Bug
1. 在GitHub Issues中搜索是否已有相同问题
2. 如果没有，创建新的Issue，请包含：
   - 操作系统和版本
   - 编译器版本
   - 详细的重现步骤
   - 期望的行为和实际行为
   - 相关的错误信息

### 功能请求
1. 在Issues中描述你希望添加的功能
2. 解释为什么这个功能有用
3. 如果可能，提供实现思路

### 代码贡献

#### 开发环境设置
1. Fork这个仓库
2. 克隆你的fork：
   ```bash
   git clone https://github.com/YOUR_USERNAME/mysh.git
   cd mysh
   ```
3. 安装依赖并构建：
   ```bash
   # Linux/macOS
   ./build.sh build
   
   # Windows
   build.bat build
   ```

#### 编码规范
- 使用C++17标准
- 遵循驼峰命名法（camelCase）
- 类名使用帕斯卡命名法（PascalCase）
- 添加适当的注释
- 保持模块化设计原则

#### 提交流程
1. 创建新分支：
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. 进行修改并测试
3. 提交修改：
   ```bash
   git add .
   git commit -m "描述你的修改"
   ```
4. 推送到你的fork：
   ```bash
   git push origin feature/your-feature-name
   ```
5. 创建Pull Request

#### Pull Request要求
- 清晰描述修改内容
- 包含相关测试
- 确保所有平台的CI测试通过
- 更新相关文档

### 测试
运行测试确保你的修改不会破坏现有功能：

```bash
# Linux/macOS
./build.sh test

# Windows
build.bat test
```

## 项目结构

```
mysh/
├── src/                 # 源代码
│   ├── main.cpp        # 程序入口
│   ├── shell.*         # Shell主类
│   ├── parser.*        # 命令解析器
│   ├── executor.*      # 命令执行器
│   ├── builtin.*       # 内置命令
│   ├── history.*       # 历史记录
│   └── platform.*      # 跨平台抽象层
├── .github/workflows/   # GitHub Actions
├── docs/               # 文档
└── tests/              # 测试文件
```

## 开发指南

### 添加新的内置命令
1. 在`builtin.h`中声明新方法
2. 在`builtin.cpp`中实现命令逻辑
3. 在`initializeBuiltins()`中注册命令
4. 更新帮助文档

### 添加新平台支持
1. 实现`PlatformInterface`接口
2. 在`platform.cpp`中添加平台检测
3. 更新CMake配置
4. 添加平台特定的CI测试

## 发布流程

### 版本号规则
我们使用语义化版本控制：
- 主版本号：不兼容的API修改
- 次版本号：向下兼容的功能性新增
- 修订号：向下兼容的问题修正

### 发布步骤
1. 更新版本号
2. 更新CHANGELOG.md
3. 创建并推送tag：
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
4. GitHub Actions会自动构建并创建Release

## 行为准则

请遵循以下准则：
- 尊重他人
- 欢迎新贡献者
- 建设性地提供反馈
- 专注于最有利于社区的行为

## 联系方式

如有疑问，可以：
- 在GitHub Issues中提问
- 在Pull Request中讨论
- 查看项目文档

感谢你的贡献！🎉