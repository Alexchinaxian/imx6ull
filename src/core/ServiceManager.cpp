/***************************************************************
 * Copyright: Alex
 * FileName: ServiceManager.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 系统服务管理器的实现
 *
 * History:
 *   1. 2025-10-15 创建文件，实现服务管理功能
 ***************************************************************/

#include "core/ServiceManager.h"
#include "core/ISysSvrInterface.h"
#include "drivers/manager/DriverManager.h"
#include "drivers/temperature/DriverTemperature.h"
#include "drivers/gpio/DriverGPIO.h"
#include "drivers/led/DriverLED.h"
#include "drivers/pwm/DriverPWM.h"
#include "drivers/scanner/SystemScanner.h"
#include "protocols/manager/ProtocolManager.h"
#include "services/temperature/TemperatureService.h"
#include "services/modbus/ModbusSlaveService.h"
#include "services/time/TimeService.h"
#include "services/weather/WeatherService.h"
#include "services/alarm/AlarmService.h"
#include "drivers/serial/DriverSerial.h"
#include <QDebug>

// 初始化静态成员
ServiceManager* ServiceManager::m_Instance = nullptr;
QMutex ServiceManager::m_InstanceMutex;

/**
 * @brief 构造函数 - 初始化服务管理器
 * 
 * 初始化所有成员变量和标志位
 */
ServiceManager::ServiceManager()
    : QObject(nullptr)
    , m_InitFlag(false)
    , m_SvrInitFlag(false)
    , m_SvrStartFlag(false)
{
    qInfo() << "[ServiceManager] 服务管理器创建";
}

/**
 * @brief 析构函数 - 清理所有服务对象
 * 
 * 释放所有注册的服务对象内存
 */
ServiceManager::~ServiceManager()
{
    qInfo() << "[ServiceManager] 服务管理器销毁，清理所有服务";
    
    // 清理所有服务对象
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        if (m_SysSvrList[i] != nullptr)
        {
            qDebug() << "  清理服务:" << m_SysSvrList[i]->GetSvrName();
            delete m_SysSvrList[i];
            m_SysSvrList[i] = nullptr;
        }
    }
    
    m_SysSvrList.clear();
}

/**
 * @brief 获取服务管理器单例实例
 * @return ServiceManager* 返回单例指针
 * 
 * 实现说明:
 *   使用双重检查锁定模式（Double-Checked Locking）
 *   确保线程安全的单例创建
 */
ServiceManager* ServiceManager::GetInstance()
{
    if (m_Instance == nullptr)
    {
        QMutexLocker locker(&m_InstanceMutex);
        if (m_Instance == nullptr)
        {
            m_Instance = new ServiceManager();
        }
    }
    
    return m_Instance;
}

/**
 * @brief 服务管理器加载初始化
 * @return true=成功, false=失败（已初始化）
 * 
 * 功能说明:
 *   创建所有服务对象，但不进行初始化
 *   只能调用一次，重复调用返回false
 */
bool ServiceManager::ManagerInitLoad()
{
    QMutexLocker locker(&m_Mutex);
    
    if (m_InitFlag)
    {
        qWarning() << "[ServiceManager] 管理器已经初始化，不能重复初始化";
        return false;
    }
    
    qInfo() << "[ServiceManager] 开始创建所有服务对象...";
    
    // 创建所有服务对象
    if (!SvrCreateInit())
    {
        qCritical() << "[ServiceManager] 服务创建失败";
        return false;
    }
    
    m_InitFlag = true;
    qInfo() << "[ServiceManager] 服务对象创建完成，共" << m_SysSvrList.size() << "个服务";
    
    return true;
}

/**
 * @brief 初始化所有服务
 * @return true=成功, false=失败
 * 
 * 功能说明:
 *   调用所有服务的SvrInit()方法进行初始化
 *   如果某个服务初始化失败，继续初始化其他服务
 */
