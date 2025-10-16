/***************************************************************
 * Copyright: Alex
 * FileName: HardwareInitService.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 硬件初始化服务实现
 ***************************************************************/

#include "services/hardware/HardwareInitService.h"
#include "core/LogManager.h"
#include "drivers/manager/DriverManager.h"
#include "drivers/scanner/SystemScanner.h"
#include "drivers/gpio/DriverGPIO.h"
#include "drivers/led/DriverLED.h"
#include "drivers/pwm/DriverPWM.h"
#include "drivers/serial/DriverSerial.h"
#include <QDebug>

/***************************************************************
 * 构造函数
 ***************************************************************/
HardwareInitService::HardwareInitService(int32_t svr_id, int32_t svr_type, QObject *parent)
    : ISysSvrInterface(svr_id, svr_type, parent)
    , m_configFile("hardware.init")
    , m_scanCompleted(false)
    , m_configLoaded(false)
{
    LOG_INFO("Hardware", "[HardwareInitService] 硬件初始化服务创建");
}

/***************************************************************
 * 析构函数
 ***************************************************************/
HardwareInitService::~HardwareInitService()
{
    LOG_INFO("Hardware", "[HardwareInitService] 硬件初始化服务销毁");
}

/***************************************************************
 * 服务初始化
 ***************************************************************/
bool HardwareInitService::SvrInit()
{
    LOG_INFO("Hardware", "");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "  硬件初始化服务初始化");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", QString("配置文件: %1").arg(m_configFile));
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "[HardwareInitService] ✓ 服务初始化成功");
    LOG_INFO("Hardware", "");
    
    return true;
}

/***************************************************************
 * 服务启动
 ***************************************************************/
bool HardwareInitService::SvrStart()
{
    LOG_INFO("Hardware", "");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "  启动硬件初始化服务");
    LOG_INFO("Hardware", "========================================");
    
    // 1. 扫描硬件
    if (!scanHardware())
    {
        LOG_ERROR("Hardware", "硬件扫描失败");
        emit initFailed("硬件扫描失败");
        return false;
    }
    
    // 2. 加载配置
    if (!loadHardwareConfig())
    {
        LOG_WARNING("Hardware", "配置加载失败，使用默认配置");
        // 配置加载失败不影响系统运行
    }
    
    // 3. 初始化设备
    if (!initHardwareDevices())
    {
        LOG_WARNING("Hardware", "部分硬件设备初始化失败");
        // 部分设备初始化失败不影响系统运行
    }
    
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "[HardwareInitService] ✓ 硬件初始化服务启动成功");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "");
    
    emit initCompleted();
    return true;
}

/***************************************************************
 * 服务停止
 ***************************************************************/
bool HardwareInitService::SvrStop()
{
    LOG_INFO("Hardware", "[HardwareInitService] 硬件初始化服务停止");
    return true;
}

/***************************************************************
 * 扫描系统硬件接口
 ***************************************************************/
bool HardwareInitService::scanHardware()
{
    LOG_INFO("Hardware", "");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "  Driver Manager & Hardware Scanning");
    LOG_INFO("Hardware", "========================================");
    
    // 获取驱动管理器单例
    DriverManager &driverMgr = DriverManager::getInstance();
    
    // 获取系统扫描器
    SystemScanner *scanner = driverMgr.getSystemScanner();
    if (!scanner)
    {
        LOG_ERROR("Hardware", "无法获取系统扫描器");
        return false;
    }
    
    // 执行扫描
    scanner->scanAll();
    LOG_INFO("Hardware", "");
    
    // 打印扫描报告（使用qInfo，因为格式化输出）
    scanner->printReport();
    
    // 获取特定类型的接口
    auto i2cInterfaces = scanner->getInterfacesByType("I2C");
    LOG_INFO("Hardware", QString("I2C interfaces found: %1").arg(i2cInterfaces.size()));
    
    auto gpioChips = scanner->getInterfacesByType("GPIO");
    LOG_INFO("Hardware", QString("GPIO chips found: %1").arg(gpioChips.size()));
    
    auto serialPorts = scanner->getInterfacesByType("Serial");
    LOG_INFO("Hardware", QString("Serial ports found: %1").arg(serialPorts.size()));
    
    LOG_INFO("Hardware", "");
    
    // 打印驱动管理器状态
    driverMgr.printDriverList();
    
    m_scanCompleted = true;
    LOG_INFO("Hardware", "✓ 硬件扫描完成");
    
    return true;
}

/***************************************************************
 * 加载硬件配置文件
 ***************************************************************/
bool HardwareInitService::loadHardwareConfig()
{
    LOG_INFO("Hardware", "");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "  加载硬件配置");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", QString("配置文件: %1").arg(m_configFile));
    
    DriverManager &driverMgr = DriverManager::getInstance();
    
    if (driverMgr.loadFromConfig(m_configFile))
    {
        LOG_INFO("Hardware", "✓ 硬件配置加载成功");
        
        // 打印设备别名映射报告
        driverMgr.printConfigReport();
        
        m_configLoaded = true;
        return true;
    }
    else
    {
        LOG_WARNING("Hardware", "⚠ 硬件配置加载失败");
        return false;
    }
}

/***************************************************************
 * 初始化硬件设备
 ***************************************************************/
bool HardwareInitService::initHardwareDevices()
{
    if (!m_configLoaded)
    {
        LOG_WARNING("Hardware", "配置未加载，跳过设备初始化");
        return false;
    }
    
    LOG_INFO("Hardware", "");
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "  初始化硬件设备");
    LOG_INFO("Hardware", "========================================");
    
    DriverManager &driverMgr = DriverManager::getInstance();
    
    // 示例1：控制风扇PWM
    DriverPWM *fanPWM = driverMgr.getPWMByAlias("风扇");
    if (fanPWM)
    {
        LOG_INFO("Hardware", "✓ 启动风扇PWM输出 (25KHz, 50%)");
        fanPWM->start();
    }
    
    // 示例2：控制继电器
    DriverGPIO *relay1 = driverMgr.getGPIOByAlias("继电器1");
    if (relay1)
    {
        LOG_INFO("Hardware", "✓ 继电器1已就绪");
    }
    
    // 示例3：点亮系统指示灯
    DriverLED *sysLED = driverMgr.getLEDByAlias("系统指示灯");
    if (sysLED)
    {
        LOG_INFO("Hardware", "✓ 点亮系统指示灯");
        sysLED->turnOn();
    }
    
    // 示例4：Modbus串口
    DriverSerial *modbusSerial = driverMgr.getSerialByAlias("Modbus串口");
    if (modbusSerial)
    {
        LOG_INFO("Hardware", "✓ Modbus串口已配置 (9600 8N1)");
    }
    
    LOG_INFO("Hardware", "========================================");
    LOG_INFO("Hardware", "✓ 硬件设备初始化完成");
    LOG_INFO("Hardware", "");
    
    return true;
}

