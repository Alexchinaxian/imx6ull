#ifndef IMX6ULL_DRIVERS_MANAGER_H
#define IMX6ULL_DRIVERS_MANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

// 前置声明
class DriverGPIO;
class DriverLED;
class DriverPWM;
class DriverSerial;
class DriverCAN;
class SystemScanner;

/***************************************************************
 * 类名: DriverManager
 * 功能: 驱动管理器类，负责统一管理所有硬件驱动
 * 说明: 采用单例模式设计，确保全局只有一个驱动管理器实例
 *       这是项目的核心管理类，提供驱动的加载、卸载、配置等功能
 * 作者: Alex
 * 日期: 2025
 ***************************************************************/

class DriverManager : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 析构函数
     */
    ~DriverManager();
    
    /**
     * @brief 获取驱动管理器的单例实例
     * @return DriverManager& 返回驱动管理器的引用
     * @note 使用单例模式，确保全局只有一个实例
     */
    static DriverManager& getInstance();
    
    // ========== LED驱动管理 ==========
    
    /**
     * @brief 创建或获取LED驱动
     * @param ledName LED名称（例如："sys-led"）
     * @return DriverLED指针，失败返回nullptr
     */
    DriverLED* getLED(const QString &ledName);
    
    /**
     * @brief 释放LED驱动
     * @param ledName LED名称
     */
    void releaseLED(const QString &ledName);
    
    // ========== GPIO驱动管理 ==========
    
    /**
     * @brief 创建或获取GPIO驱动
     * @param gpioNum GPIO编号
     * @return DriverGPIO指针，失败返回nullptr
     */
    DriverGPIO* getGPIO(int gpioNum);
    
    /**
     * @brief 释放GPIO驱动
     * @param gpioNum GPIO编号
     */
    void releaseGPIO(int gpioNum);
    
    // ========== PWM驱动管理 ==========
    
    /**
     * @brief 创建或获取PWM驱动
     * @param chipNum PWM芯片编号
     * @param channelNum PWM通道编号
     * @return DriverPWM指针，失败返回nullptr
     */
    DriverPWM* getPWM(int chipNum, int channelNum);
    
    /**
     * @brief 释放PWM驱动
     * @param chipNum PWM芯片编号
     * @param channelNum PWM通道编号
     */
    void releasePWM(int chipNum, int channelNum);
    
    // ========== 串口驱动管理 ==========
    
    /**
     * @brief 创建或获取串口驱动
     * @param portName 串口名称（例如："/dev/ttyS1"）
     * @return DriverSerial指针，失败返回nullptr
     */
    DriverSerial* getSerial(const QString &portName);
    
    /**
     * @brief 释放串口驱动
     * @param portName 串口名称
     */
    void releaseSerial(const QString &portName);
    
    // ========== CAN驱动管理 ==========
    
    /**
     * @brief 创建或获取CAN驱动
     * @param interfaceName CAN接口名称（例如："can0"）
     * @return DriverCAN指针，失败返回nullptr
     */
    DriverCAN* getCAN(const QString &interfaceName);
    
    /**
     * @brief 释放CAN驱动
     * @param interfaceName CAN接口名称
     */
    void releaseCAN(const QString &interfaceName);
    
    // ========== 系统扫描器 ==========
    
    /**
     * @brief 获取系统扫描器
     * @return SystemScanner指针
     */
    SystemScanner* getSystemScanner();
    
    // ========== 配置文件管理 ==========
    
    /**
     * @brief 从配置文件加载硬件配置
     * @param configFile 配置文件路径（如 "hardware.init"）
     * @return true=成功, false=失败
     */
    bool loadFromConfig(const QString &configFile);
    
    /**
     * @brief 通过别名获取PWM驱动
     * @param alias 设备别名（如 "风扇"）
     * @return DriverPWM指针，不存在返回nullptr
     */
    DriverPWM* getPWMByAlias(const QString &alias);
    
    /**
     * @brief 通过别名获取GPIO驱动
     * @param alias 设备别名（如 "继电器1"）
     * @return DriverGPIO指针，不存在返回nullptr
     */
    DriverGPIO* getGPIOByAlias(const QString &alias);
    
    /**
     * @brief 通过别名获取LED驱动
     * @param alias 设备别名（如 "系统指示灯"）
     * @return DriverLED指针，不存在返回nullptr
     */
    DriverLED* getLEDByAlias(const QString &alias);
    
    /**
     * @brief 通过别名获取串口驱动
     * @param alias 设备别名（如 "Modbus串口"）
     * @return DriverSerial指针，不存在返回nullptr
     */
    DriverSerial* getSerialByAlias(const QString &alias);
    
    /**
     * @brief 获取所有已配置的设备别名
     * @return 别名列表
     */
    QStringList getAllAliases() const;
    
    /**
     * @brief 打印配置报告
     */
    void printConfigReport() const;
    
    // ========== 资源管理 ==========
    
    /**
     * @brief 释放所有驱动资源
     */
    void releaseAll();
    
    /**
     * @brief 获取当前加载的驱动数量
     * @return 驱动数量
     */
    int getDriverCount() const;
    
    /**
     * @brief 打印所有已加载的驱动信息
     */
    void printDriverList() const;
    
signals:
    /**
     * @brief 驱动加载信号
     * @param driverType 驱动类型
     * @param driverName 驱动名称
     */
    void driverLoaded(const QString &driverType, const QString &driverName);
    
    /**
     * @brief 驱动卸载信号
     * @param driverType 驱动类型
     * @param driverName 驱动名称
     */
    void driverUnloaded(const QString &driverType, const QString &driverName);
    
private:
    /**
     * @brief 构造函数（私有，单例模式）
     * @param parent 父对象指针（Qt对象树管理）
     */
    explicit DriverManager(QObject *parent = nullptr);
    
    // 禁用拷贝构造和赋值操作
    DriverManager(const DriverManager&) = delete;
    DriverManager& operator=(const DriverManager&) = delete;
    
    // 驱动存储容器
    QMap<QString, DriverLED*> m_ledDrivers;           // LED驱动映射表
    QMap<int, DriverGPIO*> m_gpioDrivers;             // GPIO驱动映射表
    QMap<QString, DriverPWM*> m_pwmDrivers;           // PWM驱动映射表（key: "chip_channel"）
    QMap<QString, DriverSerial*> m_serialDrivers;     // 串口驱动映射表
    QMap<QString, DriverCAN*> m_canDrivers;           // CAN驱动映射表
    SystemScanner *m_systemScanner;                   // 系统扫描器
    
    // 别名映射表（别名 -> 驱动键）
    QMap<QString, QString> m_pwmAliases;              // PWM别名映射
    QMap<QString, int> m_gpioAliases;                 // GPIO别名映射
    QMap<QString, QString> m_ledAliases;              // LED别名映射
    QMap<QString, QString> m_serialAliases;           // 串口别名映射
    
    /**
     * @brief 生成PWM驱动的键名
     * @param chipNum 芯片编号
     * @param channelNum 通道编号
     * @return 键名字符串
     */
    QString makePWMKey(int chipNum, int channelNum) const;
};

#endif // DRIVER_MANAGER_H

