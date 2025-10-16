/***************************************************************
 * Copyright: Alex
 * FileName: AlarmService.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 闹钟服务实现
 ***************************************************************/

#include "services/alarm/AlarmService.h"
#include "core/LogManager.h"
#include <QDebug>

/***************************************************************
 * 构造函数
 ***************************************************************/
AlarmService::AlarmService(int32_t svr_id, int32_t svr_type, QObject *parent)
    : ISysSvrInterface(svr_id, svr_type, parent)
    , m_pBeep(nullptr)
    , m_alarmHour(6)
    , m_alarmMinute(0)
    , m_alarmEnabled(true)
    , m_sleepReminderHour(22)
    , m_sleepReminderMinute(0)
    , m_sleepReminderEnabled(true)
    , m_alarmPlayCount(0)
    , m_alarmMaxCount(30)  // 最多播放30次（约5分钟）
{
    // 创建定时器
    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(60000);  // 每分钟检查一次
    
    m_alarmPlayTimer = new QTimer(this);
    m_alarmPlayTimer->setInterval(10000);  // 每10秒播放一次
    
    // 连接信号槽
    connect(m_checkTimer, &QTimer::timeout, this, &AlarmService::onCheckTimer);
    connect(m_alarmPlayTimer, &QTimer::timeout, this, &AlarmService::onAlarmPlayTimer);
    
    // 初始化节假日数据
    initialize2025HolidayData();
    
    qInfo() << "[AlarmService] 闹钟服务创建";
}

/***************************************************************
 * 析构函数
 ***************************************************************/
AlarmService::~AlarmService()
{
    qInfo() << "[AlarmService] 闹钟服务销毁";
    SvrStop();
}

/***************************************************************
 * 服务初始化
 ***************************************************************/
bool AlarmService::SvrInit()
{
    qInfo() << "[AlarmService] 初始化闹钟服务...";
    qInfo() << "  起床闹钟: " << QString("%1:%2").arg(m_alarmHour, 2, 10, QChar('0')).arg(m_alarmMinute, 2, 10, QChar('0')) 
            << " (" << (m_alarmEnabled ? "启用" : "禁用") << ")";
    qInfo() << "  睡眠提示: " << QString("%1:%2").arg(m_sleepReminderHour, 2, 10, QChar('0')).arg(m_sleepReminderMinute, 2, 10, QChar('0'))
            << " (" << (m_sleepReminderEnabled ? "启用" : "禁用") << ")";
    qInfo() << "  工作日判断: 周一至周五（排除法定节假日）+ 补班日";
    qInfo() << "[AlarmService] ✓ 闹钟服务初始化成功";
    
    return true;
}

/***************************************************************
 * 服务启动
 ***************************************************************/
bool AlarmService::SvrStart()
{
    LOG_INFO("Alarm", "启动闹钟服务...");
    
    // 检查蜂鸣器设备状态
    if (m_pBeep) {
        if (m_pBeep->isAvailable()) {
            LOG_INFO("Alarm", "✓ 蜂鸣器设备可用");
        } else {
            LOG_WARNING("Alarm", "⚠ 蜂鸣器设备不可用（/sys/class/leds/beep 不存在），闹钟将静默工作");
            LOG_WARNING("Alarm", "  可运行 tools/setup_test_beep.sh 创建测试设备");
        }
    } else {
        LOG_WARNING("Alarm", "⚠ 蜂鸣器驱动未初始化");
    }
    
    // 启动定时检查（每分钟检查一次）
    m_checkTimer->start();
    LOG_INFO("Alarm", "✓ 闹钟定时器启动（检查间隔: 1分钟）");
    
    LOG_INFO("Alarm", "========================================");
    LOG_INFO("Alarm", "✓ 闹钟服务启动成功");
    LOG_INFO("Alarm", QString("🌅 起床闹钟: 每个工作日 %1:%2 (%3)")
        .arg(m_alarmHour, 2, 10, QChar('0'))
        .arg(m_alarmMinute, 2, 10, QChar('0'))
        .arg(m_alarmEnabled ? "启用" : "禁用"));
    LOG_INFO("Alarm", QString("🌙 睡眠提示: 每天晚上 %1:%2 (%3)")
        .arg(m_sleepReminderHour, 2, 10, QChar('0'))
        .arg(m_sleepReminderMinute, 2, 10, QChar('0'))
        .arg(m_sleepReminderEnabled ? "启用" : "禁用"));
    LOG_INFO("Alarm", "工作日定义: 周一至周五 + 补班日 - 法定节假日");
    LOG_INFO("Alarm", "========================================");
    
    return true;
}

