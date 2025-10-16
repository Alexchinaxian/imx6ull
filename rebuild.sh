#!/bin/bash
########################################################################
# 脚本名称: rebuild.sh
# 功能描述: 快速重新编译项目
# 作者: Alex
# 日期: 2025-10-15
########################################################################

echo "========================================="
echo "IMX6ULL项目快速编译"
echo "========================================="
echo ""

cd "$(dirname "$0")"

# 使用build-arm目录进行ARM交叉编译
BUILD_DIR="build-arm"

# 检查build-arm目录是否存在
if [ ! -d "$BUILD_DIR" ]; then
    echo "✓ 创建$BUILD_DIR目录..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    echo "✓ 运行CMake配置..."
    cmake ..
    if [ $? -ne 0 ]; then
        echo "❌ CMake配置失败"
        exit 1
    fi
elif [ ! -f "$BUILD_DIR/Makefile" ]; then
    # 目录存在但没有Makefile，需要重新配置
    echo "✓ 检测到未配置，运行CMake配置..."
    cd "$BUILD_DIR"
    cmake ..
    if [ $? -ne 0 ]; then
        echo "❌ CMake配置失败"
        exit 1
    fi
else
    cd "$BUILD_DIR"
fi

echo ""
echo "✓ 开始编译..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "✅ 编译成功！"
    echo "========================================="
    echo ""
    echo "可执行文件位置："
    echo "  $(pwd)/bin/QtImx6ullBackend"
    echo ""
    echo "部署到设备："
    echo "  cd $(dirname "$0") && ./download.sh"
    echo ""
else
    echo ""
    echo "========================================="
    echo "❌ 编译失败"
    echo "========================================="
    exit 1
fi
