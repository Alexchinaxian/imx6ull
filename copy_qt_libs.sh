#!/bin/bash
# 用于复制Qt5库到项目中的脚本
# 这样项目可以在没有Qt环境的电脑上编译

# Qt源路径（如果需要从其他位置复制Qt，请修改此路径）
QT_SOURCE_PATH="${QT_SOURCE_PATH:-/home/alex/qt-everywhere-src-5.12.9/arm-qt}"

# 项目内Qt目标路径
QT_TARGET_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/third_party/qt5"

echo "=========================================="
echo "复制Qt5库到项目"
echo "=========================================="
echo "源路径: $QT_SOURCE_PATH"
echo "目标路径: $QT_TARGET_PATH"
echo ""

# 检查源路径是否存在
if [ ! -d "$QT_SOURCE_PATH" ]; then
    echo "错误: Qt源路径不存在: $QT_SOURCE_PATH"
    echo "请设置环境变量 QT_SOURCE_PATH 指向您的Qt安装路径"
    echo "例如: export QT_SOURCE_PATH=/path/to/qt/arm-qt"
    exit 1
fi

# 创建目标目录
mkdir -p "$QT_TARGET_PATH/lib"
mkdir -p "$QT_TARGET_PATH/include"

echo "1. 复制Qt库文件..."
# 复制核心库
cp -r "$QT_SOURCE_PATH/lib/libQt5Core."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5Core"

# 复制项目需要的Qt模块
cp -r "$QT_SOURCE_PATH/lib/libQt5Gui."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5Gui"

cp -r "$QT_SOURCE_PATH/lib/libQt5SerialBus."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5SerialBus"

cp -r "$QT_SOURCE_PATH/lib/libQt5Network."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5Network"

cp -r "$QT_SOURCE_PATH/lib/libQt5SerialPort."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5SerialPort"

# 复制依赖库
cp -r "$QT_SOURCE_PATH/lib/libQt5DBus."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5DBus (依赖)"

cp -r "$QT_SOURCE_PATH/lib/libQt5XcbQpa."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5XcbQpa (依赖)"

cp -r "$QT_SOURCE_PATH/lib/libQt5EventDispatcherSupport."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5EventDispatcherSupport (依赖)"

cp -r "$QT_SOURCE_PATH/lib/libQt5FontDatabaseSupport."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5FontDatabaseSupport (依赖)"

cp -r "$QT_SOURCE_PATH/lib/libQt5ServiceSupport."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5ServiceSupport (依赖)"

cp -r "$QT_SOURCE_PATH/lib/libQt5ThemeSupport."* "$QT_TARGET_PATH/lib/" 2>/dev/null
echo "  - Qt5ThemeSupport (依赖)"

echo ""
echo "2. 复制CMake配置文件..."
cp -r "$QT_SOURCE_PATH/lib/cmake" "$QT_TARGET_PATH/" 2>/dev/null
echo "  - CMake配置文件已复制"

echo ""
echo "3. 复制Qt头文件..."
cp -r "$QT_SOURCE_PATH/include"/* "$QT_TARGET_PATH/include/" 2>/dev/null
echo "  - 头文件已复制"

echo ""
echo "4. 复制Qt插件（如果存在）..."
if [ -d "$QT_SOURCE_PATH/plugins" ]; then
    mkdir -p "$QT_TARGET_PATH/plugins"
    cp -r "$QT_SOURCE_PATH/plugins"/* "$QT_TARGET_PATH/plugins/" 2>/dev/null
    echo "  - 插件已复制"
else
    echo "  - 插件目录不存在，跳过"
fi

echo ""
echo "=========================================="
echo "Qt5库复制完成！"
echo "=========================================="
echo ""
echo "已复制的文件统计:"
echo "  - 库文件: $(ls -1 $QT_TARGET_PATH/lib/libQt5*.so.* 2>/dev/null | wc -l) 个"
echo "  - 头文件目录: $(ls -1d $QT_TARGET_PATH/include/Qt* 2>/dev/null | wc -l) 个"
echo ""
echo "注意事项:"
echo "1. CMakeLists.txt 已配置为使用项目内的Qt库"
echo "2. 现在可以将整个项目复制到其他电脑上编译"
echo "3. 其他电脑上需要有交叉编译工具链（arm-poky-linux-gnueabi-gcc）"
echo "4. 其他电脑上需要有sysroot: $CMAKE_SYSROOT"
echo ""

