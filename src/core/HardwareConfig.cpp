/***************************************************************
 * Copyright: Alex
 * FileName: HardwareConfig.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 硬件配置管理器实现
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#include "core/HardwareConfig.h"
#include <QFile>
#include <QDebug>

// 初始化静态成员
HardwareConfig* HardwareConfig::m_instance = nullptr;
QMutex HardwareConfig::m_instanceMutex;

/**
 * @brief 构造函数
 */
HardwareConfig::HardwareConfig(QObject *parent)
    : QObject(parent)
{
    qInfo() << "[HardwareConfig] 硬件配置管理器创建";
}

/**
 * @brief 析构函数
 */
HardwareConfig::~HardwareConfig()
{
    qInfo() << "[HardwareConfig] 硬件配置管理器销毁";
}

/**
 * @brief 获取单例实例
 */
HardwareConfig* HardwareConfig::getInstance()
{
    if (m_instance == nullptr)
    {
        QMutexLocker locker(&m_instanceMutex);
        if (m_instance == nullptr)
        {
            m_instance = new HardwareConfig();
        }
    }
    
    return m_instance;
}

/**
 * @brief 加载配置文件
 */
bool HardwareConfig::loadConfig(const QString &configFile)
{
    QMutexLocker locker(&m_mutex);
    
    qInfo() << "[HardwareConfig] 加载配置文件:" << configFile;
    
    // 检查文件是否存在
    if (!QFile::exists(configFile))
    {
        QString errMsg = QString("配置文件不存在: %1").arg(configFile);
        qCritical() << "[HardwareConfig]" << errMsg;
        emit error(errMsg);
        emit configLoaded(false);
        return false;
    }
    
    // 清空现有配置
    m_devices.clear();
    m_configFile = configFile;
    
    // 使用QSettings读取INI格式配置
    QSettings settings(configFile, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");  // 支持中文
    
    // 获取所有配置节
    QStringList groups = settings.childGroups();
    
    qInfo() << "[HardwareConfig] 找到" << groups.size() << "个配置节";
    
    // 解析每个配置节
    int successCount = 0;
    int failedCount = 0;
    
    for (const QString &group : groups)
    {
        settings.beginGroup(group);
        
        // 解析设备配置
        HardwareDeviceConfig deviceConfig = parseDeviceConfig(group, &settings);
        
        if (deviceConfig.type != HardwareType::Unknown)
        {
            // 添加到设备映射表
            m_devices[deviceConfig.name] = deviceConfig;
            successCount++;
            
            qDebug() << "  ✓" << hardwareTypeToString(deviceConfig.type) 
                     << "-" << deviceConfig.name
                     << (deviceConfig.enabled ? "[启用]" : "[禁用]");
        }
        else
        {
            failedCount++;
            qWarning() << "  ✗ 解析失败:" << group;
        }
        
        settings.endGroup();
    }
    
    qInfo() << "[HardwareConfig] 配置加载完成 - 成功:" << successCount 
            << "失败:" << failedCount;
    
    emit configLoaded(true);
    return true;
}

/**
 * @brief 解析单个设备配置
 */
HardwareDeviceConfig HardwareConfig::parseDeviceConfig(const QString &section, QSettings *settings)
{
    HardwareDeviceConfig config;
    
    config.section = section;
    
    // 读取基本信息
    QString typeStr = settings->value("type", "Unknown").toString();
    config.type = stringToHardwareType(typeStr);
    config.name = settings->value("name", "").toString();
    config.enabled = settings->value("enabled", false).toBool();
    config.description = settings->value("description", "").toString();
    
    // 检查必需字段
    if (config.name.isEmpty() || config.type == HardwareType::Unknown)
    {
        qWarning() << "[HardwareConfig] 配置节缺少必需字段:" << section;
        config.type = HardwareType::Unknown;
        return config;
    }
    
    // 根据设备类型读取特定参数
    switch (config.type)
    {
        case HardwareType::PWM:
            config.params["chip"] = settings->value("chip", 0).toInt();
            config.params["channel"] = settings->value("channel", 0).toInt();
            config.params["frequency"] = settings->value("frequency", 1000).toInt();
            config.params["duty_cycle"] = settings->value("duty_cycle", 50.0).toDouble();
            break;
            
        case HardwareType::GPIO:
            config.params["gpio_num"] = settings->value("gpio_num", 0).toInt();
            config.params["direction"] = settings->value("direction", "out").toString();
            config.params["initial_value"] = settings->value("initial_value", 0).toInt();
            config.params["edge"] = settings->value("edge", "none").toString();
            break;
            
        case HardwareType::LED:
            config.params["device"] = settings->value("device", "").toString();
            config.params["brightness"] = settings->value("brightness", 255).toInt();
            config.params["trigger"] = settings->value("trigger", "none").toString();
            break;
            
        case HardwareType::Serial:
            config.params["device"] = settings->value("device", "").toString();
            config.params["baudrate"] = settings->value("baudrate", 115200).toInt();
            config.params["databits"] = settings->value("databits", 8).toInt();
            config.params["parity"] = settings->value("parity", "N").toString();
            config.params["stopbits"] = settings->value("stopbits", 1).toInt();
            break;
            
        case HardwareType::CAN:
            config.params["device"] = settings->value("device", "").toString();
            config.params["bitrate"] = settings->value("bitrate", 500000).toInt();
            break;
            
        case HardwareType::I2C:
            config.params["bus"] = settings->value("bus", 0).toInt();
            config.params["address"] = settings->value("address", "0x00").toString();
            break;
            
        case HardwareType::SPI:
            config.params["bus"] = settings->value("bus", 0).toInt();
            config.params["cs"] = settings->value("cs", 0).toInt();
            config.params["speed"] = settings->value("speed", 1000000).toInt();
            break;
            
        case HardwareType::Beep:
            config.params["gpio_num"] = settings->value("gpio_num", 0).toInt();
            break;
            
        case HardwareType::Temperature:
            config.params["device"] = settings->value("device", "").toString();
            config.params["poll_interval"] = settings->value("poll_interval", 1000).toInt();
            config.params["high_threshold"] = settings->value("high_threshold", 85.0).toDouble();
            break;
            
        default:
            break;
    }
    
    return config;
}

/**
 * @brief 字符串转硬件类型
 */
HardwareType HardwareConfig::stringToHardwareType(const QString &typeStr) const
{
    static QMap<QString, HardwareType> typeMap = {
        {"PWM", HardwareType::PWM},
        {"GPIO", HardwareType::GPIO},
        {"LED", HardwareType::LED},
        {"Serial", HardwareType::Serial},
        {"CAN", HardwareType::CAN},
        {"I2C", HardwareType::I2C},
        {"SPI", HardwareType::SPI},
        {"Beep", HardwareType::Beep},
        {"Temperature", HardwareType::Temperature}
    };
    
    return typeMap.value(typeStr, HardwareType::Unknown);
}

/**
 * @brief 硬件类型转字符串
 */
QString HardwareConfig::hardwareTypeToString(HardwareType type) const
{
    static QMap<HardwareType, QString> typeMap = {
        {HardwareType::PWM, "PWM"},
        {HardwareType::GPIO, "GPIO"},
        {HardwareType::LED, "LED"},
        {HardwareType::Serial, "Serial"},
        {HardwareType::CAN, "CAN"},
        {HardwareType::I2C, "I2C"},
        {HardwareType::SPI, "SPI"},
        {HardwareType::Beep, "Beep"},
        {HardwareType::Temperature, "Temperature"}
    };
    
    return typeMap.value(type, "Unknown");
}

/**
 * @brief 通过设备别名获取配置
 */
HardwareDeviceConfig HardwareConfig::getDeviceByName(const QString &name) const
{
    QMutexLocker locker(&m_mutex);
    return m_devices.value(name, HardwareDeviceConfig());
}

/**
 * @brief 获取指定类型的所有设备
 */
QList<HardwareDeviceConfig> HardwareConfig::getDevicesByType(HardwareType type) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<HardwareDeviceConfig> result;
    
    for (const HardwareDeviceConfig &config : m_devices.values())
    {
        if (config.type == type)
        {
            result.append(config);
        }
    }
    
    return result;
}

