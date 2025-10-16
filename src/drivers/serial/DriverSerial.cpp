/***************************************************************
 * Copyright: Alex
 * FileName: driver_serial.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 串口驱动类的实现
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#include "drivers/serial/DriverSerial.h"
#include <QDebug>

/**
 * @brief 构造函数 - 初始化串口驱动
 * @param portName 串口名称
 * @param parent 父对象指针
 */
DriverSerial::DriverSerial(const QString &portName, QObject *parent)
    : QObject(parent)
    , m_pSerialPort(nullptr)
    , m_portName(portName)
    , m_isConfigured(false)
    , m_readBufferMaxSize(65536)  // 默认64KB读缓冲
    , m_isWriting(false)
{
    // 创建QSerialPort对象
    m_pSerialPort = new QSerialPort(portName, this);
    
    // 连接信号和槽
    connect(m_pSerialPort, &QSerialPort::readyRead,
            this, &DriverSerial::onReadyRead);
    
    connect(m_pSerialPort, 
            static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &DriverSerial::onError);
    
    // 连接bytesWritten信号，用于处理写缓冲
    connect(m_pSerialPort, &QSerialPort::bytesWritten,
            this, &DriverSerial::processWriteBuffer);
    
    qDebug() << "[DriverSerial] 串口驱动创建:" << portName;
}

/**
 * @brief 析构函数 - 清理串口资源
 */
DriverSerial::~DriverSerial()
{
    if (m_pSerialPort && m_pSerialPort->isOpen())
    {
        m_pSerialPort->close();
    }
    qDebug() << "[DriverSerial] 串口驱动销毁:" << m_portName;
}

// ========== 串口打开和关闭 ==========

/**
 * @brief 打开串口
 * @param mode 打开模式
 * @return true=成功, false=失败
 */
bool DriverSerial::open(QIODevice::OpenMode mode)
{
    if (!m_pSerialPort)
    {
        qCritical() << "[DriverSerial] 串口对象为空";
        return false;
    }
    
    if (m_pSerialPort->isOpen())
    {
        qWarning() << "[DriverSerial] 串口已打开:" << m_portName;
        return true;
    }
    
    // 设置串口名称
    m_pSerialPort->setPortName(m_portName);
    
    // 打开串口
    if (m_pSerialPort->open(mode))
    {
        qInfo() << "[DriverSerial] ✓ 串口打开成功:" << m_portName;
        emit opened();
        return true;
    }
    else
    {
        QString errMsg = QString("打开串口失败: %1 - %2")
                        .arg(m_portName, m_pSerialPort->errorString());
        qCritical() << "[DriverSerial]" << errMsg;
        emit error(errMsg);
        return false;
    }
}

/**
 * @brief 关闭串口
 */
void DriverSerial::close()
{
    if (m_pSerialPort && m_pSerialPort->isOpen())
    {
        m_pSerialPort->close();
        qInfo() << "[DriverSerial] 串口已关闭:" << m_portName;
        emit closed();
    }
}

/**
 * @brief 检查串口是否打开
 * @return true=已打开, false=未打开
 */
bool DriverSerial::isOpen() const
{
    return m_pSerialPort && m_pSerialPort->isOpen();
}

// ========== 串口配置 ==========

/**
 * @brief 设置波特率
 * @param baudRate 波特率
 * @return true=成功, false=失败
 */
bool DriverSerial::setBaudRate(qint32 baudRate)
{
    if (!m_pSerialPort)
    {
        return false;
    }
    
    if (m_pSerialPort->setBaudRate(baudRate))
    {
        qDebug() << "[DriverSerial] 设置波特率:" << baudRate;
        return true;
    }
    else
    {
        qWarning() << "[DriverSerial] 设置波特率失败:" << baudRate;
        return false;
    }
}

/**
 * @brief 设置数据位
 * @param dataBits 数据位
 * @return true=成功, false=失败
 */
bool DriverSerial::setDataBits(QSerialPort::DataBits dataBits)
{
    if (!m_pSerialPort)
    {
        return false;
    }
    
    return m_pSerialPort->setDataBits(dataBits);
}

/**
 * @brief 设置停止位
 * @param stopBits 停止位
 * @return true=成功, false=失败
 */
bool DriverSerial::setStopBits(QSerialPort::StopBits stopBits)
{
    if (!m_pSerialPort)
    {
        return false;
    }
    
    return m_pSerialPort->setStopBits(stopBits);
}

/**
 * @brief 设置校验位
 * @param parity 校验位
 * @return true=成功, false=失败
 */
bool DriverSerial::setParity(QSerialPort::Parity parity)
{
    if (!m_pSerialPort)
    {
        return false;
    }
    
    return m_pSerialPort->setParity(parity);
}

