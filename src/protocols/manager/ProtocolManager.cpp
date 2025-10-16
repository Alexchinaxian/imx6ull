/***************************************************************
 * Copyright: Alex
 * FileName: protocol_manager.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 协议管理器实现
 ***************************************************************/

#include "protocols/manager/ProtocolManager.h"
#include "protocols/modbus/ModbusRTU.h"
#include "protocols/modbus/ModbusTCP.h"
#include "protocols/modbus/ModbusSlave.h"
#include <QDebug>

// 静态成员初始化
ProtocolManager* ProtocolManager::m_instance = nullptr;

/***************************************************************
 * 获取单例实例
 ***************************************************************/
ProtocolManager* ProtocolManager::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new ProtocolManager();
    }
    return m_instance;
}

/***************************************************************
 * 构造函数
 ***************************************************************/
ProtocolManager::ProtocolManager(QObject *parent)
    : QObject(parent)
{
    qInfo() << "ProtocolManager initialized";
}

/***************************************************************
 * 析构函数
 ***************************************************************/
ProtocolManager::~ProtocolManager()
{
    destroyAllProtocols();
    qInfo() << "ProtocolManager destroyed";
}

/***************************************************************
 * 创建Modbus RTU协议实例
 ***************************************************************/
bool ProtocolManager::createModbusRTU(const QString &name, const QString &portName)
{
    if (hasProtocol(name)) {
        qWarning() << "Protocol already exists:" << name;
        return false;
    }
    
    ProtocolModbusRTU *protocol = new ProtocolModbusRTU(portName, this);
    
    if (registerProtocol(name, protocol)) {
        qInfo() << "Created Modbus RTU protocol:" << name << "Port:" << portName;
        emit protocolCreated(name, ProtocolType::ModbusRTU);
        return true;
    }
    
    delete protocol;
    return false;
}

/***************************************************************
 * 创建Modbus TCP协议实例
 ***************************************************************/
bool ProtocolManager::createModbusTCP(const QString &name, const QString &host, quint16 port)
{
    if (hasProtocol(name)) {
        qWarning() << "Protocol already exists:" << name;
        return false;
    }
    
    ProtocolModbusTCP *protocol = new ProtocolModbusTCP(host, port, this);
    
    if (registerProtocol(name, protocol)) {
        qInfo() << "Created Modbus TCP protocol:" << name << "Host:" << host << "Port:" << port;
        emit protocolCreated(name, ProtocolType::ModbusTCP);
        return true;
    }
    
    delete protocol;
    return false;
}

/***************************************************************
 * 创建Modbus RTU从站实例
 ***************************************************************/
bool ProtocolManager::createModbusSlave(const QString &name, const QString &portName, 
                                        quint8 slaveAddress)
{
    if (hasProtocol(name)) {
        qWarning() << "Protocol already exists:" << name;
        return false;
    }
    
    ProtocolModbusSlave *protocol = new ProtocolModbusSlave(portName, slaveAddress, this);
    
    if (registerProtocol(name, protocol)) {
        qInfo() << "Created Modbus RTU Slave:" << name 
                << "Port:" << portName 
                << "Address:" << slaveAddress;
        emit protocolCreated(name, ProtocolType::ModbusRTU);
        return true;
    }
    
    delete protocol;
    return false;
}

/***************************************************************
 * 根据名称获取协议实例
 ***************************************************************/
IProtocolInterface* ProtocolManager::getProtocol(const QString &name)
{
    if (m_protocols.contains(name)) {
        return m_protocols[name];
    }
    return nullptr;
}

/***************************************************************
 * 根据类型获取所有协议实例
 ***************************************************************/
QList<IProtocolInterface*> ProtocolManager::getProtocolsByType(ProtocolType type)
{
    QList<IProtocolInterface*> result;
    
    for (IProtocolInterface *protocol : m_protocols.values()) {
        if (protocol->getProtocolType() == type) {
            result.append(protocol);
        }
    }
    
    return result;
}

/***************************************************************
 * 获取所有协议实例
 ***************************************************************/
QList<IProtocolInterface*> ProtocolManager::getAllProtocols() const
{
    return m_protocols.values();
}

/***************************************************************
 * 检查协议是否存在
 ***************************************************************/
bool ProtocolManager::hasProtocol(const QString &name) const
{
    return m_protocols.contains(name);
}

/***************************************************************
 * 销毁协议实例
 ***************************************************************/
bool ProtocolManager::destroyProtocol(const QString &name)
{
    if (!hasProtocol(name)) {
        qWarning() << "Protocol not found:" << name;
        return false;
    }
    
    IProtocolInterface *protocol = m_protocols[name];
    
    // 断开连接
    if (protocol->isConnected()) {
        protocol->disconnect();
    }
    
    // 从映射表中移除
    m_protocols.remove(name);
    
    // 删除对象
    protocol->deleteLater();
    
    qInfo() << "Destroyed protocol:" << name;
    emit protocolDestroyed(name);
    
    return true;
}

/***************************************************************
 * 销毁所有协议实例
 ***************************************************************/
void ProtocolManager::destroyAllProtocols()
{
    QStringList names = m_protocols.keys();
    
    for (const QString &name : names) {
        destroyProtocol(name);
    }
    
    qInfo() << "All protocols destroyed";
}

/***************************************************************
 * 获取协议数量
 ***************************************************************/
int ProtocolManager::getProtocolCount() const
{
    return m_protocols.size();
}

/***************************************************************
 * 连接所有协议
 ***************************************************************/
void ProtocolManager::connectAll()
{
    for (auto it = m_protocols.begin(); it != m_protocols.end(); ++it) {
        IProtocolInterface *protocol = it.value();
        if (!protocol->isConnected()) {
            protocol->connect();
        }
    }
}

/***************************************************************
 * 断开所有协议
 ***************************************************************/
void ProtocolManager::disconnectAll()
{
    for (auto it = m_protocols.begin(); it != m_protocols.end(); ++it) {
        IProtocolInterface *protocol = it.value();
        if (protocol->isConnected()) {
            protocol->disconnect();
        }
    }
}

/***************************************************************
 * 注册协议实例
 ***************************************************************/
bool ProtocolManager::registerProtocol(const QString &name, IProtocolInterface *protocol)
{
    if (protocol == nullptr) {
        qCritical() << "Cannot register null protocol";
        return false;
    }
    
    if (hasProtocol(name)) {
        qWarning() << "Protocol already registered:" << name;
        return false;
    }
    
    m_protocols[name] = protocol;
    
    // 连接协议信号
    connectProtocolSignals(name, protocol);
    
    return true;
}

/***************************************************************
 * 连接协议信号
 ***************************************************************/
void ProtocolManager::connectProtocolSignals(const QString &name, IProtocolInterface *protocol)
{
    // 连接信号
    QObject::connect(protocol, &IProtocolInterface::connected, this, [this, name]() {
        qInfo() << "Protocol connected:" << name;
        emit protocolConnected(name);
    });
    
    QObject::connect(protocol, &IProtocolInterface::disconnected, this, [this, name]() {
        qInfo() << "Protocol disconnected:" << name;
        emit protocolDisconnected(name);
    });
    
    QObject::connect(protocol, &IProtocolInterface::error, this, [this, name](const QString &error) {
        qWarning() << "Protocol error [" << name << "]:" << error;
        emit protocolError(name, error);
    });
}