/***************************************************************
 * 服务停止
 ***************************************************************/
bool AlarmService::SvrStop()
{
    qInfo() << "[AlarmService] 停止闹钟服务...";
    
    m_checkTimer->stop();
    stopAlarm();
    
    qInfo() << "[AlarmService] ✓ 闹钟服务已停止";
    
    return true;
}

/***************************************************************
 * 设置Beep驱动
 ***************************************************************/
void AlarmService::setBeepDriver(DriverBeep *beep)
{
    m_pBeep = beep;
    qInfo() << "[AlarmService] Beep驱动已设置";
}

/***************************************************************
 * 设置闹钟时间
 ***************************************************************/
void AlarmService::setAlarmTime(int hour, int minute)
{
    if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
        m_alarmHour = hour;
        m_alarmMinute = minute;
        qInfo() << "[AlarmService] 起床闹钟时间设置为:" 
                << QString("%1:%2").arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0'));
    } else {
        qWarning() << "[AlarmService] 无效的闹钟时间:" << hour << ":" << minute;
    }
}

/***************************************************************
 * 设置睡眠提示时间
 ***************************************************************/
void AlarmService::setSleepReminderTime(int hour, int minute)
{
    if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
        m_sleepReminderHour = hour;
        m_sleepReminderMinute = minute;
        qInfo() << "[AlarmService] 睡眠提示时间设置为:" 
                << QString("%1:%2").arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0'));
    } else {
        qWarning() << "[AlarmService] 无效的睡眠提示时间:" << hour << ":" << minute;
    }
}

/***************************************************************
 * 启用/禁用闹钟
 ***************************************************************/
void AlarmService::setAlarmEnabled(bool enabled)
{
    m_alarmEnabled = enabled;
    qInfo() << "[AlarmService] 起床闹钟:" << (enabled ? "启用" : "禁用");
}

/***************************************************************
 * 启用/禁用睡眠提示
 ***************************************************************/
void AlarmService::setSleepReminderEnabled(bool enabled)
{
    m_sleepReminderEnabled = enabled;
    qInfo() << "[AlarmService] 睡眠提示:" << (enabled ? "启用" : "禁用");
}

/***************************************************************
 * 手动触发闹钟
 ***************************************************************/
void AlarmService::triggerAlarmManually()
{
    LOG_INFO("Alarm", "手动触发起床闹钟（测试模式）");
    playAlarmRingtone();
}

/***************************************************************
 * 手动触发睡眠提示
 ***************************************************************/
void AlarmService::triggerSleepReminderManually()
{
    LOG_INFO("Alarm", "手动触发睡眠提示（测试模式）");
    playSleepReminder();
}

/***************************************************************
 * 定时检查槽函数
 ***************************************************************/
void AlarmService::onCheckTimer()
{
    QDateTime now = QDateTime::currentDateTime();
    
    // 1. 检查是否到达起床闹钟时间
    if (isAlarmTime(now)) {
        // 检查是否为工作日
        if (!isWorkday(now.date())) {
            LOG_DEBUG("Alarm", QString("今天是休息日，不触发起床闹钟: %1 %2")
                .arg(now.toString("yyyy-MM-dd"))
                .arg(now.toString("dddd")));
        } else {
            // 防止短时间内重复触发
            if (m_lastAlarmTime.isValid()) {
                qint64 minutesSinceLastAlarm = m_lastAlarmTime.secsTo(now) / 60;
                if (minutesSinceLastAlarm < 30) {  // 30分钟内不重复
                    LOG_DEBUG("Alarm", QString("跳过起床闹钟（距上次仅%1分钟）").arg(minutesSinceLastAlarm));
                    return;
                }
            }
            
            // 触发起床闹钟
            LOG_INFO("Alarm", "========================================");
            LOG_INFO("Alarm", QString("🌅 工作日起床闹钟触发: %1 %2")
                .arg(now.toString("yyyy-MM-dd hh:mm:ss"))
                .arg(now.toString("dddd")));
            LOG_INFO("Alarm", "========================================");
            
            m_lastAlarmTime = now;
            emit alarmTriggered(now);
            
            // 开始播放闹钟
            playAlarmRingtone();
        }
    }
    
    // 2. 检查是否到达睡眠提示时间
    if (isSleepReminderTime(now)) {
        // 防止短时间内重复触发
        if (m_lastSleepReminderTime.isValid()) {
            qint64 minutesSinceLastReminder = m_lastSleepReminderTime.secsTo(now) / 60;
            if (minutesSinceLastReminder < 30) {  // 30分钟内不重复
                LOG_DEBUG("Alarm", QString("跳过睡眠提示（距上次仅%1分钟）").arg(minutesSinceLastReminder));
                return;
            }
        }
        
        // 触发睡眠提示
        LOG_INFO("Alarm", "========================================");
        LOG_INFO("Alarm", QString("🌙 睡眠提示触发: %1")
            .arg(now.toString("yyyy-MM-dd hh:mm:ss dddd")));
        LOG_INFO("Alarm", "========================================");
        
        m_lastSleepReminderTime = now;
        emit sleepReminderTriggered(now);
        
        // 播放睡眠提示音
        playSleepReminder();
    }
}

