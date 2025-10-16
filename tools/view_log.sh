#!/bin/bash

#######################################################################
# 日志查看工具（智能版）
# 描述: 自动检测运行环境，本地或远程访问日志
# 用法: ./view_log.sh [选项]
#######################################################################

TARGET_IP="192.168.1.24"
TARGET_USER="root"
LOG_FILE="/tmp/QtImx6ullBackend.log"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# 检测是否在目标板上运行
IS_LOCAL=false
CURRENT_IP=$(hostname -I 2>/dev/null | awk '{print $1}')
if [ "$CURRENT_IP" = "$TARGET_IP" ] || [ -f "/proc/device-tree/model" ]; then
    # 在目标板上运行
    IS_LOCAL=true
fi

# 显示帮助
show_help() {
    local mode_str="智能模式"
    if [ "$IS_LOCAL" = true ]; then
        mode_str="本地模式"
    else
        mode_str="远程模式"
    fi
    
    cat << EOF
${BLUE}========================================${NC}
${BLUE}  日志查看工具 (${mode_str})${NC}
${BLUE}========================================${NC}

用法: $0 [选项]

选项:
  -t, --tail [N]      查看最后N行（默认50）
  -f, --follow        实时跟踪日志
  -a, --all           查看全部日志
  -g, --grep PATTERN  搜索包含PATTERN的行
  -c, --clear         清空日志文件
  -s, --status        查看程序运行状态
  -h, --help          显示此帮助信息

示例:
  $0                  # 查看最后50行
  $0 -t 100           # 查看最后100行
  $0 -f               # 实时跟踪
  $0 -g "Temperature" # 搜索温度相关日志
  $0 -g "LED"         # 搜索LED相关日志
  $0 -s               # 查看程序状态

EOF
}

# 检查程序运行状态
check_status() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  程序运行状态${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    
    if [ "$IS_LOCAL" = true ]; then
        # 本地执行
        PROC_INFO=$(ps aux | grep QtImx6ullBackend | grep -v grep)
    else
        # 远程执行
        PROC_INFO=$(ssh ${TARGET_USER}@${TARGET_IP} "ps aux | grep QtImx6ullBackend | grep -v grep" 2>/dev/null)
    fi
    
    if [ -n "$PROC_INFO" ]; then
        echo -e "${GREEN}✓ 程序正在运行${NC}"
        echo ""
        echo "$PROC_INFO" | while read line; do
            echo "  $line"
        done
        
        echo ""
        PID=$(echo "$PROC_INFO" | grep "QtImx6ullBackend$" | awk '{print $2}' | head -1)
        if [ -n "$PID" ]; then
            if [ "$IS_LOCAL" = true ]; then
                RUNTIME=$(ps -p $PID -o etime= 2>/dev/null)
                MEM=$(ps -p $PID -o rss= 2>/dev/null)
            else
                RUNTIME=$(ssh ${TARGET_USER}@${TARGET_IP} "ps -p $PID -o etime= 2>/dev/null")
                MEM=$(ssh ${TARGET_USER}@${TARGET_IP} "ps -p $PID -o rss= 2>/dev/null")
            fi
            
            echo -e "${CYAN}PID:${NC} $PID"
            echo -e "${CYAN}运行时间:${NC} $RUNTIME"
            echo -e "${CYAN}内存占用:${NC} $((MEM/1024)) MB"
        fi
        
        # 检查日志文件
        if [ "$IS_LOCAL" = true ]; then
            LOG_SIZE=$(ls -lh ${LOG_FILE} 2>/dev/null | awk '{print $5}')
        else
            LOG_SIZE=$(ssh ${TARGET_USER}@${TARGET_IP} "ls -lh ${LOG_FILE} 2>/dev/null | awk '{print \$5}'")
        fi
        if [ -n "$LOG_SIZE" ]; then
            echo -e "${CYAN}日志大小:${NC} $LOG_SIZE"
        fi
    else
        echo -e "${RED}✗ 程序未运行${NC}"
        echo ""
        echo -e "${YELLOW}启动程序:${NC}"
        if [ "$IS_LOCAL" = true ]; then
            echo -e "  cd ~ && ./QtImx6ullBackend &"
        else
            echo -e "  cd /home/alex/imx6ull && ./download.sh --background"
        fi
    fi
    
    echo ""
}

