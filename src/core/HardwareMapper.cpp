/***************************************************************
 * Copyright: Alex
 * FileName: HardwareMapper.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 硬件设备映射管理器实现
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#include "core/HardwareMapper.h"
#include "drivers/pwm/DriverPWM.h"
#include "drivers/gpio/DriverGPIO.h"
#include "drivers/led/DriverLED.h"
#include "drivers/serial/DriverSerial.h"
#include "drivers/beep/DriverBeep.h"
#include "drivers/temperature/DriverTemperature.h"
#include <QDebug>
#include <QSerialPort>

// 初始化静态成员
HardwareMapper* HardwareMapper::m_instance = nullptr;
QMutex HardwareMapper::m_instanceMutex;

/**
 * @brief 构造函数
 */
HardwareMapper::HardwareMapper(QObject *parent)
    : QObject(parent)
{
    qInfo() << "[HardwareMapper] 硬件设备映射管理器创建";
}

/**
 * @brief 析构函数
 */
HardwareMapper::~HardwareMapper()
{
    qInfo() << "[HardwareMapper] 硬件设备映射管理器销毁";
    
    // 清理所有驱动对象
    stopAll();
    
    // 删除PWM驱动
    for (auto it = m_pwmDrivers.begin(); it != m_pwmDrivers.end(); ++it)
    {
        delete it.value();
    }
    m_pwmDrivers.clear();
    
    // 删除GPIO驱动
    for (auto it = m_gpioDrivers.begin(); it != m_gpioDrivers.end(); ++it)
    {
        delete it.value();
    }
    m_gpioDrivers.clear();
    
    // 删除LED驱动
    for (auto it = m_ledDrivers.begin(); it != m_ledDrivers.end(); ++it)
    {
        delete it.value();
    }
    m_ledDrivers.clear();
    
    // 删除串口驱动
    for (auto it = m_serialDrivers.begin(); it != m_serialDrivers.end(); ++it)
    {
        delete it.value();
    }
    m_serialDrivers.clear();
    
    // 删除蜂鸣器驱动
    for (auto it = m_beepDrivers.begin(); it != m_beepDrivers.end(); ++it)
    {
        delete it.value();
    }
    m_beepDrivers.clear();
    
    // 删除温度驱动
    for (auto it = m_temperatureDrivers.begin(); it != m_temperatureDrivers.end(); ++it)
    {
        delete it.value();
    }
    m_temperatureDrivers.clear();
}

/**
 * @brief 获取单例实例
 */
HardwareMapper* HardwareMapper::getInstance()
{
    if (m_instance == nullptr)
    {
        QMutexLocker locker(&m_instanceMutex);
        if (m_instance == nullptr)
        {
            m_instance = new HardwareMapper();
        }
    }
    
    return m_instance;
}

/**
 * @brief 从配置文件初始化所有硬件设备
 */
bool HardwareMapper::initializeFromConfig(const QString &configFile)
{
    qInfo() << "[HardwareMapper] 从配置文件初始化硬件设备:" << configFile;
    
    // 加载配置
    HardwareConfig *config = HardwareConfig::getInstance();
    if (!config->loadConfig(configFile))
    {
        qCritical() << "[HardwareMapper] 加载配置文件失败";
        emit initialized(false);
        return false;
    }
    
    // 打印配置报告
    config->printReport();
    
    // 获取所有已启用的设备
    QList<HardwareDeviceConfig> enabledDevices = config->getEnabledDevices();
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  Hardware Device Initialization";
    qInfo() << "========================================";
    qInfo() << "准备初始化" << enabledDevices.size() << "个已启用的硬件设备";
    qInfo() << "";
    
    int successCount = 0;
    int failedCount = 0;
    
    // 遍历所有已启用的设备，创建相应的驱动
    for (const HardwareDeviceConfig &deviceConfig : enabledDevices)
    {
        bool success = false;
        
        switch (deviceConfig.type)
        {
            case HardwareType::PWM:
                success = createPWMDriver(deviceConfig);
                break;
                
            case HardwareType::GPIO:
                success = createGPIODriver(deviceConfig);
                break;
                
            case HardwareType::LED:
                success = createLEDDriver(deviceConfig);
                break;
                
            case HardwareType::Serial:
                success = createSerialDriver(deviceConfig);
                break;
                
            case HardwareType::Beep:
                success = createBeepDriver(deviceConfig);
                break;
                
            case HardwareType::Temperature:
                success = createTemperatureDriver(deviceConfig);
                break;
                
            default:
                qWarning() << "  ✗ 不支持的设备类型:" << deviceConfig.name;
                failedCount++;
                continue;
        }
        
        if (success)
        {
            successCount++;
        }
        else
        {
            failedCount++;
        }
    }
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "硬件设备初始化完成";
    qInfo() << "成功:" << successCount << "失败:" << failedCount;
    qInfo() << "========================================";
    qInfo() << "";
    
    emit initialized(true);
    return true;
}

