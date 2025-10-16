#!/bin/bash
# i.MX6ULL 交叉编译环境设置脚本

# 设置基础路径 - 根据您的实际路径修正
export TOOLCHAIN_BASE="/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi"
export TOOLCHAIN_PATH="/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin"
export SYSROOT_PATH="/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi"
# 使用项目内的Qt库（便于项目移植）
export QT_ARM_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/third_party/qt5"

# 将工具链添加到 PATH
export PATH="$TOOLCHAIN_BASE:$TOOLCHAIN_PATH:$PATH"

# 设置交叉编译变量
export CROSS_COMPILE="arm-poky-linux-gnueabi-"
export ARCH="arm"

# 设置编译器完整路径
export CC="${TOOLCHAIN_BASE}/${CROSS_COMPILE}gcc"
export CXX="${TOOLCHAIN_BASE}/${CROSS_COMPILE}g++"
export AR="${TOOLCHAIN_BASE}/${CROSS_COMPILE}ar"
export STRIP="${TOOLCHAIN_BASE}/${CROSS_COMPILE}strip"
export RANLIB="${TOOLCHAIN_BASE}/${CROSS_COMPILE}ranlib"

# 设置 pkg-config 环境
export PKG_CONFIG_SYSROOT_DIR="$SYSROOT_PATH"
export PKG_CONFIG_PATH="$SYSROOT_PATH/usr/lib/pkgconfig:$SYSROOT_PATH/usr/share/pkgconfig"
export PKG_CONFIG_LIBDIR="$SYSROOT_PATH/usr/lib/pkgconfig"

# 设置 CMAKE 变量
export CMAKE_TOOLCHAIN_FILE="$(pwd)/cmake/arm-imx6ull-toolchain.cmake"

echo "=========================================="
echo "i.MX6ULL 交叉编译环境已设置"
echo "工具链基础路径: $TOOLCHAIN_BASE"
echo "工具链路径: $TOOLCHAIN_PATH"
echo "Sysroot 路径: $SYSROOT_PATH"
echo "Qt 路径: $QT_ARM_PATH"
echo "=========================================="

# 验证工具链
echo "1. 验证 C++ 编译器路径:"
echo "编译器路径: $CXX"
if [ -f "$CXX" ]; then
    echo "✓ 编译器文件存在"
    $CXX --version
else
    echo "✗ 编译器文件不存在: $CXX"
    echo "请检查路径是否正确"
fi
echo ""

echo "2. 验证目标架构:"
$CXX -dumpmachine
echo ""

echo "3. 查看编译器搜索路径:"
$CXX -print-search-dirs | grep programs | head -3
echo ""

echo "4. 验证 Qt 库是否存在:"
if [ -d "$QT_ARM_PATH/lib" ]; then
    echo "✓ Qt 库目录存在: $QT_ARM_PATH/lib"
    ls $QT_ARM_PATH/lib/libQt5Core.so.* 2>/dev/null | head -1 || echo "未找到 libQt5Core.so"
else
    echo "✗ Qt 库目录不存在: $QT_ARM_PATH/lib"
fi
echo ""

echo "5. 验证 Sysroot 库:"
if [ -d "$SYSROOT_PATH/usr/lib" ]; then
    echo "✓ Sysroot 库目录存在"
    ls $SYSROOT_PATH/usr/lib/libc.so.* 2>/dev/null | head -1 || echo "未找到 libc.so"
else
    echo "✗ Sysroot 库目录不存在"
fi
echo ""

echo "环境设置完成！"