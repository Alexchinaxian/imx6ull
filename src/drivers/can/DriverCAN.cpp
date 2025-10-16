#include "drivers/can/DriverCAN.h"
#include <QCanBus>
#include <QDebug>
#include <QFile>
#include <QDir>

/***************************************************************
 * 实现文件: DriverCAN.cpp
 * 功能: CAN总线驱动的实现
 * 说明: 基于Qt QCanBus框架实现的SocketCAN驱动
 ***************************************************************/

/**
 * @brief 构造函数
 */
DriverCAN::DriverCAN(const QString &interfaceName, QObject *parent)
    : QObject(parent)
    , m_canDevice(nullptr)
    , m_interfaceName(interfaceName)
    , m_bitrate(0)
    , m_isOpen(false)
    , m_receivedFrameCount(0)
    , m_sentFrameCount(0)
    , m_receiveBufferMaxSize(1000)  // 默认最多缓存1000帧
{
    qInfo() << "[DriverCAN] 初始化CAN接口:" << m_interfaceName;
}

/**
 * @brief 析构函数
 */
DriverCAN::~DriverCAN()
{
    qInfo() << "[DriverCAN] 清理CAN接口:" << m_interfaceName;
    close();
}

// ========== CAN设备打开和关闭 ==========

/**
 * @brief 创建CAN设备对象
 */
bool DriverCAN::createDevice()
{
    if (m_canDevice) {
        qWarning() << "[DriverCAN] 设备已创建";
        return true;
    }
    
    QString errorString;
    
    // 使用socketcan插件创建CAN设备
    m_canDevice = QCanBus::instance()->createDevice(
        QStringLiteral("socketcan"),
        m_interfaceName,
        &errorString
    );
    
    if (!m_canDevice) {
        m_lastError = QString("无法创建CAN设备: %1").arg(errorString);
        qCritical() << "[DriverCAN]" << m_lastError;
        emit error(ConfigurationError, m_lastError);
        return false;
    }
    
    // 连接设备信号
    connectDeviceSignals();
    
    return true;
}

/**
 * @brief 连接设备信号
 */
void DriverCAN::connectDeviceSignals()
{
    if (!m_canDevice) return;
    
    connect(m_canDevice, &QCanBusDevice::framesReceived,
            this, &DriverCAN::onFramesReceived);
    
    connect(m_canDevice, &QCanBusDevice::errorOccurred,
            this, &DriverCAN::onError);
    
    connect(m_canDevice, &QCanBusDevice::stateChanged,
            this, &DriverCAN::onStateChanged);
}

/**
 * @brief 打开CAN设备
 */
bool DriverCAN::open(quint32 bitrate)
{
    if (m_isOpen) {
        qWarning() << "[DriverCAN] 设备已打开";
        return true;
    }
    
    // 创建设备（如果还没有创建）
    if (!m_canDevice && !createDevice()) {
        return false;
    }
    
    // 设置波特率（如果指定）
    if (bitrate > 0) {
        m_bitrate = bitrate;
        m_canDevice->setConfigurationParameter(
            QCanBusDevice::BitRateKey,
            QVariant::fromValue(bitrate)
        );
    }
    
    // 打开设备
    if (!m_canDevice->connectDevice()) {
        m_lastError = QString("无法打开CAN设备: %1").arg(m_canDevice->errorString());
        qCritical() << "[DriverCAN]" << m_lastError;
        emit error(ConnectionError, m_lastError);
        return false;
    }
    
    m_isOpen = true;
    
    // 读取实际波特率
    QVariant bitrateVar = m_canDevice->configurationParameter(QCanBusDevice::BitRateKey);
    if (bitrateVar.isValid()) {
        m_bitrate = bitrateVar.toUInt();
    }
    
    qInfo() << "[DriverCAN] 设备已打开:" << m_interfaceName 
            << "波特率:" << m_bitrate;
    
    emit opened();
    return true;
}

/**
 * @brief 关闭CAN设备
 */
void DriverCAN::close()
{
    if (!m_isOpen) {
        return;
    }
    
    if (m_canDevice) {
        m_canDevice->disconnectDevice();
        m_canDevice->deleteLater();
        m_canDevice = nullptr;
    }
    
    m_isOpen = false;
    qInfo() << "[DriverCAN] 设备已关闭:" << m_interfaceName;
    emit closed();
}

