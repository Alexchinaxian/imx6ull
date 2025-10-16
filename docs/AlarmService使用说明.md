# AlarmService 闹钟服务使用说明

## 功能概述

AlarmService 是一个工作日早晨起床闹钟服务，具有以下特性：

1. **自动识别工作日**：支持中国法定节假日和调休补班日
2. **定时闹铃**：默认每天早上6:00触发
3. **有节奏的铃声**：使用蜂鸣器播放有节奏的起床铃声（3短1长循环）
4. **智能防重复**：30分钟内不会重复触发

## 工作日判断逻辑

### 基本规则
- ✅ **工作日**：周一至周五（不包括法定节假日）
- ✅ **补班日**：因调休安排的周末补班日
- ❌ **休息日**：周六、周日（不包括补班日）
- ❌ **法定节假日**：元旦、春节、清明、劳动节、端午、中秋、国庆

### 2025年节假日数据

服务已预置2025年完整的节假日和补班日数据：

#### 元旦（1月1日）
- 放假：1月1日（周三）

#### 春节（1月28日-2月4日）
- 放假：1月28日-2月4日（共8天）
- 补班：1月26日（周日）、2月8日（周六）

#### 清明节（4月4-6日）
- 放假：4月4-6日（共3天）

#### 劳动节（5月1-5日）
- 放假：5月1-5日（共5天）
- 补班：4月27日（周日）

#### 端午节（5月31日-6月2日）
- 放假：5月31日-6月2日（共3天）

#### 中秋节（10月6-8日）
- 放假：10月6-8日（共3天）
- 补班：9月28日（周日）

#### 国庆节（10月1-7日，与中秋连休）
- 放假：10月1-7日（共7天）
- 补班：9月29日（周一）、10月11日（周六）

## 闹钟铃声模式

### 播放模式
闹钟铃声采用有节奏的播放模式：

1. **初始播放**：连续5次短促蜂鸣（间隔100ms）
2. **循环播放**：每10秒播放一次
   - 前3次：2短音（间隔150ms）
   - 第4次：1长音（800ms）
3. **自动停止**：最多播放30次（约5分钟）

### 铃声示例
```
初始: 哔-哔-哔-哔-哔
循环: 哔哔--哔哔--哔哔--哔————
```

## 服务配置

### 默认配置
```cpp
// 默认闹钟时间：早上6:00
m_alarmHour = 6;
m_alarmMinute = 0;

// 默认状态：启用
m_alarmEnabled = true;

// 最大播放次数：30次（约5分钟）
m_alarmMaxCount = 30;
```

### 修改闹钟时间
在 `ServiceManager.cpp` 的 `SvrCreateInit()` 函数中修改：

```cpp
// 配置闹钟服务
pAlarmSvr->setAlarmTime(6, 30);  // 改为6:30
pAlarmSvr->setAlarmEnabled(true);
```

### 禁用闹钟
```cpp
pAlarmSvr->setAlarmEnabled(false);  // 禁用闹钟
```

## 手动测试

### 测试闹钟铃声
可以通过手动触发闹钟来测试铃声效果：

```cpp
AlarmService *alarmSvr = ServiceManager::GetInstance()->GetAlarmSvrObj();
if (alarmSvr) {
    alarmSvr->triggerAlarmManually();
}
```

### 测试工作日判断
```cpp
AlarmService *alarmSvr = ServiceManager::GetInstance()->GetAlarmSvrObj();
if (alarmSvr) {
    QDate testDate = QDate(2025, 10, 1);  // 2025年10月1日国庆节
    bool isWork = alarmSvr->isWorkday(testDate);
    qInfo() << testDate << "是否为工作日:" << (isWork ? "是" : "否");
}
```

## 信号与槽

### 可用信号
```cpp
// 闹钟触发信号
void alarmTriggered(const QDateTime &time);

// 闹钟播放结束信号
void alarmFinished();
```

