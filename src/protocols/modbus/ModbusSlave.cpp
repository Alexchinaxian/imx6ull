/***************************************************************
 * Copyright: Alex
 * FileName: protocol_modbus_slave.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus RTU从站协议驱动实现
 ***************************************************************/

#include "protocols/modbus/ModbusSlave.h"
#include <QDebug>

// Modbus异常码
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION     0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS 0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE   0x03
#define MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE 0x04

/***************************************************************
 * 构造函数
 ***************************************************************/
ProtocolModbusSlave::ProtocolModbusSlave(const QString &portName, quint8 slaveAddress, 
                                         QObject *parent)
    : IProtocolInterface(parent)
    , m_portName(portName)
    , m_slaveAddress(slaveAddress)
    , m_baudrate(9600)
    , m_dataBits(QSerialPort::Data8)
    , m_parity(QSerialPort::NoParity)
    , m_stopBits(QSerialPort::OneStop)
{
    // 创建串口对象
    m_serialPort = new QSerialPort(this);
    m_serialPort->setPortName(m_portName);
    
    // 创建帧间隔定时器（用于检测帧结束）
    m_frameTimer = new QTimer(this);
    m_frameTimer->setSingleShot(true);
    m_frameTimer->setInterval(50);  // 50ms帧间隔
    
    // 连接信号槽
    QObject::connect(m_serialPort, &QSerialPort::readyRead, 
            this, &ProtocolModbusSlave::onSerialDataReceived);
    QObject::connect(m_frameTimer, &QTimer::timeout, this, [this]() {
        // 帧接收完成，处理请求
        if (m_receiveBuffer.size() >= 4) {
            processRequest(m_receiveBuffer);
        }
        m_receiveBuffer.clear();
    });
    
    // 初始化寄存器
    m_holdingRegisters.resize(MAX_REGISTERS);
    m_holdingRegisters.fill(0);
    
    m_inputRegisters.resize(MAX_REGISTERS);
    m_inputRegisters.fill(0);
}

/***************************************************************
 * 析构函数
 ***************************************************************/
ProtocolModbusSlave::~ProtocolModbusSlave()
{
    disconnect();
}

/***************************************************************
 * 连接串口
 ***************************************************************/
bool ProtocolModbusSlave::connect()
{
    if (isConnected()) {
        qWarning() << "Modbus Slave already connected";
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
    
    qInfo() << "Modbus RTU Slave started:"
            << "Port:" << m_portName
            << "Address:" << m_slaveAddress
            << "Baudrate:" << m_baudrate;
    
    return true;
}

/***************************************************************
 * 断开连接
 ***************************************************************/
void ProtocolModbusSlave::disconnect()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        setState(ProtocolState::Disconnected);
        emit disconnected();
        qInfo() << "Modbus RTU Slave stopped";
    }
}

/***************************************************************
 * 检查是否已连接
 ***************************************************************/
bool ProtocolModbusSlave::isConnected() const
{
    return m_serialPort->isOpen();
}

/***************************************************************
 * 配置协议参数
 ***************************************************************/
bool ProtocolModbusSlave::configure(const QMap<QString, QVariant> &config)
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
    
    if (needReconnect) {
        return connect();
    }
    
    return true;
}

/***************************************************************
 * 设置温度值
 ***************************************************************/
void ProtocolModbusSlave::setTemperature(float temperature)
{
    // 将温度映射到寄存器
    // 寄存器0x0000: 温度整数部分
    // 寄存器0x0001: 温度小数部分x100
    
    quint16 intPart = static_cast<quint16>(temperature);
    quint16 fracPart = static_cast<quint16>((temperature - intPart) * 100);
    
    setHoldingRegister(0x0000, intPart);
    setHoldingRegister(0x0001, fracPart);
    
    // 同时更新输入寄存器（只读）
    m_inputRegisters[0x0000] = intPart;
    m_inputRegisters[0x0001] = fracPart;
}

/***************************************************************
 * 设置系统状态
 ***************************************************************/
void ProtocolModbusSlave::setSystemStatus(quint16 status)
{
    setHoldingRegister(0x0002, status);
    m_inputRegisters[0x0002] = status;
}

/***************************************************************
 * 设置保持寄存器值
 ***************************************************************/
