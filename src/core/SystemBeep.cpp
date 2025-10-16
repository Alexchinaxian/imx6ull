/***************************************************************
 * Copyright: Alex
 * FileName: SystemBeep.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 系统蜂鸣器提示管理
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#include "core/SystemBeep.h"
#include "drivers/beep/DriverBeep.h"
#include <QThread>
#include <QDebug>

// 初始化静态成员
SystemBeep* SystemBeep::m_instance = nullptr;
QMutex SystemBeep::m_instanceMutex;

/**
 * @brief 构造函数
 */
SystemBeep::SystemBeep(QObject *parent)
    : QObject(parent)
    , m_beepDriver(nullptr)
    , m_enabled(true)
{
    // 创建蜂鸣器驱动
    m_beepDriver = new DriverBeep("beep", this);
    
    qInfo() << "[SystemBeep] 系统蜂鸣器管理器创建";
}

/**
 * @brief 析构函数
 */
SystemBeep::~SystemBeep()
{
    qInfo() << "[SystemBeep] 系统蜂鸣器管理器销毁";
}

/**
 * @brief 获取单例实例
 */
SystemBeep* SystemBeep::getInstance()
{
    if (m_instance == nullptr)
    {
        QMutexLocker locker(&m_instanceMutex);
        if (m_instance == nullptr)
        {
            m_instance = new SystemBeep();
        }
    }
    
    return m_instance;
}

/**
 * @brief 播放初始化完成提示音
 */
void SystemBeep::playInitComplete()
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qInfo() << "[SystemBeep] 🔔 播放初始化完成提示音（滴滴）";
    
    // 短促响2次
    m_beepDriver->beep(2, 100);
}

/**
 * @brief 播放配置加载成功提示音
 */
void SystemBeep::playConfigLoaded()
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qInfo() << "[SystemBeep] 🔔 播放配置加载成功提示音（滴）";
    
    // 短促响1次
    m_beepDriver->beep(1, 150);
}

/**
 * @brief 播放错误警告音
 */
void SystemBeep::playError()
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qWarning() << "[SystemBeep] 🚨 播放错误警告音（长响）";
    
    // 长响1次
    m_beepDriver->alarm(1, 500, 200);
}

/**
 * @brief 播放成功提示音
 */
void SystemBeep::playSuccess()
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qInfo() << "[SystemBeep] 🔔 播放成功提示音（滴滴滴）";
    
    // 快速短促响3次
    m_beepDriver->beep(3, 80);
}

/**
 * @brief 播放警告音
 */
void SystemBeep::playWarning()
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qWarning() << "[SystemBeep] ⚠ 播放警告提示音（滴~滴~）";
    
    // 中等响2次
    m_beepDriver->alarm(2, 200, 200);
}

/**
 * @brief 播放关机提示音
 */
void SystemBeep::playShutdown()
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qInfo() << "[SystemBeep] 🔔 播放关机提示音（滴滴滴滴）";
    
    // 快速短促响4次
    m_beepDriver->beep(4, 80);
    
    // 等待播放完成
    QThread::msleep(400);
}

/**
 * @brief 播放自定义提示音
 */
void SystemBeep::playCustom(int count, int duration, int interval)
{
    if (!m_enabled || !m_beepDriver || !m_beepDriver->isAvailable())
    {
        return;
    }
    
    qInfo() << "[SystemBeep] 🔔 播放自定义提示音"
            << "次数:" << count 
            << "时长:" << duration << "ms"
            << "间隔:" << interval << "ms";
    
    if (duration <= 100)
    {
        m_beepDriver->beep(count, interval);
    }
    else
    {
        m_beepDriver->alarm(count, duration, interval);
    }
}

/**
 * @brief 启用/禁用蜂鸣器
 */
void SystemBeep::setEnabled(bool enabled)
{
    m_enabled = enabled;
    qInfo() << "[SystemBeep] 蜂鸣器" << (enabled ? "启用" : "禁用");
}

/**
 * @brief 检查蜂鸣器是否可用
 */
bool SystemBeep::isAvailable() const
{
    return m_beepDriver && m_beepDriver->isAvailable();
}