/**
 * @brief 设置流控制
 * @param flowControl 流控制
 * @return true=成功, false=失败
 */
bool DriverSerial::setFlowControl(QSerialPort::FlowControl flowControl)
{
    if (!m_pSerialPort)
    {
        return false;
    }
    
    return m_pSerialPort->setFlowControl(flowControl);
}

/**
 * @brief 快速配置串口
 * @param baudRate 波特率
 * @param dataBits 数据位
 * @param parity 校验位
 * @param stopBits 停止位
 * @return true=成功, false=失败
 */
bool DriverSerial::configure(qint32 baudRate,
                             QSerialPort::DataBits dataBits,
                             QSerialPort::Parity parity,
                             QSerialPort::StopBits stopBits)
{
    if (!m_pSerialPort)
    {
        return false;
    }
    
    qInfo() << "[DriverSerial] 配置串口:" << m_portName
            << "波特率:" << baudRate
            << "数据位:" << dataBits
            << "校验:" << parity
            << "停止位:" << stopBits;
    
    // 设置各项参数
    bool success = true;
    success &= setBaudRate(baudRate);
    success &= setDataBits(dataBits);
    success &= setParity(parity);
    success &= setStopBits(stopBits);
    success &= setFlowControl(QSerialPort::NoFlowControl);
    
    if (success)
    {
        m_isConfigured = true;
        qInfo() << "[DriverSerial] ✓ 串口配置成功";
    }
    else
    {
        qWarning() << "[DriverSerial] ✗ 串口配置失败";
    }
    
    return success;
}

// ========== 数据读写 ==========

/**
 * @brief 写入数据（使用写缓冲）
 * @param data 要写入的数据
 * @return 实际写入缓冲区的字节数
 */
qint64 DriverSerial::write(const QByteArray &data)
{
    if (!m_pSerialPort || !m_pSerialPort->isOpen())
    {
        qWarning() << "[DriverSerial] 串口未打开，无法写入";
        return -1;
    }
    
    if (data.isEmpty())
    {
        return 0;
    }
    
    // 将数据添加到写缓冲区
    m_writeBuffer.append(data);
    
    qDebug() << "[DriverSerial] 数据加入写缓冲:" << data.size() << "字节，缓冲区总计:" << m_writeBuffer.size() << "字节";
    
    // 如果当前没有在发送，立即开始发送
    if (!m_isWriting)
    {
        processWriteBuffer();
    }
    
    return data.size();
}

/**
 * @brief 写入字符串
 * @param data 要写入的字符串
 * @return 实际写入的字节数
 */
qint64 DriverSerial::write(const QString &data)
{
    return write(data.toUtf8());
}

/**
 * @brief 读取所有可用数据（从读缓冲区）
 * @return 读取到的数据
 */
QByteArray DriverSerial::readAll()
{
    QByteArray data = m_readBuffer;
    m_readBuffer.clear();
    
    qDebug() << "[DriverSerial] 从读缓冲读取:" << data.size() << "字节";
    
    return data;
}

/**
 * @brief 读取指定字节数（从读缓冲区）
 * @param maxSize 最大读取字节数
 * @return 读取到的数据
 */
QByteArray DriverSerial::read(qint64 maxSize)
{
    qint64 readSize = qMin(maxSize, (qint64)m_readBuffer.size());
    
    if (readSize <= 0)
    {
        return QByteArray();
    }
    
    QByteArray data = m_readBuffer.left(readSize);
    m_readBuffer.remove(0, readSize);
    
    qDebug() << "[DriverSerial] 从读缓冲读取:" << data.size() << "字节，剩余:" << m_readBuffer.size() << "字节";
    
    return data;
}

/**
 * @brief 读取一行数据（从读缓冲区）
 * @return 读取到的一行数据
 */
QByteArray DriverSerial::readLine()
{
    // 查找换行符
    int newlineIndex = m_readBuffer.indexOf('\n');
    
    if (newlineIndex < 0)
    {
        // 没有找到换行符，返回空
        return QByteArray();
    }
    
    // 读取一行（包含换行符）
    QByteArray line = m_readBuffer.left(newlineIndex + 1);
    m_readBuffer.remove(0, newlineIndex + 1);
    
    qDebug() << "[DriverSerial] 从读缓冲读取一行:" << line.size() << "字节，剩余:" << m_readBuffer.size() << "字节";
    
    return line;
}

/**
 * @brief 获取可读字节数（读缓冲区中的数据）
 * @return 可读字节数
 */
qint64 DriverSerial::bytesAvailable() const
{
    return m_readBuffer.size();
}

/**
 * @brief 等待数据可读
 * @param msecs 等待时间（毫秒）
 * @return true=有数据, false=超时
 */
bool DriverSerial::waitForReadyRead(int msecs)
{
    if (!m_pSerialPort || !m_pSerialPort->isOpen())
    {
        return false;
    }
    
    return m_pSerialPort->waitForReadyRead(msecs);
}

