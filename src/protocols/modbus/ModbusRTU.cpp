/***************************************************************
 * Copyright: Alex
 * FileName: protocol_modbus_rtu.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus RTU协议驱动实现
 ***************************************************************/

#include "protocols/modbus/ModbusRTU.h"
#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QCoreApplication>

/***************************************************************
 * 构造函数
 ***************************************************************/
ProtocolModbusRTU::ProtocolModbusRTU(const QString &portName, QObject *parent)
    : IProtocolInterface(parent)
    , m_portName(portName)
    , m_slaveAddress(1)
    , m_timeout(1000)
    , m_waitingForResponse(false)
    , m_baudrate(9600)
    , m_dataBits(QSerialPort::Data8)
    , m_parity(QSerialPort::NoParity)
    , m_stopBits(QSerialPort::OneStop)
{
    // 创建串口对象
    m_serialPort = new QSerialPort(this);
    m_serialPort->setPortName(m_portName);
    
    // 创建超时定时器
    m_responseTimer = new QTimer(this);
    m_responseTimer->setSingleShot(true);
    
    // 连接信号槽
    QObject::connect(m_serialPort, &QSerialPort::readyRead, 
            this, &ProtocolModbusRTU::onSerialDataReceived);
    QObject::connect(m_responseTimer, &QTimer::timeout, 
            this, &ProtocolModbusRTU::onResponseTimeout);
}

/***************************************************************
 * 析构函数
 ***************************************************************/
ProtocolModbusRTU::~ProtocolModbusRTU()
{
    disconnect();
}

/***************************************************************
 * 连接串口
 ***************************************************************/
bool ProtocolModbusRTU::connect()
{
    if (isConnected()) {
        qWarning() << "Modbus RTU already connected";
        return true;
    }
    
    // 配置串口参数
    m_serialPort->setBaudRate(m_baudrate);
    m_serialPort->setDataBits(m_dataBits);
    m_serialPort->setParity(m_parity);
    m_serialPort->setStopBits(m_stopBits);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    // 打开串口
    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        QString error = QString("Failed to open serial port: %1").arg(m_serialPort->errorString());
        setError(error);
        return false;
    }
    
    setState(ProtocolState::Connected);
    emit connected();
    
    qInfo() << "Modbus RTU connected:" << m_portName 
            << "Baudrate:" << m_baudrate
            << "Slave:" << m_slaveAddress;
    
    return true;
}

/***************************************************************
 * 断开连接
 ***************************************************************/
void ProtocolModbusRTU::disconnect()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        setState(ProtocolState::Disconnected);
        emit disconnected();
        qInfo() << "Modbus RTU disconnected";
    }
}

/***************************************************************
 * 检查是否已连接
 ***************************************************************/
bool ProtocolModbusRTU::isConnected() const
{
    return m_serialPort->isOpen();
}

/***************************************************************
 * 配置协议参数
 ***************************************************************/
bool ProtocolModbusRTU::configure(const QMap<QString, QVariant> &config)
{
    bool needReconnect = isConnected();
    
    if (needReconnect) {
        disconnect();
    }
    
    // 波特率
    if (config.contains("baudrate")) {
        m_baudrate = config["baudrate"].toInt();
    }
    
    // 数据位
    if (config.contains("databits")) {
        int bits = config["databits"].toInt();
        switch (bits) {
            case 5: m_dataBits = QSerialPort::Data5; break;
            case 6: m_dataBits = QSerialPort::Data6; break;
            case 7: m_dataBits = QSerialPort::Data7; break;
            case 8: m_dataBits = QSerialPort::Data8; break;
            default: m_dataBits = QSerialPort::Data8; break;
        }
    }
    
    // 校验位
    if (config.contains("parity")) {
        QString parity = config["parity"].toString().toUpper();
        if (parity == "N" || parity == "NONE") {
            m_parity = QSerialPort::NoParity;
        } else if (parity == "E" || parity == "EVEN") {
            m_parity = QSerialPort::EvenParity;
        } else if (parity == "O" || parity == "ODD") {
            m_parity = QSerialPort::OddParity;
        }
    }
    
    // 停止位
    if (config.contains("stopbits")) {
        int stopBits = config["stopbits"].toInt();
        if (stopBits == 1) {
            m_stopBits = QSerialPort::OneStop;
        } else if (stopBits == 2) {
            m_stopBits = QSerialPort::TwoStop;
        }
    }
    
    // 从站地址
    if (config.contains("slave_address")) {
        m_slaveAddress = config["slave_address"].toUInt();
    }
    
    // 超时时间
    if (config.contains("timeout")) {
        m_timeout = config["timeout"].toInt();
    }
    
    if (needReconnect) {
        return connect();
    }
    
    return true;
}