/***************************************************************
 * 闹钟播放槽函数
 ***************************************************************/
void AlarmService::onAlarmPlayTimer()
{
    m_alarmPlayCount++;
    
    // 检查是否达到最大播放次数
    if (m_alarmPlayCount >= m_alarmMaxCount) {
        LOG_INFO("Alarm", QString("闹钟播放结束（已播放%1次）").arg(m_alarmPlayCount));
        stopAlarm();
        emit alarmFinished();
        return;
    }
    
    // 继续播放闹钟
    if (m_pBeep && m_pBeep->isAvailable()) {
        // 有节奏的播放：3短1长
        if (m_alarmPlayCount % 4 == 0) {
            // 每4次播放一个长音
            m_pBeep->beep(1, 800);
        } else {
            // 播放短音
            m_pBeep->beep(2, 150);
        }
    }
    
    LOG_DEBUG("Alarm", QString("闹钟播放中... (%1/%2)").arg(m_alarmPlayCount).arg(m_alarmMaxCount));
}

/***************************************************************
 * 播放起床闹钟
 ***************************************************************/
void AlarmService::playAlarmRingtone()
{
    if (!m_pBeep) {
        LOG_WARNING("Alarm", "蜂鸣器未初始化，无法播放起床闹钟");
        return;
    }
    
    if (!m_pBeep->isAvailable()) {
        LOG_WARNING("Alarm", "蜂鸣器设备不可用：/sys/class/leds/beep 不存在");
        return;
    }
    
    // 重置播放计数
    m_alarmPlayCount = 0;
    
    // 首次播放：连续短促的蜂鸣（起床铃声）
    m_pBeep->beep(5, 100);
    
    // 启动定时播放
    m_alarmPlayTimer->start();
    
    LOG_INFO("Alarm", "🔔 起床闹钟开始播放（有节奏铃声）");
}

/***************************************************************
 * 播放睡眠提示音
 ***************************************************************/
void AlarmService::playSleepReminder()
{
    if (!m_pBeep) {
        LOG_WARNING("Alarm", "蜂鸣器未初始化，无法播放睡眠提示");
        return;
    }
    
    if (!m_pBeep->isAvailable()) {
        LOG_WARNING("Alarm", "蜂鸣器设备不可用：/sys/class/leds/beep 不存在");
        return;
    }
    
    // 温和的提示音：3次短促蜂鸣
    m_pBeep->beep(3, 200);
    
    LOG_INFO("Alarm", "🌙 睡眠提示音播放（温和提示）");
}

/***************************************************************
 * 停止闹钟播放
 ***************************************************************/
void AlarmService::stopAlarm()
{
    m_alarmPlayTimer->stop();
    m_alarmPlayCount = 0;
    
    if (m_pBeep) {
        m_pBeep->stopAll();
    }
}

/***************************************************************
 * 检查是否到达闹钟时间
 ***************************************************************/
bool AlarmService::isAlarmTime(const QDateTime &time) const
{
    if (!m_alarmEnabled) {
        return false;
    }
    
    int currentHour = time.time().hour();
    int currentMinute = time.time().minute();
    
    return (currentHour == m_alarmHour && currentMinute == m_alarmMinute);
}

/***************************************************************
 * 检查是否到达睡眠提示时间
 ***************************************************************/
bool AlarmService::isSleepReminderTime(const QDateTime &time) const
{
    if (!m_sleepReminderEnabled) {
        return false;
    }
    
    int currentHour = time.time().hour();
    int currentMinute = time.time().minute();
    
    return (currentHour == m_sleepReminderHour && currentMinute == m_sleepReminderMinute);
}

/***************************************************************
 * 检查是否为工作日
 ***************************************************************/