/**
 * @brief 检查CAN设备是否已打开
 */
bool DriverCAN::isOpen() const
{
    return m_isOpen && m_canDevice && 
           m_canDevice->state() == QCanBusDevice::ConnectedState;
}

// ========== CAN配置 ==========

/**
 * @brief 设置波特率
 */
bool DriverCAN::setBitrate(quint32 bitrate)
{
    if (m_isOpen) {
        qWarning() << "[DriverCAN] 无法在设备打开时设置波特率";
        return false;
    }
    
    m_bitrate = bitrate;
    
    if (m_canDevice) {
        m_canDevice->setConfigurationParameter(
            QCanBusDevice::BitRateKey,
            QVariant::fromValue(bitrate)
        );
    }
    
    qInfo() << "[DriverCAN] 波特率已设置:" << bitrate;
    return true;
}

/**
 * @brief 获取当前波特率
 */
quint32 DriverCAN::getBitrate() const
{
    return m_bitrate;
}

/**
 * @brief 设置接收过滤器
 */
bool DriverCAN::setFilter(quint32 filterId, quint32 filterMask)
{
    if (!m_canDevice) {
        qWarning() << "[DriverCAN] 设备未创建";
        return false;
    }
    
    QCanBusDevice::Filter filter;
    filter.frameId = filterId;
    filter.frameIdMask = filterMask;
    filter.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
    filter.type = QCanBusFrame::DataFrame;
    
    QList<QCanBusDevice::Filter> filters;
    filters.append(filter);
    
    m_canDevice->setConfigurationParameter(
        QCanBusDevice::RawFilterKey,
        QVariant::fromValue(filters)
    );
    
    qInfo() << QString("[DriverCAN] 过滤器已设置: ID=0x%1 Mask=0x%2")
               .arg(filterId, 0, 16).arg(filterMask, 0, 16);
    return true;
}

/**
 * @brief 清除所有过滤器
 */
void DriverCAN::clearFilters()
{
    if (!m_canDevice) return;
    
    QList<QCanBusDevice::Filter> emptyFilters;
    m_canDevice->setConfigurationParameter(
        QCanBusDevice::RawFilterKey,
        QVariant::fromValue(emptyFilters)
    );
    
    qInfo() << "[DriverCAN] 过滤器已清除";
}

// ========== CAN帧发送 ==========

/**
 * @brief 发送标准数据帧
 */
bool DriverCAN::writeFrame(quint32 frameId, const QByteArray &data)
{
    QCanBusFrame frame(frameId, data);
    frame.setExtendedFrameFormat(false);
    return writeFrame(frame);
}

/**
 * @brief 发送扩展数据帧
 */
bool DriverCAN::writeExtendedFrame(quint32 frameId, const QByteArray &data)
{
    QCanBusFrame frame(frameId, data);
    frame.setExtendedFrameFormat(true);
    return writeFrame(frame);
}

/**
 * @brief 发送远程请求帧
 */
bool DriverCAN::writeRemoteFrame(quint32 frameId, quint8 dlc)
{
    QCanBusFrame frame(QCanBusFrame::RemoteRequestFrame);
    frame.setFrameId(frameId);
    frame.setPayload(QByteArray(dlc, 0));
    return writeFrame(frame);
}

/**
 * @brief 发送QCanBusFrame对象
 */
bool DriverCAN::writeFrame(const QCanBusFrame &frame)
{
    if (!isOpen()) {
        m_lastError = "CAN设备未打开";
        qWarning() << "[DriverCAN]" << m_lastError;
        emit error(WriteError, m_lastError);
        return false;
    }
    
    if (!m_canDevice->writeFrame(frame)) {
        m_lastError = QString("发送失败: %1").arg(m_canDevice->errorString());
        qWarning() << "[DriverCAN]" << m_lastError;
        emit error(WriteError, m_lastError);
        return false;
    }
    
    m_sentFrameCount++;
    
    qDebug() << "[DriverCAN] 发送帧:" << frameToString(frame);
    return true;
}

// ========== 状态查询 ==========

/**
 * @brief 获取CAN接口名称
 */
QString DriverCAN::getInterfaceName() const
{
    return m_interfaceName;
}

