#include "drivers/scanner/SystemScanner.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSerialPortInfo>

/***************************************************************
 * 实现文件: system_scanner.cpp
 * 功能: 系统硬件接口扫描器的具体实现
 * 说明: 遍历/sys/class/目录，识别各类硬件接口
 ***************************************************************/

/**
 * @brief 构造函数 - 初始化扫描器
 * @param parent 父对象指针
 */
SystemScanner::SystemScanner(QObject *parent) : QObject(parent)
{
}

SystemScanner::~SystemScanner()
{
}

bool SystemScanner::pathExists(const QString &path)
{
    return QDir(path).exists() || QFile::exists(path);
}

QStringList SystemScanner::listDirectory(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return QStringList();
    }
    
    return dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);
}

QString SystemScanner::readSysFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    QString content = in.readAll().trimmed();
    file.close();
    
    return content;
}

void SystemScanner::scanDirectory(const QString &basePath, const QString &type)
{
    QStringList entries = listDirectory(basePath);
    
    for (const QString &entry : entries) {
        QString fullPath = basePath + "/" + entry;
        
        InterfaceInfo info;
        info.type = type;
        info.name = entry;
        info.path = fullPath;
        
        // 读取常见属性
        if (QFile::exists(fullPath + "/name")) {
            info.attributes["name"] = readSysFile(fullPath + "/name");
        }
        if (QFile::exists(fullPath + "/uevent")) {
            info.attributes["uevent"] = readSysFile(fullPath + "/uevent");
        }
        
        m_interfaces.append(info);
        emit interfaceFound(type, entry);
    }
}

void SystemScanner::scanGPIO()
{
    qInfo() << "[Scanner] Scanning GPIO...";
    
    // 扫描 /sys/class/gpio
    QString gpioPath = "/sys/class/gpio";
    if (pathExists(gpioPath)) {
        QStringList entries = listDirectory(gpioPath);
        
        for (const QString &entry : entries) {
            if (entry.startsWith("gpiochip")) {
                QString fullPath = gpioPath + "/" + entry;
                
                InterfaceInfo info;
                info.type = "GPIO";
                info.name = entry;
                info.path = fullPath;
                info.attributes["base"] = readSysFile(fullPath + "/base");
                info.attributes["ngpio"] = readSysFile(fullPath + "/ngpio");
                info.attributes["label"] = readSysFile(fullPath + "/label");
                
                QString base = info.attributes["base"];
                QString ngpio = info.attributes["ngpio"];
                info.description = QString("GPIO chip: base=%1, count=%2").arg(base, ngpio);
                
                m_interfaces.append(info);
                emit interfaceFound("GPIO", entry);
                
                qInfo() << "  Found:" << entry << "- base:" << base << "ngpio:" << ngpio;
            }
        }
    }
}

void SystemScanner::scanI2C()
{
    qInfo() << "[Scanner] Scanning I2C...";
    
    // 扫描 /sys/class/i2c-adapter
    QString i2cPath = "/sys/class/i2c-adapter";
    if (pathExists(i2cPath)) {
        QStringList entries = listDirectory(i2cPath);
        
        for (const QString &entry : entries) {
            QString fullPath = i2cPath + "/" + entry;
            
            InterfaceInfo info;
            info.type = "I2C";
            info.name = entry;
            info.path = fullPath;
            info.attributes["name"] = readSysFile(fullPath + "/name");
            
            QString devPath = "/dev/" + entry;
            if (QFile::exists(devPath)) {
                info.attributes["device"] = devPath;
            }
            
            info.description = QString("I2C Adapter: %1").arg(info.attributes["name"]);
            
            m_interfaces.append(info);
            emit interfaceFound("I2C", entry);
            
            qInfo() << "  Found:" << entry << "-" << info.attributes["name"];
        }
    }
    
    // 扫描 /sys/class/i2c-dev
    QString i2cDevPath = "/sys/class/i2c-dev";
    if (pathExists(i2cDevPath)) {
        QStringList entries = listDirectory(i2cDevPath);
        for (const QString &entry : entries) {
            qInfo() << "  I2C device:" << entry;
        }
    }
}

