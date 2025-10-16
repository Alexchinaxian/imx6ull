#!/bin/bash

# 程序和库文件部署运行脚本

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

TARGET_IP="192.168.1.24"
TARGET_USER="root"
TARGET_DIR="~"  # home目录（程序和库都部署到这里）
PROGRAM_NAME="QtImx6ullBackend"
LIB_NAME="HardwarePlugin"  # 统一的硬件插件库

BUILD_DIR="/home/alex/imx6ull/build"
BUILD_FILE="$BUILD_DIR/bin/$PROGRAM_NAME"
BUILD_LIB_DIR="$BUILD_DIR/lib"

# 参数处理
RUN_AFTER_DOWNLOAD=true
RUN_IN_BACKGROUND=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --no-run)
            RUN_AFTER_DOWNLOAD=false
            shift
            ;;
        --background|-b)
            RUN_IN_BACKGROUND=true
            shift
            ;;
        --help|-h)
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  --no-run        只下载，不运行程序"
            echo "  --background,-b 在后台运行程序"
            echo "  --help,-h       显示帮助信息"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}未知参数: $1${NC}"
            echo "使用 --help 查看帮助"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  程序和库文件部署脚本${NC}"
echo -e "${BLUE}========================================${NC}"

# ==========================================
# 步骤1：检查本地文件
# ==========================================
echo -e "${YELLOW}[1/6]${NC} 检查本地文件..."

# 检查可执行文件
if [ ! -f "$BUILD_FILE" ]; then
    echo -e "${RED}✗ 错误: 程序文件不存在${NC}"
    echo "文件路径: $BUILD_FILE"
    echo ""
    echo -e "${YELLOW}请先运行: ./rebuild.sh${NC}"
    exit 1
fi

echo -e "${GREEN}✓ 找到程序文件${NC}"
echo -e "  路径: ${CYAN}$BUILD_FILE${NC}"
ls -lh "$BUILD_FILE" | awk '{print "  大小: " $5 " | 时间: " $6 " " $7 " " $8}'

# 检查硬件插件库文件（可选）
if [ -d "$BUILD_LIB_DIR" ]; then
    LIB_FILES=$(find "$BUILD_LIB_DIR" -name "lib${LIB_NAME}.so*" -type f 2>/dev/null)
    LIB_COUNT=$(echo "$LIB_FILES" | grep -c "\.so\." 2>/dev/null || echo 0)
    
    if [ $LIB_COUNT -gt 0 ]; then
        echo -e "${GREEN}✓ 找到硬件插件库${NC}"
        echo "$LIB_FILES" | while read lib; do
            [ -f "$lib" ] && ls -lh "$lib" | awk '{print "  " $9 " (" $5 ")"}'
        done
    else
        echo -e "${YELLOW}⚠ 硬件插件库不存在（已禁用）${NC}"
    fi
else
    echo -e "${YELLOW}⚠ 硬件插件库目录不存在（已禁用）${NC}"
fi
echo ""

# ==========================================
# 步骤2：停止设备上的旧程序
# ==========================================
echo -e "${YELLOW}[2/6]${NC} 停止设备上的旧程序..."
ssh $TARGET_USER@$TARGET_IP "killall -9 $PROGRAM_NAME 2>/dev/null; sleep 1; rm -f $TARGET_DIR/$PROGRAM_NAME" 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} 旧程序已停止并删除"
else
    echo -e "${YELLOW}⚠${NC} 设备上可能没有旧程序运行"
fi
echo ""

# ==========================================
# 步骤3：部署硬件插件库文件
# ==========================================
echo -e "${YELLOW}[3/6]${NC} 部署硬件插件库..."

if [ -d "$BUILD_LIB_DIR" ] && [ $LIB_COUNT -gt 0 ]; then
    echo -e "  目标目录: ${CYAN}$TARGET_USER@$TARGET_IP:$TARGET_DIR${NC}"
    
    # 上传所有.so文件到~目录
    echo "$LIB_FILES" | while read lib; do
        if [ -f "$lib" ]; then
            LIB_FILENAME=$(basename "$lib")
            echo -n "  上传 $LIB_FILENAME... "
            scp -q "$lib" $TARGET_USER@$TARGET_IP:$TARGET_DIR/
            if [ $? -eq 0 ]; then
                echo -e "${GREEN}成功${NC}"
            else
                echo -e "${RED}失败${NC}"
                exit 1
            fi
        fi
    done
    
    # 创建符号链接
    ssh $TARGET_USER@$TARGET_IP << EOF
