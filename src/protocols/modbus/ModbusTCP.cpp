/***************************************************************
 * Copyright: Alex
 * FileName: protocol_modbus_tcp.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus TCP协议驱动实现
 ***************************************************************/

#include "protocols/modbus/ModbusTCP.h"
#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QCoreApplication>

/***************************************************************
 * 构造函数
 ***************************************************************/
ProtocolModbusTCP::ProtocolModbusTCP(const QString &host, quint16 port, QObject *parent)
    : IProtocolInterface(parent)
    , m_host(host)
    , m_port(port)
    , m_unitId(1)
    , m_timeout(3000)
    , m_waitingForResponse(false)
    , m_transactionId(0)
{
    // 创建TCP套接字
    m_tcpSocket = new QTcpSocket(this);
    
    // 创建超时定时器
    m_responseTimer = new QTimer(this);
    m_responseTimer->setSingleShot(true);
    
    // 连接信号槽
    QObject::connect(m_tcpSocket, &QTcpSocket::connected, 
            this, &ProtocolModbusTCP::onTcpConnected);
    QObject::connect(m_tcpSocket, &QTcpSocket::disconnected, 
            this, &ProtocolModbusTCP::onTcpDisconnected);
    QObject::connect(m_tcpSocket, &QTcpSocket::readyRead, 
            this, &ProtocolModbusTCP::onTcpDataReceived);
    QObject::connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &ProtocolModbusTCP::onTcpError);
    QObject::connect(m_responseTimer, &QTimer::timeout, 
            this, &ProtocolModbusTCP::onResponseTimeout);
}

/***************************************************************
 * 析构函数
 ***************************************************************/
ProtocolModbusTCP::~ProtocolModbusTCP()
{
    disconnect();
}

/***************************************************************
 * 连接到服务器
 ***************************************************************/
bool ProtocolModbusTCP::connect()
{
    if (isConnected()) {
        qWarning() << "Modbus TCP already connected";
        return true;
    }
    
    setState(ProtocolState::Connecting);
    m_tcpSocket->connectToHost(m_host, m_port);
    
    // 等待连接完成（最多5秒）
    if (!m_tcpSocket->waitForConnected(5000)) {
        QString error = QString("Failed to connect: %1").arg(m_tcpSocket->errorString());
        setError(error);
        setState(ProtocolState::Disconnected);
        return false;
    }
    
    return true;
}

/***************************************************************
 * 断开连接
 ***************************************************************/
void ProtocolModbusTCP::disconnect()
{
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
        if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            m_tcpSocket->waitForDisconnected(1000);
        }
    }
}

/***************************************************************
 * 检查是否已连接
 ***************************************************************/
bool ProtocolModbusTCP::isConnected() const
{
    return m_tcpSocket->state() == QAbstractSocket::ConnectedState;
}

/***************************************************************
 * 配置协议参数
 ***************************************************************/
bool ProtocolModbusTCP::configure(const QMap<QString, QVariant> &config)
{
    // 服务器地址
    if (config.contains("host")) {
        m_host = config["host"].toString();
    }
    
    // 服务器端口
    if (config.contains("port")) {
        m_port = config["port"].toUInt();
    }
    
    // 单元ID
    if (config.contains("unit_id")) {
        m_unitId = config["unit_id"].toUInt();
    }
    
    // 超时时间
    if (config.contains("timeout")) {
        m_timeout = config["timeout"].toInt();
    }
    
    return true;
}

/***************************************************************
 * 设置单元ID
 ***************************************************************/
void ProtocolModbusTCP::setUnitId(quint8 unitId)
{
    m_unitId = unitId;
}

/***************************************************************
 * 设置超时时间
 ***************************************************************/
void ProtocolModbusTCP::setTimeout(int timeout)
{
    m_timeout = timeout;
}

/***************************************************************
 * 读取线圈状态
 ***************************************************************/
QVector<bool> ProtocolModbusTCP::readCoils(quint16 startAddress, quint16 count)
{
    if (!isConnected() || count == 0 || count > 2000) {
        return QVector<bool>();
    }
    
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    
    QByteArray response = sendRequest(buildRequest(0x01, data));
    
    if (response.isEmpty() || response.size() < 10) {
        return QVector<bool>();
    }
    
    // 跳过MBAP头(7字节) + 功能码(1字节) + 字节数(1字节)
    quint8 byteCount = static_cast<quint8>(response[8]);
    QByteArray coilData = response.mid(9, byteCount);
    
    return parseBooleans(coilData, count);
}

/***************************************************************
 * 读取离散输入状态
 ***************************************************************/
QVector<bool> ProtocolModbusTCP::readDiscreteInputs(quint16 startAddress, quint16 count)
{
    if (!isConnected() || count == 0 || count > 2000) {
        return QVector<bool>();
    }
    
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    
    QByteArray response = sendRequest(buildRequest(0x02, data));
    
    if (response.isEmpty() || response.size() < 10) {
        return QVector<bool>();
    }
    
    quint8 byteCount = static_cast<quint8>(response[8]);
    QByteArray inputData = response.mid(9, byteCount);
    
    return parseBooleans(inputData, count);
}

