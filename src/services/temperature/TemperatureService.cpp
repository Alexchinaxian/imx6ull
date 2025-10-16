/***************************************************************
 * Copyright: Alex
 * FileName: TemperatureService.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 温度监控服务适配器实现
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#include "services/temperature/TemperatureService.h"
#include <QDebug>

/**
 * @brief 构造函数 - 初始化温度服务
 * @param svr_id 服务ID
 * @param svr_type 服务类型
 * @param parent 父对象指针
 */
TemperatureService::TemperatureService(int32_t svr_id, int32_t svr_type, QObject *parent)
    : ISysSvrInterface(svr_id, svr_type, parent)
    , m_pTempDriver(nullptr)
    , m_pThread(nullptr)
    , m_IsInitialized(false)
    , m_IsStarted(false)
{
    qInfo() << "[TemperatureService] 温度服务创建";
}

/**
 * @brief 析构函数 - 清理资源
 */
TemperatureService::~TemperatureService()
{
    qInfo() << "[TemperatureService] 温度服务销毁";
    
    // 停止服务
    SvrStop();
    
    // 清理驱动对象（由Qt对象树自动管理）
    // 清理线程对象（由Qt对象树自动管理）
}

/**
 * @brief 服务初始化
 * @return true=成功, false=失败
 * 
 * 实现步骤:
 *   1. 创建温度驱动对象
 *   2. 创建独立线程
 *   3. 将驱动移到独立线程
 *   4. 连接信号和槽
 */
bool TemperatureService::SvrInit()
{
    if (m_IsInitialized)
    {
        qWarning() << "[TemperatureService] 服务已初始化";
        return false;
    }
    
    qInfo() << "[TemperatureService] 初始化温度监控服务...";
    
    // 1. 创建温度驱动对象
    m_pTempDriver = new DriverTemperature();
    if (!m_pTempDriver)
    {
        qCritical() << "[TemperatureService] 创建温度驱动失败";
        return false;
    }
    
    // 2. 创建独立线程
    m_pThread = new QThread();
    if (!m_pThread)
    {
        qCritical() << "[TemperatureService] 创建线程失败";
        delete m_pTempDriver;
        m_pTempDriver = nullptr;
        return false;
    }
    
    // 3. 将驱动移到独立线程
    m_pTempDriver->moveToThread(m_pThread);
    
    // 4. 连接信号和槽
    // 线程启动时初始化驱动
    connect(m_pThread, &QThread::started, 
            m_pTempDriver, &DriverTemperature::initialize);
    
    // 驱动初始化完成后启动监控
    connect(m_pTempDriver, &DriverTemperature::initialized,
            this, &TemperatureService::onDriverInitialized);
    
    // 驱动启动完成
    connect(m_pTempDriver, &DriverTemperature::started,
            this, &TemperatureService::onDriverStarted);
    
    // 驱动错误处理
    connect(m_pTempDriver, &DriverTemperature::error,
            this, &TemperatureService::onDriverError);
    
    // 转发温度变化信号
    connect(m_pTempDriver, &DriverTemperature::temperatureChanged,
            this, &TemperatureService::temperatureChanged);
    
    // 转发高温报警信号
    connect(m_pTempDriver, &DriverTemperature::temperatureHigh,
            this, &TemperatureService::temperatureHigh);
    
    // 转发温度恢复信号
    connect(m_pTempDriver, &DriverTemperature::temperatureNormal,
            this, &TemperatureService::temperatureNormal);
    
    // 清理资源
    connect(m_pThread, &QThread::finished,
            m_pTempDriver, &QObject::deleteLater);
    connect(m_pThread, &QThread::finished,
            m_pThread, &QObject::deleteLater);
    
    m_IsInitialized = true;
    qInfo() << "[TemperatureService] ✓ 温度服务初始化成功";
    
    return true;
}

/**
 * @brief 服务启动
 * @return true=成功, false=失败
 * 
 * 实现步骤:
 *   1. 检查初始化状态
 *   2. 启动独立线程
 */
bool TemperatureService::SvrStart()
{
    if (!m_IsInitialized)
    {
        qWarning() << "[TemperatureService] 服务未初始化，无法启动";
        return false;
    }
    
    if (m_IsStarted)
    {
        qWarning() << "[TemperatureService] 服务已启动";
        return false;
    }
    
    qInfo() << "[TemperatureService] 启动温度监控服务...";
    
    // 启动线程（会触发驱动的initialize）
    m_pThread->start();
    
    m_IsStarted = true;
    qInfo() << "[TemperatureService] ✓ 温度服务启动成功";
    
    return true;
}

/**
 * @brief 服务停止
 * @return true=成功, false=失败
 * 
 * 实现步骤:
 *   1. 停止温度驱动
 *   2. 退出线程
 *   3. 等待线程结束
 */
bool TemperatureService::SvrStop()
{
    if (!m_IsStarted)
    {
        return true;  // 未启动，无需停止
    }
    
    qInfo() << "[TemperatureService] 停止温度监控服务...";
    
    // 停止温度驱动
    if (m_pTempDriver)
    {
        QMetaObject::invokeMethod(m_pTempDriver, "stop", Qt::QueuedConnection);
    }
    
    // 退出线程
    if (m_pThread && m_pThread->isRunning())
    {
        m_pThread->quit();
        m_pThread->wait(3000);  // 等待最多3秒
    }
    
    m_IsStarted = false;
    qInfo() << "[TemperatureService] ✓ 温度服务已停止";
    
    return true;
}

/**
 * @brief 获取当前温度
 * @return float 当前温度（°C）
 */
float TemperatureService::GetCurrentTemperature() const
{
    if (m_pTempDriver)
    {
        return m_pTempDriver->getTemperatureInfo().currentTemp;
    }
    return 0.0f;
}

/**
 * @brief 获取温度信息
 * @return TemperatureInfo 温度信息结构
 */
TemperatureInfo TemperatureService::GetTemperatureInfo() const
{
    if (m_pTempDriver)
    {
        return m_pTempDriver->getTemperatureInfo();
    }
    
    TemperatureInfo info;
    info.currentTemp = 0.0f;
    info.maxTemp = 0.0f;
    info.minTemp = 0.0f;
    info.sensorType = "N/A";
    return info;
}

/**
 * @brief 设置高温报警阈值
 * @param threshold 阈值温度（°C）
 */
void TemperatureService::SetHighThreshold(float threshold)
{
    qInfo() << "[TemperatureService] 设置高温阈值:" << threshold << "°C";
}

// ========== 私有槽函数实现 ==========

/**
 * @brief 驱动初始化完成槽函数
 */
void TemperatureService::onDriverInitialized()
{
    qInfo() << "[TemperatureService] 驱动初始化完成，启动温度监控...";
    
    // 驱动初始化完成后自动启动监控
    QMetaObject::invokeMethod(m_pTempDriver, "start", Qt::QueuedConnection);
}

/**
 * @brief 驱动启动完成槽函数
 */
void TemperatureService::onDriverStarted()
{
    qInfo() << "[TemperatureService] ✓ 温度监控已启动";
    emit statusChanged(m_SvrId, "运行中");
}

/**
 * @brief 驱动错误槽函数
 * @param errorString 错误信息
 */
void TemperatureService::onDriverError(const QString &errorString)
{
    qCritical() << "[TemperatureService] ✗ 驱动错误:" << errorString;
    emit error(m_SvrId, errorString);
}