bool ServiceManager::SvrInit()
{
    QMutexLocker locker(&m_Mutex);
    
    if (!m_InitFlag)
    {
        qWarning() << "[ServiceManager] 管理器未加载，请先调用ManagerInitLoad()";
        return false;
    }
    
    if (m_SvrInitFlag)
    {
        qWarning() << "[ServiceManager] 服务已经初始化，不能重复初始化";
        return false;
    }
    
    qInfo() << "[ServiceManager] 开始初始化所有服务...";
    
    int successCount = 0;
    int failCount = 0;
    
    // 初始化所有服务
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        ISysSvrInterface* svr = m_SysSvrList[i];
        qInfo() << "  初始化服务:" << svr->GetSvrName();
        
        if (svr->SvrInit())
        {
            successCount++;
            emit serviceInitialized(svr->GetSvrId());
        }
        else
        {
            failCount++;
            qWarning() << "  ✗ 服务初始化失败:" << svr->GetSvrName();
            emit serviceError(svr->GetSvrId(), "初始化失败");
        }
    }
    
    qInfo() << "[ServiceManager] 服务初始化完成 - 成功:" << successCount 
            << "失败:" << failCount;
    
    // 建立服务间的依赖关系（在初始化完成后、启动之前）
    qInfo() << "[ServiceManager] 建立服务间依赖关系...";
    SetupServiceDependencies();
    qInfo() << "[ServiceManager] ✓ 依赖关系建立完成";
    
    m_SvrInitFlag = true;
    return (failCount == 0);
}

/**
 * @brief 启动所有服务
 * @return true=成功, false=失败
 * 
 * 功能说明:
 *   调用所有服务的SvrStart()方法启动服务
 *   如果某个服务启动失败，继续启动其他服务
 */
bool ServiceManager::SvrStart()
{
    QMutexLocker locker(&m_Mutex);
    
    if (!m_InitFlag || !m_SvrInitFlag)
    {
        qWarning() << "[ServiceManager] 服务未初始化，请先调用SvrInit()";
        return false;
    }
    
    if (m_SvrStartFlag)
    {
        qWarning() << "[ServiceManager] 服务已经启动，不能重复启动";
        return false;
    }
    
    qInfo() << "[ServiceManager] 开始启动所有服务...";
    
    int successCount = 0;
    int failCount = 0;
    
    // 启动所有服务
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        ISysSvrInterface* svr = m_SysSvrList[i];
        qInfo() << "  启动服务:" << svr->GetSvrName();
        
        if (svr->SvrStart())
        {
            successCount++;
            emit serviceStarted(svr->GetSvrId());
        }
        else
        {
            failCount++;
            qWarning() << "  ✗ 服务启动失败:" << svr->GetSvrName();
            emit serviceError(svr->GetSvrId(), "启动失败");
        }
    }
    
    qInfo() << "[ServiceManager] 服务启动完成 - 成功:" << successCount 
            << "失败:" << failCount;
    
    m_SvrStartFlag = true;
    return (failCount == 0);
}

/**
 * @brief 停止所有服务
 * @return true=成功, false=失败
 * 
 * 功能说明:
 *   调用所有服务的SvrStop()方法停止服务
 */
bool ServiceManager::SvrStop()
{
    QMutexLocker locker(&m_Mutex);
    
    if (!m_SvrStartFlag)
    {
        qWarning() << "[ServiceManager] 服务未启动，无需停止";
        return false;
    }
    
    qInfo() << "[ServiceManager] 开始停止所有服务...";
    
    // 反向停止所有服务（与启动顺序相反）
    for (int i = m_SysSvrList.size() - 1; i >= 0; i--)
    {
        ISysSvrInterface* svr = m_SysSvrList[i];
        qInfo() << "  停止服务:" << svr->GetSvrName();
        svr->SvrStop();
    }
    
    m_SvrStartFlag = false;
    qInfo() << "[ServiceManager] 所有服务已停止";
    
    return true;
}

/**
 * @brief 创建所有系统服务
 * @return true=成功, false=失败
 * 
 * 功能说明:
 *   创建所有硬件驱动服务对象并注册到管理器
 *   建立服务间的依赖关系
 * 
 * 注意：这里的服务对象暂时不直接使用现有的驱动类，
 *       而是需要创建包装类来适配ISysSvrInterface接口
 */
