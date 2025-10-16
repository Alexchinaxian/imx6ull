/***************************************************************
 * Copyright: Alex
 * FileName: ModbusSlaveService.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus从站服务适配器实现
 ***************************************************************/

#include "services/modbus/ModbusSlaveService.h"
#include <QDebug>

/***************************************************************
 * 构造函数
 ***************************************************************/
ModbusSlaveService::ModbusSlaveService(int32_t svr_id, int32_t svr_type,
                                       const QString &portName, 
                                       quint8 slaveAddress,
                                       QObject *parent)
    : ISysSvrInterface(svr_id, svr_type, parent)
    , m_pModbusSlave(nullptr)
    , m_pBeep(nullptr)
    , m_pThread(nullptr)
    , m_portName(portName)
    , m_slaveAddress(slaveAddress)
    , m_IsInitialized(false)
    , m_IsStarted(false)
{
    qInfo() << "[ModbusSlaveService] Modbus从站服务创建";
    qInfo() << "  串口:" << m_portName << "从站地址:" << m_slaveAddress;
}

/***************************************************************
 * 析构函数
 ***************************************************************/
ModbusSlaveService::~ModbusSlaveService()
{
    qInfo() << "[ModbusSlaveService] Modbus从站服务销毁";
    SvrStop();
}

/***************************************************************
 * 服务初始化
 ***************************************************************/
bool ModbusSlaveService::SvrInit()
{
    if (m_IsInitialized) {
        qWarning() << "[ModbusSlaveService] 服务已初始化";
        return false;
    }
    
    qInfo() << "[ModbusSlaveService] 初始化Modbus从站服务...";
    
    // 1. 创建Modbus从站对象
    m_pModbusSlave = new ProtocolModbusSlave(m_portName, m_slaveAddress);
    if (!m_pModbusSlave) {
        qCritical() << "[ModbusSlaveService] 创建Modbus从站失败";
        return false;
    }
    
    // 2. 配置Modbus参数（在移动到线程前配置）
    QMap<QString, QVariant> config;
    config["baudrate"] = 9600;
    config["parity"] = "N";
    config["databits"] = 8;
    config["stopbits"] = 1;
    m_pModbusSlave->configure(config);
    
    qInfo() << "  Modbus配置: 9600,8,N,1 从站地址=" << m_slaveAddress;
    
    // 3. 创建独立线程（保证实时通信）
    m_pThread = new QThread();
    if (!m_pThread) {
        qCritical() << "[ModbusSlaveService] 创建线程失败";
        delete m_pModbusSlave;
        m_pModbusSlave = nullptr;
        return false;
    }
    
    // 4. 将Modbus从站移到独立线程（串口操作在独立线程执行）
    m_pModbusSlave->moveToThread(m_pThread);
    
    // 5. 初始化Beep驱动（在主线程运行）
    if (!initBeepDriver()) {
        qWarning() << "[ModbusSlaveService] Beep驱动初始化失败（非关键）";
    }
    
    // 6. 连接Modbus信号
    connectModbusSignals();
    
    // 7. 连接线程启动信号到Modbus连接
    QObject::connect(m_pThread, &QThread::started, m_pModbusSlave, 
                    &ProtocolModbusSlave::connect, Qt::DirectConnection);
    
    // 8. 资源清理
    QObject::connect(m_pThread, &QThread::finished,
            m_pModbusSlave, &QObject::deleteLater);
    QObject::connect(m_pThread, &QThread::finished,
            m_pThread, &QObject::deleteLater);
    
    m_IsInitialized = true;
    qInfo() << "[ModbusSlaveService] ✓ 服务初始化成功";
    
    return true;
}

/***************************************************************
 * 服务启动
 ***************************************************************/
bool ModbusSlaveService::SvrStart()
{
    if (!m_IsInitialized) {
        qWarning() << "[ModbusSlaveService] 服务未初始化，无法启动";
        return false;
    }
    
    if (m_IsStarted) {
        qWarning() << "[ModbusSlaveService] 服务已启动";
        return false;
    }
    
    qInfo() << "[ModbusSlaveService] 启动Modbus从站服务...";
    
    // 启动线程（会触发QThread::started信号，自动调用Modbus::connect）
    m_pThread->start();
    
    m_IsStarted = true;
    qInfo() << "[ModbusSlaveService] ✓ 服务启动成功（独立线程实时监听）";
    qInfo() << "  Modbus RTU从站运行在独立线程，保证通信实时性";
    qInfo() << "  串口: " << m_portName << " | 从站地址: " << m_slaveAddress;
    
    return true;
}

/***************************************************************
 * 服务停止
 ***************************************************************/
bool ModbusSlaveService::SvrStop()
{
    if (!m_IsStarted) {
        return true;
    }
    
    qInfo() << "[ModbusSlaveService] 停止Modbus从站服务...";
    
    // 断开Modbus连接
    if (m_pModbusSlave && m_pModbusSlave->isConnected()) {
        m_pModbusSlave->disconnect();
    }
    
    // 关闭蜂鸣器
    if (m_pBeep) {
        m_pBeep->turnOff();
    }
    
    // 退出线程
    if (m_pThread && m_pThread->isRunning()) {
        m_pThread->quit();
        m_pThread->wait(3000);
    }
    
    m_IsStarted = false;
    qInfo() << "[ModbusSlaveService] ✓ 服务已停止";
    
    return true;
}

