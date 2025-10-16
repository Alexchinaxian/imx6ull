/***************************************************************
 * Copyright: Alex
 * FileName: ServiceManager.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 系统服务管理类，统一管理所有硬件驱动服务对象
 *
 * 功能说明:
 *   本类采用单例模式设计，负责创建、初始化、启动和管理所有
 *   系统服务。支持服务的注册、查找、生命周期管理等功能。
 *
 * 设计模式:
 *   - 单例模式: 确保全局只有一个服务管理器实例
 *   - 工厂模式: 统一创建和管理所有服务对象
 *   - 观察者模式: 服务间可以相互关联和通知
 *
 * 生命周期:
 *   1. ManagerInitLoad() - 创建所有服务对象
 *   2. SvrInit()         - 初始化所有服务
 *   3. SvrStart()        - 启动所有服务
 *
 * History:
 *   1. 2025-10-15 创建文件，适配IMX6ULL项目
 ***************************************************************/

#ifndef IMX6ULL_CORE_SERVICE_MANAGER_H
#define IMX6ULL_CORE_SERVICE_MANAGER_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QString>

// 前向声明
class ISysSvrInterface;
class DriverTemperature;
class DriverGPIO;
class DriverLED;
class DriverPWM;
class SystemScanner;
class DriverManager;
class ProtocolManager;
class TemperatureService;
class ModbusSlaveService;
class TimeService;
class WeatherService;
class AlarmService;

/***************************************************************
 * 枚举: Enum_SysSvrTypeDef
 * 功能: 定义所有系统服务的类型
 * 说明: 每个服务都有一个唯一的类型标识
 ***************************************************************/
enum Enum_SysSvrTypeDef {
    SYS_SVR_TYPE_DRIVER_MANAGER = 0,    // 驱动管理器服务
    SYS_SVR_TYPE_TEMPERATURE_SVR,       // 温度监控服务
    SYS_SVR_TYPE_MODBUS_SLAVE_SVR,      // Modbus从站服务
    SYS_SVR_TYPE_TIME_SVR,              // 时间服务
    SYS_SVR_TYPE_WEATHER_SVR,           // 天气服务
    SYS_SVR_TYPE_ALARM_SVR,             // 闹钟服务
    SYS_SVR_TYPE_GPIO_SVR,              // GPIO控制服务
    SYS_SVR_TYPE_LED_SVR,               // LED控制服务
    SYS_SVR_TYPE_PWM_SVR,               // PWM控制服务
    SYS_SVR_TYPE_SCANNER_SVR,           // 系统扫描服务
    SYS_SVR_TYPE_PROTOCOL_MANAGER,      // 协议管理器服务
    SYS_SVR_TYPE_LOGIC_SVR,             // 业务逻辑服务（预留）
    SYS_SVR_TYPE_NETWORK_SVR,           // 网络通信服务（预留）
    SYS_SVR_TYPE_STORAGE_SVR,           // 数据存储服务（预留）
    SYS_SVR_TYPE_DEBUG_SVR,             // 调试服务（预留）
};

/***************************************************************
 * 枚举: Enum_SysSvrIdDef
 * 功能: 定义所有系统服务的ID
 * 说明: 每个服务都有一个唯一的ID标识，与类型配合使用
 ***************************************************************/