cd ~
for lib in lib${LIB_NAME}.so.*.*.*; do
    [ -f "\$lib" ] || continue
    SONAME="\${lib%.*.*}"
    LINKNAME="\${lib%.*.*.*}"
    ln -sf "\$lib" "\$SONAME" 2>/dev/null
    ln -sf "\$SONAME" "\$LINKNAME" 2>/dev/null
done
EOF
    
    echo -e "${GREEN}✓${NC} 硬件插件库部署完成"
else
    echo -e "${YELLOW}⚠ 跳过硬件插件库部署（库文件不存在）${NC}"
fi
echo ""

# ==========================================
# 步骤4：部署主程序
# ==========================================
echo -e "${YELLOW}[4/6]${NC} 部署主程序..."
echo -e "  目标: ${CYAN}$TARGET_USER@$TARGET_IP:$TARGET_DIR/$PROGRAM_NAME${NC}"

scp -q "$BUILD_FILE" $TARGET_USER@$TARGET_IP:$TARGET_DIR/

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} 程序上传成功"
else
    echo -e "${RED}✗${NC} 程序上传失败"
    exit 1
fi
echo ""

# ==========================================
# 步骤5：部署工具脚本
# ==========================================
echo -e "${YELLOW}[5/7]${NC} 部署工具脚本..."

TOOLS_DIR="$(dirname "$0")/tools"

if [ -d "$TOOLS_DIR" ]; then
    echo -e "  源目录: ${CYAN}$TOOLS_DIR${NC}"
    echo -e "  目标: ${CYAN}$TARGET_USER@$TARGET_IP:~/tools/${NC}"
    
    # 上传tools目录
    scp -r -q "$TOOLS_DIR" $TARGET_USER@$TARGET_IP:~/
    
    if [ $? -eq 0 ]; then
        # 设置脚本执行权限
        ssh $TARGET_USER@$TARGET_IP "cd ~/tools && chmod +x *.sh 2>/dev/null"
        echo -e "${GREEN}✓${NC} 工具脚本部署完成"
        
        # 显示已部署的工具
        TOOL_COUNT=$(ssh $TARGET_USER@$TARGET_IP "ls -1 ~/tools/*.sh 2>/dev/null | wc -l")
        echo -e "  已部署 ${CYAN}${TOOL_COUNT}${NC} 个工具脚本"
    else
        echo -e "${YELLOW}⚠${NC} 工具脚本上传失败（非关键）"
    fi
else
    echo -e "${YELLOW}⚠${NC} 工具目录不存在，跳过"
fi
echo ""

# ==========================================
# 步骤6：设置权限
# ==========================================
echo -e "${YELLOW}[6/7]${NC} 设置权限..."
ssh $TARGET_USER@$TARGET_IP "chmod +x $TARGET_DIR/$PROGRAM_NAME"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} 权限设置完成"
    
    # 如果硬件插件库存在，也设置权限
    if [ -d "$BUILD_LIB_DIR" ] && [ $LIB_COUNT -gt 0 ]; then
        ssh $TARGET_USER@$TARGET_IP "chmod 755 $TARGET_DIR/lib${LIB_NAME}.so* 2>/dev/null"
    fi
else
    echo -e "${RED}✗${NC} 权限设置失败"
    exit 1
fi
echo ""

# ==========================================
# 步骤7：验证部署
# ==========================================
echo -e "${YELLOW}[7/7]${NC} 验证部署..."

# 验证主程序
REMOTE_PROG_INFO=$(ssh $TARGET_USER@$TARGET_IP "ls -lh $TARGET_DIR/$PROGRAM_NAME 2>/dev/null | awk '{print \$5}' && file $TARGET_DIR/$PROGRAM_NAME 2>/dev/null | grep -o 'ARM.*'")
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} 主程序验证成功"
    echo "$REMOTE_PROG_INFO" | sed 's/^/  /'
else
    echo -e "${RED}✗${NC} 主程序验证失败"
    exit 1
fi

# 验证硬件插件库（如果存在）
if [ -d "$BUILD_LIB_DIR" ] && [ $LIB_COUNT -gt 0 ]; then
    REMOTE_LIB_CHECK=$(ssh $TARGET_USER@$TARGET_IP "ls -1 $TARGET_DIR/lib${LIB_NAME}.so.*.*.* 2>/dev/null")
    if [ -n "$REMOTE_LIB_CHECK" ]; then
        echo -e "${GREEN}✓${NC} 硬件插件库验证成功"
        ssh $TARGET_USER@$TARGET_IP "ls -lh $TARGET_DIR/lib${LIB_NAME}.so* 2>/dev/null" | sed 's/^/  /'
    else
        echo -e "${RED}✗${NC} 硬件插件库验证失败"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠${NC} 硬件插件库验证跳过（已禁用）"
