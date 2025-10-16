#!/bin/bash
########################################################################
# 脚本名称: setup_test_beep.sh
# 功能描述: 创建模拟蜂鸣器设备用于测试
# 作者: Alex
# 日期: 2025-10-15
########################################################################

echo "========================================="
echo "创建模拟蜂鸣器设备（用于测试）"
echo "========================================="
echo ""

# 检查是否有root权限
if [ "$EUID" -ne 0 ]; then 
    echo "❌ 需要root权限来创建设备"
    echo "请使用: sudo $0"
    exit 1
fi

# 创建蜂鸣器设备目录
BEEP_DIR="/sys/class/leds/beep"

echo "1. 创建设备目录: $BEEP_DIR"
mkdir -p "$BEEP_DIR"

# 创建brightness文件
echo "2. 创建brightness控制文件"
echo "0" > "$BEEP_DIR/brightness"
chmod 666 "$BEEP_DIR/brightness"

# 检查创建结果
if [ -f "$BEEP_DIR/brightness" ]; then
    echo "✅ 模拟蜂鸣器设备创建成功"
    echo ""
    echo "设备信息:"
    ls -la "$BEEP_DIR/"
    echo ""
    echo "========================================="
    echo "现在可以测试SystemBeep了！"
    echo "========================================="
else
    echo "❌ 设备创建失败"
    exit 1
fi