bool ServiceManager::SvrCreateInit()
{
    if (m_InitFlag)
    {
        qWarning() << "[ServiceManager] 服务已创建，不能重复创建";
        return false;
    }
    
    qInfo() << "[ServiceManager] 创建驱动服务对象...";
    
    // ========== 创建温度监控服务 ==========
    qInfo() << "  创建温度监控服务...";
    TemperatureService *pTempSvr = new TemperatureService(
        SYS_SVR_ID_TEMPERATURE_SVR,
        SYS_SVR_TYPE_TEMPERATURE_SVR
    );
    if (!RegisterSvrObj(pTempSvr))
    {
        qCritical() << "  ✗ 温度监控服务注册失败";
        delete pTempSvr;
        return false;
    }
    qInfo() << "  ✓ 温度监控服务创建成功";
    
    // ========== 创建Modbus从站服务 ==========
    qInfo() << "  创建Modbus从站服务...";
    ModbusSlaveService *pModbusSvr = new ModbusSlaveService(
        SYS_SVR_ID_MODBUS_SLAVE_SVR,
        SYS_SVR_TYPE_MODBUS_SLAVE_SVR,
        "/dev/ttymxc2",  // 串口
        1                 // 从站地址
    );
    if (!RegisterSvrObj(pModbusSvr))
    {
        qCritical() << "  ✗ Modbus从站服务注册失败";
        delete pModbusSvr;
        return false;
    }
    qInfo() << "  ✓ Modbus从站服务创建成功";
    
    // ========== 创建时间服务 ==========
    qInfo() << "  创建时间服务...";
    TimeService *pTimeSvr = new TimeService(
        SYS_SVR_ID_TIME_SVR,
        SYS_SVR_TYPE_TIME_SVR
    );
    if (!RegisterSvrObj(pTimeSvr))
    {
        qCritical() << "  ✗ 时间服务注册失败";
        delete pTimeSvr;
        return false;
    }
    // 配置时间服务
    pTimeSvr->setNTPServer("ntp.aliyun.com");  // 使用阿里云NTP服务器
    pTimeSvr->setAutoSyncInterval(24);         // 每24小时自动对时
    pTimeSvr->setHalfHourBeepEnabled(true);    // 启用半点蜂鸣
    qInfo() << "  ✓ 时间服务创建成功";
    
    // ========== 创建天气服务 ==========
    qInfo() << "  创建天气服务...";
    WeatherService *pWeatherSvr = new WeatherService(
        SYS_SVR_ID_WEATHER_SVR,
        SYS_SVR_TYPE_WEATHER_SVR
    );
    if (!RegisterSvrObj(pWeatherSvr))
    {
        qCritical() << "  ✗ 天气服务注册失败";
        delete pWeatherSvr;
        return false;
    }
    // 配置天气服务
    pWeatherSvr->setLocation("陕西省西安市雁塔区中建群贤汇");  // 位置
    pWeatherSvr->setUpdateInterval(5);         // 每5分钟获取一次
    pWeatherSvr->setApiKey("337242f0c7384952aaea612209735b30");  // 和风天气API密钥
    qInfo() << "  ✓ 天气服务创建成功（✅ 真实天气API已启用）";
    
    // ========== 创建闹钟服务 ==========
    qInfo() << "  创建闹钟服务...";
    AlarmService *pAlarmSvr = new AlarmService(
        SYS_SVR_ID_ALARM_SVR,
        SYS_SVR_TYPE_ALARM_SVR
    );
    if (!RegisterSvrObj(pAlarmSvr))
    {
        qCritical() << "  ✗ 闹钟服务注册失败";
        delete pAlarmSvr;
        return false;
    }
    // 配置闹钟服务
    pAlarmSvr->setAlarmTime(6, 0);             // 早上6:00起床闹钟
    pAlarmSvr->setAlarmEnabled(true);          // 启用起床闹钟
    pAlarmSvr->setSleepReminderTime(22, 0);    // 晚上22:00睡眠提示
    pAlarmSvr->setSleepReminderEnabled(true);  // 启用睡眠提示
    qInfo() << "  ✓ 闹钟服务创建成功（起床闹钟6:00 + 睡眠提示22:00）";
    
    // ========== 后续可以添加更多服务 ==========
    // 其他服务（GPIO、LED、PWM等）可以根据需要创建适配器类
    // 目前这些驱动可以在应用层直接使用
    
    // 注意：依赖关系将在SvrInit()中建立（所有服务初始化完成后）
    
    qInfo() << "[ServiceManager] 服务对象创建完成，共" << m_SysSvrList.size() << "个服务";
    
    return true;
}