/**
 * @brief 获取错误信息
 */
QString DriverCAN::getErrorString() const
{
    if (m_canDevice) {
        return m_canDevice->errorString();
    }
    return m_lastError;
}

/**
 * @brief 获取设备状态
 */
QString DriverCAN::getStateString() const
{
    if (!m_canDevice) {
        return "未初始化";
    }
    
    switch (m_canDevice->state()) {
        case QCanBusDevice::UnconnectedState:
            return "未连接";
        case QCanBusDevice::ConnectingState:
            return "连接中";
        case QCanBusDevice::ConnectedState:
            return "已连接";
        case QCanBusDevice::ClosingState:
            return "关闭中";
        default:
            return "未知状态";
    }
}

/**
 * @brief 检查总线是否处于错误状态
 */
bool DriverCAN::hasBusError() const
{
    if (!m_canDevice) return false;
    return m_canDevice->state() != QCanBusDevice::ConnectedState;
}

/**
 * @brief 获取接收帧计数
 */
quint64 DriverCAN::getReceivedFrameCount() const
{
    return m_receivedFrameCount;
}

/**
 * @brief 获取发送帧计数
 */
quint64 DriverCAN::getSentFrameCount() const
{
    return m_sentFrameCount;
}

// ========== 静态辅助方法 ==========

/**
 * @brief 获取系统中所有可用CAN接口
 */
QStringList DriverCAN::getAvailableInterfaces()
{
    QStringList interfaces;
    
    // 通过QCanBus获取socketcan插件支持的接口
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        QStringLiteral("socketcan")
    );
    
    for (const QCanBusDeviceInfo &info : devices) {
        interfaces.append(info.name());
    }
    
    // 如果QCanBus没有找到，尝试从/sys/class/net扫描
    if (interfaces.isEmpty()) {
        QDir netDir("/sys/class/net");
        if (netDir.exists()) {
            QStringList entries = netDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                QString typePath = QString("/sys/class/net/%1/type").arg(entry);
                QFile typeFile(typePath);
                if (typeFile.open(QIODevice::ReadOnly)) {
                    QString type = typeFile.readAll().trimmed();
                    if (type == "280") {  // CAN设备的type为280
                        interfaces.append(entry);
                    }
                    typeFile.close();
                }
            }
        }
    }
    
    return interfaces;
}

/**
 * @brief 打印所有可用CAN接口信息
 */
void DriverCAN::printAvailableInterfaces()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  可用的CAN接口";
    qInfo() << "========================================";
    
    QStringList interfaces = getAvailableInterfaces();
    
    if (interfaces.isEmpty()) {
        qInfo() << "  未发现CAN接口";
    } else {
        qInfo() << "  发现" << interfaces.size() << "个CAN接口:";
        for (const QString &iface : interfaces) {
            qInfo() << "    -" << iface;
            
            // 读取接口状态
            QString statePath = QString("/sys/class/net/%1/operstate").arg(iface);
            QFile stateFile(statePath);
            if (stateFile.open(QIODevice::ReadOnly)) {
                QString state = stateFile.readAll().trimmed();
                qInfo() << "      状态:" << state;
                stateFile.close();
            }
        }
    }
    
    qInfo() << "========================================";
    qInfo() << "";
}

/**
 * @brief 检查CAN接口是否存在
 */
bool DriverCAN::interfaceExists(const QString &interfaceName)
{
    return getAvailableInterfaces().contains(interfaceName);
}

/**
 * @brief CAN帧转字符串
 */
QString DriverCAN::frameToString(const QCanBusFrame &frame)
{
    QString result;
    
    // 帧ID
    result += QString("ID=0x%1").arg(frame.frameId(), 0, 16);
    
    // 帧类型
    if (frame.hasExtendedFrameFormat()) {
        result += " [EXT]";
    } else {
        result += " [STD]";
    }
    
    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame) {
        result += " [RTR]";
    } else if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        result += " [ERR]";
    }
    
    // 数据
    QByteArray payload = frame.payload();
    result += QString(" DLC=%1").arg(payload.size());
    
    if (!payload.isEmpty()) {
        result += " DATA=[";
        for (int i = 0; i < payload.size(); ++i) {
            if (i > 0) result += " ";
            result += QString("%1").arg((quint8)payload[i], 2, 16, QChar('0')).toUpper();
        }
        result += "]";
    }
    
    return result;
}