/**
 * @brief 清空接收缓冲区（包括读缓冲和底层缓冲）
 */
void DriverSerial::clear()
{
    m_readBuffer.clear();
    
    if (m_pSerialPort)
    {
        m_pSerialPort->clear();
    }
    
    qDebug() << "[DriverSerial] 清空接收缓冲区";
}

/**
 * @brief 刷新发送缓冲区（立即发送所有待发送数据）
 */
void DriverSerial::flush()
{
    // 处理所有待发送数据
    while (!m_writeBuffer.isEmpty() && m_pSerialPort && m_pSerialPort->isOpen())
    {
        qint64 bytesWritten = m_pSerialPort->write(m_writeBuffer);
        if (bytesWritten > 0)
        {
            m_writeBuffer.remove(0, bytesWritten);
            m_pSerialPort->flush();
        }
        else
        {
            break;
        }
    }
    
    qDebug() << "[DriverSerial] 刷新发送缓冲区，剩余:" << m_writeBuffer.size() << "字节";
}

// ========== 缓冲区管理 ==========

/**
 * @brief 设置读缓冲区大小
 * @param size 缓冲区大小（字节）
 */
void DriverSerial::setReadBufferSize(qint64 size)
{
    m_readBufferMaxSize = size;
    qDebug() << "[DriverSerial] 设置读缓冲区大小:" << size << "字节";
}

/**
 * @brief 获取读缓冲区中的数据大小
 * @return 缓冲区数据大小
 */
qint64 DriverSerial::getReadBufferSize() const
{
    return m_readBuffer.size();
}

/**
 * @brief 获取写缓冲区中待发送的数据大小
 * @return 待发送数据大小
 */
qint64 DriverSerial::getWriteBufferSize() const
{
    return m_writeBuffer.size();
}

/**
 * @brief 清空读缓冲区
 */
void DriverSerial::clearReadBuffer()
{
    m_readBuffer.clear();
    qDebug() << "[DriverSerial] 清空读缓冲区";
}

/**
 * @brief 清空写缓冲区
 */
void DriverSerial::clearWriteBuffer()
{
    m_writeBuffer.clear();
    m_isWriting = false;
    qDebug() << "[DriverSerial] 清空写缓冲区";
}

// ========== 状态查询 ==========

/**
 * @brief 获取串口名称
 * @return 串口名称
 */
QString DriverSerial::getPortName() const
{
    return m_portName;
}

/**
 * @brief 获取波特率
 * @return 当前波特率
 */
qint32 DriverSerial::getBaudRate() const
{
    if (m_pSerialPort)
    {
        return m_pSerialPort->baudRate();
    }
    return 0;
}

/**
 * @brief 获取错误信息
 * @return 错误描述字符串
 */
QString DriverSerial::getErrorString() const
{
    if (m_pSerialPort)
    {
        return m_pSerialPort->errorString();
    }
    return QString();
}

// ========== 静态辅助方法 ==========

/**
 * @brief 获取系统中所有可用串口列表
 * @return 串口信息列表
 */
QList<QSerialPortInfo> DriverSerial::getAvailablePorts()
{
    return QSerialPortInfo::availablePorts();
}

/**
 * @brief 打印所有可用串口信息
 */
void DriverSerial::printAvailablePorts()
{
    qInfo() << "========================================";
    qInfo() << "  Available Serial Ports";
    qInfo() << "========================================";
    
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    if (ports.isEmpty())
    {
        qWarning() << "未发现可用串口";
        return;
    }
    
    qInfo() << "发现" << ports.size() << "个串口：";
    qInfo() << "";
    
    for (int i = 0; i < ports.size(); i++)
    {
        const QSerialPortInfo &port = ports[i];
        
        qInfo() << "----------------------------------------";
        qInfo() << "串口" << (i + 1) << ":";
        qInfo() << "  名称       :" << port.portName();
        qInfo() << "  设备路径   :" << port.systemLocation();
        qInfo() << "  描述       :" << port.description();
        qInfo() << "  制造商     :" << port.manufacturer();
        qInfo() << "  序列号     :" << port.serialNumber();
        qInfo() << "  VID        :" << QString("0x%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
        qInfo() << "  PID        :" << QString("0x%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
        qInfo() << "  是否繁忙   :" << (port.isBusy() ? "是" : "否");
        
        // 打印系统位置（Linux下是/dev/ttyXXX）
        if (port.hasVendorIdentifier())
        {
            qInfo() << "  厂商ID有效 : 是";
        }
        if (port.hasProductIdentifier())
        {
            qInfo() << "  产品ID有效 : 是";
        }
    }
    
    qInfo() << "========================================";
}

/**
 * @brief 根据描述查找串口
 * @param description 串口描述关键字
 * @return 串口名称，未找到返回空字符串
 */
QString DriverSerial::findPortByDescription(const QString &description)
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &port : ports)
    {
        if (port.description().contains(description, Qt::CaseInsensitive))
        {
            qInfo() << "[DriverSerial] 找到串口:" << port.portName()
                    << "描述:" << port.description();
            return port.portName();
        }
    }
    
    qWarning() << "[DriverSerial] 未找到包含描述的串口:" << description;
    return QString();
}

