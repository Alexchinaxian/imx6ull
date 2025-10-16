#include "drivers/manager/DriverManager.h"
#include "drivers/gpio/DriverGPIO.h"
#include "drivers/led/DriverLED.h"
#include "drivers/pwm/DriverPWM.h"
#include "drivers/serial/DriverSerial.h"
#include "drivers/can/DriverCAN.h"
#include "drivers/scanner/SystemScanner.h"
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QSerialPort>

/***************************************************************
 * 实现文件: driver_manager.cpp
 * 功能: 驱动管理器的实现
 * 说明: 提供统一的驱动管理接口，方便后续扩展
 ***************************************************************/

/**
 * @brief 构造函数
 * @param parent 父对象指针
 * 
 * 初始化驱动管理器，加载驱动配置
 */
DriverManager::DriverManager(QObject *parent) 
    : QObject(parent)
    , m_systemScanner(nullptr)
{
    qInfo() << "DriverManager: Initializing...";
}

/**
 * @brief 析构函数
 * 
 * 清理驱动管理器资源，确保所有驱动正确卸载
 */
DriverManager::~DriverManager()
{
    qInfo() << "DriverManager: Cleaning up...";
    releaseAll();
}

/**
 * @brief 获取驱动管理器的单例实例
 * @return DriverManager& 返回驱动管理器的引用
 * 
 * 实现单例模式的关键函数：
 * 1. 使用static局部变量，确保只创建一次实例
 * 2. 线程安全（C++11保证static局部变量的线程安全初始化）
 * 3. 自动管理生命周期（程序结束时自动销毁）
 */
DriverManager& DriverManager::getInstance()
{
    static DriverManager instance;  // 静态局部变量，只初始化一次
    return instance;
}

// ========== LED驱动管理 ==========

/**
 * @brief 创建或获取LED驱动
 */
DriverLED* DriverManager::getLED(const QString &ledName)
{
    // 检查是否已存在
    if (m_ledDrivers.contains(ledName)) {
        qInfo() << "DriverManager: Reusing existing LED driver:" << ledName;
        return m_ledDrivers[ledName];
    }
    
    // 创建新的LED驱动
    qInfo() << "DriverManager: Creating new LED driver:" << ledName;
    DriverLED *led = new DriverLED(ledName, this);
    m_ledDrivers[ledName] = led;
    
    emit driverLoaded("LED", ledName);
    return led;
}

/**
 * @brief 释放LED驱动
 */
void DriverManager::releaseLED(const QString &ledName)
{
    if (m_ledDrivers.contains(ledName)) {
        qInfo() << "DriverManager: Releasing LED driver:" << ledName;
        DriverLED *led = m_ledDrivers.take(ledName);
        delete led;
        emit driverUnloaded("LED", ledName);
    }
}

// ========== GPIO驱动管理 ==========

/**
 * @brief 创建或获取GPIO驱动
 */
DriverGPIO* DriverManager::getGPIO(int gpioNum)
{
    // 检查是否已存在
    if (m_gpioDrivers.contains(gpioNum)) {
        qInfo() << "DriverManager: Reusing existing GPIO driver:" << gpioNum;
        return m_gpioDrivers[gpioNum];
    }
    
    // 创建新的GPIO驱动
    qInfo() << "DriverManager: Creating new GPIO driver:" << gpioNum;
    DriverGPIO *gpio = new DriverGPIO(gpioNum, this);
    m_gpioDrivers[gpioNum] = gpio;
    
    emit driverLoaded("GPIO", QString::number(gpioNum));
    return gpio;
}

/**
 * @brief 释放GPIO驱动
 */
void DriverManager::releaseGPIO(int gpioNum)
{
    if (m_gpioDrivers.contains(gpioNum)) {
        qInfo() << "DriverManager: Releasing GPIO driver:" << gpioNum;
        DriverGPIO *gpio = m_gpioDrivers.take(gpioNum);
        delete gpio;
        emit driverUnloaded("GPIO", QString::number(gpioNum));
    }
}

// ========== PWM驱动管理 ==========

/**
 * @brief 生成PWM驱动的键名
 */
QString DriverManager::makePWMKey(int chipNum, int channelNum) const
{
    return QString("%1_%2").arg(chipNum).arg(channelNum);
}

