#include "drivers/pwm/DriverPWM.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDebug>

/***************************************************************
 * 实现文件: driver_pwm.cpp
 * 功能: PWM驱动的具体实现
 * 说明: 通过读写sysfs文件系统来控制PWM
 ***************************************************************/

/**
 * @brief 构造函数 - 初始化PWM驱动实例
 * @param chipNum PWM芯片编号
 * @param channelNum PWM通道编号
 * @param parent 父对象指针
 * 
 * 初始化默认值：
 * - m_period: 1000000ns = 1ms = 1KHz
 * - m_dutyCycle: 500000ns = 50%占空比
 * - m_basePath: /sys/class/pwm/pwmchipN
 * - m_pwmPath: /sys/class/pwm/pwmchipN/pwmM
 */
DriverPWM::DriverPWM(int chipNum, int channelNum, QObject *parent)
    : QObject(parent)
    , m_chipNum(chipNum)
    , m_channelNum(channelNum)
    , m_exported(false)
    , m_enabled(false)
    , m_period(1000000)   // 默认1ms周期（1KHz频率）
    , m_dutyCycle(500000) // 默认50%占空比
{
    // 构造PWM基础路径和通道路径
    m_basePath = QString("/sys/class/pwm/pwmchip%1").arg(chipNum);
    m_pwmPath = m_basePath + QString("/pwm%1").arg(channelNum);
}

/**
 * @brief 析构函数 - 清理PWM资源
 * 
 * 自动调用unexportPWM()释放PWM资源
 */
DriverPWM::~DriverPWM()
{
    unexportPWM();  // 释放PWM资源
}

bool DriverPWM::writeToFile(const QString &filename, const QString &value)
{
    QString path = (filename.startsWith("/")) ? filename : (m_pwmPath + "/" + filename);
    QFile file(path);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit error(QString("Failed to write %1: %2").arg(path, file.errorString()));
        return false;
    }
    
    QTextStream out(&file);
    out << value;
    file.close();
    
    return true;
}

QString DriverPWM::readFromFile(const QString &filename)
{
    QString path = m_pwmPath + "/" + filename;
    QFile file(path);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    QString content = in.readAll().trimmed();
    file.close();
    
    return content;
}

bool DriverPWM::exportPWM()
{
    if (m_exported) {
        return true;
    }
    
    // 检查是否已导出
    if (QFile::exists(m_pwmPath)) {
        m_exported = true;
        qInfo() << "PWM" << m_chipNum << ":" << m_channelNum << "already exported";
        return true;
    }
    
    // 导出PWM
    QString exportPath = m_basePath + "/export";
    if (!writeToFile(exportPath, QString::number(m_channelNum))) {
        return false;
    }
    
    // 等待sysfs节点创建
    for (int i = 0; i < 10; i++) {
        if (QFile::exists(m_pwmPath)) {
            m_exported = true;
            qInfo() << "PWM" << m_chipNum << ":" << m_channelNum << "exported successfully";
            return true;
        }
        QThread::msleep(10);
    }
    
    emit error(QString("PWM%1:%2 export timeout").arg(m_chipNum).arg(m_channelNum));
    return false;
}

bool DriverPWM::unexportPWM()
{
    if (!m_exported) {
        return true;
    }
    
    // 先禁用
    setEnable(false);
    
    QString unexportPath = m_basePath + "/unexport";
    if (!writeToFile(unexportPath, QString::number(m_channelNum))) {
        return false;
    }
    
    m_exported = false;
    qInfo() << "PWM" << m_chipNum << ":" << m_channelNum << "unexported";
    return true;
}

/**
 * @brief 设置PWM周期
 * @param period_ns 周期值（纳秒）
 * @return true=成功, false=失败
 * 
 * 重要说明：
 * 1. 修改周期时必须先禁用PWM（硬件要求）
 * 2. 修改完成后恢复之前的使能状态
 * 3. 周期决定了PWM频率：freq = 1,000,000,000 / period_ns
 * 
 * 示例：
 * - period_ns = 1000000 (1ms) -> 1KHz
 * - period_ns = 20000 (20μs) -> 50KHz
 */
