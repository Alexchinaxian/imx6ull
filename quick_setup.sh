#!/bin/bash
# IMX6ULL 项目一键部署脚本
# 自动完成所有环境配置和项目编译

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}IMX6ULL 项目一键部署${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "项目路径: $PROJECT_ROOT"
echo ""

# 步骤1: 检查依赖
echo -e "${BLUE}[1/5] 检查系统依赖...${NC}"
MISSING_DEPS=()

for cmd in cmake make git gcc g++; do
    if ! command -v $cmd &> /dev/null; then
        MISSING_DEPS+=($cmd)
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${YELLOW}缺少依赖: ${MISSING_DEPS[*]}${NC}"
    echo "尝试安装依赖..."
    
    if command -v apt-get &> /dev/null; then
        sudo apt-get update
        sudo apt-get install -y build-essential cmake git
    elif command -v yum &> /dev/null; then
        sudo yum install -y gcc gcc-c++ make cmake git
    else
        echo -e "${RED}无法自动安装依赖，请手动安装: ${MISSING_DEPS[*]}${NC}"
        exit 1
    fi
fi
echo -e "${GREEN}✓ 系统依赖检查完成${NC}"
echo ""

# 步骤2: 检查交叉编译工具链
echo -e "${BLUE}[2/5] 检查交叉编译工具链...${NC}"
TOOLCHAIN_PATH="/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi"
COMPILER="${TOOLCHAIN_PATH}/arm-poky-linux-gnueabi-gcc"

if [ ! -f "$COMPILER" ]; then
    echo -e "${YELLOW}未找到交叉编译工具链${NC}"
    echo "工具链安装选项:"
    echo "  1. 自动安装（需要root权限）"
    echo "  2. 手动安装后继续"
    echo "  3. 跳过（仅配置项目）"
    read -p "请选择 (1-3): " choice
    
    case $choice in
        1)
            if [ ! -f "$PROJECT_ROOT/setup_toolchain.sh" ]; then
                echo -e "${RED}错误: setup_toolchain.sh 不存在${NC}"
                exit 1
            fi
            sudo bash "$PROJECT_ROOT/setup_toolchain.sh"
            ;;
        2)
            echo "请手动安装工具链后重新运行此脚本"
            exit 0
            ;;
        3)
            echo -e "${YELLOW}跳过工具链检查${NC}"
            ;;
    esac
else
    echo -e "${GREEN}✓ 工具链已安装${NC}"
    "$COMPILER" --version | head -1
fi
echo ""

# 步骤3: 检查Qt库
echo -e "${BLUE}[3/5] 检查项目Qt库...${NC}"
QT_LIB_DIR="$PROJECT_ROOT/third_party/qt5/lib"

# 检查必需的Qt库文件
REQUIRED_QT_LIBS=(
    "libQt5Core.so.5.12.9"
    "libQt5Gui.so.5.12.9"
    "libQt5Network.so.5.12.9"
    "libQt5SerialBus.so.5.12.9"
    "libQt5SerialPort.so.5.12.9"
)

MISSING_LIBS=()
for lib in "${REQUIRED_QT_LIBS[@]}"; do
    if [ ! -f "$QT_LIB_DIR/$lib" ]; then
        MISSING_LIBS+=("$lib")
    fi
done

if [ ${#MISSING_LIBS[@]} -ne 0 ]; then
    echo -e "${RED}✗ Qt库文件缺失!${NC}"
    echo "缺少以下库文件:"
    for lib in "${MISSING_LIBS[@]}"; do
        echo "  - $lib"
    done
    echo ""
    echo -e "${YELLOW}可能的原因:${NC}"
    echo "  1. Git克隆不完整 - 尝试重新克隆或运行: git pull"
    echo "  2. 文件被删除 - 运行: git reset --hard origin/master"
    echo "  3. 需要从本地Qt安装复制 - 设置QT_SOURCE_PATH并运行 ./copy_qt_libs.sh"
    echo ""
    echo -e "${BLUE}解决方案:${NC}"
    echo "  查看详细说明: docs/Qt库问题解决方案.md"
    echo ""
    read -p "是否继续? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    # 检查文件大小，确保不是空文件
    TOTAL_SIZE=$(du -sh "$QT_LIB_DIR" 2>/dev/null | awk '{print $1}')
    QT_LIBS_COUNT=${#REQUIRED_QT_LIBS[@]}
    echo -e "${GREEN}✓ Qt库已就绪${NC}"
    echo "  - 库文件数量: $QT_LIBS_COUNT"
    echo "  - Qt库大小: $TOTAL_SIZE"
fi
echo ""

# 步骤4: 配置项目
echo -e "${BLUE}[4/5] 配置项目...${NC}"

# 加载工具链环境
if [ -f "/etc/profile.d/imx6ull-toolchain.sh" ]; then
    source /etc/profile.d/imx6ull-toolchain.sh
    echo "已加载工具链环境"
elif [ -f "$PROJECT_ROOT/env-setup.sh" ]; then
    source "$PROJECT_ROOT/env-setup.sh"
    echo "已加载项目环境"
fi

# 创建构建目录
BUILD_DIR="$PROJECT_ROOT/build-arm"
if [ -d "$BUILD_DIR" ]; then
    echo "清理旧的构建目录..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "运行CMake配置..."
if cmake .. ; then
    echo -e "${GREEN}✓ 项目配置成功${NC}"
else
    echo -e "${RED}✗ 项目配置失败${NC}"
    exit 1
fi
echo ""

# 步骤5: 编译项目
echo -e "${BLUE}[5/5] 编译项目...${NC}"
CPU_CORES=$(nproc)
echo "使用 $CPU_CORES 个CPU核心编译..."

if make -j$CPU_CORES ; then
    echo -e "${GREEN}✓ 项目编译成功${NC}"
else
    echo -e "${RED}✗ 项目编译失败${NC}"
    exit 1
fi
echo ""

# 验证构建结果
echo -e "${BLUE}验证构建结果...${NC}"
EXECUTABLE="$BUILD_DIR/bin/QtImx6ullBackend"

if [ -f "$EXECUTABLE" ]; then
    echo -e "${GREEN}✓ 可执行文件已生成${NC}"
    echo "  路径: $EXECUTABLE"
    echo "  大小: $(du -h "$EXECUTABLE" | cut -f1)"
    
    # 检查架构
    FILE_INFO=$(file "$EXECUTABLE")
    echo "  架构: $FILE_INFO"
    
    # 运行验证脚本
    if [ -f "$PROJECT_ROOT/verify_qt_libs.sh" ]; then
        echo ""
        echo -e "${BLUE}运行配置验证...${NC}"
        bash "$PROJECT_ROOT/verify_qt_libs.sh"
    fi
else
    echo -e "${RED}✗ 未找到可执行文件${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓✓✓ 部署完成！ ✓✓✓${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "构建信息:"
echo "  - 可执行文件: $EXECUTABLE"
echo "  - 构建目录: $BUILD_DIR"
echo "  - Qt库目录: $QT_LIB_DIR"
echo ""
echo "下一步:"
echo "  1. 将可执行文件和Qt库部署到目标设备"
echo "  2. 在设备上运行: ./QtImx6ullBackend"
echo "  3. 查看日志和测试功能"
echo ""
echo "常用命令:"
echo "  - 重新编译: cd $BUILD_DIR && make -j$CPU_CORES"
echo "  - 清理构建: rm -rf $BUILD_DIR"
echo "  - 验证配置: $PROJECT_ROOT/verify_qt_libs.sh"
echo ""