/**
 * @brief 创建或获取PWM驱动
 */
DriverPWM* DriverManager::getPWM(int chipNum, int channelNum)
{
    QString key = makePWMKey(chipNum, channelNum);
    
    // 检查是否已存在
    if (m_pwmDrivers.contains(key)) {
        qInfo() << "DriverManager: Reusing existing PWM driver:" << key;
        return m_pwmDrivers[key];
    }
    
    // 创建新的PWM驱动
    qInfo() << "DriverManager: Creating new PWM driver:" << key;
    DriverPWM *pwm = new DriverPWM(chipNum, channelNum, this);
    m_pwmDrivers[key] = pwm;
    
    emit driverLoaded("PWM", key);
    return pwm;
}

/**
 * @brief 释放PWM驱动
 */
void DriverManager::releasePWM(int chipNum, int channelNum)
{
    QString key = makePWMKey(chipNum, channelNum);
    
    if (m_pwmDrivers.contains(key)) {
        qInfo() << "DriverManager: Releasing PWM driver:" << key;
        DriverPWM *pwm = m_pwmDrivers.take(key);
        delete pwm;
        emit driverUnloaded("PWM", key);
    }
}

// ========== 串口驱动管理 ==========

/**
 * @brief 创建或获取串口驱动
 */
DriverSerial* DriverManager::getSerial(const QString &portName)
{
    // 检查是否已存在
    if (m_serialDrivers.contains(portName)) {
        qInfo() << "DriverManager: Reusing existing Serial driver:" << portName;
        return m_serialDrivers[portName];
    }
    
    // 创建新的串口驱动
    qInfo() << "DriverManager: Creating new Serial driver:" << portName;
    DriverSerial *serial = new DriverSerial(portName, this);
    m_serialDrivers[portName] = serial;
    
    emit driverLoaded("Serial", portName);
    return serial;
}

/**
 * @brief 释放串口驱动
 */
void DriverManager::releaseSerial(const QString &portName)
{
    if (m_serialDrivers.contains(portName)) {
        qInfo() << "DriverManager: Releasing Serial driver:" << portName;
        DriverSerial *serial = m_serialDrivers.take(portName);
        delete serial;
        emit driverUnloaded("Serial", portName);
    }
}

// ========== CAN驱动管理 ==========

/**
 * @brief 创建或获取CAN驱动
 */
DriverCAN* DriverManager::getCAN(const QString &interfaceName)
{
    // 检查是否已存在
    if (m_canDrivers.contains(interfaceName)) {
        qInfo() << "DriverManager: Reusing existing CAN driver:" << interfaceName;
        return m_canDrivers[interfaceName];
    }
    
    // 创建新的CAN驱动
    qInfo() << "DriverManager: Creating new CAN driver:" << interfaceName;
    DriverCAN *can = new DriverCAN(interfaceName, this);
    m_canDrivers[interfaceName] = can;
    
    emit driverLoaded("CAN", interfaceName);
    return can;
}

/**
 * @brief 释放CAN驱动
 */
void DriverManager::releaseCAN(const QString &interfaceName)
{
    if (m_canDrivers.contains(interfaceName)) {
        qInfo() << "DriverManager: Releasing CAN driver:" << interfaceName;
        DriverCAN *can = m_canDrivers.take(interfaceName);
        delete can;
        emit driverUnloaded("CAN", interfaceName);
    }
}

// ========== 系统扫描器 ==========

/**
 * @brief 获取系统扫描器
 */
SystemScanner* DriverManager::getSystemScanner()
{
    if (!m_systemScanner) {
        qInfo() << "DriverManager: Creating SystemScanner";
        m_systemScanner = new SystemScanner(this);
        emit driverLoaded("Scanner", "SystemScanner");
    }
    return m_systemScanner;
}

// ========== 资源管理 ==========

/**
 * @brief 释放所有驱动资源
 */
void DriverManager::releaseAll()
{
    qInfo() << "DriverManager: Releasing all drivers...";
    
    // 释放LED驱动
    qDeleteAll(m_ledDrivers);
    m_ledDrivers.clear();
    
    // 释放GPIO驱动
    qDeleteAll(m_gpioDrivers);
    m_gpioDrivers.clear();
    
    // 释放PWM驱动
    qDeleteAll(m_pwmDrivers);
    m_pwmDrivers.clear();
    
    // 释放串口驱动
    qDeleteAll(m_serialDrivers);
    m_serialDrivers.clear();
    
    // 释放CAN驱动
    qDeleteAll(m_canDrivers);
    m_canDrivers.clear();
    
    // 释放系统扫描器
    if (m_systemScanner) {
        delete m_systemScanner;
        m_systemScanner = nullptr;
    }
    
    qInfo() << "DriverManager: All drivers released";
}

