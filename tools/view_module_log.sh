#!/bin/bash

# 查看特定模块日志的工具脚本

TARGET_IP="192.168.1.24"
TARGET_USER="root"
LOG_DIR="/tmp/imx6ull_logs"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# 显示帮助信息
show_help() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  模块日志查看工具${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    echo "用法: $0 [选项] [模块名]"
    echo ""
    echo "选项:"
    echo "  -l, --list      列出所有日志文件"
    echo "  -f, --follow    实时跟踪日志"
    echo "  -t, --tail N    显示最后N行（默认50）"
    echo "  -h, --help      显示帮助"
    echo ""
    echo "模块名称:"
    echo "  system          系统日志"
    echo "  weather         天气服务日志"
    echo "  temperature     温度服务日志"
    echo "  modbus          Modbus服务日志"
    echo "  time            时间服务日志"
    echo "  all             所有日志"
    echo ""
    echo "示例:"
    echo "  $0 weather              # 查看天气日志（最后50行）"
    echo "  $0 -f weather           # 实时跟踪天气日志"
    echo "  $0 -t 100 weather       # 查看最后100行天气日志"
    echo "  $0 -l                   # 列出所有日志文件"
    echo ""
}

# 列出所有日志文件
list_logs() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  日志文件列表${NC}"
    echo -e "${BLUE}========================================${NC}"
    ssh $TARGET_USER@$TARGET_IP "ls -lh $LOG_DIR/*.log 2>/dev/null" | while read line; do
        echo -e "${CYAN}$line${NC}"
    done
    echo ""
    
    echo -e "${YELLOW}统计信息:${NC}"
    ssh $TARGET_USER@$TARGET_IP "
    for f in $LOG_DIR/*.log; do
        [ -f \"\$f\" ] || continue
        module=\$(basename \"\$f\" .log)
        lines=\$(wc -l < \"\$f\")
        size=\$(du -h \"\$f\" | cut -f1)
        echo \"  \$module: \$lines 条日志 (\$size)\"
    done
    "
}

# 解析参数
FOLLOW=false
TAIL_LINES=50
MODULE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -l|--list)
            list_logs
            exit 0
            ;;
        -f|--follow)
            FOLLOW=true
            shift
            ;;
        -t|--tail)
            TAIL_LINES=$2
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            MODULE=$1
            shift
            ;;
    esac
done

# 如果没有指定模块，显示帮助
if [ -z "$MODULE" ]; then
    show_help
    exit 1
fi

# 构建日志文件路径
if [ "$MODULE" == "all" ]; then
    LOG_FILE="$LOG_DIR/*.log"
else
    LOG_FILE="$LOG_DIR/${MODULE}.log"
fi

# 检查日志文件是否存在
FILE_EXISTS=$(ssh $TARGET_USER@$TARGET_IP "ls $LOG_FILE 2>/dev/null")
if [ -z "$FILE_EXISTS" ]; then
    echo -e "${YELLOW}⚠ 日志文件不存在: $LOG_FILE${NC}"
    echo ""
    echo "可用的日志文件:"
    list_logs
    exit 1
fi

# 查看日志
echo -e "${BLUE}========================================${NC}"
if [ "$FOLLOW" = true ]; then
    echo -e "${BLUE}  实时跟踪日志: $MODULE${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo -e "${YELLOW}按 Ctrl+C 停止${NC}"
    echo ""
    ssh $TARGET_USER@$TARGET_IP "tail -f $LOG_FILE"
else
    echo -e "${BLUE}  查看日志: $MODULE (最后${TAIL_LINES}行)${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    ssh $TARGET_USER@$TARGET_IP "tail -n $TAIL_LINES $LOG_FILE"
fi