fi

# 验证工具脚本
if [ -d "$TOOLS_DIR" ]; then
    REMOTE_TOOLS_COUNT=$(ssh $TARGET_USER@$TARGET_IP "ls -1 ~/tools/*.sh 2>/dev/null | wc -l")
    if [ "$REMOTE_TOOLS_COUNT" -gt 0 ]; then
        echo -e "${GREEN}✓${NC} 工具脚本验证成功 (${REMOTE_TOOLS_COUNT}个)"
    else
        echo -e "${YELLOW}⚠${NC} 工具脚本验证失败"
    fi
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  部署完成！${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# ==========================================
# 运行程序
# ==========================================
if [ "$RUN_AFTER_DOWNLOAD" = true ]; then
    echo -e "${CYAN}>>> 正在启动程序...${NC}"
    echo ""
    
    if [ "$RUN_IN_BACKGROUND" = true ]; then
        # 后台运行
        echo -e "${BLUE}模式: 后台运行${NC}"
        ssh $TARGET_USER@$TARGET_IP "cd ~ && export LD_LIBRARY_PATH=.:\$LD_LIBRARY_PATH && nohup ./$PROGRAM_NAME > /tmp/${PROGRAM_NAME}.log 2>&1 &"
        sleep 1
        
        # 检查是否运行
        PID=$(ssh $TARGET_USER@$TARGET_IP "pgrep -f $PROGRAM_NAME" 2>/dev/null)
        if [ -n "$PID" ]; then
            echo -e "${GREEN}✓ 程序已在后台启动 (PID: $PID)${NC}"
            echo ""
            echo -e "${YELLOW}远程工具（在开发机上）:${NC}"
            echo -e "  ${CYAN}./tools/view_log.sh -f${NC}         # 实时日志"
            echo -e "  ${CYAN}./tools/view_log.sh -s${NC}         # 查看状态"
            echo -e "  ${CYAN}./tools/control_app.sh restart${NC}  # 重启程序"
            echo ""
            echo -e "${YELLOW}本地工具（在目标板上）:${NC}"
            echo -e "  ${CYAN}ssh $TARGET_USER@$TARGET_IP 'cd ~/tools && ./local_view_log.sh -f'${NC}"
            echo ""
            echo -e "${YELLOW}或直接操作:${NC}"
            echo -e "  ${CYAN}ssh $TARGET_USER@$TARGET_IP 'tail -f /tmp/${PROGRAM_NAME}.log'${NC}  # 查看日志"
            echo -e "  ${CYAN}ssh $TARGET_USER@$TARGET_IP 'killall $PROGRAM_NAME'${NC}            # 停止程序"
        else
            echo -e "${RED}✗ 程序启动失败${NC}"
            echo ""
            echo "查看日志:"
            echo -e "  ${CYAN}ssh $TARGET_USER@$TARGET_IP 'cat /tmp/${PROGRAM_NAME}.log'${NC}"
            exit 1
        fi
    else
        # 前台运行
        echo -e "${BLUE}模式: 前台运行 (Ctrl+C 停止)${NC}"
        echo -e "${BLUE}========================================${NC}"
        echo ""
        ssh $TARGET_USER@$TARGET_IP "cd ~ && export LD_LIBRARY_PATH=.:\$LD_LIBRARY_PATH && ./$PROGRAM_NAME"
        EXIT_CODE=$?
        echo ""
        echo -e "${BLUE}========================================${NC}"
        if [ $EXIT_CODE -eq 0 ]; then
            echo -e "${GREEN}程序正常退出${NC}"
        else
            echo -e "${YELLOW}程序退出码: $EXIT_CODE${NC}"
        fi
    fi
else
    echo -e "${YELLOW}提示: 使用 --no-run 参数，程序不会自动运行${NC}"
    echo ""
    echo -e "${BLUE}手动运行命令:${NC}"
    echo -e "  ${GREEN}ssh $TARGET_USER@$TARGET_IP 'cd ~ && export LD_LIBRARY_PATH=.:\$LD_LIBRARY_PATH && ./$PROGRAM_NAME'${NC}"
    echo ""
    echo -e "${BLUE}或后台运行:${NC}"
    echo -e "  ${GREEN}ssh $TARGET_USER@$TARGET_IP 'cd ~ && export LD_LIBRARY_PATH=.:\$LD_LIBRARY_PATH && nohup ./$PROGRAM_NAME > /tmp/${PROGRAM_NAME}.log 2>&1 &'${NC}"
fi

echo ""