# 查看日志尾部
view_tail() {
    local lines=${1:-50}
    
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  日志 (最后${lines}行)${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    
    if [ "$IS_LOCAL" = true ]; then
        tail -n ${lines} ${LOG_FILE} 2>/dev/null || {
            echo -e "${RED}✗ 无法读取日志文件${NC}"
            exit 1
        }
    else
        ssh ${TARGET_USER}@${TARGET_IP} "tail -n ${lines} ${LOG_FILE}" 2>/dev/null || {
            echo -e "${RED}✗ 无法读取日志文件${NC}"
            exit 1
        }
    fi
}

# 实时跟踪日志
follow_log() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  实时日志 (Ctrl+C 退出)${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    
    if [ "$IS_LOCAL" = true ]; then
        # 本地直接tail -f
        tail -f ${LOG_FILE} 2>/dev/null
    else
        # 远程SSH tail -f
        ssh ${TARGET_USER}@${TARGET_IP} "tail -f ${LOG_FILE}" 2>/dev/null
    fi
}

# 查看全部日志
view_all() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  完整日志${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    
    if [ "$IS_LOCAL" = true ]; then
        cat ${LOG_FILE} 2>/dev/null || {
            echo -e "${RED}✗ 无法读取日志文件${NC}"
            exit 1
        }
    else
        ssh ${TARGET_USER}@${TARGET_IP} "cat ${LOG_FILE}" 2>/dev/null || {
            echo -e "${RED}✗ 无法读取日志文件${NC}"
            exit 1
        }
    fi
}

# 搜索日志
grep_log() {
    local pattern="$1"
    
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  搜索日志: \"${pattern}\"${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    
    if [ "$IS_LOCAL" = true ]; then
        grep --color=always -i "${pattern}" ${LOG_FILE} 2>/dev/null || {
            echo -e "${YELLOW}未找到匹配的日志${NC}"
        }
    else
        ssh ${TARGET_USER}@${TARGET_IP} "grep --color=always -i '${pattern}' ${LOG_FILE}" 2>/dev/null || {
            echo -e "${YELLOW}未找到匹配的日志${NC}"
        }
    fi
}

# 清空日志
clear_log() {
    echo -e "${YELLOW}确认清空日志文件? (yes/no):${NC} "
    read confirm
    
    if [ "$confirm" = "yes" ]; then
        if [ "$IS_LOCAL" = true ]; then
            echo '' > ${LOG_FILE} 2>/dev/null && {
                echo -e "${GREEN}✓ 日志已清空${NC}"
            } || {
                echo -e "${RED}✗ 清空失败${NC}"
            }
        else
            ssh ${TARGET_USER}@${TARGET_IP} "echo '' > ${LOG_FILE}" 2>/dev/null && {
                echo -e "${GREEN}✓ 日志已清空${NC}"
            } || {
                echo -e "${RED}✗ 清空失败${NC}"
            }
        fi
    else
        echo "操作已取消"
    fi
}

# 解析参数
case "$1" in
    -h|--help)
        show_help
        exit 0
        ;;
    -s|--status)
        check_status
        exit 0
        ;;
    -t|--tail)
        view_tail "$2"
        exit 0
        ;;
    -f|--follow)
        follow_log
        exit 0
        ;;
    -a|--all)
        view_all
        exit 0
        ;;
    -g|--grep)
        if [ -z "$2" ]; then
            echo -e "${RED}错误: 需要指定搜索模式${NC}"
            echo "用法: $0 -g PATTERN"
            exit 1
        fi
        grep_log "$2"
        exit 0
        ;;
    -c|--clear)
        clear_log
        exit 0
        ;;
    "")
        # 默认：查看最后50行
        view_tail 50
        exit 0
        ;;
    *)
        echo -e "${RED}未知选项: $1${NC}"
        echo ""
        show_help
        exit 1
        ;;
esac
