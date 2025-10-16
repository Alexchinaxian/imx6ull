/***************************************************************
 * Copyright: Alex
 * FileName: AlarmService.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 闹钟服务 - 工作日早上6点起床闹钟
 *
 * 功能说明:
 *   1. 每天早上6:00检查是否为中国工作日
 *   2. 工作日包括：周一至周五（排除法定节假日）+ 补班日
 *   3. 播放有节奏的起床铃声（使用beep）
 *
 * 设计模式:
 *   - 服务模式（继承ISysSvrInterface）
 *   - 观察者模式（信号槽）
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_SERVICES_ALARM_H
#define IMX6ULL_SERVICES_ALARM_H

#include "core/ISysSvrInterface.h"
#include "drivers/beep/DriverBeep.h"
#include <QTimer>
#include <QDateTime>
#include <QDate>
#include <QSet>

/***************************************************************
 * 类名: AlarmService
 * 功能: 闹钟服务 - 工作日早上6点起床闹钟
 * 
 * 说明:
 *   在每个中国工作日的早上6:00播放有节奏的起床铃声
 *   自动识别法定节假日和调休补班日
 * 
 * 使用示例:
 *   AlarmService *alarmSvr = new AlarmService(
 *       SYS_SVR_ID_ALARM_SVR,
 *       SYS_SVR_TYPE_ALARM_SVR
 *   );
 *   alarmSvr->setBeepDriver(beep);
 *   alarmSvr->setAlarmTime(6, 0);  // 早上6:00
 *   alarmSvr->SvrInit();
 *   alarmSvr->SvrStart();
 ***************************************************************/
class AlarmService : public ISysSvrInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @param parent 父对象指针
     */
    explicit AlarmService(int32_t svr_id, int32_t svr_type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~AlarmService();
    
    // ========== 实现ISysSvrInterface接口 ==========
    
    virtual bool SvrInit() override;
    virtual bool SvrStart() override;
    virtual bool SvrStop() override;
    
    virtual QString GetSvrName() const override {
        return "AlarmService";
    }
    
    // ========== 闹钟服务特有接口 ==========
    
    /**
     * @brief 设置Beep驱动
     * @param beep Beep驱动指针
     */
    void setBeepDriver(DriverBeep *beep);
    
    /**
     * @brief 设置闹钟时间
     * @param hour 小时（0-23）
     * @param minute 分钟（0-59）
     */
    void setAlarmTime(int hour, int minute);
    
    /**
     * @brief 设置睡眠提示时间
     * @param hour 小时（0-23）
     * @param minute 分钟（0-59）
     */
    void setSleepReminderTime(int hour, int minute);
    
    /**
     * @brief 启用/禁用闹钟
     * @param enabled true=启用, false=禁用
     */
    void setAlarmEnabled(bool enabled);
    
    /**
     * @brief 启用/禁用睡眠提示
     * @param enabled true=启用, false=禁用
     */
    void setSleepReminderEnabled(bool enabled);
    
    /**
     * @brief 手动触发闹钟（测试用）
     */
    void triggerAlarmManually();
    
    /**
     * @brief 手动触发睡眠提示（测试用）
     */
    void triggerSleepReminderManually();
    
    /**
     * @brief 检查指定日期是否为工作日
     * @param date 要检查的日期
     * @return true=工作日, false=休息日
     */
    bool isWorkday(const QDate &date) const;
    
    /**
     * @brief 添加法定节假日
     * @param date 节假日日期
     */
    void addHoliday(const QDate &date);
    
    /**
     * @brief 添加补班日
     * @param date 补班日日期
     */
    void addWorkday(const QDate &date);

signals:
    /**
     * @brief 闹钟触发信号
     * @param time 触发时间
     */
    void alarmTriggered(const QDateTime &time);
    
    /**
     * @brief 闹钟完成信号
     */
    void alarmFinished();
    
    /**
     * @brief 睡眠提示触发信号
     * @param time 触发时间
     */
    void sleepReminderTriggered(const QDateTime &time);

private slots:
    /**
     * @brief 定时检查槽函数（每分钟检查一次）
     */
    void onCheckTimer();
    
    /**
     * @brief 闹钟播放槽函数
     */
    void onAlarmPlayTimer();

private:
    /**
     * @brief 初始化2025年的节假日和补班日数据
     */
    void initialize2025HolidayData();
    
    /**
     * @brief 播放起床闹钟（有节奏的铃声）
     */
    void playAlarmRingtone();
    
    /**
     * @brief 播放睡眠提示音（温和的提示）
     */
    void playSleepReminder();
    
    /**
     * @brief 停止闹钟播放
     */
    void stopAlarm();
    
    /**
     * @brief 检查是否到达闹钟时间
     * @param time 当前时间
     * @return true=到达, false=未到达
     */
    bool isAlarmTime(const QDateTime &time) const;
    
    /**
     * @brief 检查是否到达睡眠提示时间
     * @param time 当前时间
     * @return true=到达, false=未到达
     */
    bool isSleepReminderTime(const QDateTime &time) const;

private:
    QTimer *m_checkTimer;           // 检查定时器（每分钟）
    QTimer *m_alarmPlayTimer;       // 闹钟播放定时器
    
    DriverBeep *m_pBeep;            // Beep驱动指针
    
    int m_alarmHour;                // 闹钟小时
    int m_alarmMinute;              // 闹钟分钟
    bool m_alarmEnabled;            // 闹钟是否启用
    
    int m_sleepReminderHour;        // 睡眠提示小时
    int m_sleepReminderMinute;      // 睡眠提示分钟
    bool m_sleepReminderEnabled;    // 睡眠提示是否启用
    
    QDateTime m_lastAlarmTime;      // 上次闹钟时间（防止重复触发）
    QDateTime m_lastSleepReminderTime;  // 上次睡眠提示时间（防止重复触发）
    
    // 节假日和补班日数据
    QSet<QDate> m_holidays;         // 法定节假日集合
    QSet<QDate> m_workdays;         // 调休补班日集合
    
    // 闹钟播放控制
    int m_alarmPlayCount;           // 闹钟播放计数
    int m_alarmMaxCount;            // 最大播放次数
};

#endif // ALARMSERVICE_H

