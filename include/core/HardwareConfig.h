/***************************************************************
 * Copyright: Alex
 * FileName: HardwareConfig.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 硬件配置管理器头文件
 *
 * 功能说明:
 *   1. 从hardware.init文件加载硬件配置
 *   2. 管理硬件设备的别名映射
 *   3. 提供硬件设备的配置信息查询
 *   4. 支持通过别名访问硬件设备
 *
 * 支持的硬件类型:
 *   - PWM (脉宽调制)
 *   - GPIO (通用输入输出)
 *   - LED (发光二极管)
 *   - Serial (串口)
 *   - CAN (CAN总线)
 *   - I2C (I2C总线)
 *   - SPI (SPI总线)
 *   - Beep (蜂鸣器)
 *   - Temperature (温度传感器)
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QSettings>
#include <QMutex>

/**
 * @brief 硬件设备类型枚举
 */
enum class HardwareType
{
    Unknown = 0,
    PWM,            // PWM设备
    GPIO,           // GPIO设备
    LED,            // LED设备
    Serial,         // 串口设备
    CAN,            // CAN总线
    I2C,            // I2C总线
    SPI,            // SPI总线
    Beep,           // 蜂鸣器
    Temperature     // 温度传感器
};

/**
 * @brief 硬件设备配置结构
 */
struct HardwareDeviceConfig
{
    HardwareType type;              // 设备类型
    QString name;                   // 设备别名（如"风扇"）
    QString section;                // 配置节名称（如"PWM/风扇"）
    bool enabled;                   // 是否启用
    QString description;            // 设备描述
    QMap<QString, QVariant> params; // 设备参数（如chip、channel等）
    
    // 构造函数
    HardwareDeviceConfig()
        : type(HardwareType::Unknown)
        , enabled(false)
    {
    }
};

/**
 * @brief 硬件配置管理器类
 * 
 * 功能说明:
 *   1. 加载hardware.init配置文件
 *   2. 解析硬件设备配置
 *   3. 提供设备别名到配置的映射
 *   4. 支持设备配置的查询和访问
 * 
 * 使用示例:
 *   HardwareConfig *config = HardwareConfig::getInstance();
 *   if (config->loadConfig("hardware.init")) {
 *       auto fanConfig = config->getDeviceByName("风扇");
 *       int chip = fanConfig.params["chip"].toInt();
 *       int channel = fanConfig.params["channel"].toInt();
 *   }
 */
class HardwareConfig : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 获取单例实例
     * @return HardwareConfig* 单例指针
     */
    static HardwareConfig* getInstance();
    
    /**
     * @brief 析构函数
     */
    ~HardwareConfig();
    
    /**
     * @brief 加载配置文件
     * @param configFile 配置文件路径
     * @return true=成功, false=失败
     */
    bool loadConfig(const QString &configFile);
    
    /**
     * @brief 通过设备别名获取配置
     * @param name 设备别名（如"风扇"）
     * @return HardwareDeviceConfig 设备配置
     */
    HardwareDeviceConfig getDeviceByName(const QString &name) const;
    
    /**
     * @brief 获取指定类型的所有设备
     * @param type 设备类型
     * @return QList<HardwareDeviceConfig> 设备配置列表
     */
    QList<HardwareDeviceConfig> getDevicesByType(HardwareType type) const;
    
    /**
     * @brief 获取所有已启用的设备
     * @return QList<HardwareDeviceConfig> 设备配置列表
     */
    QList<HardwareDeviceConfig> getEnabledDevices() const;
    
    /**
     * @brief 检查设备是否存在
     * @param name 设备别名
     * @return true=存在, false=不存在
     */
    bool hasDevice(const QString &name) const;
    
    /**
     * @brief 获取所有设备名称列表
     * @return QStringList 设备名称列表
     */
    QStringList getAllDeviceNames() const;
    
    /**
     * @brief 打印配置报告
     */
    void printReport() const;
    
    /**
     * @brief 获取配置文件路径
     * @return QString 配置文件路径
     */
    QString getConfigFilePath() const { return m_configFile; }
    
signals:
    /**
     * @brief 配置加载完成信号
     * @param success 是否成功
     */
    void configLoaded(bool success);
    
    /**
     * @brief 错误信号
     * @param error 错误信息
     */
    void error(const QString &error);
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    explicit HardwareConfig(QObject *parent = nullptr);
    
    /**
     * @brief 解析单个设备配置
     * @param section 配置节名称
     * @param settings QSettings对象
     * @return HardwareDeviceConfig 设备配置
     */
    HardwareDeviceConfig parseDeviceConfig(const QString &section, QSettings *settings);
    
    /**
     * @brief 字符串转硬件类型
     * @param typeStr 类型字符串
     * @return HardwareType 硬件类型
     */
    HardwareType stringToHardwareType(const QString &typeStr) const;
    
    /**
     * @brief 硬件类型转字符串
     * @param type 硬件类型
     * @return QString 类型字符串
     */
    QString hardwareTypeToString(HardwareType type) const;
    
private:
    static HardwareConfig *m_instance;      // 单例实例
    static QMutex m_instanceMutex;          // 实例锁
    
    QString m_configFile;                   // 配置文件路径
    QMap<QString, HardwareDeviceConfig> m_devices;  // 设备名称 -> 设备配置
    mutable QMutex m_mutex;                 // 访问锁
};

#endif // HARDWARECONFIG_H