// ========== 私有槽函数 ==========

/**
 * @brief 数据可读槽函数（将数据添加到读缓冲区）
 * 
 * 当串口有数据可读时，Qt会触发readyRead信号，
 * 此函数被调用，读取数据并添加到读缓冲区
 */
void DriverSerial::onReadyRead()
{
    if (!m_pSerialPort)
    {
        return;
    }
    
    // 读取所有可用数据
    QByteArray data = m_pSerialPort->readAll();
    
    if (!data.isEmpty())
    {
        // 添加到读缓冲区
        m_readBuffer.append(data);
        
        // 检查缓冲区是否超出最大限制
        if (m_readBuffer.size() > m_readBufferMaxSize)
        {
            qWarning() << "[DriverSerial] 读缓冲区溢出，清空旧数据";
            qWarning() << "  当前大小:" << m_readBuffer.size() << "最大:" << m_readBufferMaxSize;
            
            // 只保留最新的数据
            m_readBuffer = m_readBuffer.right(m_readBufferMaxSize / 2);
        }
        
        qDebug() << "[DriverSerial] 接收数据:" << data.size() << "字节"
                 << "读缓冲区总计:" << m_readBuffer.size() << "字节";
        
        // 发送数据接收信号（通知上层有新数据）
        emit dataReceived(data);
    }
}

/**
 * @brief 错误处理槽函数
 * @param serialError 串口错误类型
 */
void DriverSerial::onError(QSerialPort::SerialPortError serialError)
{
    // 忽略NoError
    if (serialError == QSerialPort::NoError)
    {
        return;
    }
    
    // 获取错误描述
    QString errorMsg;
    
    switch (serialError)
    {
        case QSerialPort::DeviceNotFoundError:
            errorMsg = "设备未找到";
            break;
        case QSerialPort::PermissionError:
            errorMsg = "权限错误";
            break;
        case QSerialPort::OpenError:
            errorMsg = "打开失败";
            break;
        case QSerialPort::WriteError:
            errorMsg = "写入错误";
            break;
        case QSerialPort::ReadError:
            errorMsg = "读取错误";
            break;
        case QSerialPort::ResourceError:
            errorMsg = "资源错误（串口已断开）";
            break;
        case QSerialPort::UnsupportedOperationError:
            errorMsg = "不支持的操作";
            break;
        case QSerialPort::TimeoutError:
            errorMsg = "超时错误";
            break;
        default:
            errorMsg = QString("未知错误(%1)").arg(serialError);
            break;
    }
    
    QString fullMsg = QString("[%1] %2: %3")
                      .arg(m_portName, errorMsg, m_pSerialPort->errorString());
    
    qCritical() << "[DriverSerial]" << fullMsg;
    emit error(fullMsg);
}

/**
 * @brief 处理写缓冲区数据（异步发送）
 * 
 * 当底层串口完成一次写操作时，会触发bytesWritten信号，
 * 此函数被调用，继续发送写缓冲区中的剩余数据
 */
void DriverSerial::processWriteBuffer()
{
    if (!m_pSerialPort || !m_pSerialPort->isOpen())
    {
        m_isWriting = false;
        return;
    }
    
    // 如果写缓冲区为空，标记发送完成
    if (m_writeBuffer.isEmpty())
    {
        m_isWriting = false;
        qDebug() << "[DriverSerial] 写缓冲区已发送完成";
        return;
    }
    
    // 标记正在发送
    m_isWriting = true;
    
    // 一次最多发送4KB数据（避免阻塞）
    const qint64 chunkSize = 4096;
    qint64 sendSize = qMin((qint64)m_writeBuffer.size(), chunkSize);
    
    QByteArray chunk = m_writeBuffer.left(sendSize);
    
    // 发送数据
    qint64 bytesWritten = m_pSerialPort->write(chunk);
    
    if (bytesWritten > 0)
    {
        // 从缓冲区移除已发送的数据
        m_writeBuffer.remove(0, bytesWritten);
        
        qDebug() << "[DriverSerial] 发送数据:" << bytesWritten << "字节"
                 << "写缓冲区剩余:" << m_writeBuffer.size() << "字节";
    }
    else
    {
        qWarning() << "[DriverSerial] 发送失败";
        m_isWriting = false;
    }
}

