/***************************************************************
 * Copyright: Alex
 * FileName: driver_beep.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 蜂鸣器驱动实现
 ***************************************************************/

#include "drivers/beep/DriverBeep.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

/***************************************************************
 * 构造函数
 ***************************************************************/
DriverBeep::DriverBeep(const QString &beepName, QObject *parent)
    : QObject(parent)
    , m_beepName(beepName)
    , m_currentCount(0)
    , m_targetCount(0)
    , m_duration(0)
    , m_interval(0)
    , m_timerState(false)
{
    m_sysfsPath = QString("/sys/class/leds/%1").arg(m_beepName);
    
    // 创建定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DriverBeep::onTimerTimeout);
    
    // 检查蜂鸣器是否可用
    if (!isAvailable()) {
        qWarning() << "Beep device not found:" << m_sysfsPath;
    } else {
        qInfo() << "Beep device initialized:" << m_sysfsPath;
    }
}

/***************************************************************
 * 析构函数
 ***************************************************************/
DriverBeep::~DriverBeep()
{
    stopAll();
    turnOff();
}

/***************************************************************
 * 打开蜂鸣器
 ***************************************************************/
bool DriverBeep::turnOn()
{
    if (writeBrightness(255)) {
        emit stateChanged(true);
        return true;
    }
    return false;
}

/***************************************************************
 * 关闭蜂鸣器
 ***************************************************************/
bool DriverBeep::turnOff()
{
    if (writeBrightness(0)) {
        emit stateChanged(false);
        return true;
    }
    return false;
}

/***************************************************************
 * 切换状态
 ***************************************************************/
bool DriverBeep::toggle()
{
    if (isOn()) {
        return turnOff();
    } else {
        return turnOn();
    }
}

/***************************************************************
 * 设置强度
 ***************************************************************/
bool DriverBeep::setIntensity(int intensity)
{
    if (intensity < 0) intensity = 0;
    if (intensity > 255) intensity = 255;
    
    return writeBrightness(intensity);
}

/***************************************************************
 * 获取当前状态
 ***************************************************************/
bool DriverBeep::isOn() const
{
    return (readBrightness() > 0);
}

/***************************************************************
 * 蜂鸣N次
 ***************************************************************/
void DriverBeep::beep(int count, int intervalMs)
{
    if (count <= 0 || intervalMs < 50) {
        return;
    }
    
    // 检查设备是否可用
    if (!isAvailable()) {
        qWarning() << "[DriverBeep] 设备不可用，忽略蜂鸣请求:" << m_sysfsPath;
        return;
    }
    
    stopAll();
    
    m_targetCount = count;
    m_currentCount = 0;
    m_duration = 100;  // 短促：100ms
    m_interval = intervalMs;
    m_timerState = true;  // 从开启状态开始
    
    // 立即开启第一次
    turnOn();
    m_timer->start(m_duration);
}

/***************************************************************
 * 报警模式
 ***************************************************************/
void DriverBeep::alarm(int count, int durationMs, int intervalMs)
{
    if (count <= 0 || durationMs < 50 || intervalMs < 50) {
        return;
    }
    
    // 检查设备是否可用
    if (!isAvailable()) {
        qWarning() << "[DriverBeep] 设备不可用，忽略报警请求:" << m_sysfsPath;
        return;
    }
    
    stopAll();
    
    m_targetCount = count;
    m_currentCount = 0;
    m_duration = durationMs;
    m_interval = intervalMs;
    m_timerState = true;
    
    // 立即开启第一次
    turnOn();
    m_timer->start(m_duration);
}

/***************************************************************
 * 停止所有定时操作
 ***************************************************************/
void DriverBeep::stopAll()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    m_currentCount = 0;
    m_targetCount = 0;
}

/***************************************************************
 * 检查是否可用
 ***************************************************************/
bool DriverBeep::isAvailable() const
{
    return checkPath();
}

/***************************************************************
 * 定时器槽函数
 ***************************************************************/
void DriverBeep::onTimerTimeout()
{
    if (m_timerState) {
        // 当前是开启状态，切换到关闭
        turnOff();
        m_timerState = false;
        m_currentCount++;
        
        if (m_currentCount >= m_targetCount) {
            // 完成所有次数
            stopAll();
            emit finished();
            return;
        }
        
        // 设置间隔定时器
        m_timer->start(m_interval);
    } else {
        // 当前是关闭状态，切换到开启
        turnOn();
        m_timerState = true;
        
        // 设置持续定时器
        m_timer->start(m_duration);
    }
}

/***************************************************************
 * 写入brightness文件
 ***************************************************************/
bool DriverBeep::writeBrightness(int value)
{
    QString brightnessPath = m_sysfsPath + "/brightness";
    
    QFile file(brightnessPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QString errorMsg = QString("Failed to open: %1").arg(brightnessPath);
        emit error(errorMsg);
        return false;
    }
    
    QTextStream out(&file);
    out << value;
    file.close();
    
    return true;
}

/***************************************************************
 * 读取brightness文件
 ***************************************************************/
int DriverBeep::readBrightness() const
{
    QString brightnessPath = m_sysfsPath + "/brightness";
    
    QFile file(brightnessPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }
    
    QTextStream in(&file);
    QString value = in.readLine().trimmed();
    file.close();
    
    return value.toInt();
}

/***************************************************************
 * 检查路径是否存在
 ***************************************************************/
bool DriverBeep::checkPath() const
{
    return QDir(m_sysfsPath).exists();
}