bool AlarmService::isWorkday(const QDate &date) const
{
    // 1. 检查是否为补班日（调休工作日）
    if (m_workdays.contains(date)) {
        return true;
    }
    
    // 2. 检查是否为法定节假日
    if (m_holidays.contains(date)) {
        return false;
    }
    
    // 3. 检查是否为周末
    int dayOfWeek = date.dayOfWeek();  // 1=周一, 7=周日
    if (dayOfWeek == 6 || dayOfWeek == 7) {  // 周六或周日
        return false;
    }
    
    // 4. 周一至周五，且不是节假日，视为工作日
    return true;
}

/***************************************************************
 * 添加法定节假日
 ***************************************************************/
void AlarmService::addHoliday(const QDate &date)
{
    m_holidays.insert(date);
}

/***************************************************************
 * 添加补班日
 ***************************************************************/
void AlarmService::addWorkday(const QDate &date)
{
    m_workdays.insert(date);
}

/***************************************************************
 * 初始化2025年的节假日和补班日数据
 ***************************************************************/
void AlarmService::initialize2025HolidayData()
{
    // 清空现有数据
    m_holidays.clear();
    m_workdays.clear();
    
    // ========== 2025年元旦（1月1日，放假1天）==========
    m_holidays.insert(QDate(2025, 1, 1));  // 周三
    
    // ========== 2025年春节（1月28日-2月4日，放假8天）==========
    m_holidays.insert(QDate(2025, 1, 28));  // 除夕（周二）
    m_holidays.insert(QDate(2025, 1, 29));  // 初一（周三）
    m_holidays.insert(QDate(2025, 1, 30));  // 初二（周四）
    m_holidays.insert(QDate(2025, 1, 31));  // 初三（周五）
    m_holidays.insert(QDate(2025, 2, 1));   // 初四（周六）
    m_holidays.insert(QDate(2025, 2, 2));   // 初五（周日）
    m_holidays.insert(QDate(2025, 2, 3));   // 初六（周一）
    m_holidays.insert(QDate(2025, 2, 4));   // 初七（周二）
    // 补班日
    m_workdays.insert(QDate(2025, 1, 26));  // 周日补班
    m_workdays.insert(QDate(2025, 2, 8));   // 周六补班
    
    // ========== 2025年清明节（4月4-6日，放假3天）==========
    m_holidays.insert(QDate(2025, 4, 4));   // 周五
    m_holidays.insert(QDate(2025, 4, 5));   // 周六
    m_holidays.insert(QDate(2025, 4, 6));   // 周日
    
    // ========== 2025年劳动节（5月1-5日，放假5天）==========
    m_holidays.insert(QDate(2025, 5, 1));   // 周四
    m_holidays.insert(QDate(2025, 5, 2));   // 周五
    m_holidays.insert(QDate(2025, 5, 3));   // 周六
    m_holidays.insert(QDate(2025, 5, 4));   // 周日
    m_holidays.insert(QDate(2025, 5, 5));   // 周一
    // 补班日
    m_workdays.insert(QDate(2025, 4, 27));  // 周日补班
    
    // ========== 2025年端午节（5月31日-6月2日，放假3天）==========
    m_holidays.insert(QDate(2025, 5, 31));  // 周六
    m_holidays.insert(QDate(2025, 6, 1));   // 周日
    m_holidays.insert(QDate(2025, 6, 2));   // 周一
    
    // ========== 2025年中秋节（10月6-8日，放假3天）==========
    m_holidays.insert(QDate(2025, 10, 6));  // 周一
    m_holidays.insert(QDate(2025, 10, 7));  // 周二
    m_holidays.insert(QDate(2025, 10, 8));  // 周三
    // 补班日
    m_workdays.insert(QDate(2025, 9, 28));  // 周日补班
    
    // ========== 2025年国庆节（10月1-7日，放假7天，与中秋连休）==========
    m_holidays.insert(QDate(2025, 10, 1));  // 周三
    m_holidays.insert(QDate(2025, 10, 2));  // 周四
    m_holidays.insert(QDate(2025, 10, 3));  // 周五
    m_holidays.insert(QDate(2025, 10, 4));  // 周六
    m_holidays.insert(QDate(2025, 10, 5));  // 周日
    // 10月6-8日已在中秋节中添加
    // 补班日
    m_workdays.insert(QDate(2025, 9, 29));  // 周一补班
    m_workdays.insert(QDate(2025, 10, 11)); // 周六补班
    
    qInfo() << "[AlarmService] 2025年节假日数据已加载:";
    qInfo() << "  法定节假日:" << m_holidays.size() << "天";
    qInfo() << "  调休补班日:" << m_workdays.size() << "天";
}