void SystemScanner::scanSPI()
{
    qInfo() << "[Scanner] Scanning SPI...";
    
    // 扫描 /sys/class/spi_master
    QString spiPath = "/sys/class/spi_master";
    if (pathExists(spiPath)) {
        QStringList entries = listDirectory(spiPath);
        
        for (const QString &entry : entries) {
            QString fullPath = spiPath + "/" + entry;
            
            InterfaceInfo info;
            info.type = "SPI";
            info.name = entry;
            info.path = fullPath;
            
            QString devName = QString(entry).replace("spi", "spidev");
            QString devPath = "/dev/" + devName;
            if (QFile::exists(devPath)) {
                info.attributes["device"] = devPath;
            }
            
            info.description = QString("SPI Master");
            
            m_interfaces.append(info);
            emit interfaceFound("SPI", entry);
            
            qInfo() << "  Found:" << entry;
        }
    }
}

void SystemScanner::scanCAN()
{
    qInfo() << "[Scanner] Scanning CAN...";
    
    // 扫描网络接口中的CAN设备
    QString netPath = "/sys/class/net";
    if (pathExists(netPath)) {
        QStringList entries = listDirectory(netPath);
        
        for (const QString &entry : entries) {
            QString typePath = netPath + "/" + entry + "/type";
            QString typeStr = readSysFile(typePath);
            
            // CAN设备的type通常是280
            if (typeStr == "280") {
                QString fullPath = netPath + "/" + entry;
                
                InterfaceInfo info;
                info.type = "CAN";
                info.name = entry;
                info.path = fullPath;
                info.attributes["type"] = typeStr;
                info.attributes["operstate"] = readSysFile(fullPath + "/operstate");
                info.attributes["mtu"] = readSysFile(fullPath + "/mtu");
                
                info.description = QString("CAN Interface: %1").arg(info.attributes["operstate"]);
                
                m_interfaces.append(info);
                emit interfaceFound("CAN", entry);
                
                qInfo() << "  Found:" << entry << "- state:" << info.attributes["operstate"];
            }
        }
    }
}

void SystemScanner::scanPWM()
{
    qInfo() << "[Scanner] Scanning PWM...";
    
    // 扫描 /sys/class/pwm
    QString pwmPath = "/sys/class/pwm";
    if (pathExists(pwmPath)) {
        QStringList entries = listDirectory(pwmPath);
        
        for (const QString &entry : entries) {
            if (entry.startsWith("pwmchip")) {
                QString fullPath = pwmPath + "/" + entry;
                
                InterfaceInfo info;
                info.type = "PWM";
                info.name = entry;
                info.path = fullPath;
                info.attributes["npwm"] = readSysFile(fullPath + "/npwm");
                
                info.description = QString("PWM chip: %1 channels").arg(info.attributes["npwm"]);
                
                m_interfaces.append(info);
                emit interfaceFound("PWM", entry);
                
                qInfo() << "  Found:" << entry << "- channels:" << info.attributes["npwm"];
            }
        }
    }
}

void SystemScanner::scanThermal()
{
    qInfo() << "[Scanner] Scanning Thermal zones...";
    
    // 扫描 /sys/class/thermal
    QString thermalPath = "/sys/class/thermal";
    if (pathExists(thermalPath)) {
        QStringList entries = listDirectory(thermalPath);
        
        for (const QString &entry : entries) {
            if (entry.startsWith("thermal_zone")) {
                QString fullPath = thermalPath + "/" + entry;
                
                InterfaceInfo info;
                info.type = "Thermal";
                info.name = entry;
                info.path = fullPath;
                info.attributes["type"] = readSysFile(fullPath + "/type");
                info.attributes["temp"] = readSysFile(fullPath + "/temp");
                
                // 转换温度
                bool ok;
                int tempValue = info.attributes["temp"].toInt(&ok);
                if (ok) {
                    float temp = tempValue / 1000.0f;
                    info.description = QString("Temperature: %1°C (%2)").arg(temp).arg(info.attributes["type"]);
                } else {
                    info.description = info.attributes["type"];
                }
                
                m_interfaces.append(info);
                emit interfaceFound("Thermal", entry);
                
                qInfo() << "  Found:" << entry << "-" << info.description;
            }
        }
    }
}

