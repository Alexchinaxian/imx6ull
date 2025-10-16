/***************************************************************
 * Copyright: Alex
 * FileName: HardwareMapper.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 硬件设备映射管理器头文件
 *
 * 功能说明:
 *   1. 根据hardware.init配置创建实际的驱动对象
 *   2. 管理设备别名到驱动实例的映射
 *   3. 提供通过别名访问驱动对象的接口
 *   4. 自动初始化和启动配置的硬件设备
 *
 * 使用示例:
 *   HardwareMapper *mapper = HardwareMapper::getInstance();
 *   mapper->initializeFromConfig("hardware.init");
 *   
 *   // 通过别名获取PWM驱动
 *   DriverPWM *fan = mapper->getPWM("风扇");
 *   fan->start();
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef HARDWAREMAPPER_H
#define HARDWAREMAPPER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>
#include "core/HardwareConfig.h"

// 前向声明
class DriverPWM;
class DriverGPIO;
class DriverLED;
class DriverSerial;
class DriverBeep;
class DriverTemperature;

/**
 * @brief 硬件设备映射管理器类
 * 
 * 功能说明:
 *   1. 从HardwareConfig加载配置
 *   2. 根据配置创建驱动实例
 *   3. 维护别名到驱动实例的映射
 *   4. 提供通过别名访问驱动的接口
 */
class HardwareMapper : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 获取单例实例
     * @return HardwareMapper* 单例指针
     */
    static HardwareMapper* getInstance();
    
    /**
     * @brief 析构函数
     */
    ~HardwareMapper();
    
    /**
     * @brief 从配置文件初始化所有硬件设备
     * @param configFile 配置文件路径
     * @return true=成功, false=失败
     */
    bool initializeFromConfig(const QString &configFile);
    
    /**
     * @brief 通过别名获取PWM驱动
     * @param name 设备别名（如"风扇"）
     * @return DriverPWM* 驱动指针，不存在返回nullptr
     */
    DriverPWM* getPWM(const QString &name);
    
    /**
     * @brief 通过别名获取GPIO驱动
     * @param name 设备别名（如"继电器1"）
     * @return DriverGPIO* 驱动指针，不存在返回nullptr
     */
    DriverGPIO* getGPIO(const QString &name);
    
    /**
     * @brief 通过别名获取LED驱动
     * @param name 设备别名（如"系统指示灯"）
     * @return DriverLED* 驱动指针，不存在返回nullptr
     */
    DriverLED* getLED(const QString &name);
    
    /**
     * @brief 通过别名获取串口驱动
     * @param name 设备别名（如"Modbus串口"）
     * @return DriverSerial* 驱动指针，不存在返回nullptr
     */
    DriverSerial* getSerial(const QString &name);
    
    /**
     * @brief 通过别名获取蜂鸣器驱动
     * @param name 设备别名（如"系统蜂鸣器"）
     * @return DriverBeep* 驱动指针，不存在返回nullptr
     */
    DriverBeep* getBeep(const QString &name);
    
    /**
     * @brief 通过别名获取温度驱动
     * @param name 设备别名（如"CPU温度"）
     * @return DriverTemperature* 驱动指针，不存在返回nullptr
     */
    DriverTemperature* getTemperature(const QString &name);
    
    /**
     * @brief 获取所有PWM驱动的别名列表
     * @return QStringList 别名列表
     */
    QStringList getPWMNames() const;
    
    /**
     * @brief 获取所有GPIO驱动的别名列表
     * @return QStringList 别名列表
     */
    QStringList getGPIONames() const;
    
    /**
     * @brief 获取所有LED驱动的别名列表
     * @return QStringList 别名列表
     */
    QStringList getLEDNames() const;
    
    /**
     * @brief 获取所有串口驱动的别名列表
     * @return QStringList 别名列表
     */
    QStringList getSerialNames() const;
    
    /**
     * @brief 打印映射报告
     */
    void printReport() const;
    
    /**
     * @brief 停止所有硬件设备
     */
    void stopAll();
    
signals:
    /**
     * @brief 初始化完成信号
     * @param success 是否成功
     */
    void initialized(bool success);
    
    /**
     * @brief 错误信号
     * @param error 错误信息
     */
    void error(const QString &error);
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    explicit HardwareMapper(QObject *parent = nullptr);
    
    /**
     * @brief 创建PWM驱动
     * @param config 设备配置
     * @return true=成功, false=失败
     */
    bool createPWMDriver(const HardwareDeviceConfig &config);
    
    /**
     * @brief 创建GPIO驱动
     * @param config 设备配置
     * @return true=成功, false=失败
     */
    bool createGPIODriver(const HardwareDeviceConfig &config);
    
    /**
     * @brief 创建LED驱动
     * @param config 设备配置
     * @return true=成功, false=失败
     */
    bool createLEDDriver(const HardwareDeviceConfig &config);
    
    /**
     * @brief 创建串口驱动
     * @param config 设备配置
     * @return true=成功, false=失败
     */
    bool createSerialDriver(const HardwareDeviceConfig &config);
    
    /**
     * @brief 创建蜂鸣器驱动
     * @param config 设备配置
     * @return true=成功, false=失败
     */
    bool createBeepDriver(const HardwareDeviceConfig &config);
    
    /**
     * @brief 创建温度驱动
     * @param config 设备配置
     * @return true=成功, false=失败
     */
    bool createTemperatureDriver(const HardwareDeviceConfig &config);
    
private:
    static HardwareMapper *m_instance;      // 单例实例
    static QMutex m_instanceMutex;          // 实例锁
    
    // 设备别名 -> 驱动实例映射表
    QMap<QString, DriverPWM*> m_pwmDrivers;
    QMap<QString, DriverGPIO*> m_gpioDrivers;
    QMap<QString, DriverLED*> m_ledDrivers;
    QMap<QString, DriverSerial*> m_serialDrivers;
    QMap<QString, DriverBeep*> m_beepDrivers;
    QMap<QString, DriverTemperature*> m_temperatureDrivers;
    
    mutable QMutex m_mutex;                 // 访问锁
};

#endif // HARDWAREMAPPER_H