/**
 * @brief 获取当前加载的驱动数量
 */
int DriverManager::getDriverCount() const
{
    int count = 0;
    count += m_ledDrivers.size();
    count += m_gpioDrivers.size();
    count += m_pwmDrivers.size();
    count += m_serialDrivers.size();
    count += m_canDrivers.size();
    count += (m_systemScanner != nullptr) ? 1 : 0;
    return count;
}

/**
 * @brief 打印所有已加载的驱动信息
 */
void DriverManager::printDriverList() const
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  Driver Manager Status";
    qInfo() << "========================================";
    qInfo() << "Total Drivers:" << getDriverCount();
    
    if (!m_ledDrivers.isEmpty()) {
        qInfo() << "";
        qInfo() << "LED Drivers (" << m_ledDrivers.size() << "):";
        for (auto it = m_ledDrivers.begin(); it != m_ledDrivers.end(); ++it) {
            qInfo() << "  -" << it.key();
        }
    }
    
    if (!m_gpioDrivers.isEmpty()) {
        qInfo() << "";
        qInfo() << "GPIO Drivers (" << m_gpioDrivers.size() << "):";
        for (auto it = m_gpioDrivers.begin(); it != m_gpioDrivers.end(); ++it) {
            qInfo() << "  - GPIO" << it.key();
        }
    }
    
    if (!m_pwmDrivers.isEmpty()) {
        qInfo() << "";
        qInfo() << "PWM Drivers (" << m_pwmDrivers.size() << "):";
        for (auto it = m_pwmDrivers.begin(); it != m_pwmDrivers.end(); ++it) {
            qInfo() << "  - PWM" << it.key();
        }
    }
    
    if (!m_serialDrivers.isEmpty()) {
        qInfo() << "";
        qInfo() << "Serial Drivers (" << m_serialDrivers.size() << "):";
        for (auto it = m_serialDrivers.begin(); it != m_serialDrivers.end(); ++it) {
            qInfo() << "  -" << it.key();
        }
    }
    
    if (!m_canDrivers.isEmpty()) {
        qInfo() << "";
        qInfo() << "CAN Drivers (" << m_canDrivers.size() << "):";
        for (auto it = m_canDrivers.begin(); it != m_canDrivers.end(); ++it) {
            qInfo() << "  -" << it.key();
        }
    }
    
    if (m_systemScanner) {
        qInfo() << "";
        qInfo() << "System Scanner: Active";
    }
    
    qInfo() << "========================================";
    qInfo() << "";
}

// ========== 配置文件管理 ==========

/**
 * @brief 从配置文件加载硬件配置
 */
