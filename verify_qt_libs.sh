#!/bin/bash
# 验证项目是否正确使用项目内的Qt库

echo "=========================================="
echo "验证Qt库配置"
echo "=========================================="
echo ""

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$PROJECT_ROOT/build-arm/bin/QtImx6ullBackend"
QT_LIB_DIR="$PROJECT_ROOT/third_party/qt5/lib"

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "1. 检查可执行文件..."
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}✗ 可执行文件不存在: $EXECUTABLE${NC}"
    echo "请先运行编译: cd build-arm && cmake .. && make"
    exit 1
fi
echo -e "${GREEN}✓ 可执行文件存在${NC}"
echo "  路径: $EXECUTABLE"
echo ""

echo "2. 检查可执行文件架构..."
FILE_INFO=$(file "$EXECUTABLE")
echo "  $FILE_INFO"
if echo "$FILE_INFO" | grep -q "ARM"; then
    echo -e "${GREEN}✓ ARM架构正确${NC}"
else
    echo -e "${RED}✗ 架构不正确${NC}"
fi
echo ""

echo "3. 检查RPATH设置..."
RPATH=$(readelf -d "$EXECUTABLE" | grep "RPATH" | awk '{print $5}')
echo "  RPATH: $RPATH"
if echo "$RPATH" | grep -q "third_party/qt5/lib"; then
    echo -e "${GREEN}✓ RPATH包含项目内的Qt库路径${NC}"
else
    echo -e "${RED}✗ RPATH未包含项目内的Qt库路径${NC}"
fi
echo ""

echo "4. 检查Qt库依赖..."
echo "  所需的Qt库:"
readelf -d "$EXECUTABLE" | grep "NEEDED" | grep "libQt5" | while read line; do
    LIB_NAME=$(echo "$line" | sed -n 's/.*\[\(.*\)\]/\1/p')
    echo "    - $LIB_NAME"
    
    # 检查库文件是否存在
    if [ -L "$QT_LIB_DIR/$LIB_NAME" ] || [ -f "$QT_LIB_DIR/$LIB_NAME" ]; then
        echo -e "      ${GREEN}✓ 存在于项目中${NC}"
    else
        echo -e "      ${RED}✗ 不存在于项目中${NC}"
    fi
done
echo ""

echo "5. 检查项目内的Qt库文件..."
QT_LIBS_COUNT=$(ls -1 "$QT_LIB_DIR"/libQt5*.so.5.12.9 2>/dev/null | wc -l)
echo "  找到 $QT_LIBS_COUNT 个Qt库文件:"
ls -1 "$QT_LIB_DIR"/libQt5*.so.5 2>/dev/null | while read lib; do
    LIB_NAME=$(basename "$lib")
    LIB_SIZE=$(du -h "$QT_LIB_DIR/$(readlink "$lib")" 2>/dev/null | awk '{print $1}')
    echo "    - $LIB_NAME ($LIB_SIZE)"
done
echo ""

echo "6. 检查CMake配置..."
if [ -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
    QT_PATH_IN_CMAKE=$(grep "set(QT_ARM_PATH" "$PROJECT_ROOT/CMakeLists.txt" | grep "third_party/qt5")
    if [ ! -z "$QT_PATH_IN_CMAKE" ]; then
        echo -e "${GREEN}✓ CMakeLists.txt配置使用项目内Qt库${NC}"
        echo "  $QT_PATH_IN_CMAKE"
    else
        echo -e "${RED}✗ CMakeLists.txt未配置使用项目内Qt库${NC}"
    fi
else
    echo -e "${RED}✗ CMakeLists.txt不存在${NC}"
fi
echo ""

echo "7. 验证路径解析..."
# 从可执行文件所在目录计算相对路径
EXEC_DIR="$(dirname "$EXECUTABLE")"
RELATIVE_QT_LIB="$EXEC_DIR/../../third_party/qt5/lib"
RESOLVED_PATH=$(cd "$RELATIVE_QT_LIB" 2>/dev/null && pwd)
if [ -d "$RESOLVED_PATH" ]; then
    echo -e "${GREEN}✓ 相对路径可以正确解析${NC}"
    echo "  可执行文件目录: $EXEC_DIR"
    echo "  相对Qt库路径: ../../third_party/qt5/lib"
    echo "  解析后的路径: $RESOLVED_PATH"
else
    echo -e "${RED}✗ 相对路径无法解析${NC}"
fi
echo ""

echo "=========================================="
echo "验证总结"
echo "=========================================="
echo ""
echo "配置状态:"
echo "  - 可执行文件: $EXECUTABLE"
echo "  - Qt库目录: $QT_LIB_DIR"
echo "  - RPATH: $RPATH"
echo ""

# 检查是否所有必需的库都存在
REQUIRED_LIBS=("libQt5Core.so.5" "libQt5Gui.so.5" "libQt5SerialBus.so.5" "libQt5Network.so.5" "libQt5SerialPort.so.5")
ALL_LIBS_PRESENT=true

for lib in "${REQUIRED_LIBS[@]}"; do
    if [ ! -L "$QT_LIB_DIR/$lib" ] && [ ! -f "$QT_LIB_DIR/$lib" ]; then
        ALL_LIBS_PRESENT=false
        echo -e "${RED}缺少库: $lib${NC}"
    fi
done

if [ "$ALL_LIBS_PRESENT" = true ]; then
    echo -e "${GREEN}✓ 所有Qt库配置正确！${NC}"
    echo ""
    echo "项目现在使用内置的Qt库，可以在其他电脑上编译。"
    echo "注意: 其他电脑仍需要:"
    echo "  1. ARM交叉编译工具链"
    echo "  2. 相应的sysroot"
    echo ""
else
    echo -e "${RED}✗ 配置不完整，请运行 ./copy_qt_libs.sh${NC}"
    exit 1
fi