void SystemScanner::scanLED()
{
    qInfo() << "[Scanner] Scanning LEDs...";
    
    // 扫描 /sys/class/leds
    QString ledPath = "/sys/class/leds";
    if (pathExists(ledPath)) {
        QStringList entries = listDirectory(ledPath);
        
        for (const QString &entry : entries) {
            QString fullPath = ledPath + "/" + entry;
            
            InterfaceInfo info;
            info.type = "LED";
            info.name = entry;
            info.path = fullPath;
            info.attributes["brightness"] = readSysFile(fullPath + "/brightness");
            info.attributes["max_brightness"] = readSysFile(fullPath + "/max_brightness");
            info.attributes["trigger"] = readSysFile(fullPath + "/trigger");
            
            info.description = QString("LED: brightness=%1/%2").arg(
                info.attributes["brightness"], info.attributes["max_brightness"]);
            
            m_interfaces.append(info);
            emit interfaceFound("LED", entry);
            
            qInfo() << "  Found:" << entry << "- brightness:" << info.attributes["brightness"];
        }
    }
}

void SystemScanner::scanInput()
{
    qInfo() << "[Scanner] Scanning Input devices...";
    
    // 扫描 /sys/class/input
    QString inputPath = "/sys/class/input";
    if (pathExists(inputPath)) {
        QStringList entries = listDirectory(inputPath);
        
        for (const QString &entry : entries) {
            if (entry.startsWith("event")) {
                QString fullPath = inputPath + "/" + entry;
                
                InterfaceInfo info;
                info.type = "Input";
                info.name = entry;
                info.path = fullPath;
                
                QString devPath = "/dev/input/" + entry;
                if (QFile::exists(devPath)) {
                    info.attributes["device"] = devPath;
                }
                
                // 尝试读取设备名称
                QString namePath = fullPath + "/device/name";
                if (QFile::exists(namePath)) {
                    info.attributes["name"] = readSysFile(namePath);
                    info.description = info.attributes["name"];
                }
                
                m_interfaces.append(info);
                emit interfaceFound("Input", entry);
                
                qInfo() << "  Found:" << entry << "-" << info.attributes["name"];
            }
        }
    }
}

void SystemScanner::scanNetwork()
{
    qInfo() << "[Scanner] Scanning Network interfaces...";
    
    // 扫描 /sys/class/net
    QString netPath = "/sys/class/net";
    if (pathExists(netPath)) {
        QStringList entries = listDirectory(netPath);
        
        for (const QString &entry : entries) {
            QString fullPath = netPath + "/" + entry;
            
            InterfaceInfo info;
            info.type = "Network";
            info.name = entry;
            info.path = fullPath;
            info.attributes["address"] = readSysFile(fullPath + "/address");
            info.attributes["operstate"] = readSysFile(fullPath + "/operstate");
            info.attributes["type"] = readSysFile(fullPath + "/type");
            info.attributes["mtu"] = readSysFile(fullPath + "/mtu");
            
            // 判断接口类型
            QString ifType = "Unknown";
            if (info.attributes["type"] == "1") ifType = "Ethernet";
            else if (info.attributes["type"] == "280") ifType = "CAN";
            else if (info.attributes["type"] == "772") ifType = "Loopback";
            
            info.description = QString("%1: %2 (%3)").arg(
                ifType, info.attributes["operstate"], info.attributes["address"]);
            
            m_interfaces.append(info);
            emit interfaceFound("Network", entry);
            
            qInfo() << "  Found:" << entry << "-" << info.description;
        }
    }
}

/**
 * @brief 扫描串口设备
 * 
 * 功能说明:
 *   使用QSerialPortInfo扫描系统中所有可用的串口设备
 *   包括USB串口、板载串口等
 * 
 * 串口信息包括:
 *   - 串口名称和路径
 *   - 描述信息
 *   - 厂商和产品ID
 *   - 序列号
 *   - 是否繁忙
 */
