#!/bin/bash

# MyShell 语法高亮演示脚本
echo "=== MyShell 语法高亮功能演示 ==="
echo

# 启动MyShell并展示语法高亮
cat << 'EOF' | ./mysh
set syntax-highlight on
echo "语法高亮已启用！"
echo

echo "=== 内置命令高亮演示 ==="
help
pwd
cd /tmp
cd -

echo 
echo "=== 外部命令高亮演示 ==="
ls -la | head -3
echo "外部命令会以不同颜色显示"

echo
echo "=== 环境变量高亮演示 ==="
echo "HOME目录: $HOME"
echo "用户名: $USER"
echo "PATH: $PATH" | head -c 50

echo
echo "=== 字符串和选项高亮演示 ==="
echo "这是一个字符串"
echo -n "这是不换行的输出"
echo

echo
echo "=== 操作符高亮演示 ==="
echo "管道操作: ls | wc -l"
echo "重定向: echo 'test' > /tmp/test.txt"
echo "后台运行: sleep 1 &"

echo
echo "=== 配置查看 ==="
set

echo
echo "演示完成！"
exit
EOF