/**
 * @brief 注册服务对象到管理器
 * @param svrobj 服务接口指针
 * @return true=成功, false=失败（已存在或指针为空）
 * 
 * 功能说明:
 *   将服务对象添加到管理列表
 *   检查服务ID是否重复
 */
bool ServiceManager::RegisterSvrObj(ISysSvrInterface* const svrobj)
{
    if (svrobj == nullptr)
    {
        qWarning() << "[ServiceManager] 注册服务失败：服务对象为空";
        return false;
    }
    
    // 检查服务ID是否已存在
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        if (m_SysSvrList[i]->IsYesSvrId(svrobj->GetSvrId()))
        {
            qWarning() << "[ServiceManager] 注册服务失败：服务ID已存在" 
                       << svrobj->GetSvrId();
            return false;
        }
    }
    
    m_SysSvrList.append(svrobj);
    qDebug() << "[ServiceManager] 注册服务成功:" << svrobj->GetSvrName();
    
    return true;
}

/**
 * @brief 建立服务间的依赖关系
 * 
 * 功能说明:
 *   在所有服务创建完成后，建立服务间的相互引用和依赖
 *   例如：LED服务可能依赖于GPIO服务
 */
void ServiceManager::SetupServiceDependencies()
{
    qInfo() << "[ServiceManager] 建立服务间依赖关系...";
    
    // 注意：此方法在SvrCreateInit()中调用，不能再次加锁
    // 直接遍历服务列表查找
    TemperatureService *pTempSvr = nullptr;
    ModbusSlaveService *pModbusSvr = nullptr;
    TimeService *pTimeSvr = nullptr;
    WeatherService *pWeatherSvr = nullptr;
    AlarmService *pAlarmSvr = nullptr;
    
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        if (m_SysSvrList[i]->IsYesSvrId(SYS_SVR_ID_TEMPERATURE_SVR))
        {
            pTempSvr = dynamic_cast<TemperatureService*>(m_SysSvrList[i]);
        }
        else if (m_SysSvrList[i]->IsYesSvrId(SYS_SVR_ID_MODBUS_SLAVE_SVR))
        {
            pModbusSvr = dynamic_cast<ModbusSlaveService*>(m_SysSvrList[i]);
        }
        else if (m_SysSvrList[i]->IsYesSvrId(SYS_SVR_ID_TIME_SVR))
        {
            pTimeSvr = dynamic_cast<TimeService*>(m_SysSvrList[i]);
        }
        else if (m_SysSvrList[i]->IsYesSvrId(SYS_SVR_ID_WEATHER_SVR))
        {
            pWeatherSvr = dynamic_cast<WeatherService*>(m_SysSvrList[i]);
        }
        else if (m_SysSvrList[i]->IsYesSvrId(SYS_SVR_ID_ALARM_SVR))
        {
            pAlarmSvr = dynamic_cast<AlarmService*>(m_SysSvrList[i]);
        }
    }
    
    // ========== 连接温度服务和Modbus从站服务 ==========
    if (pTempSvr && pModbusSvr)
    {
        // 温度变化时更新Modbus寄存器
        QObject::connect(pTempSvr, &TemperatureService::temperatureChanged,
                        pModbusSvr, &ModbusSlaveService::updateTemperature);
        
        // 高温告警时更新状态寄存器 + 蜂鸣器报警
        QObject::connect(pTempSvr, &TemperatureService::temperatureHigh,
                        [pModbusSvr](float temp) {
            Q_UNUSED(temp);
            pModbusSvr->updateSystemStatus(1);  // 状态=告警
            
            // 触发蜂鸣器报警
            DriverBeep *beep = pModbusSvr->GetBeepDriver();
            if (beep) {
                beep->alarm(3, 500, 200);
            }
        });
        
        // 温度恢复正常时更新状态寄存器 + 关闭蜂鸣器
        QObject::connect(pTempSvr, &TemperatureService::temperatureNormal,
                        [pModbusSvr](float temp) {
            Q_UNUSED(temp);
            pModbusSvr->updateSystemStatus(0);  // 状态=正常
            
            // 关闭蜂鸣器
            DriverBeep *beep = pModbusSvr->GetBeepDriver();
            if (beep) {
                beep->turnOff();
            }
        });
        
        qDebug() << "  ✓ 温度服务 <-> Modbus从站服务 依赖关系已建立";
    }
    
    // ========== 连接时间服务和Beep驱动 ==========
    if (pTimeSvr && pModbusSvr)
    {
        // 时间服务需要使用Beep驱动
        DriverBeep *beep = pModbusSvr->GetBeepDriver();
        if (beep) {
            pTimeSvr->setBeepDriver(beep);
            qDebug() << "  ✓ 时间服务 <-> Beep驱动 依赖关系已建立";
        }
        
        // 可以连接时间服务的信号
        QObject::connect(pTimeSvr, &TimeService::halfHourReached, 
                        [](const QDateTime &time) {
            qInfo() << "⏰ 半点提示:" << time.toString("hh:mm");
        });
        
        QObject::connect(pTimeSvr, &TimeService::fullHourReached,
                        [](const QDateTime &time) {
            qInfo() << "⏰ 整点提示:" << time.toString("hh:mm");
        });
        
        QObject::connect(pTimeSvr, &TimeService::timeSynced,
                        [](const QDateTime &syncTime) {
            qInfo() << "🕐 NTP对时成功:" << syncTime.toString("yyyy-MM-dd hh:mm:ss") << "(北京时间)";
        });
    }
    
    // ========== 连接闹钟服务和Beep驱动 ==========
    if (pAlarmSvr && pModbusSvr)
    {
        // 闹钟服务需要使用Beep驱动
        DriverBeep *beep = pModbusSvr->GetBeepDriver();
        if (beep) {
            pAlarmSvr->setBeepDriver(beep);
            qDebug() << "  ✓ 闹钟服务 <-> Beep驱动 依赖关系已建立";
        }
        
        // 连接闹钟触发信号
        QObject::connect(pAlarmSvr, &AlarmService::alarmTriggered,
                        [](const QDateTime &time) {
            qInfo() << "🌅 工作日起床闹钟触发:" << time.toString("yyyy-MM-dd hh:mm:ss dddd");
        });
        
        QObject::connect(pAlarmSvr, &AlarmService::alarmFinished,
                        []() {
            qInfo() << "⏰ 起床闹钟播放结束";
        });
        
        QObject::connect(pAlarmSvr, &AlarmService::sleepReminderTriggered,
                        [](const QDateTime &time) {
            qInfo() << "🌙 睡眠提示触发（该睡觉了）:" << time.toString("yyyy-MM-dd hh:mm:ss dddd");
        });
    }
    
    // ========== 连接天气服务和串口驱动 ==========
    if (pWeatherSvr)
    {
        // 创建专用串口驱动（使用ttymxc1）
        DriverSerial *weatherSerial = new DriverSerial("/dev/ttymxc1", this);
        weatherSerial->setBaudRate(115200);
        weatherSerial->setDataBits(QSerialPort::Data8);
        weatherSerial->setParity(QSerialPort::NoParity);
        weatherSerial->setStopBits(QSerialPort::OneStop);
        
        if (weatherSerial->open(QIODevice::ReadWrite)) {
            pWeatherSvr->setSerialDriver(weatherSerial);
            qDebug() << "  ✓ 天气服务 <-> 串口驱动 依赖关系已建立 (/dev/ttymxc1, 115200)";
        } else {
            qDebug() << "  ⚠ 天气串口驱动打开失败，天气信息仅输出到日志";
            delete weatherSerial;
        }
        
        // 可以连接天气服务的信号
        QObject::connect(pWeatherSvr, &WeatherService::weatherUpdated,
                        [](const WeatherData &weather) {
            qDebug() << "☁️  天气更新:" << weather.location << weather.weather 
                     << weather.temperature << "°C";
        });
    }
    
    // 未来可以添加更多服务间的依赖关系
    
    qInfo() << "[ServiceManager] 依赖关系建立完成";
}