/***************************************************************
 * 读取保持寄存器
 ***************************************************************/
QVector<quint16> ProtocolModbusTCP::readHoldingRegisters(quint16 startAddress, quint16 count)
{
    if (!isConnected() || count == 0 || count > 125) {
        return QVector<quint16>();
    }
    
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    
    QByteArray response = sendRequest(buildRequest(0x03, data));
    
    if (response.isEmpty() || response.size() < 10) {
        return QVector<quint16>();
    }
    
    quint8 byteCount = static_cast<quint8>(response[8]);
    QByteArray registerData = response.mid(9, byteCount);
    
    return parseRegisters(registerData);
}

/***************************************************************
 * 读取输入寄存器
 ***************************************************************/
QVector<quint16> ProtocolModbusTCP::readInputRegisters(quint16 startAddress, quint16 count)
{
    if (!isConnected() || count == 0 || count > 125) {
        return QVector<quint16>();
    }
    
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    
    QByteArray response = sendRequest(buildRequest(0x04, data));
    
    if (response.isEmpty() || response.size() < 10) {
        return QVector<quint16>();
    }
    
    quint8 byteCount = static_cast<quint8>(response[8]);
    QByteArray registerData = response.mid(9, byteCount);
    
    return parseRegisters(registerData);
}

/***************************************************************
 * 写单个线圈
 ***************************************************************/
bool ProtocolModbusTCP::writeSingleCoil(quint16 address, bool value)
{
    if (!isConnected()) {
        return false;
    }
    
    QByteArray data;
    data.append((address >> 8) & 0xFF);
    data.append(address & 0xFF);
    data.append(value ? 0xFF : 0x00);
    data.append(static_cast<char>(0x00));
    
    QByteArray response = sendRequest(buildRequest(0x05, data));
    
    return !response.isEmpty();
}

/***************************************************************
 * 写单个寄存器
 ***************************************************************/
bool ProtocolModbusTCP::writeSingleRegister(quint16 address, quint16 value)
{
    if (!isConnected()) {
        return false;
    }
    
    QByteArray data;
    data.append((address >> 8) & 0xFF);
    data.append(address & 0xFF);
    data.append((value >> 8) & 0xFF);
    data.append(value & 0xFF);
    
    QByteArray response = sendRequest(buildRequest(0x06, data));
    
    return !response.isEmpty();
}

/***************************************************************
 * 写多个线圈
 ***************************************************************/
bool ProtocolModbusTCP::writeMultipleCoils(quint16 startAddress, const QVector<bool> &values)
{
    if (!isConnected() || values.isEmpty() || values.size() > 1968) {
        return false;
    }
    
    quint16 count = values.size();
    quint8 byteCount = (count + 7) / 8;
    
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    data.append(byteCount);
    
    for (int i = 0; i < byteCount; i++) {
        quint8 byte = 0;
        for (int j = 0; j < 8 && (i * 8 + j) < count; j++) {
            if (values[i * 8 + j]) {
                byte |= (1 << j);
            }
        }
        data.append(byte);
    }
    
    QByteArray response = sendRequest(buildRequest(0x0F, data));
    
    return !response.isEmpty();
}

/***************************************************************
 * 写多个寄存器
 ***************************************************************/
bool ProtocolModbusTCP::writeMultipleRegisters(quint16 startAddress, const QVector<quint16> &values)
{
    if (!isConnected() || values.isEmpty() || values.size() > 123) {
        return false;
    }
    
    quint16 count = values.size();
    quint8 byteCount = count * 2;
    
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    data.append(byteCount);
    
    for (quint16 value : values) {
        data.append((value >> 8) & 0xFF);
        data.append(value & 0xFF);
    }
    
    QByteArray response = sendRequest(buildRequest(0x10, data));
    
    return !response.isEmpty();
}

/***************************************************************
 * TCP连接成功槽函数
 ***************************************************************/
void ProtocolModbusTCP::onTcpConnected()
{
    setState(ProtocolState::Connected);
    emit connected();
    qInfo() << "Modbus TCP connected:" << m_host << ":" << m_port << "Unit:" << m_unitId;
}

/***************************************************************
 * TCP断开连接槽函数
 ***************************************************************/
void ProtocolModbusTCP::onTcpDisconnected()
{
    setState(ProtocolState::Disconnected);
    emit disconnected();
    qInfo() << "Modbus TCP disconnected";
}

/***************************************************************
 * TCP数据接收槽函数
 ***************************************************************/
void ProtocolModbusTCP::onTcpDataReceived()
{
    if (!m_waitingForResponse) {
        return;
    }
    
    m_receiveBuffer.append(m_tcpSocket->readAll());
}

