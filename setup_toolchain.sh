#!/bin/bash
# IMX6ULL 交叉编译工具链自动安装脚本
# 用于在新机器上快速部署开发环境

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 工具链配置
TOOLCHAIN_VERSION="4.1.15-2.1.0"
TOOLCHAIN_NAME="fsl-imx-x11-glibc-x86_64-meta-toolchain-qt5-cortexa7hf-neon-toolchain-${TOOLCHAIN_VERSION}.sh"
TOOLCHAIN_INSTALL_DIR="/opt/fsl-imx-x11/${TOOLCHAIN_VERSION}"

# 下载URL（根据实际情况修改）
# 可以从NXP官网、公司内部服务器或其他来源下载
TOOLCHAIN_URL="https://example.com/toolchain/${TOOLCHAIN_NAME}"
# 或者使用本地路径
TOOLCHAIN_LOCAL_PATH="/home/shared/toolchains/${TOOLCHAIN_NAME}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}IMX6ULL 交叉编译工具链安装程序${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查是否以root权限运行
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}错误: 此脚本需要root权限运行${NC}"
    echo "请使用: sudo $0"
    exit 1
fi

# 检查工具链是否已安装
if [ -d "$TOOLCHAIN_INSTALL_DIR" ]; then
    echo -e "${YELLOW}工具链已安装在: $TOOLCHAIN_INSTALL_DIR${NC}"
    read -p "是否重新安装? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${GREEN}保持现有安装，退出。${NC}"
        exit 0
    fi
    echo -e "${YELLOW}删除现有安装...${NC}"
    rm -rf "$TOOLCHAIN_INSTALL_DIR"
fi

# 检查磁盘空间（需要至少5GB）
available_space=$(df /opt | tail -1 | awk '{print $4}')
required_space=$((5 * 1024 * 1024))  # 5GB in KB

if [ "$available_space" -lt "$required_space" ]; then
    echo -e "${RED}错误: /opt 分区空间不足${NC}"
    echo "需要至少 5GB 空闲空间"
    exit 1
fi

# 创建安装目录
echo -e "${BLUE}1. 创建安装目录...${NC}"
mkdir -p "$(dirname "$TOOLCHAIN_INSTALL_DIR")"

# 获取工具链安装包
echo -e "${BLUE}2. 获取工具链安装包...${NC}"
INSTALLER_PATH=""

# 优先检查本地路径
if [ -f "$TOOLCHAIN_LOCAL_PATH" ]; then
    echo -e "${GREEN}找到本地工具链文件: $TOOLCHAIN_LOCAL_PATH${NC}"
    INSTALLER_PATH="$TOOLCHAIN_LOCAL_PATH"
# 检查当前目录
elif [ -f "./${TOOLCHAIN_NAME}" ]; then
    echo -e "${GREEN}使用当前目录的工具链文件${NC}"
    INSTALLER_PATH="./${TOOLCHAIN_NAME}"
else
    echo -e "${YELLOW}本地未找到工具链，提供以下选项：${NC}"
    echo "1. 从URL下载 (需要配置TOOLCHAIN_URL)"
    echo "2. 手动指定路径"
    echo "3. 退出"
    read -p "请选择 (1-3): " choice
    
    case $choice in
        1)
            if [ "$TOOLCHAIN_URL" = "https://example.com/toolchain/${TOOLCHAIN_NAME}" ]; then
                echo -e "${RED}错误: 请在脚本中配置正确的TOOLCHAIN_URL${NC}"
                exit 1
            fi
            echo -e "${YELLOW}从 $TOOLCHAIN_URL 下载...${NC}"
            wget -O "/tmp/${TOOLCHAIN_NAME}" "$TOOLCHAIN_URL"
            INSTALLER_PATH="/tmp/${TOOLCHAIN_NAME}"
            ;;
        2)
            read -p "请输入工具链安装包完整路径: " user_path
            if [ ! -f "$user_path" ]; then
                echo -e "${RED}错误: 文件不存在: $user_path${NC}"
                exit 1
            fi
            INSTALLER_PATH="$user_path"
            ;;
        *)
            echo "退出安装"
            exit 0
            ;;
    esac
fi

# 安装工具链
echo -e "${BLUE}3. 安装工具链...${NC}"
echo "安装位置: $TOOLCHAIN_INSTALL_DIR"
echo "这可能需要几分钟时间..."

chmod +x "$INSTALLER_PATH"
"$INSTALLER_PATH" -d "$TOOLCHAIN_INSTALL_DIR" -y

if [ $? -ne 0 ]; then
    echo -e "${RED}错误: 工具链安装失败${NC}"
    exit 1
fi

# 验证安装
echo -e "${BLUE}4. 验证安装...${NC}"
COMPILER="${TOOLCHAIN_INSTALL_DIR}/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc"

if [ ! -f "$COMPILER" ]; then
    echo -e "${RED}错误: 未找到编译器: $COMPILER${NC}"
    exit 1
fi

echo "编译器版本:"
"$COMPILER" --version | head -1

# 创建环境变量文件
echo -e "${BLUE}5. 创建环境配置文件...${NC}"
ENV_FILE="/etc/profile.d/imx6ull-toolchain.sh"

cat > "$ENV_FILE" << EOF
# IMX6ULL 交叉编译工具链环境变量
export TOOLCHAIN_BASE="${TOOLCHAIN_INSTALL_DIR}/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi"
export TOOLCHAIN_PATH="${TOOLCHAIN_INSTALL_DIR}/sysroots/x86_64-pokysdk-linux/usr/bin"
export SYSROOT_PATH="${TOOLCHAIN_INSTALL_DIR}/sysroots/cortexa7hf-neon-poky-linux-gnueabi"

# 添加到PATH
export PATH="\$TOOLCHAIN_BASE:\$TOOLCHAIN_PATH:\$PATH"

# 交叉编译工具
export CROSS_COMPILE="arm-poky-linux-gnueabi-"
export ARCH="arm"
export CC="\${TOOLCHAIN_BASE}/arm-poky-linux-gnueabi-gcc"
export CXX="\${TOOLCHAIN_BASE}/arm-poky-linux-gnueabi-g++"
EOF

chmod +x "$ENV_FILE"

echo -e "${GREEN}环境配置文件已创建: $ENV_FILE${NC}"

# 创建便捷脚本
echo -e "${BLUE}6. 创建便捷工具...${NC}"

# 创建工具链环境加载脚本
cat > "/usr/local/bin/imx6ull-env" << 'EOF'
#!/bin/bash
# 加载IMX6ULL工具链环境
source /etc/profile.d/imx6ull-toolchain.sh
echo "IMX6ULL工具链环境已加载"
echo "编译器: $CC"
echo "Sysroot: $SYSROOT_PATH"
EOF
chmod +x /usr/local/bin/imx6ull-env

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ 工具链安装完成！${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "安装信息:"
echo "  - 安装路径: $TOOLCHAIN_INSTALL_DIR"
echo "  - 编译器: ${TOOLCHAIN_INSTALL_DIR}/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc"
echo "  - Sysroot: ${TOOLCHAIN_INSTALL_DIR}/sysroots/cortexa7hf-neon-poky-linux-gnueabi"
echo ""
echo "使用方法:"
echo "  1. 重新登录或运行: source /etc/profile.d/imx6ull-toolchain.sh"
echo "  2. 或使用快捷命令: imx6ull-env"
echo "  3. 然后就可以编译项目了"
echo ""
echo -e "${YELLOW}提示: 建议重新登录以确保环境变量生效${NC}"