bool DriverManager::loadFromConfig(const QString &configFile)
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  Loading Hardware Configuration";
    qInfo() << "========================================";
    qInfo() << "配置文件:" << configFile;
    
    // 检查文件是否存在
    if (!QFile::exists(configFile))
    {
        qCritical() << "配置文件不存在:" << configFile;
        return false;
    }
    
    // 使用QSettings读取INI格式配置
    QSettings settings(configFile, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");  // 支持中文
    
    // 获取所有配置节
    QStringList groups = settings.childGroups();
    
    qInfo() << "找到" << groups.size() << "个配置节";
    qInfo() << "";
    
    int successCount = 0;
    int failedCount = 0;
    
    // 解析每个配置节
    for (const QString &group : groups)
    {
        settings.beginGroup(group);
        
        QString type = settings.value("type", "Unknown").toString();
        QString name = settings.value("name", "").toString();
        bool enabled = settings.value("enabled", false).toBool();
        QString description = settings.value("description", "").toString();
        
        // 只处理已启用的设备
        if (!enabled)
        {
            settings.endGroup();
            continue;
        }
        
        bool success = false;
        
        // 根据设备类型创建驱动
        if (type == "PWM")
        {
            int chip = settings.value("chip", 0).toInt();
            int channel = settings.value("channel", 0).toInt();
            int frequency = settings.value("frequency", 1000).toInt();
            double dutyCycle = settings.value("duty_cycle", 50.0).toDouble();
            
            qInfo() << QString("  ✓ [PWM] %1 (chip=%2, channel=%3, freq=%4Hz, duty=%5%)")
                       .arg(name, -12)
                       .arg(chip)
                       .arg(channel)
                       .arg(frequency)
                       .arg(dutyCycle);
            
            // 创建PWM驱动
            DriverPWM *pwm = getPWM(chip, channel);
            if (pwm)
            {
                // 导出PWM
                pwm->exportPWM();
                
                // 设置频率和占空比
                pwm->setFrequency(frequency, dutyCycle);
                
                // 注册别名
                QString key = makePWMKey(chip, channel);
                m_pwmAliases[name] = key;
                
                success = true;
            }
        }
        else if (type == "GPIO")
        {
            int gpioNum = settings.value("gpio_num", 0).toInt();
            QString direction = settings.value("direction", "out").toString();
            int initialValue = settings.value("initial_value", 0).toInt();
            
            qInfo() << QString("  ✓ [GPIO] %1 (gpio=%2, dir=%3, init=%4)")
                       .arg(name, -12)
                       .arg(gpioNum)
                       .arg(direction)
                       .arg(initialValue);
            
            // 创建GPIO驱动
            DriverGPIO *gpio = getGPIO(gpioNum);
            if (gpio)
            {
                // 导出GPIO
                gpio->exportGPIO();
                
                // 设置方向
                DriverGPIO::Direction dir = (direction == "in") ? DriverGPIO::Input : DriverGPIO::Output;
                gpio->setDirection(dir);
                
                // 设置初始值（输出模式）
                if (dir == DriverGPIO::Output)
                {
                    DriverGPIO::Value val = (initialValue == 0) ? DriverGPIO::Low : DriverGPIO::High;
                    gpio->setValue(val);
                }
                
                // 注册别名
                m_gpioAliases[name] = gpioNum;
                
                success = true;
            }
        }
        else if (type == "LED")
        {
            QString device = settings.value("device", "").toString();
            int brightness = settings.value("brightness", 255).toInt();
            
            qInfo() << QString("  ✓ [LED] %1 (device=%2, brightness=%3)")
                       .arg(name, -12)
                       .arg(device)
                       .arg(brightness);
            
            // 创建LED驱动
            DriverLED *led = getLED(device);
            if (led)
            {
                // 设置亮度
                led->setBrightness(brightness);
                
                // 注册别名
                m_ledAliases[name] = device;
                
                success = true;
            }
        }
        else if (type == "Serial")
        {
            QString device = settings.value("device", "").toString();
            int baudrate = settings.value("baudrate", 115200).toInt();
            int databits = settings.value("databits", 8).toInt();
            QString parity = settings.value("parity", "N").toString();
            int stopbits = settings.value("stopbits", 1).toInt();
            
            qInfo() << QString("  ✓ [Serial] %1 (device=%2, baudrate=%3, %4%5%6)")
                       .arg(name, -12)
                       .arg(device)
                       .arg(baudrate)
                       .arg(databits)
                       .arg(parity)
                       .arg(stopbits);
            
            // 创建串口驱动
            DriverSerial *serial = getSerial(device);
            if (serial)
            {
                // 配置串口参数
                serial->configure(baudrate);
                
                // 设置数据位
                if (databits == 7) {
                    serial->setDataBits(QSerialPort::Data7);
                } else {
                    serial->setDataBits(QSerialPort::Data8);
                }
                
                // 设置校验位
                if (parity == "E") {
                    serial->setParity(QSerialPort::EvenParity);
                } else if (parity == "O") {
                    serial->setParity(QSerialPort::OddParity);
                } else {
                    serial->setParity(QSerialPort::NoParity);
                }
                
                // 设置停止位
                if (stopbits == 2) {
                    serial->setStopBits(QSerialPort::TwoStop);
                } else {
                    serial->setStopBits(QSerialPort::OneStop);
                }
                
                // 注册别名
                m_serialAliases[name] = device;
                
                success = true;
            }
        }
        else
        {
            qWarning() << "  ✗ 不支持的设备类型:" << type << "-" << name;
        }
        
        if (success)
        {
            successCount++;
        }
        else
        {
            failedCount++;
        }
        
        settings.endGroup();
    }
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "配置加载完成 - 成功:" << successCount << "失败:" << failedCount;
    qInfo() << "========================================";
    qInfo() << "";
    
    return true;
}