enum Enum_SysSvrIdDef {
    SYS_SVR_ID_DRIVER_MANAGER = 0,      // 驱动管理器ID
    SYS_SVR_ID_TEMPERATURE_SVR,         // 温度监控服务ID
    SYS_SVR_ID_MODBUS_SLAVE_SVR,        // Modbus从站服务ID
    SYS_SVR_ID_TIME_SVR,                // 时间服务ID
    SYS_SVR_ID_WEATHER_SVR,             // 天气服务ID
    SYS_SVR_ID_ALARM_SVR,               // 闹钟服务ID
    SYS_SVR_ID_GPIO_SVR,                // GPIO控制服务ID
    SYS_SVR_ID_LED_SVR,                 // LED控制服务ID
    SYS_SVR_ID_PWM_SVR,                 // PWM控制服务ID
    SYS_SVR_ID_SCANNER_SVR,             // 系统扫描服务ID
    SYS_SVR_ID_PROTOCOL_MANAGER,        // 协议管理器ID
    SYS_SVR_ID_LOGIC_SVR,               // 业务逻辑服务ID（预留）
    SYS_SVR_ID_NETWORK_SVR,             // 网络通信服务ID（预留）
    SYS_SVR_ID_STORAGE_SVR,             // 数据存储服务ID（预留）
    SYS_SVR_ID_DEBUG_SVR,               // 调试服务ID（预留）
};

/***************************************************************
 * 类名: ServiceManager
 * 功能: 系统服务管理器
 * 
 * 职责:
 *   1. 统一创建和管理所有系统服务对象
 *   2. 管理服务的生命周期（创建、初始化、启动）
 *   3. 提供服务查找和获取接口
 *   4. 处理服务间的依赖关系
 * 
 * 使用示例:
 *   // 1. 获取管理器实例
 *   ServiceManager *mgr = ServiceManager::GetInstance();
 *   
 *   // 2. 创建所有服务
 *   mgr->ManagerInitLoad();
 *   
 *   // 3. 初始化所有服务
 *   mgr->SvrInit();
 *   
 *   // 4. 启动所有服务
 *   mgr->SvrStart();
 *   
 *   // 5. 获取特定服务
 *   ISysSvrInterface *tempSvr = mgr->GetSvrObj(SYS_SVR_ID_TEMPERATURE_SVR);
 * 
 * 注意事项:
 *   - 线程安全: 使用QMutex保护关键操作
 *   - 生命周期: 服务对象由管理器负责销毁
 *   - 错误处理: 所有操作返回bool表示成功/失败
 ***************************************************************/
class ServiceManager : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 析构函数
     * @note 负责清理所有注册的服务对象
     */
    ~ServiceManager();
    
    /**
     * @brief 获取服务管理器的单例实例
     * @return ServiceManager* 返回服务管理器指针
     * @note 线程安全，使用双重检查锁定模式
     */
    static ServiceManager* GetInstance();
    
    // ========== 服务查找接口 ==========
    
    /**
     * @brief 根据服务ID和类型获取服务对象
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @return ISysSvrInterface* 返回服务接口指针，未找到返回NULL
     */
    ISysSvrInterface* GetSvrObj(int32_t svr_id, int32_t svr_type);
    
    /**
     * @brief 根据服务ID获取服务对象
     * @param svr_id 服务ID
     * @return ISysSvrInterface* 返回服务接口指针，未找到返回NULL
     */
    ISysSvrInterface* GetSvrObj(int32_t svr_id);
    
    /**
     * @brief 获取温度监控服务对象
     * @return DriverTemperature* 返回温度服务指针
     */
    DriverTemperature* GetTemperatureSvrObj();
    
    /**
     * @brief 获取GPIO控制服务对象
     * @return DriverGPIO* 返回GPIO服务指针
     */
    DriverGPIO* GetGPIOSvrObj();
    
    /**
     * @brief 获取LED控制服务对象
     * @return DriverLED* 返回LED服务指针
     */
    DriverLED* GetLEDSvrObj();
    
    /**
     * @brief 获取PWM控制服务对象
     * @return DriverPWM* 返回PWM服务指针
     */
    DriverPWM* GetPWMSvrObj();
    
    /**
     * @brief 获取系统扫描服务对象
     * @return SystemScanner* 返回扫描服务指针
     */
    SystemScanner* GetScannerSvrObj();
    
    /**
     * @brief 获取驱动管理器服务对象
     * @return DriverManager* 返回驱动管理器指针
     */
    DriverManager* GetDriverManagerSvrObj();
    
    /**
     * @brief 获取协议管理器对象
     * @return ProtocolManager* 返回协议管理器指针
     */
    ProtocolManager* GetProtocolManager();
    
    /**
     * @brief 获取Modbus从站服务对象
     * @return ModbusSlaveService* 返回Modbus从站服务指针
     */
    ModbusSlaveService* GetModbusSlaveSvrObj();
    
    /**
     * @brief 获取时间服务对象
     * @return TimeService* 返回时间服务指针
     */
    TimeService* GetTimeSvrObj();
    
    /**
     * @brief 获取天气服务对象
     * @return WeatherService* 返回天气服务指针
     */
    WeatherService* GetWeatherSvrObj();
    
    /**
     * @brief 获取闹钟服务对象
     * @return AlarmService* 返回闹钟服务指针
     */
    AlarmService* GetAlarmSvrObj();
    
    // ========== 生命周期管理接口 ==========
    
    /**
     * @brief 服务管理器加载初始化
     * @return true=成功, false=失败（已初始化）
     * @note 创建所有服务对象，但不初始化
     */
    bool ManagerInitLoad();
    
    /**
     * @brief 初始化所有服务
     * @return true=成功, false=失败
     * @note 调用所有服务的SvrInit()方法
     */
    bool SvrInit();
    
    /**
     * @brief 启动所有服务
     * @return true=成功, false=失败
     * @note 调用所有服务的SvrStart()方法
     */
    bool SvrStart();
    
    /**
     * @brief 停止所有服务
     * @return true=成功, false=失败
     * @note 调用所有服务的SvrStop()方法（如果实现）
     */
    bool SvrStop();
    
    /**
     * @brief 获取服务数量
     * @return int 返回已注册的服务数量
     */
    int GetServiceCount() const;
    
    /**
     * @brief 检查管理器是否已初始化
     * @return true=已初始化, false=未初始化
     */
    bool IsInitialized() const { return m_InitFlag; }
    
    /**
     * @brief 检查服务是否已启动
     * @return true=已启动, false=未启动
     */
    bool IsStarted() const { return m_SvrStartFlag; }