void SystemScanner::scanSerialPorts()
{
    qInfo() << "[Scanner] Scanning Serial Ports...";
    
    // 获取所有可用串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    if (ports.isEmpty())
    {
        qInfo() << "  未发现串口设备";
        return;
    }
    
    for (const QSerialPortInfo &portInfo : ports)
    {
        InterfaceInfo info;
        info.type = "Serial";
        info.name = portInfo.portName();
        info.path = portInfo.systemLocation();
        
        // 读取串口属性
        info.attributes["portName"] = portInfo.portName();
        info.attributes["systemLocation"] = portInfo.systemLocation();
        info.attributes["description"] = portInfo.description();
        info.attributes["manufacturer"] = portInfo.manufacturer();
        info.attributes["serialNumber"] = portInfo.serialNumber();
        info.attributes["busy"] = portInfo.isBusy() ? "yes" : "no";
        
        // VID/PID信息
        if (portInfo.hasVendorIdentifier())
        {
            info.attributes["vendorId"] = QString("0x%1")
                .arg(portInfo.vendorIdentifier(), 4, 16, QChar('0'));
        }
        
        if (portInfo.hasProductIdentifier())
        {
            info.attributes["productId"] = QString("0x%1")
                .arg(portInfo.productIdentifier(), 4, 16, QChar('0'));
        }
        
        // 构造描述信息
        QString desc = portInfo.description();
        if (!portInfo.manufacturer().isEmpty())
        {
            desc += QString(" (%1)").arg(portInfo.manufacturer());
        }
        info.description = desc;
        
        m_interfaces.append(info);
        emit interfaceFound("Serial", portInfo.portName());
        
        qInfo() << "  Found:" << portInfo.portName() 
                << "-" << portInfo.description()
                << "@" << portInfo.systemLocation();
    }
}

void SystemScanner::scanAll()
{
    qInfo() << "========================================";
    qInfo() << "  System Interface Scanner";
    qInfo() << "========================================";
    
    emit scanStarted();
    
    m_interfaces.clear();
    
    // 扫描各类接口
    scanGPIO();
    scanI2C();
    scanSPI();
    scanCAN();
    scanPWM();
    scanThermal();
    scanLED();
    scanInput();
    scanNetwork();
    scanSerialPorts();  // 新增：扫描串口
    
    qInfo() << "========================================";
    qInfo() << "Scan completed. Total interfaces found:" << m_interfaces.size();
    qInfo() << "========================================";
    
    emit scanCompleted(m_interfaces.size());
}

QVector<InterfaceInfo> SystemScanner::getAllInterfaces() const
{
    return m_interfaces;
}

QVector<InterfaceInfo> SystemScanner::getInterfacesByType(const QString &type) const
{
    QVector<InterfaceInfo> result;
    
    for (const InterfaceInfo &info : m_interfaces) {
        if (info.type == type) {
            result.append(info);
        }
    }
    
    return result;
}

QString SystemScanner::generateReport() const
{
    QString report;
    QTextStream out(&report);
    
    out << "========================================\n";
    out << "  System Interface Scan Report\n";
    out << "========================================\n";
    out << "Total interfaces found: " << m_interfaces.size() << "\n\n";
    
    // 按类型分组
    QMap<QString, QVector<InterfaceInfo>> grouped;
    for (const InterfaceInfo &info : m_interfaces) {
        grouped[info.type].append(info);
    }
    
    // 生成每个类型的报告
    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        QString type = it.key();
        QVector<InterfaceInfo> interfaces = it.value();
        
        out << "----------------------------------------\n";
        out << type << " Interfaces (" << interfaces.size() << ")\n";
        out << "----------------------------------------\n";
        
        for (const InterfaceInfo &info : interfaces) {
            out << "  • " << info.name << "\n";
            out << "    Path: " << info.path << "\n";
            if (!info.description.isEmpty()) {
                out << "    Desc: " << info.description << "\n";
            }
            
            // 打印属性
            for (auto attrIt = info.attributes.begin(); attrIt != info.attributes.end(); ++attrIt) {
                QString key = attrIt.key();
                QString value = attrIt.value();
                if (!value.isEmpty() && value.length() < 100) {
                    out << "    " << key << ": " << value << "\n";
                }
            }
            out << "\n";
        }
    }
    
    out << "========================================\n";
    
    return report;
}

void SystemScanner::printReport() const
{
    QString report = generateReport();
    qInfo().noquote() << report;
}

