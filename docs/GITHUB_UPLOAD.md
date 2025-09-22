# GitHub上传指南

## 步骤1: 初始化Git仓库

```bash
cd /home/snow/workspace/mysh

# 初始化Git仓库
git init

# 添加所有文件
git add .

# 创建初始提交
git commit -m "Initial commit: Cross-platform shell implementation

- 支持Linux、macOS、Windows三大平台
- 完整的shell功能实现
- 模块化架构设计
- CMake跨平台构建系统
- GitHub Actions自动化构建"
```

## 步骤2: 在GitHub上创建仓库

1. 访问 https://github.com
2. 点击右上角的 "+" → "New repository"
3. 填写仓库信息：
   - Repository name: `mysh`
   - Description: `跨平台Shell工具 - 支持Linux/macOS/Windows的C++命令行解释器`
   - 选择 Public 或 Private
   - 不要勾选 "Add a README file"（我们已经有了）
   - 不要勾选 "Add .gitignore"（我们已经有了）
   - License: 选择 "MIT License"（或留空，我们已经有了）

## 步骤3: 连接本地仓库到GitHub

```bash
# 添加远程仓库（替换YOUR_USERNAME为你的GitHub用户名）
git remote add origin https://github.com/YOUR_USERNAME/mysh.git

# 验证远程仓库
git remote -v

# 推送到GitHub
git branch -M main
git push -u origin main
```

## 步骤4: 验证上传

访问你的GitHub仓库页面，应该能看到：
- 完整的项目文件
- README.md显示项目介绍
- GitHub Actions开始自动构建
- 跨平台支持的徽章

## 步骤5: 配置GitHub Pages（可选）

如果想要项目文档网站：

1. 在仓库设置中找到 "Pages"
2. Source选择 "Deploy from a branch"
3. Branch选择 "main"
4. Folder选择 "/ (root)"
5. Save

## 项目特色

这个项目的GitHub仓库将展示：

### 🌟 主要特色
- ✅ 跨平台支持（Linux/macOS/Windows）
- ✅ 现代C++17实现
- ✅ 模块化架构设计
- ✅ 完整的CI/CD流程
- ✅ 自动化构建和测试
- ✅ 详细的文档

### 📁 文件结构
```
mysh/
├── .github/workflows/    # GitHub Actions
├── src/                  # 源代码
├── docs/                 # 文档
├── CMakeLists.txt       # 跨平台构建
├── README.md            # 项目介绍
├── CONTRIBUTING.md      # 贡献指南
├── CHANGELOG.md         # 变更日志
├── LICENSE              # 开源协议
└── .gitignore          # Git忽略文件
```

### 🔄 自动化功能
- 推送代码时自动构建测试
- 创建tag时自动发布release
- 多平台并行构建
- 构建产物自动上传

## 后续维护

### 日常开发流程
```bash
# 创建新功能分支
git checkout -b feature/new-feature

# 开发和测试
# ...

# 提交更改
git add .
git commit -m "Add new feature: description"

# 推送分支
git push origin feature/new-feature

# 在GitHub上创建Pull Request
```

### 发布新版本
```bash
# 更新版本号和CHANGELOG
# ...

# 创建并推送tag
git tag v1.1.0
git push origin v1.1.0

# GitHub Actions会自动创建Release
```

这样你就有了一个专业的、功能完整的GitHub项目！