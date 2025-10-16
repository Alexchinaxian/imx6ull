/***************************************************************
 * Copyright: Alex
 * FileName: TimeService.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 时间服务实现
 ***************************************************************/

#include "services/time/TimeService.h"
#include "core/LogManager.h"
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QHostInfo>
#include <QHostAddress>

// NTP相关常量
#define NTP_PORT 123
#define NTP_TIMEOUT 5000
#define NTP_TIMESTAMP_DELTA 2208988800ull  // 1900-1970年的秒数差

/***************************************************************
 * 构造函数
 ***************************************************************/
TimeService::TimeService(int32_t svr_id, int32_t svr_type, QObject *parent)
    : ISysSvrInterface(svr_id, svr_type, parent)
    , m_ntpServer("ntp.aliyun.com")
    , m_pBeep(nullptr)
    , m_isSynced(false)
    , m_halfHourBeepEnabled(true)
    , m_autoSyncIntervalHours(24)
    , m_lastMinute(-1)
{
    // 设置Qt的默认时区为香港（UTC+8，与上海相同）
    qputenv("TZ", "Asia/Hong_Kong");
    
    // 创建定时器
    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(1000);  // 每秒检查一次
    
    m_autoSyncTimer = new QTimer(this);
    m_autoSyncTimer->setInterval(m_autoSyncIntervalHours * 3600 * 1000);  // 默认24小时
    
    // 创建NTP UDP套接字
    m_ntpSocket = new QUdpSocket(this);
    
    // 连接信号槽
    connect(m_checkTimer, &QTimer::timeout, this, &TimeService::onCheckTimer);
    connect(m_autoSyncTimer, &QTimer::timeout, this, &TimeService::onAutoSyncTimer);
    connect(m_ntpSocket, &QUdpSocket::readyRead, this, &TimeService::onNtpDataReceived);
    
    qInfo() << "[TimeService] 时间服务创建（时区: Asia/Hong_Kong UTC+8）";
}

/***************************************************************
 * 析构函数
 ***************************************************************/
TimeService::~TimeService()
{
    qInfo() << "[TimeService] 时间服务销毁";
    SvrStop();
}

/***************************************************************
 * 服务初始化
 ***************************************************************/
bool TimeService::SvrInit()
{
    qInfo() << "[TimeService] 初始化时间服务...";
    qInfo() << "  NTP服务器: " << m_ntpServer;
    qInfo() << "  自动对时间隔: " << m_autoSyncIntervalHours << " 小时";
    qInfo() << "  半点蜂鸣提示: " << (m_halfHourBeepEnabled ? "启用" : "禁用");
    qInfo() << "[TimeService] ✓ 时间服务初始化成功";
    
    return true;
}

/***************************************************************
 * 服务启动
 ***************************************************************/