// ========== 服务查找接口实现 ==========

ISysSvrInterface* ServiceManager::GetSvrObj(int32_t svr_id, int32_t svr_type)
{
    QMutexLocker locker(&m_Mutex);
    
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        if (m_SysSvrList[i]->IsSvrIdAndType(svr_id, svr_type))
        {
            return m_SysSvrList[i];
        }
    }
    
    return nullptr;
}

ISysSvrInterface* ServiceManager::GetSvrObj(int32_t svr_id)
{
    QMutexLocker locker(&m_Mutex);
    
    for (int i = 0; i < m_SysSvrList.size(); i++)
    {
        if (m_SysSvrList[i]->IsYesSvrId(svr_id))
        {
            return m_SysSvrList[i];
        }
    }
    
    return nullptr;
}

DriverTemperature* ServiceManager::GetTemperatureSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_TEMPERATURE_SVR);
    if (svr)
    {
        TemperatureService* tempSvr = dynamic_cast<TemperatureService*>(svr);
        if (tempSvr)
        {
            return tempSvr->GetDriver();
        }
    }
    return nullptr;
}

DriverGPIO* ServiceManager::GetGPIOSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_GPIO_SVR);
    return nullptr;
}

DriverLED* ServiceManager::GetLEDSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_LED_SVR);
    return nullptr;
}