signals:
    /**
     * @brief 服务初始化完成信号
     * @param svrId 服务ID
     */
    void serviceInitialized(int svrId);
    
    /**
     * @brief 服务启动完成信号
     * @param svrId 服务ID
     */
    void serviceStarted(int svrId);
    
    /**
     * @brief 服务错误信号
     * @param svrId 服务ID
     * @param errorMsg 错误消息
     */
    void serviceError(int svrId, const QString &errorMsg);

private:
    /**
     * @brief 构造函数（私有，单例模式）
     */
    ServiceManager();
    
    /**
     * @brief 拷贝构造函数（禁用）
     */
    ServiceManager(const ServiceManager&) = delete;
    
    /**
     * @brief 赋值操作符（禁用）
     */
    ServiceManager& operator=(const ServiceManager&) = delete;
    
    /**
     * @brief 创建所有系统服务
     * @return true=成功, false=失败
     * @note 创建服务对象并注册到管理器
     */
    bool SvrCreateInit();
    
    /**
     * @brief 注册服务对象
     * @param svrobj 服务接口指针
     * @return true=注册成功, false=失败（已存在或指针为空）
     */
    bool RegisterSvrObj(ISysSvrInterface* const svrobj);
    
    /**
     * @brief 建立服务间的依赖关系
     * @note 在所有服务创建完成后调用
     */
    void SetupServiceDependencies();

private:
    QList<ISysSvrInterface*> m_SysSvrList;  // 服务对象列表
    QMutex m_Mutex;                          // 线程互斥锁
    
    bool m_InitFlag;                         // 初始化标记
    bool m_SvrInitFlag;                      // 服务初始化标记
    bool m_SvrStartFlag;                     // 服务启动标记
    
    static ServiceManager* m_Instance;       // 单例实例指针
    static QMutex m_InstanceMutex;           // 实例创建互斥锁
};

#endif // SERVICEMANAGER_H