bool TimeService::SvrStart()
{
    LOG_INFO("Time", "启动时间服务...");
    
    // 1. 设置时区为中国时区（UTC+8，使用Hong_Kong）
    LOG_INFO("Time", "设置时区为Asia/Hong_Kong (UTC+8)...");
    QProcess tzProcess;
    tzProcess.start("sh", QStringList() << "-c" << "ln -sf /usr/share/zoneinfo/Asia/Hong_Kong /etc/localtime && echo 'Asia/Hong_Kong' > /etc/timezone");
    if (tzProcess.waitForFinished(3000)) {
        LOG_INFO("Time", "✓ 时区设置成功: Asia/Hong_Kong (UTC+8 中国时区)");
    } else {
        LOG_WARNING("Time", "⚠ 时区设置失败（使用当前时区）");
    }
    
    // 2. 立即执行一次NTP对时
    LOG_INFO("Time", "执行初始NTP对时...");
    if (performNTPSync()) {
        LOG_INFO("Time", "✓ NTP对时成功");
    } else {
        LOG_WARNING("Time", "✗ NTP对时失败（使用系统时间）");
    }
    
    // 3. 检查蜂鸣器设备状态
    if (m_pBeep) {
        if (m_pBeep->isAvailable()) {
            LOG_INFO("Time", "✓ 蜂鸣器设备可用");
        } else {
            LOG_WARNING("Time", "⚠ 蜂鸣器设备不可用（/sys/class/leds/beep 不存在），半点提醒将静默工作");
            LOG_WARNING("Time", "  可运行 tools/setup_test_beep.sh 创建测试设备");
        }
    } else {
        LOG_WARNING("Time", "⚠ 蜂鸣器驱动未初始化");
    }
    
    // 4. 启动定时检查（每秒检查，用于半点提醒）
    m_checkTimer->start();
    LOG_INFO("Time", QString("✓ 半点提醒定时器启动 (检查间隔: 1秒, 状态: %1)")
        .arg(m_halfHourBeepEnabled ? "启用" : "禁用"));
    
    // 5. 启动自动对时定时器
    if (m_autoSyncIntervalHours > 0) {
        m_autoSyncTimer->start();
        LOG_INFO("Time", QString("✓ 自动对时已启用（每%1小时）").arg(m_autoSyncIntervalHours));
    }
    
    LOG_INFO("Time", "========================================");
    LOG_INFO("Time", "✓ 时间服务启动成功");
    LOG_INFO("Time", QString("当前时间: %1 %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(QDateTime::currentDateTime().toString("dddd")));
    LOG_INFO("Time", "时区: Asia/Hong_Kong (UTC+8 中国时区)");
    LOG_INFO("Time", "========================================");
    
    return true;
}

/***************************************************************
 * 服务停止
 ***************************************************************/
bool TimeService::SvrStop()
{
    qInfo() << "[TimeService] 停止时间服务...";
    
    m_checkTimer->stop();
    m_autoSyncTimer->stop();
    
    if (m_pBeep) {
        m_pBeep->stopAll();
    }
    
    qInfo() << "[TimeService] ✓ 时间服务已停止";
    
    return true;
}

/***************************************************************
 * 设置NTP服务器
 ***************************************************************/
void TimeService::setNTPServer(const QString &server)
{
    m_ntpServer = server;
    qInfo() << "[TimeService] NTP服务器设置为:" << m_ntpServer;
}

/***************************************************************
 * 立即执行NTP对时
 ***************************************************************/
bool TimeService::syncTimeNow()
{
    qInfo() << "[TimeService] 手动触发NTP对时...";
    return performNTPSync();
}

/***************************************************************
 * 设置自动对时间隔
 ***************************************************************/
void TimeService::setAutoSyncInterval(int intervalHours)
{
    m_autoSyncIntervalHours = intervalHours;
    
    if (intervalHours > 0) {
        m_autoSyncTimer->setInterval(intervalHours * 3600 * 1000);
        qInfo() << "[TimeService] 自动对时间隔设置为:" << intervalHours << "小时";
    } else {
        m_autoSyncTimer->stop();
        qInfo() << "[TimeService] 自动对时已禁用";
    }
}

/***************************************************************
 * 设置Beep驱动
 ***************************************************************/
void TimeService::setBeepDriver(DriverBeep *beep)
{
    m_pBeep = beep;
    qInfo() << "[TimeService] Beep驱动已设置";
}

/***************************************************************
 * 启用/禁用半点蜂鸣
 ***************************************************************/
void TimeService::setHalfHourBeepEnabled(bool enabled)
{
    m_halfHourBeepEnabled = enabled;
    qInfo() << "[TimeService] 半点蜂鸣提示:" << (enabled ? "启用" : "禁用");
}

/***************************************************************
 * 获取当前时间
 ***************************************************************/
QDateTime TimeService::getCurrentTime() const
{
    return QDateTime::currentDateTime();
}

/***************************************************************
 * 获取上次对时时间
 ***************************************************************/
QDateTime TimeService::getLastSyncTime() const
{
    return m_lastSyncTime;
}

/***************************************************************
 * 定时检查槽函数
 ***************************************************************/
void TimeService::onCheckTimer()
{
    QDateTime now = QDateTime::currentDateTime();
    int currentMinute = now.time().minute();
    
    // 避免重复触发（检查分钟是否变化）
    if (currentMinute == m_lastMinute) {
        return;
    }
    
    m_lastMinute = currentMinute;
    
    // 检查是否为半点时刻（XX:00或XX:30）- 生产模式
    if (currentMinute == 0 || currentMinute == 30) {
        if (m_halfHourBeepEnabled && m_pBeep && m_pBeep->isAvailable()) {
            // 避免短时间内重复蜂鸣
            if (m_lastBeepTime.isValid()) {
                qint64 secondsSinceLastBeep = m_lastBeepTime.secsTo(now);
                if (secondsSinceLastBeep < 50) {  // 50秒内不重复
                    LOG_DEBUG("Time", QString("跳过半点提醒（距上次仅%1秒）").arg(secondsSinceLastBeep));
                    return;
                }
            }
            
            // 触发蜂鸣提示
            QString timeStr = now.toString("yyyy-MM-dd hh:mm:ss");
            if (currentMinute == 0) {
                LOG_INFO("Time", QString("🕐 整点报时: %1 (蜂鸣2次)").arg(timeStr));
                m_pBeep->beep(2, 300);  // 整点2次
                emit fullHourReached(now);
            } else {
                LOG_INFO("Time", QString("🕑 半点提醒: %1 (蜂鸣1次)").arg(timeStr));
                m_pBeep->beep(1, 300);  // 半点1次
                emit halfHourReached(now);
            }
            
            m_lastBeepTime = now;
        } else if (currentMinute == 0 || currentMinute == 30) {
            // 半点提醒功能被禁用或蜂鸣器不可用
            if (!m_halfHourBeepEnabled) {
                LOG_DEBUG("Time", "半点提醒功能已禁用");
            } else if (!m_pBeep) {
                LOG_WARNING("Time", "蜂鸣器驱动未初始化，无法执行半点提醒");
            } else if (!m_pBeep->isAvailable()) {
                LOG_WARNING("Time", "蜂鸣器设备不可用：/sys/class/leds/beep 不存在");
            }
        }
    }
}

/***************************************************************
 * 自动对时定时器槽函数
 ***************************************************************/
void TimeService::onAutoSyncTimer()
{
    qInfo() << "[TimeService] 自动NTP对时触发...";
    performNTPSync();
}

/***************************************************************
 * 执行NTP时间同步
 ***************************************************************/
bool TimeService::performNTPSync()
{
    qInfo() << "[TimeService] 连接NTP服务器:" << m_ntpServer;
    
    // 构建NTP请求包
    QByteArray ntpPacket;
    ntpPacket.resize(48);
    ntpPacket.fill(0);
    
    // NTP版本3，客户端模式
    ntpPacket[0] = 0x1B;  // LI=0, VN=3, Mode=3
    
    // 解析NTP服务器地址
    QHostAddress serverAddr;
    if (!serverAddr.setAddress(m_ntpServer)) {
        // 如果不是IP地址，需要DNS解析
        QHostInfo hostInfo = QHostInfo::fromName(m_ntpServer);
        if (hostInfo.error() != QHostInfo::NoError || hostInfo.addresses().isEmpty()) {
            qWarning() << "[TimeService] DNS解析失败:" << hostInfo.errorString();
            emit syncFailed("DNS解析失败");
            return false;
        }
        serverAddr = hostInfo.addresses().first();
    }
    
    qInfo() << "  NTP服务器地址:" << serverAddr.toString();
    
    // 发送NTP请求
    qint64 sentBytes = m_ntpSocket->writeDatagram(ntpPacket, serverAddr, NTP_PORT);
    if (sentBytes < 0) {
        qWarning() << "[TimeService] NTP请求发送失败";
        emit syncFailed("发送请求失败");
        return false;
    }
    
    // 等待响应（最多5秒）
    if (!m_ntpSocket->waitForReadyRead(NTP_TIMEOUT)) {
        qWarning() << "[TimeService] NTP响应超时";
        emit syncFailed("响应超时");
        return false;
    }
    
    // 读取响应
    QByteArray response;
    response.resize(m_ntpSocket->pendingDatagramSize());
    m_ntpSocket->readDatagram(response.data(), response.size());
    
    if (response.size() < 48) {
        qWarning() << "[TimeService] NTP响应数据不完整";
        emit syncFailed("响应数据不完整");
        return false;
    }
    
    // 解析NTP时间戳
    quint64 timestamp = parseNTPResponse(response);
    if (timestamp == 0) {
        qWarning() << "[TimeService] NTP时间戳解析失败";
        emit syncFailed("时间戳解析失败");
        return false;
    }
    
    // 设置系统时间
    QDateTime ntpTime = QDateTime::fromSecsSinceEpoch(timestamp, Qt::UTC);
    
    if (setSystemTime(ntpTime)) {
        m_lastSyncTime = QDateTime::currentDateTime();
        m_isSynced = true;
        
        LOG_INFO("Time", "✓ NTP对时成功");
        LOG_INFO("Time", QString("  UTC时间: %1").arg(ntpTime.toString("yyyy-MM-dd hh:mm:ss")));
        LOG_INFO("Time", QString("  本地时间: %1 %2").arg(m_lastSyncTime.toString("yyyy-MM-dd hh:mm:ss")).arg(m_lastSyncTime.toString("dddd")));
        LOG_INFO("Time", "  时区: Asia/Hong_Kong (UTC+8 中国时区)");
        
        // 同步到硬件时钟
        QProcess hwclock;
        hwclock.start("sh", QStringList() << "-c" << "hwclock -w 2>/dev/null");
        hwclock.waitForFinished(2000);
        
        emit timeSynced(m_lastSyncTime);
        return true;
    } else {
        LOG_ERROR("Time", "系统时间设置失败");
        emit syncFailed("设置系统时间失败");
        return false;
    }
}

/***************************************************************
 * NTP UDP数据接收槽函数
 ***************************************************************/
void TimeService::onNtpDataReceived()
{
    // UDP方式接收NTP响应（备用方案）
    while (m_ntpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_ntpSocket->pendingDatagramSize());
        
        m_ntpSocket->readDatagram(datagram.data(), datagram.size());
        
        if (datagram.size() >= 48) {
            quint64 timestamp = parseNTPResponse(datagram);
            if (timestamp > 0) {
                QDateTime ntpTime = QDateTime::fromSecsSinceEpoch(timestamp);
                qInfo() << "[TimeService] NTP时间:" << ntpTime.toString("yyyy-MM-dd hh:mm:ss");
                
                // 设置系统时间
                if (setSystemTime(ntpTime)) {
                    m_lastSyncTime = ntpTime;
                    m_isSynced = true;
                    emit timeSynced(ntpTime);
                }
            }
        }
    }
}

/***************************************************************
 * 解析NTP响应包
 ***************************************************************/
quint64 TimeService::parseNTPResponse(const QByteArray &data)
{
    if (data.size() < 48) {
        return 0;
    }
    
    // NTP时间戳在第40-43字节（Transmit Timestamp）
    quint32 ntpTimestamp = (static_cast<quint8>(data[40]) << 24) |
                          (static_cast<quint8>(data[41]) << 16) |
                          (static_cast<quint8>(data[42]) << 8) |
                          static_cast<quint8>(data[43]);
    
    // 转换为Unix时间戳
    if (ntpTimestamp > NTP_TIMESTAMP_DELTA) {
        return ntpTimestamp - NTP_TIMESTAMP_DELTA;
    }
    
    return 0;
}

/***************************************************************
 * 设置系统时间
 ***************************************************************/
bool TimeService::setSystemTime(const QDateTime &dateTime)
{
    // 使用date命令设置系统时间（需要root权限）
    // 格式：MMDDhhmmYYYY.ss
    QString timeStr = dateTime.toUTC().toString("MMddhhmmyyyy.ss");
    
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("TZ", "UTC");  // date命令使用UTC时间
    process.setProcessEnvironment(env);
    
    // 使用-u参数设置UTC时间
    process.start("date", QStringList() << "-u" << "-s" << dateTime.toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        qInfo() << "[TimeService] ✓ 系统时间已设置";
        return true;
    }
    
    // 如果上面的方法失败，尝试使用传统格式
    process.start("date", QStringList() << "-u" << timeStr);
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        qInfo() << "[TimeService] ✓ 系统时间已设置";
        return true;
    }
    
    QString error = QString::fromUtf8(process.readAllStandardError());
    qWarning() << "[TimeService] ✗ 系统时间设置失败:" << error;
    return false;
}

/***************************************************************
 * 检查是否为半点时刻
 ***************************************************************/
bool TimeService::isHalfHour(const QDateTime &time)
{
    int minute = time.time().minute();
    int second = time.time().second();
    
    // 检查是否为XX:00:00或XX:30:00（允许±2秒误差）
    if (second <= 2 || second >= 58) {
        return (minute == 0 || minute == 30 || minute == 59);
    }
    
    return false;
}