bool ProtocolModbusSlave::setHoldingRegister(quint16 address, quint16 value)
{
    if (address >= MAX_REGISTERS) {
        return false;
    }
    
    m_holdingRegisters[address] = value;
    return true;
}

/***************************************************************
 * 获取保持寄存器值
 ***************************************************************/
quint16 ProtocolModbusSlave::getHoldingRegister(quint16 address) const
{
    if (address >= MAX_REGISTERS) {
        return 0;
    }
    
    return m_holdingRegisters[address];
}

/***************************************************************
 * 设置从站地址
 ***************************************************************/
void ProtocolModbusSlave::setSlaveAddress(quint8 address)
{
    if (address >= 1 && address <= 247) {
        m_slaveAddress = address;
        qInfo() << "Modbus Slave address changed to:" << address;
    }
}

/***************************************************************
 * 串口数据接收槽函数
 ***************************************************************/
void ProtocolModbusSlave::onSerialDataReceived()
{
    // 读取数据并添加到缓冲区
    QByteArray data = m_serialPort->readAll();
    m_receiveBuffer.append(data);
    
    // 重启帧间隔定时器
    m_frameTimer->start();
}

/***************************************************************
 * 处理Modbus请求
 ***************************************************************/
void ProtocolModbusSlave::processRequest(const QByteArray &request)
{
    // 最小请求长度：地址(1) + 功能码(1) + 数据(>=2) + CRC(2) = 6字节
    if (request.size() < 6) {
        return;
    }
    
    // 验证CRC
    if (!verifyCRC(request)) {
        qWarning() << "Modbus Slave: CRC check failed";
        return;
    }
    
    // 检查从站地址
    quint8 address = static_cast<quint8>(request[0]);
    if (address != m_slaveAddress) {
        return;  // 不是发给本从站的请求，忽略
    }
    
    // 获取功能码
    quint8 functionCode = static_cast<quint8>(request[1]);
    
    // 根据功能码处理请求
    switch (functionCode) {
        case 0x03:  // 读保持寄存器
            handleReadHoldingRegisters(request);
            break;
            
        case 0x04:  // 读输入寄存器
            handleReadInputRegisters(request);
            break;
            
        case 0x06:  // 写单个寄存器
            handleWriteSingleRegister(request);
            break;
            
        case 0x10:  // 写多个寄存器
            handleWriteMultipleRegisters(request);
            break;
            
        default:
            // 不支持的功能码
            sendException(functionCode, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
            break;
    }
}

/***************************************************************
 * 处理读保持寄存器请求
 ***************************************************************/
void ProtocolModbusSlave::handleReadHoldingRegisters(const QByteArray &request)
{
    // 解析请求：[地址][功能码][起始地址H][起始地址L][数量H][数量L][CRC]
    quint16 startAddr = (static_cast<quint8>(request[2]) << 8) | static_cast<quint8>(request[3]);
    quint16 count = (static_cast<quint8>(request[4]) << 8) | static_cast<quint8>(request[5]);
    
    // 检查参数
    if (count == 0 || count > 125 || (startAddr + count) > MAX_REGISTERS) {
        sendException(0x03, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 构建响应：[地址][功能码][字节数][数据...][CRC]
    QByteArray response;
    response.append(m_slaveAddress);
    response.append(static_cast<char>(0x03));
    response.append(static_cast<char>(count * 2));  // 字节数
    
    // 添加寄存器数据
    for (int i = 0; i < count; i++) {
        quint16 value = m_holdingRegisters[startAddr + i];
        response.append((value >> 8) & 0xFF);
        response.append(value & 0xFF);
    }
    
    sendResponse(response);
    
    emit readRequest(0x03, startAddr, count);
}

/***************************************************************
 * 处理读输入寄存器请求
 ***************************************************************/
void ProtocolModbusSlave::handleReadInputRegisters(const QByteArray &request)
{
    quint16 startAddr = (static_cast<quint8>(request[2]) << 8) | static_cast<quint8>(request[3]);
    quint16 count = (static_cast<quint8>(request[4]) << 8) | static_cast<quint8>(request[5]);
    
    if (count == 0 || count > 125 || (startAddr + count) > MAX_REGISTERS) {
        sendException(0x04, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    QByteArray response;
    response.append(m_slaveAddress);
    response.append(static_cast<char>(0x04));
    response.append(static_cast<char>(count * 2));
    
    for (int i = 0; i < count; i++) {
        quint16 value = m_inputRegisters[startAddr + i];
        response.append((value >> 8) & 0xFF);
        response.append(value & 0xFF);
    }
    
    sendResponse(response);
    
    emit readRequest(0x04, startAddr, count);
}

/***************************************************************
 * 处理写单个寄存器请求
 ***************************************************************/
void ProtocolModbusSlave::handleWriteSingleRegister(const QByteArray &request)
{
    quint16 address = (static_cast<quint8>(request[2]) << 8) | static_cast<quint8>(request[3]);
    quint16 value = (static_cast<quint8>(request[4]) << 8) | static_cast<quint8>(request[5]);
    
    if (address >= MAX_REGISTERS) {
        sendException(0x06, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 写入寄存器
    m_holdingRegisters[address] = value;
    
    // 回显请求（标准Modbus响应）
    QByteArray response = request.left(6);  // 去掉CRC
    sendResponse(response);
    
    emit writeRequest(0x06, address, value);
}

/***************************************************************
 * 处理写多个寄存器请求
 ***************************************************************/
void ProtocolModbusSlave::handleWriteMultipleRegisters(const QByteArray &request)
{
    quint16 startAddr = (static_cast<quint8>(request[2]) << 8) | static_cast<quint8>(request[3]);
    quint16 count = (static_cast<quint8>(request[4]) << 8) | static_cast<quint8>(request[5]);
    quint8 byteCount = static_cast<quint8>(request[6]);
    
    if (count == 0 || count > 123 || (startAddr + count) > MAX_REGISTERS || byteCount != count * 2) {
        sendException(0x10, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 写入寄存器
    for (int i = 0; i < count; i++) {
        quint16 value = (static_cast<quint8>(request[7 + i * 2]) << 8) | 
                       static_cast<quint8>(request[7 + i * 2 + 1]);
        m_holdingRegisters[startAddr + i] = value;
    }
    
    // 响应：[地址][功能码][起始地址H][起始地址L][数量H][数量L][CRC]
    QByteArray response;
    response.append(m_slaveAddress);
    response.append(static_cast<char>(0x10));
    response.append((startAddr >> 8) & 0xFF);
    response.append(startAddr & 0xFF);
    response.append((count >> 8) & 0xFF);
    response.append(count & 0xFF);
    
    sendResponse(response);
    
    emit writeRequest(0x10, startAddr, count);
}

/***************************************************************
 * 发送响应帧
 ***************************************************************/
void ProtocolModbusSlave::sendResponse(const QByteArray &response)
{
    // 计算CRC并添加
    quint16 crc = calculateCRC16(response);
    
    QByteArray frame = response;
    frame.append(crc & 0xFF);         // CRC低字节
    frame.append((crc >> 8) & 0xFF);  // CRC高字节
    
    // 发送响应
    m_serialPort->write(frame);
    m_serialPort->flush();
    
    qDebug() << "Modbus Slave: Sent response" << frame.size() << "bytes";
}

/***************************************************************
 * 发送异常响应
 ***************************************************************/
void ProtocolModbusSlave::sendException(quint8 functionCode, quint8 exceptionCode)
{
    QByteArray response;
    response.append(m_slaveAddress);
    response.append(functionCode | 0x80);  // 功能码最高位置1表示异常
    response.append(exceptionCode);
    
    sendResponse(response);
    
    qWarning() << "Modbus Slave: Exception" 
               << QString("0x%1").arg(exceptionCode, 2, 16, QChar('0'));
}

/***************************************************************
 * 计算CRC16校验码
 ***************************************************************/
quint16 ProtocolModbusSlave::calculateCRC16(const QByteArray &data)
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
 * 验证CRC
 ***************************************************************/
bool ProtocolModbusSlave::verifyCRC(const QByteArray &data)
{
    if (data.size() < 4) {
        return false;
    }
    
    // 提取接收到的CRC
    quint16 receivedCRC = (static_cast<quint8>(data[data.size() - 2])) |
                          (static_cast<quint8>(data[data.size() - 1]) << 8);
    
    // 计算CRC（不含最后2字节）
    QByteArray dataWithoutCRC = data.left(data.size() - 2);
    quint16 calculatedCRC = calculateCRC16(dataWithoutCRC);
    
    return (receivedCRC == calculatedCRC);
}