DriverPWM* ServiceManager::GetPWMSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_PWM_SVR);
    return nullptr;
}

SystemScanner* ServiceManager::GetScannerSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_SCANNER_SVR);
    return nullptr;
}

DriverManager* ServiceManager::GetDriverManagerSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_DRIVER_MANAGER);
    return nullptr;
}

ProtocolManager* ServiceManager::GetProtocolManager()
{
    // 协议管理器使用单例模式，不通过服务列表管理
    return ProtocolManager::getInstance();
}

ModbusSlaveService* ServiceManager::GetModbusSlaveSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_MODBUS_SLAVE_SVR);
    if (svr)
    {
        ModbusSlaveService* modbusSvr = dynamic_cast<ModbusSlaveService*>(svr);
        if (modbusSvr)
        {
            return modbusSvr;
        }
    }
    return nullptr;
}

TimeService* ServiceManager::GetTimeSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_TIME_SVR);
    if (svr)
    {
        TimeService* timeSvr = dynamic_cast<TimeService*>(svr);
        if (timeSvr)
        {
            return timeSvr;
        }
    }
    return nullptr;
}

WeatherService* ServiceManager::GetWeatherSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_WEATHER_SVR);
    if (svr)
    {
        WeatherService* weatherSvr = dynamic_cast<WeatherService*>(svr);
        if (weatherSvr)
        {
            return weatherSvr;
        }
    }
    return nullptr;
}

AlarmService* ServiceManager::GetAlarmSvrObj()
{
    ISysSvrInterface* svr = GetSvrObj(SYS_SVR_ID_ALARM_SVR);
    if (svr)
    {
        AlarmService* alarmSvr = dynamic_cast<AlarmService*>(svr);
        if (alarmSvr)
        {
            return alarmSvr;
        }
    }
    return nullptr;
}

int ServiceManager::GetServiceCount() const
{
    return m_SysSvrList.size();
}


