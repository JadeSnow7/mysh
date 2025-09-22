#!/bin/bash

# MyShell功能测试脚本

echo "开始测试MyShell功能..."
echo "================================"

# 创建测试文件
echo "Creating test files..."
echo "test line 1" > test1.txt
echo "test line 2" > test2.txt
echo "hello world" > hello.txt

# 启动mysh并执行测试命令
./build/linux/x86_64/release/mysh << 'EOF'
# 测试基本命令
pwd
echo "当前目录测试完成"

# 测试cd命令
cd /tmp
pwd
cd -
pwd

# 测试echo命令
echo "Hello from MyShell"
echo -n "No newline"
echo ""

# 测试环境变量
echo $HOME
echo $USER

# 测试重定向
echo "Redirection test" > output.txt
cat output.txt

# 测试追加重定向
echo "Appended line" >> output.txt
cat output.txt

# 测试管道（如果有可用命令）
ls -la | head -5

# 测试which命令
which ls
which nonexistent_command

# 测试history命令
history

# 测试help命令
help

# 退出shell
exit 0
EOF

echo "================================"
echo "测试完成！"

# 清理测试文件
rm -f test1.txt test2.txt hello.txt output.txt