/**
 * @brief 获取所有已启用的设备
 */
QList<HardwareDeviceConfig> HardwareConfig::getEnabledDevices() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<HardwareDeviceConfig> result;
    
    for (const HardwareDeviceConfig &config : m_devices.values())
    {
        if (config.enabled)
        {
            result.append(config);
        }
    }
    
    return result;
}

/**
 * @brief 检查设备是否存在
 */
bool HardwareConfig::hasDevice(const QString &name) const
{
    QMutexLocker locker(&m_mutex);
    return m_devices.contains(name);
}

/**
 * @brief 获取所有设备名称列表
 */
QStringList HardwareConfig::getAllDeviceNames() const
{
    QMutexLocker locker(&m_mutex);
    return m_devices.keys();
}

/**
 * @brief 打印配置报告
 */
void HardwareConfig::printReport() const
{
    QMutexLocker locker(&m_mutex);
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  Hardware Configuration Report";
    qInfo() << "========================================";
    qInfo() << "配置文件:" << m_configFile;
    qInfo() << "总设备数:" << m_devices.size();
    
    // 统计各类型设备数量
    QMap<HardwareType, int> typeCount;
    QMap<HardwareType, int> enabledCount;
    
    for (const HardwareDeviceConfig &config : m_devices.values())
    {
        typeCount[config.type]++;
        if (config.enabled)
        {
            enabledCount[config.type]++;
        }
    }
    
    // 打印各类型统计
    qInfo() << "";
    qInfo() << "设备类型统计:";
    
    for (auto it = typeCount.begin(); it != typeCount.end(); ++it)
    {
        QString typeName = hardwareTypeToString(it.key());
        int total = it.value();
        int enabled = enabledCount.value(it.key(), 0);
        
        qInfo() << QString("  • %1: %2 个（已启用 %3 个）")
                   .arg(typeName, -12)
                   .arg(total)
                   .arg(enabled);
    }
    
    // 打印已启用的设备详情
    qInfo() << "";
    qInfo() << "已启用设备详情:";
    
    for (const HardwareDeviceConfig &config : m_devices.values())
    {
        if (config.enabled)
        {
            QString typeName = hardwareTypeToString(config.type);
            qInfo() << QString("  ✓ [%1] %2 - %3")
                       .arg(typeName, -12)
                       .arg(config.name, -12)
                       .arg(config.description);
        }
    }
    
    qInfo() << "========================================";
    qInfo() << "";
}