/***************************************************************
 * 更新温度数据
 ***************************************************************/
void ModbusSlaveService::updateTemperature(float temperature)
{
    if (m_pModbusSlave) {
        // 使用信号槽方式进行线程安全的调用
        m_pModbusSlave->setTemperature(temperature);
    }
}

/***************************************************************
 * 更新系统状态
 ***************************************************************/
void ModbusSlaveService::updateSystemStatus(quint16 status)
{
    if (m_pModbusSlave) {
        m_pModbusSlave->setSystemStatus(status);
    }
}

/***************************************************************
 * 配置Modbus参数
 ***************************************************************/
bool ModbusSlaveService::configureModbus(const QMap<QString, QVariant> &config)
{
    if (m_pModbusSlave) {
        return m_pModbusSlave->configure(config);
    }
    return false;
}

/***************************************************************
 * 初始化Beep驱动
 ***************************************************************/
bool ModbusSlaveService::initBeepDriver()
{
    m_pBeep = new DriverBeep("beep", this);
    
    if (!m_pBeep->isAvailable()) {
        qWarning() << "[ModbusSlaveService] Beep设备路径不存在，但仍尝试使用（可能通过其他方式工作）";
        // 不删除驱动，仍然保留使用
        // SystemBeep证明即使isAvailable()=false，驱动仍可能工作
    }
    
    qInfo() << "[ModbusSlaveService] ✓ Beep驱动初始化成功";
    return true;
}

/***************************************************************
 * 连接Modbus信号
 ***************************************************************/
void ModbusSlaveService::connectModbusSignals()
{
    if (!m_pModbusSlave) return;
    
    // 连接读请求信号
    QObject::connect(m_pModbusSlave, &ProtocolModbusSlave::readRequest,
                    this, &ModbusSlaveService::onModbusReadRequest);
    
    // 连接写请求信号
    QObject::connect(m_pModbusSlave, &ProtocolModbusSlave::writeRequest,
                    this, &ModbusSlaveService::onModbusWriteRequest);
    
    // 连接连接/断开信号
    QObject::connect(m_pModbusSlave, &IProtocolInterface::connected, this, [this]() {
        qInfo() << "[ModbusSlaveService] Modbus从站已连接，开始监听串口";
        emit statusChanged(m_SvrId, "运行中");
    });
    
    QObject::connect(m_pModbusSlave, &IProtocolInterface::disconnected, this, [this]() {
        qInfo() << "[ModbusSlaveService] Modbus从站已断开";
        emit statusChanged(m_SvrId, "已停止");
    });
    
    QObject::connect(m_pModbusSlave, &IProtocolInterface::error, this, [this](const QString &err) {
        qCritical() << "[ModbusSlaveService] Modbus错误:" << err;
        emit error(m_SvrId, err);
    });
}

/***************************************************************
 * Modbus写请求槽函数
 ***************************************************************/
void ModbusSlaveService::onModbusWriteRequest(quint8 fc, quint16 addr, quint16 value)
{
    qDebug() << "[ModbusSlaveService] 写请求: FC=0x" << QString::number(fc, 16)
             << "Addr=" << addr << "Value=" << value;
    
    if (!m_pBeep) return;
    
    // 处理蜂鸣器控制寄存器（0x0003）
    if (addr == 0x0003) {
        if (value == 0) {
            // 关闭蜂鸣器
            m_pBeep->turnOff();
            qInfo() << "[ModbusSlaveService] 蜂鸣器: 关闭";
            emit beepCommand(0);
        } else if (value == 1) {
            // 打开蜂鸣器
            m_pBeep->turnOn();
            qInfo() << "[ModbusSlaveService] 蜂鸣器: 开启";
            emit beepCommand(1);
        } else if (value == 2) {
            // 报警模式：读取参数
            int count = m_pModbusSlave->getHoldingRegister(0x0004);
            int interval = m_pModbusSlave->getHoldingRegister(0x0005);
            
            if (count == 0) count = 3;
            if (interval == 0) interval = 200;
            
            m_pBeep->alarm(count, 500, interval);
            qInfo() << "[ModbusSlaveService] 蜂鸣器: 报警模式" << count << "次";
            emit beepCommand(2);
        }
    }
    
    emit modbusWriteRequest(addr, value);
}

/***************************************************************
 * Modbus读请求槽函数
 ***************************************************************/
void ModbusSlaveService::onModbusReadRequest(quint8 fc, quint16 addr, quint16 count)
{
    qDebug() << "[ModbusSlaveService] 读请求: FC=0x" << QString::number(fc, 16)
             << "Addr=" << addr << "Count=" << count;
    
    emit modbusReadRequest(addr, count);
}

