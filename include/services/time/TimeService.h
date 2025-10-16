/***************************************************************
 * Copyright: Alex
 * FileName: TimeService.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 时间服务 - NTP对时和定时任务
 *
 * 功能说明:
 *   1. 网络对时（NTP同步）
 *   2. 定时任务（每半点蜂鸣提示）
 *   3. 时间查询和管理
 *
 * 设计模式:
 *   - 单例服务模式
 *   - 观察者模式（信号槽）
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_SERVICES_TIME_H
#define IMX6ULL_SERVICES_TIME_H

#include "core/ISysSvrInterface.h"
#include "drivers/beep/DriverBeep.h"
#include <QTimer>
#include <QDateTime>
#include <QUdpSocket>

/***************************************************************
 * 类名: TimeService
 * 功能: 时间服务
 * 
 * 说明:
 *   提供NTP网络对时和定时任务功能
 *   每半点（XX:00和XX:30）触发蜂鸣提示
 * 
 * 使用示例:
 *   TimeService *timeSvr = new TimeService(
 *       SYS_SVR_ID_TIME_SVR,
 *       SYS_SVR_TYPE_TIME_SVR
 *   );
 *   timeSvr->setNTPServer("ntp.aliyun.com");
 *   timeSvr->setBeepDriver(beep);
 *   timeSvr->SvrInit();
 *   timeSvr->SvrStart();
 ***************************************************************/
class TimeService : public ISysSvrInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @param parent 父对象指针
     */
    explicit TimeService(int32_t svr_id, int32_t svr_type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~TimeService();
    
    // ========== 实现ISysSvrInterface接口 ==========
    
    virtual bool SvrInit() override;
    virtual bool SvrStart() override;
    virtual bool SvrStop() override;
    
    virtual QString GetSvrName() const override {
        return "TimeService";
    }
    
    // ========== 时间服务特有接口 ==========
    
    /**
     * @brief 设置NTP服务器地址
     * @param server NTP服务器（IP或域名）
     */
    void setNTPServer(const QString &server);
    
    /**
     * @brief 立即执行NTP对时
     * @return true=成功, false=失败
     */
    bool syncTimeNow();
    
    /**
     * @brief 设置自动对时间隔
     * @param intervalHours 间隔小时数（0=禁用自动对时）
     */
    void setAutoSyncInterval(int intervalHours);
    
    /**
     * @brief 设置Beep驱动
     * @param beep Beep驱动指针
     */
    void setBeepDriver(DriverBeep *beep);
    
    /**
     * @brief 启用/禁用半点蜂鸣提示
     * @param enabled true=启用, false=禁用
     */
    void setHalfHourBeepEnabled(bool enabled);
    
    /**
     * @brief 获取当前时间
     * @return QDateTime 当前系统时间
     */
    QDateTime getCurrentTime() const;
    
    /**
     * @brief 获取上次对时时间
     * @return QDateTime 上次对时时间
     */
    QDateTime getLastSyncTime() const;
    
    /**
     * @brief 获取时间同步状态
     * @return true=已同步, false=未同步
     */
    bool isSynced() const { return m_isSynced; }

signals:
    /**
     * @brief 时间同步成功信号
     * @param syncTime 同步时间
     */
    void timeSynced(const QDateTime &syncTime);
    
    /**
     * @brief 时间同步失败信号
     * @param error 错误信息
     */
    void syncFailed(const QString &error);
    
    /**
     * @brief 半点到达信号
     * @param time 当前时间
     */
    void halfHourReached(const QDateTime &time);
    
    /**
     * @brief 整点到达信号
     * @param time 当前时间
     */
    void fullHourReached(const QDateTime &time);

private slots:
    /**
     * @brief 定时检查槽函数（每秒调用）
     */
    void onCheckTimer();
    
    /**
     * @brief 自动对时定时器槽函数
     */
    void onAutoSyncTimer();
    
    /**
     * @brief NTP UDP数据接收槽函数
     */
    void onNtpDataReceived();

private:
    /**
     * @brief 执行NTP时间同步
     * @return true=成功, false=失败
     */
    bool performNTPSync();
    
    /**
     * @brief 解析NTP响应包
     * @param data NTP数据包
     * @return 时间戳（Unix时间）
     */
    quint64 parseNTPResponse(const QByteArray &data);
    
    /**
     * @brief 设置系统时间
     * @param dateTime 要设置的时间
     * @return true=成功, false=失败
     */
    bool setSystemTime(const QDateTime &dateTime);
    
    /**
     * @brief 检查是否为半点时刻
     * @param time 要检查的时间
     * @return true=是半点, false=不是
     */
    bool isHalfHour(const QDateTime &time);

private:
    QTimer *m_checkTimer;           // 检查定时器（每秒）
    QTimer *m_autoSyncTimer;        // 自动对时定时器
    QUdpSocket *m_ntpSocket;        // NTP UDP套接字
    
    QString m_ntpServer;            // NTP服务器地址
    QDateTime m_lastSyncTime;       // 上次对时时间
    QDateTime m_lastBeepTime;       // 上次蜂鸣时间
    
    DriverBeep *m_pBeep;            // Beep驱动指针
    
    bool m_isSynced;                // 是否已同步
    bool m_halfHourBeepEnabled;     // 半点蜂鸣是否启用
    int m_autoSyncIntervalHours;    // 自动对时间隔（小时）
    
    int m_lastMinute;               // 上次检查的分钟数
};

#endif // TIMESERVICE_H