/***************************************************************
 * 设置从站地址
 ***************************************************************/
void ProtocolModbusRTU::setSlaveAddress(quint8 address)
{
    if (address >= 1 && address <= 247) {
        m_slaveAddress = address;
    } else {
        qWarning() << "Invalid Modbus slave address:" << address;
    }
}

/***************************************************************
 * 设置超时时间
 ***************************************************************/
void ProtocolModbusRTU::setTimeout(int timeout)
{
    m_timeout = timeout;
}

/***************************************************************
 * 读取线圈状态 (功能码0x01)
 ***************************************************************/
QVector<bool> ProtocolModbusRTU::readCoils(quint16 startAddress, quint16 count)
{
    if (!isConnected() || count == 0 || count > 2000) {
        return QVector<bool>();
    }
    
    // 构建请求数据：[起始地址高][起始地址低][数量高][数量低]
    QByteArray data;
    data.append((startAddress >> 8) & 0xFF);
    data.append(startAddress & 0xFF);
    data.append((count >> 8) & 0xFF);
    data.append(count & 0xFF);
    
    // 发送请求
    QByteArray response = sendRequest(buildRequest(0x01, data));
    
    if (response.isEmpty() || response.size() < 3) {
        return QVector<bool>();
    }
    
    // 解析响应：[从站地址][功能码][字节数][数据...][CRC]
    quint8 byteCount = static_cast<quint8>(response[2]);
    QByteArray coilData = response.mid(3, byteCount);
    
    return parseBooleans(coilData, count);
}

/***************************************************************
 * 读取离散输入状态 (功能码0x02)
 ***************************************************************/
QVector<bool> ProtocolModbusRTU::readDiscreteInputs(quint16 startAddress, quint16 count)
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
    
    if (response.isEmpty() || response.size() < 3) {
        return QVector<bool>();
    }
    
    quint8 byteCount = static_cast<quint8>(response[2]);
    QByteArray inputData = response.mid(3, byteCount);
    
    return parseBooleans(inputData, count);
}

/***************************************************************
 * 读取保持寄存器 (功能码0x03)
 ***************************************************************/
QVector<quint16> ProtocolModbusRTU::readHoldingRegisters(quint16 startAddress, quint16 count)
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
    
    if (response.isEmpty() || response.size() < 3) {
        return QVector<quint16>();
    }
    
    quint8 byteCount = static_cast<quint8>(response[2]);
    QByteArray registerData = response.mid(3, byteCount);
    
    return parseRegisters(registerData);
}

/***************************************************************
 * 读取输入寄存器 (功能码0x04)
 ***************************************************************/
QVector<quint16> ProtocolModbusRTU::readInputRegisters(quint16 startAddress, quint16 count)
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
    
    if (response.isEmpty() || response.size() < 3) {
        return QVector<quint16>();
    }
    
    quint8 byteCount = static_cast<quint8>(response[2]);
    QByteArray registerData = response.mid(3, byteCount);
    
    return parseRegisters(registerData);
}

/***************************************************************
 * 写单个线圈 (功能码0x05)
 ***************************************************************/
bool ProtocolModbusRTU::writeSingleCoil(quint16 address, bool value)
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
 * 写单个寄存器 (功能码0x06)
 ***************************************************************/
bool ProtocolModbusRTU::writeSingleRegister(quint16 address, quint16 value)
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
 * 写多个线圈 (功能码0x0F)
 ***************************************************************/
bool ProtocolModbusRTU::writeMultipleCoils(quint16 startAddress, const QVector<bool> &values)
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
    
    // 转换布尔值到字节
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
 * 写多个寄存器 (功能码0x10)
 ***************************************************************/
