#!/bin/bash
########################################################################
# 脚本名称: test_system_beep.sh
# 功能描述: 测试SystemBeep各种提示音
# 作者: Alex
# 日期: 2025-10-15
########################################################################

BEEP_FILE="/sys/class/leds/beep/brightness"

echo "========================================="
echo "SystemBeep 蜂鸣器测试"
echo "========================================="
echo ""

# 检查设备是否存在
if [ ! -f "$BEEP_FILE" ]; then
    echo "❌ 蜂鸣器设备不存在: $BEEP_FILE"
    echo ""
    echo "请先运行: sudo tools/setup_test_beep.sh"
    echo ""
    exit 1
fi

echo "✅ 蜂鸣器设备存在"
echo ""

# 测试函数
test_beep() {
    local name=$1
    local count=$2
    local duration=$3
    
    echo "🔔 测试: $name"
    echo "   次数: $count, 时长: ${duration}ms"
    
    for ((i=1; i<=$count; i++)); do
        echo "255" > "$BEEP_FILE"
        sleep $(echo "scale=3; $duration / 1000" | bc)
        echo "0" > "$BEEP_FILE"
        if [ $i -lt $count ]; then
            sleep 0.1
        fi
    done
    
    echo "   ✓ 完成"
    echo ""
    sleep 1
}

echo "开始测试..."
echo ""

# 1. 初始化完成提示音（滴滴）
test_beep "初始化完成提示音" 2 100

# 2. 配置加载成功（滴）
test_beep "配置加载成功" 1 150

# 3. 成功提示音（滴滴滴）
test_beep "成功提示音" 3 80

# 4. 警告提示音（滴~滴~）
test_beep "警告提示音" 2 200

# 5. 错误警告音（长响）
test_beep "错误警告音" 1 500

# 6. 关机提示音（滴滴滴滴）
test_beep "关机提示音" 4 80

echo "========================================="
echo "✅ 所有测试完成"
echo "========================================="
echo ""
echo "💡 如果听到蜂鸣声，说明SystemBeep工作正常！"
echo ""