/**
 * @brief 通过别名获取PWM驱动
 */
DriverPWM* DriverManager::getPWMByAlias(const QString &alias)
{
    if (m_pwmAliases.contains(alias))
    {
        QString key = m_pwmAliases[alias];
        return m_pwmDrivers.value(key, nullptr);
    }
    return nullptr;
}

/**
 * @brief 通过别名获取GPIO驱动
 */
DriverGPIO* DriverManager::getGPIOByAlias(const QString &alias)
{
    if (m_gpioAliases.contains(alias))
    {
        int gpioNum = m_gpioAliases[alias];
        return m_gpioDrivers.value(gpioNum, nullptr);
    }
    return nullptr;
}

/**
 * @brief 通过别名获取LED驱动
 */
DriverLED* DriverManager::getLEDByAlias(const QString &alias)
{
    if (m_ledAliases.contains(alias))
    {
        QString device = m_ledAliases[alias];
        return m_ledDrivers.value(device, nullptr);
    }
    return nullptr;
}

/**
 * @brief 通过别名获取串口驱动
 */
DriverSerial* DriverManager::getSerialByAlias(const QString &alias)
{
    if (m_serialAliases.contains(alias))
    {
        QString device = m_serialAliases[alias];
        return m_serialDrivers.value(device, nullptr);
    }
    return nullptr;
}

/**
 * @brief 获取所有已配置的设备别名
 */
QStringList DriverManager::getAllAliases() const
{
    QStringList aliases;
    aliases << m_pwmAliases.keys();
    aliases << m_gpioAliases.keys();
    aliases << m_ledAliases.keys();
    aliases << m_serialAliases.keys();
    return aliases;
}

/**
 * @brief 打印配置报告
 */
void DriverManager::printConfigReport() const
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  Device Alias Mapping Report";
    qInfo() << "========================================";
    
    if (!m_pwmAliases.isEmpty())
    {
        qInfo() << "";
        qInfo() << "PWM设备别名 (" << m_pwmAliases.size() << "):";
        for (auto it = m_pwmAliases.begin(); it != m_pwmAliases.end(); ++it)
        {
            qInfo() << "  •" << it.key() << "->" << it.value();
        }
    }
    
    if (!m_gpioAliases.isEmpty())
    {
        qInfo() << "";
        qInfo() << "GPIO设备别名 (" << m_gpioAliases.size() << "):";
        for (auto it = m_gpioAliases.begin(); it != m_gpioAliases.end(); ++it)
        {
            qInfo() << "  •" << it.key() << "-> GPIO" << it.value();
        }
    }
    
    if (!m_ledAliases.isEmpty())
    {
        qInfo() << "";
        qInfo() << "LED设备别名 (" << m_ledAliases.size() << "):";
        for (auto it = m_ledAliases.begin(); it != m_ledAliases.end(); ++it)
        {
            qInfo() << "  •" << it.key() << "->" << it.value();
        }
    }
    
    if (!m_serialAliases.isEmpty())
    {
        qInfo() << "";
        qInfo() << "串口设备别名 (" << m_serialAliases.size() << "):";
        for (auto it = m_serialAliases.begin(); it != m_serialAliases.end(); ++it)
        {
            qInfo() << "  •" << it.key() << "->" << it.value();
        }
    }
    
    qInfo() << "";
    qInfo() << "💡 使用示例:";
    if (!m_pwmAliases.isEmpty())
    {
        QString firstPwmAlias = m_pwmAliases.keys().first();
        qInfo() << "  • DriverPWM *pwm = driverMgr.getPWMByAlias(\"" << firstPwmAlias << "\");";
    }
    if (!m_gpioAliases.isEmpty())
    {
        QString firstGpioAlias = m_gpioAliases.keys().first();
        qInfo() << "  • DriverGPIO *gpio = driverMgr.getGPIOByAlias(\"" << firstGpioAlias << "\");";
    }
    
    qInfo() << "========================================";
    qInfo() << "";
}