// ========== 私有槽函数 ==========

/**
 * @brief 帧接收就绪槽函数
 */
void DriverCAN::onFramesReceived()
{
    if (!m_canDevice) return;
    
    while (m_canDevice->framesAvailable()) {
        QCanBusFrame frame = m_canDevice->readFrame();
        
        if (frame.isValid()) {
            m_receivedFrameCount++;
            
            // 添加到接收缓冲区
            m_receiveBuffer.append(frame);
            
            // 检查缓冲区是否溢出
            if (m_receiveBuffer.size() > m_receiveBufferMaxSize)
            {
                qWarning() << "[DriverCAN] 接收缓冲区溢出，丢弃旧帧";
                qWarning() << "  当前帧数:" << m_receiveBuffer.size() << "最大:" << m_receiveBufferMaxSize;
                
                // 移除最旧的帧
                m_receiveBuffer.remove(0, m_receiveBuffer.size() - m_receiveBufferMaxSize);
            }
            
            qDebug() << "[DriverCAN] 接收帧:" << frameToString(frame)
                     << "缓冲区:" << m_receiveBuffer.size() << "帧";
            
            // 发送帧接收信号
            emit frameReceived(frame);
        }
    }
}

/**
 * @brief 错误处理槽函数
 */
void DriverCAN::onError(QCanBusDevice::CanBusError error)
{
    if (!m_canDevice) return;
    
    m_lastError = m_canDevice->errorString();
    
    CANError canError;
    switch (error) {
        case QCanBusDevice::ReadError:
            canError = ReadError;
            break;
        case QCanBusDevice::WriteError:
            canError = WriteError;
            break;
        case QCanBusDevice::ConnectionError:
            canError = ConnectionError;
            break;
        case QCanBusDevice::ConfigurationError:
            canError = ConfigurationError;
            break;
        default:
            canError = UnknownError;
            break;
    }
    
    qWarning() << "[DriverCAN] 错误:" << m_lastError;
    emit this->error(canError, m_lastError);
}

/**
 * @brief 状态变化槽函数
 */
void DriverCAN::onStateChanged(QCanBusDevice::CanBusDeviceState state)
{
    qInfo() << "[DriverCAN] 状态变化:" << getStateString();
    emit stateChanged(state);
}

// ========== 缓冲区管理 ==========

/**
 * @brief 从接收缓冲区读取一帧
 * @return CAN帧，缓冲区为空返回无效帧
 */
QCanBusFrame DriverCAN::readFrame()
{
    if (m_receiveBuffer.isEmpty())
    {
        return QCanBusFrame();
    }
    
    QCanBusFrame frame = m_receiveBuffer.takeFirst();
    
    qDebug() << "[DriverCAN] 从缓冲区读取一帧，剩余:" << m_receiveBuffer.size() << "帧";
    
    return frame;
}

/**
 * @brief 从接收缓冲区读取所有帧
 * @return CAN帧列表
 */
QVector<QCanBusFrame> DriverCAN::readAllFrames()
{
    QVector<QCanBusFrame> frames = m_receiveBuffer;
    m_receiveBuffer.clear();
    
    qDebug() << "[DriverCAN] 从缓冲区读取:" << frames.size() << "帧";
    
    return frames;
}

/**
 * @brief 获取接收缓冲区中的帧数量
 * @return 帧数量
 */
int DriverCAN::getBufferedFrameCount() const
{
    return m_receiveBuffer.size();
}

/**
 * @brief 清空接收缓冲区
 */
void DriverCAN::clearReceiveBuffer()
{
    int count = m_receiveBuffer.size();
    m_receiveBuffer.clear();
    
    qDebug() << "[DriverCAN] 清空接收缓冲区，丢弃:" << count << "帧";
}

/**
 * @brief 设置接收缓冲区最大帧数
 * @param maxFrames 最大帧数
 */
void DriverCAN::setReceiveBufferMaxSize(int maxFrames)
{
    m_receiveBufferMaxSize = maxFrames;
    qDebug() << "[DriverCAN] 设置接收缓冲区最大帧数:" << maxFrames;
}

