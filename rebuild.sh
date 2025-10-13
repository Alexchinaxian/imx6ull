#!/bin/bash

# 定义构建参数
BUILD_DIR="build-arm"
THREADS=16
CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=Release"

echo "=== 优化构建脚本 (线程数: $THREADS) ==="

# 安全检查
if [ ! -f "CMakeLists.txt" ] && [ ! -f "../CMakeLists.txt" ]; then
    echo "错误：未检测到CMakeLists.txt文件"
    exit 1
fi

# 创建并进入构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

echo "工作目录: $(pwd)"
echo "清理构建目录..."

# 安全清理
find . -maxdepth 1 -mindepth 1 -exec rm -rf {} + 2>/dev/null

echo "运行CMake配置..."
echo "CMake选项: $CMAKE_OPTIONS"
if ! cmake .. $CMAKE_OPTIONS; then
    echo "CMake配置失败"
    exit 1
fi

echo "开始编译 (使用 $THREADS 线程)..."
# 使用16线程编译，并显示详细进度
if ! make -j$THREADS VERBOSE=1; then
    echo "编译失败，尝试使用单线程编译进行错误诊断..."
    make VERBOSE=1
    exit 1
fi

echo "✅ 构建成功完成！"
echo "📊 使用了 $THREADS 个并行线程"
echo "💾 最终目标文件大小:"
find . -name "*.so" -o -name "*.a" -o -name "*.exe" -type f 2>/dev/null | xargs ls -lh 2>/dev/null || echo "未找到可执行文件或库文件"