### 使用示例
```cpp
connect(alarmSvr, &AlarmService::alarmTriggered,
        [](const QDateTime &time) {
    qInfo() << "闹钟触发了！时间:" << time;
});

connect(alarmSvr, &AlarmService::alarmFinished,
        []() {
    qInfo() << "闹钟播放结束";
});
```

## 日志输出

### 服务启动日志
```
[AlarmService] 初始化闹钟服务...
  闹钟时间: 06:00
  闹钟状态: 启用
  工作日判断: 周一至周五（排除法定节假日）+ 补班日
[AlarmService] ✓ 闹钟服务初始化成功
```

### 闹钟触发日志
```
========================================
⏰ 工作日闹钟触发: 2025-10-15 06:00:00 星期三
========================================
🔔 起床闹钟开始播放（有节奏铃声）
⏰ 闹钟播放中... (1/30)
⏰ 闹钟播放中... (2/30)
...
⏰ 闹钟播放结束
```

### 休息日日志
```
今天是休息日，不触发闹钟: 2025-10-11 星期六
```

## 依赖关系

### 必需组件
1. **DriverBeep**：蜂鸣器驱动（通过ModbusSlaveService获取）
2. **TimeService**：时间服务（确保系统时间准确）

### 自动配置
服务管理器会自动建立依赖关系：
```cpp
// 在 SetupServiceDependencies() 中自动配置
DriverBeep *beep = pModbusSvr->GetBeepDriver();
pAlarmSvr->setBeepDriver(beep);
```

## 扩展功能

### 添加自定义节假日
```cpp
// 添加自定义节假日（例如公司年假）
alarmSvr->addHoliday(QDate(2025, 12, 25));  // 圣诞节放假

// 添加自定义补班日
alarmSvr->addWorkday(QDate(2025, 12, 21));  // 周六补班
```

### 更新未来年份的节假日数据
修改 `AlarmService.cpp` 中的 `initialize2025HolidayData()` 函数，添加新一年的节假日数据。

## 注意事项

1. **时区设置**：确保系统时区设置为中国时区（Asia/Hong_Kong UTC+8）
2. **时间准确性**：依赖NTP时间同步，确保TimeService正常工作
3. **蜂鸣器可用性**：确保硬件蜂鸣器正常工作
4. **节假日更新**：每年需要更新节假日数据

## 故障排除

### 闹钟不响
1. 检查闹钟是否启用：`setAlarmEnabled(true)`
2. 检查蜂鸣器是否可用：`beep->isAvailable()`
3. 检查系统时间是否准确
4. 检查是否为工作日

### 休息日触发
1. 确认节假日数据是否正确
2. 检查日志中的工作日判断结果

### 铃声异常
1. 测试蜂鸣器硬件：`beep->beep(1, 100)`
2. 检查蜂鸣器sysfs路径是否存在

## 技术架构

### 类继承关系
```
QObject
  └── ISysSvrInterface
        └── AlarmService
```

### 主要成员变量
```cpp
QTimer *m_checkTimer;          // 每分钟检查一次
QTimer *m_alarmPlayTimer;      // 控制铃声播放节奏
DriverBeep *m_pBeep;           // 蜂鸣器驱动
QSet<QDate> m_holidays;        // 法定节假日集合
QSet<QDate> m_workdays;        // 补班日集合
```

### 定时器机制
- **检查定时器**：每60秒执行一次，检查是否到达闹钟时间
- **播放定时器**：每10秒执行一次，控制铃声播放节奏

## 版本历史

### V1.0.0 (2025-10-15)
- ✅ 初始版本
- ✅ 支持工作日闹钟
- ✅ 内置2025年节假日数据
- ✅ 有节奏的铃声播放
- ✅ 智能防重复触发

---

**作者**: Alex  
**日期**: 2025-10-15  
**文件**: `src/services/alarm/AlarmService.cpp`