/***************************************************************
 * TCP错误槽函数
 ***************************************************************/
void ProtocolModbusTCP::onTcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QString errorMsg = m_tcpSocket->errorString();
    setError(errorMsg);
    setState(ProtocolState::Error);
}

/***************************************************************
 * 响应超时槽函数
 ***************************************************************/
void ProtocolModbusTCP::onResponseTimeout()
{
    if (m_waitingForResponse) {
        setError("Response timeout");
        m_waitingForResponse = false;
    }
}

/***************************************************************
 * 发送Modbus TCP请求并等待响应
 ***************************************************************/
QByteArray ProtocolModbusTCP::sendRequest(const QByteArray &request)
{
    if (!isConnected()) {
        return QByteArray();
    }
    
    m_receiveBuffer.clear();
    m_waitingForResponse = true;
    
    // 获取当前事务ID
    quint16 currentTransactionId = m_transactionId - 1;
    
    // 发送请求
    m_tcpSocket->write(request);
    m_tcpSocket->flush();
    
    // 启动超时定时器
    m_responseTimer->start(m_timeout);
    
    // 等待响应
    while (m_waitingForResponse && m_responseTimer->isActive()) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
        
        if (m_receiveBuffer.size() >= 8) {
            // 检查MBAP头中的长度字段
            quint16 length = (static_cast<quint8>(m_receiveBuffer[4]) << 8) | 
                            static_cast<quint8>(m_receiveBuffer[5]);
            
            if (m_receiveBuffer.size() >= (6 + length)) {
                if (validateResponse(m_receiveBuffer, currentTransactionId)) {
                    m_waitingForResponse = false;
                    m_responseTimer->stop();
                    break;
                }
            }
        }
    }
    
    m_waitingForResponse = false;
    m_responseTimer->stop();
    
    return m_receiveBuffer;
}

/***************************************************************
 * 构建Modbus TCP请求帧
 ***************************************************************/
QByteArray ProtocolModbusTCP::buildRequest(quint8 functionCode, const QByteArray &data)
{
    QByteArray request;
    
    // MBAP头
    request.append((m_transactionId >> 8) & 0xFF);  // 事务ID高字节
    request.append(m_transactionId & 0xFF);         // 事务ID低字节
    request.append(static_cast<char>(0x00));        // 协议ID高字节（固定为0）
    request.append(static_cast<char>(0x00));        // 协议ID低字节（固定为0）
    
    quint16 length = data.size() + 2;               // 长度 = 单元ID + 功能码 + 数据
    request.append((length >> 8) & 0xFF);           // 长度高字节
    request.append(length & 0xFF);                  // 长度低字节
    request.append(m_unitId);                       // 单元ID
    
    // PDU（协议数据单元）
    request.append(functionCode);                   // 功能码
    request.append(data);                           // 数据
    
    m_transactionId++;  // 递增事务ID
    
    return request;
}

/***************************************************************
 * 验证响应帧
 ***************************************************************/
bool ProtocolModbusTCP::validateResponse(const QByteArray &response, quint16 expectedTransactionId)
{
    if (response.size() < 8) {
        return false;
    }
    
    // 检查事务ID
    quint16 transactionId = (static_cast<quint8>(response[0]) << 8) | 
                           static_cast<quint8>(response[1]);
    if (transactionId != expectedTransactionId) {
        return false;
    }
    
    // 检查协议ID（应该为0）
    if (response[2] != 0x00 || response[3] != 0x00) {
        return false;
    }
    
    // 检查单元ID
    if (static_cast<quint8>(response[6]) != m_unitId) {
        return false;
    }
    
    // 检查异常响应
    quint8 functionCode = static_cast<quint8>(response[7]);
    if (functionCode & 0x80) {
        quint8 exceptionCode = static_cast<quint8>(response[8]);
        emit modbusException(exceptionCode);
        setError(QString("Modbus exception: 0x%1").arg(exceptionCode, 2, 16, QChar('0')));
        return false;
    }
    
    return true;
}

/***************************************************************
 * 解析布尔值列表
 ***************************************************************/
QVector<bool> ProtocolModbusTCP::parseBooleans(const QByteArray &data, quint16 count)
{
    QVector<bool> result;
    
    for (int i = 0; i < count; i++) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        
        if (byteIndex < data.size()) {
            quint8 byte = static_cast<quint8>(data[byteIndex]);
            result.append((byte & (1 << bitIndex)) != 0);
        }
    }
    
    return result;
}

/***************************************************************
 * 解析寄存器值列表
 ***************************************************************/
QVector<quint16> ProtocolModbusTCP::parseRegisters(const QByteArray &data)
{
    QVector<quint16> result;
    
    for (int i = 0; i < data.size(); i += 2) {
        if (i + 1 < data.size()) {
            quint16 value = (static_cast<quint8>(data[i]) << 8) | 
                           static_cast<quint8>(data[i + 1]);
            result.append(value);
        }
    }
    
    return result;
}

