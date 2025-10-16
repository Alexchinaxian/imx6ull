#include "drivers/temperature/DriverTemperature.h"
#include <QFile>
#include <QDebug>

/***************************************************************
 * 实现文件: driver_temperature.cpp
 * 功能: 温度监控驱动的具体实现
 * 说明: 读取thermal zone温度，实现温度监控和报警
 ***************************************************************/

/**
 * @brief 构造函数 - 初始化温度监控驱动
 * @param parent 父对象指针
 * 
 * 初始化参数：
 * - m_currentTemp: 当前温度初始化为0°C
 * - m_maxTemp: 最高温度初始化为-273°C（绝对零度）
 * - m_minTemp: 最低温度初始化为1000°C（一个很大的值）
 * - m_highThreshold: 高温报警阈值60°C
 * - m_isHighTemp: 高温状态标志初始为false
 * - m_sensorPath: 温度传感器路径（thermal_zone0）
 * 
 * 连接定时器信号到readTemperature槽函数
 */
DriverTemperature::DriverTemperature(QObject *parent) 
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_currentTemp(0.0f)
    , m_maxTemp(-273.0f)      // 绝对零度，确保第一次读取会更新
    , m_minTemp(1000.0f)      // 一个很高的温度，确保第一次读取会更新
    , m_highThreshold(60.0f)  // 高温报警阈值：60°C
    , m_isHighTemp(false)     // 初始状态：非高温
    , m_sensorPath("/sys/class/thermal/thermal_zone0/temp")
{
    // 连接定时器信号：定时器超时时调用readTemperature()
    connect(m_timer, &QTimer::timeout, this, &DriverTemperature::readTemperature);
}

DriverTemperature::~DriverTemperature()
{
    stop();
}

void DriverTemperature::initialize()
{
    qInfo() << "[Temperature] Initializing...";
    
    // 检查温度传感器文件是否存在
    QFile file(m_sensorPath);
    if (!file.exists()) {
        emit error("Temperature sensor not found: " + m_sensorPath);
        return;
    }
    
    qInfo() << "[Temperature] Sensor found:" << m_sensorPath;
    emit initialized();
}

void DriverTemperature::start()
{
    qInfo() << "[Temperature] Starting monitor...";
    m_timer->start(1000); // 每秒读取一次
    emit started();
}

void DriverTemperature::stop()
{
    qInfo() << "[Temperature] Stopping monitor...";
    m_timer->stop();
    emit stopped();
}

/**
 * @brief 读取温度（定时器回调函数）
 * 
 * 实现流程：
 * 1. 打开温度传感器文件
 * 2. 读取温度值（单位：毫摄氏度）
 * 3. 转换为摄氏度（除以1000）
 * 4. 更新最高/最低温度统计
 * 5. 发送温度变化信号
 * 6. 检查是否需要发送高温报警/恢复信号
 * 
 * 报警逻辑：
 * - 温度超过阈值 且 之前不是高温状态 -> 发送高温报警
 * - 温度低于阈值 且 之前是高温状态 -> 发送恢复正常信号
 * - 避免重复发送相同的报警信号
 * 
 * 注意：此函数由定时器触发，每秒调用一次
 */
void DriverTemperature::readTemperature()
{
    // 打开温度传感器文件
    QFile file(m_sensorPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;  // 打开失败，静默返回
    }
    
    // 读取温度数据
    QString data = file.readAll().trimmed();
    file.close();
    
    // 转换为整数
    bool ok;
    int tempValue = data.toInt(&ok);
    if (!ok) {
        return;  // 转换失败，静默返回
    }
    
    // 转换为摄氏度（sysfs中的值单位是毫摄氏度）
    // 例如：45000 表示 45.000°C
    m_currentTemp = tempValue / 1000.0f;
    
    // 更新最大值统计
    if (m_currentTemp > m_maxTemp) {
        m_maxTemp = m_currentTemp;
    }
    
    // 更新最小值统计
    if (m_currentTemp < m_minTemp) {
        m_minTemp = m_currentTemp;
    }
    
    // 发送温度变化信号（每次读取都发送）
    emit temperatureChanged(m_currentTemp);
    
    // 检查高温报警条件
    if (m_currentTemp > m_highThreshold) {
        // 温度超过阈值
        if (!m_isHighTemp) {
            // 之前不是高温状态，现在进入高温状态
            m_isHighTemp = true;
            emit temperatureHigh(m_currentTemp);  // 发送高温报警信号
        }
        // 如果已经是高温状态，不重复发送信号
    } else {
        // 温度在正常范围
        if (m_isHighTemp) {
            // 之前是高温状态，现在恢复正常
            m_isHighTemp = false;
            emit temperatureNormal(m_currentTemp);  // 发送恢复正常信号
        }
        // 如果本来就是正常状态，不发送信号
    }
}

TemperatureInfo DriverTemperature::getTemperatureInfo() const
{
    TemperatureInfo info;
    info.currentTemp = m_currentTemp;
    info.maxTemp = m_maxTemp;
    info.minTemp = m_minTemp;
    info.sensorType = "CPU Thermal Zone";
    return info;
}