bool ProtocolModbusRTU::writeMultipleRegisters(quint16 startAddress, const QVector<quint16> &values)
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
    
    // 添加寄存器值
    for (quint16 value : values) {
        data.append((value >> 8) & 0xFF);
        data.append(value & 0xFF);
    }
    
    QByteArray response = sendRequest(buildRequest(0x10, data));
    
    return !response.isEmpty();
}

/***************************************************************
 * 串口数据接收槽函数
 ***************************************************************/
void ProtocolModbusRTU::onSerialDataReceived()
{
    if (!m_waitingForResponse) {
        return;
    }
    
    // 读取所有可用数据
    m_receiveBuffer.append(m_serialPort->readAll());
}

/***************************************************************
 * 响应超时槽函数
 ***************************************************************/
void ProtocolModbusRTU::onResponseTimeout()
{
    if (m_waitingForResponse) {
        setError("Response timeout");
        m_waitingForResponse = false;
    }
}

/***************************************************************
 * 发送Modbus请求并等待响应
 ***************************************************************/
QByteArray ProtocolModbusRTU::sendRequest(const QByteArray &request)
{
    if (!isConnected()) {
        return QByteArray();
    }
    
    // 清空接收缓冲区
    m_receiveBuffer.clear();
    m_waitingForResponse = true;
    
    // 发送请求
    m_serialPort->write(request);
    m_serialPort->flush();
    
    // 启动超时定时器
    m_responseTimer->start(m_timeout);
    
    // 等待响应（使用事件循环）
    QEventLoop loop;
    QTimer::singleShot(m_timeout + 100, &loop, &QEventLoop::quit);
    
    while (m_waitingForResponse && m_responseTimer->isActive()) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
        
        // 检查是否收到完整响应
        if (m_receiveBuffer.size() >= 5) {  // 最小响应长度
            if (validateResponse(m_receiveBuffer)) {
                m_waitingForResponse = false;
                m_responseTimer->stop();
                break;
            }
        }
    }
    
    m_waitingForResponse = false;
    m_responseTimer->stop();
    
    return m_receiveBuffer;
}

/***************************************************************
 * 构建Modbus请求帧
 ***************************************************************/
QByteArray ProtocolModbusRTU::buildRequest(quint8 functionCode, const QByteArray &data)
{
    QByteArray request;
    request.append(m_slaveAddress);
    request.append(functionCode);
    request.append(data);
    
    // 计算并添加CRC16
    quint16 crc = calculateCRC16(request);
    request.append(crc & 0xFF);         // CRC低字节
    request.append((crc >> 8) & 0xFF);  // CRC高字节
    
    return request;
}

/***************************************************************
 * 验证响应帧
 ***************************************************************/
bool ProtocolModbusRTU::validateResponse(const QByteArray &response)
{
    if (response.size() < 5) {
        return false;
    }
    
    // 检查从站地址
    if (static_cast<quint8>(response[0]) != m_slaveAddress) {
        return false;
    }
    
    // 检查CRC
    QByteArray dataWithoutCRC = response.left(response.size() - 2);
    quint16 receivedCRC = (static_cast<quint8>(response[response.size() - 2])) |
                          (static_cast<quint8>(response[response.size() - 1]) << 8);
    quint16 calculatedCRC = calculateCRC16(dataWithoutCRC);
    
    if (receivedCRC != calculatedCRC) {
        setError("CRC check failed");
        return false;
    }
    
    // 检查异常响应
    quint8 functionCode = static_cast<quint8>(response[1]);
    if (functionCode & 0x80) {
        quint8 exceptionCode = static_cast<quint8>(response[2]);
        emit modbusException(exceptionCode);
        setError(QString("Modbus exception: 0x%1").arg(exceptionCode, 2, 16, QChar('0')));
        return false;
    }
    
    return true;
}

/***************************************************************
 * 计算CRC16校验码
 ***************************************************************/
quint16 ProtocolModbusRTU::calculateCRC16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    
    for (int i = 0; i < data.size(); i++) {
        crc ^= static_cast<quint8>(data[i]);
        
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

/***************************************************************
 * 解析布尔值列表
 ***************************************************************/
QVector<bool> ProtocolModbusRTU::parseBooleans(const QByteArray &data, quint16 count)
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
QVector<quint16> ProtocolModbusRTU::parseRegisters(const QByteArray &data)
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