/**
 * @brief 创建PWM驱动
 */
bool HardwareMapper::createPWMDriver(const HardwareDeviceConfig &config)
{
    int chip = config.params.value("chip", 0).toInt();
    int channel = config.params.value("channel", 0).toInt();
    int frequency = config.params.value("frequency", 1000).toInt();
    double dutyCycle = config.params.value("duty_cycle", 50.0).toDouble();
    
    qInfo() << QString("  ✓ [PWM] 创建驱动: %1 (chip=%2, channel=%3, freq=%4Hz, duty=%5%)")
               .arg(config.name, -12)
               .arg(chip)
               .arg(channel)
               .arg(frequency)
               .arg(dutyCycle);
    
    // 创建PWM驱动实例
    DriverPWM *driver = new DriverPWM(chip, channel, this);
    
    // 导出PWM
    if (!driver->exportPWM())
    {
        qWarning() << "    ⚠ PWM导出失败，但驱动已创建（可能已被其他程序导出）";
    }
    
    // 设置频率和占空比
    if (!driver->setFrequency(frequency, dutyCycle))
    {
        qWarning() << "    ⚠ 设置频率/占空比失败";
    }
    
    // 添加到映射表
    m_pwmDrivers[config.name] = driver;
    
    return true;
}

/**
 * @brief 创建GPIO驱动
 */
bool HardwareMapper::createGPIODriver(const HardwareDeviceConfig &config)
{
    int gpioNum = config.params.value("gpio_num", 0).toInt();
    QString direction = config.params.value("direction", "out").toString();
    int initialValue = config.params.value("initial_value", 0).toInt();
    
    qInfo() << QString("  ✓ [GPIO] 创建驱动: %1 (gpio=%2, dir=%3, init=%4)")
               .arg(config.name, -12)
               .arg(gpioNum)
               .arg(direction)
               .arg(initialValue);
    
    // 创建GPIO驱动实例
    DriverGPIO *driver = new DriverGPIO(gpioNum, this);
    
    // 导出GPIO
    if (!driver->exportGPIO())
    {
        qWarning() << "    ⚠ GPIO导出失败";
    }
    
    // 设置方向
    DriverGPIO::Direction dir = (direction == "in") ? DriverGPIO::Input : DriverGPIO::Output;
    if (!driver->setDirection(dir))
    {
        qWarning() << "    ⚠ 设置GPIO方向失败";
    }
    
    // 如果是输出模式，设置初始值
    if (dir == DriverGPIO::Output)
    {
        DriverGPIO::Value val = (initialValue == 0) ? DriverGPIO::Low : DriverGPIO::High;
        driver->setValue(val);
    }
    
    // 添加到映射表
    m_gpioDrivers[config.name] = driver;
    
    return true;
}

/**
 * @brief 创建LED驱动
 */
bool HardwareMapper::createLEDDriver(const HardwareDeviceConfig &config)
{
    QString device = config.params.value("device", "").toString();
    int brightness = config.params.value("brightness", 255).toInt();
    
    qInfo() << QString("  ✓ [LED] 创建驱动: %1 (device=%2, brightness=%3)")
               .arg(config.name, -12)
               .arg(device)
               .arg(brightness);
    
    // 创建LED驱动实例
    DriverLED *driver = new DriverLED(device, this);
    
    // 设置亮度
    if (!driver->setBrightness(brightness))
    {
        qWarning() << "    ⚠ 设置LED亮度失败";
    }
    
    // 添加到映射表
    m_ledDrivers[config.name] = driver;
    
    return true;
}

/**
 * @brief 创建串口驱动
 */
bool HardwareMapper::createSerialDriver(const HardwareDeviceConfig &config)
{
    QString device = config.params.value("device", "").toString();
    int baudrate = config.params.value("baudrate", 115200).toInt();
    
    qInfo() << QString("  ✓ [Serial] 创建驱动: %1 (device=%2, baudrate=%3)")
               .arg(config.name, -12)
               .arg(device)
               .arg(baudrate);
    
    // 创建串口驱动实例
    DriverSerial *driver = new DriverSerial(device, this);
    
    // 配置串口（但不打开，由应用层根据需要打开）
    driver->configure(baudrate);
    
    // 添加到映射表
    m_serialDrivers[config.name] = driver;
    
    return true;
}

/**
 * @brief 创建蜂鸣器驱动
 */
bool HardwareMapper::createBeepDriver(const HardwareDeviceConfig &config)
{
    QString beepName = config.params.value("device", "beep").toString();
    
    qInfo() << QString("  ✓ [Beep] 创建驱动: %1 (device=%2)")
               .arg(config.name, -12)
               .arg(beepName);
    
    // 创建蜂鸣器驱动实例（使用LED子系统名称）
    DriverBeep *driver = new DriverBeep(beepName, this);
    
    // 添加到映射表
    m_beepDrivers[config.name] = driver;
    
    return true;
}

/**
 * @brief 创建温度驱动
 */