bool DriverPWM::setPeriod(int period_ns)
{
    if (!m_exported) {
        return false;
    }
    
    // 保存当前使能状态
    bool wasEnabled = m_enabled;
    
    // 修改周期前必须先禁用PWM（硬件限制）
    if (wasEnabled) {
        setEnable(false);
    }
    
    // 写入新的周期值
    bool result = writeToFile("period", QString::number(period_ns));
    if (result) {
        m_period = period_ns;  // 更新缓存值
    }
    
    // 恢复之前的使能状态
    if (wasEnabled) {
        setEnable(true);
    }
    
    return result;
}

bool DriverPWM::setDutyCycle(int duty_ns)
{
    if (!m_exported) {
        return false;
    }
    
    if (duty_ns > m_period) {
        qWarning() << "Duty cycle cannot exceed period";
        return false;
    }
    
    bool result = writeToFile("duty_cycle", QString::number(duty_ns));
    if (result) {
        m_dutyCycle = duty_ns;
    }
    
    return result;
}

bool DriverPWM::setDutyCyclePercent(float percent)
{
    if (percent < 0.0f || percent > 100.0f) {
        return false;
    }
    
    int duty_ns = (int)(m_period * percent / 100.0f);
    return setDutyCycle(duty_ns);
}

bool DriverPWM::setPolarity(bool inverted)
{
    if (!m_exported) {
        return false;
    }
    
    QString polarity = inverted ? "inversed" : "normal";
    return writeToFile("polarity", polarity);
}

bool DriverPWM::setEnable(bool enable)
{
    if (!m_exported) {
        return false;
    }
    
    QString value = enable ? "1" : "0";
    if (writeToFile("enable", value)) {
        m_enabled = enable;
        emit stateChanged(enable);
        return true;
    }
    
    return false;
}

int DriverPWM::getPeriod()
{
    QString value = readFromFile("period");
    bool ok;
    int period = value.toInt(&ok);
    return ok ? period : 0;
}

int DriverPWM::getDutyCycle()
{
    QString value = readFromFile("duty_cycle");
    bool ok;
    int duty = value.toInt(&ok);
    return ok ? duty : 0;
}

float DriverPWM::getDutyCyclePercent()
{
    if (m_period == 0) {
        return 0.0f;
    }
    return (float)m_dutyCycle * 100.0f / (float)m_period;
}

bool DriverPWM::isEnabled()
{
    QString value = readFromFile("enable");
    return (value == "1");
}

bool DriverPWM::start()
{
    return setEnable(true);
}

bool DriverPWM::stop()
{
    return setEnable(false);
}

/**
 * @brief 设置PWM频率和占空比（便捷方法）
 * @param freq_hz 频率（Hz）
 * @param dutyCycle_percent 占空比百分比，默认50%
 * @return true=成功, false=失败
 * 
 * 这是一个便捷函数，通过指定频率自动计算周期：
 * period_ns = 1,000,000,000 / freq_hz
 * 
 * 示例：
 * - setFrequency(1000, 50.0)  设置1KHz，50%占空比
 * - setFrequency(20000, 25.0) 设置20KHz，25%占空比
 */
bool DriverPWM::setFrequency(int freq_hz, float dutyCycle_percent)
{
    if (freq_hz <= 0) {
        return false;
    }
    
    // 根据频率计算周期（纳秒）
    // 频率1Hz = 周期1秒 = 1,000,000,000纳秒
    int period_ns = 1000000000 / freq_hz;
    
    // 设置周期
    if (!setPeriod(period_ns)) {
        return false;
    }
    
    // 设置占空比
    return setDutyCyclePercent(dutyCycle_percent);
}