bool HardwareMapper::createTemperatureDriver(const HardwareDeviceConfig &config)
{
    qInfo() << QString("  ✓ [Temperature] %1 - 由TemperatureService管理，跳过")
               .arg(config.name, -12);
    
    // 温度驱动由TemperatureService统一管理，这里不重复创建
    // 如果需要独立的温度驱动实例，可以在这里创建
    
    return true;
}

/**
 * @brief 获取PWM驱动
 */
DriverPWM* HardwareMapper::getPWM(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    return m_pwmDrivers.value(name, nullptr);
}

/**
 * @brief 获取GPIO驱动
 */
DriverGPIO* HardwareMapper::getGPIO(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    return m_gpioDrivers.value(name, nullptr);
}

/**
 * @brief 获取LED驱动
 */
DriverLED* HardwareMapper::getLED(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    return m_ledDrivers.value(name, nullptr);
}

/**
 * @brief 获取串口驱动
 */
DriverSerial* HardwareMapper::getSerial(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    return m_serialDrivers.value(name, nullptr);
}

/**
 * @brief 获取蜂鸣器驱动
 */
DriverBeep* HardwareMapper::getBeep(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    return m_beepDrivers.value(name, nullptr);
}

/**
 * @brief 获取温度驱动
 */
DriverTemperature* HardwareMapper::getTemperature(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    return m_temperatureDrivers.value(name, nullptr);
}

/**
 * @brief 获取PWM驱动别名列表
 */
QStringList HardwareMapper::getPWMNames() const
{
    QMutexLocker locker(&m_mutex);
    return m_pwmDrivers.keys();
}

/**
 * @brief 获取GPIO驱动别名列表
 */
QStringList HardwareMapper::getGPIONames() const
{
    QMutexLocker locker(&m_mutex);
    return m_gpioDrivers.keys();
}

/**
 * @brief 获取LED驱动别名列表
 */
QStringList HardwareMapper::getLEDNames() const
{
    QMutexLocker locker(&m_mutex);
    return m_ledDrivers.keys();
}

/**
 * @brief 获取串口驱动别名列表
 */
QStringList HardwareMapper::getSerialNames() const
{
    QMutexLocker locker(&m_mutex);
    return m_serialDrivers.keys();
}

/**
 * @brief 打印映射报告
 */
void HardwareMapper::printReport() const
{
    QMutexLocker locker(&m_mutex);
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  Hardware Device Mapper Report";
    qInfo() << "========================================";
    qInfo() << "PWM设备数量:" << m_pwmDrivers.size();
    for (const QString &name : m_pwmDrivers.keys())
    {
        qInfo() << "  • PWM:" << name;
    }
    
    qInfo() << "";
    qInfo() << "GPIO设备数量:" << m_gpioDrivers.size();
    for (const QString &name : m_gpioDrivers.keys())
    {
        qInfo() << "  • GPIO:" << name;
    }
    
    qInfo() << "";
    qInfo() << "LED设备数量:" << m_ledDrivers.size();
    for (const QString &name : m_ledDrivers.keys())
    {
        qInfo() << "  • LED:" << name;
    }
    
    qInfo() << "";
    qInfo() << "串口设备数量:" << m_serialDrivers.size();
    for (const QString &name : m_serialDrivers.keys())
    {
        qInfo() << "  • Serial:" << name;
    }
    
    qInfo() << "";
    qInfo() << "蜂鸣器设备数量:" << m_beepDrivers.size();
    for (const QString &name : m_beepDrivers.keys())
    {
        qInfo() << "  • Beep:" << name;
    }
    
    qInfo() << "";
    qInfo() << "温度设备数量:" << m_temperatureDrivers.size();
    for (const QString &name : m_temperatureDrivers.keys())
    {
        qInfo() << "  • Temperature:" << name;
    }
    
    qInfo() << "========================================";
    qInfo() << "";
}

/**
 * @brief 停止所有硬件设备
 */
void HardwareMapper::stopAll()
{
    qInfo() << "[HardwareMapper] 停止所有硬件设备";
    
    // 停止所有PWM
    for (auto it = m_pwmDrivers.begin(); it != m_pwmDrivers.end(); ++it)
    {
        if (it.value())
        {
            it.value()->stop();
            it.value()->unexportPWM();
        }
    }
    
    // 关闭所有LED
    for (auto it = m_ledDrivers.begin(); it != m_ledDrivers.end(); ++it)
    {
        if (it.value())
        {
            it.value()->turnOff();
        }
    }
    
    // 关闭所有串口
    for (auto it = m_serialDrivers.begin(); it != m_serialDrivers.end(); ++it)
    {
        if (it.value() && it.value()->isOpen())
        {
            it.value()->close();
        }
    }
    
    // Unexport所有GPIO
    for (auto it = m_gpioDrivers.begin(); it != m_gpioDrivers.end(); ++it)
    {
        if (it.value())
        {
            it.value()->unexportGPIO();
        }
    }
}

