/***************************************************************
 * Copyright: Alex
 * FileName: DriverCANHighPerf.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 高性能CAN总线驱动实现
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#include "drivers/can/DriverCANHighPerf.h"
#include <QDebug>

// ========================================
// CANReceiveThread 实现
// ========================================

/**
 * @brief 构造函数
 */
CANReceiveThread::CANReceiveThread(QCanBusDevice *device, QObject *parent)
    : QThread(parent)
    , m_device(device)
    , m_maxBufferSize(1000)
    , m_receivedCount(0)
    , m_droppedCount(0)
{
    m_running.store(0);
    qInfo() << "[CANReceiveThread] 创建独立接收线程";
}

/**
 * @brief 析构函数
 */
CANReceiveThread::~CANReceiveThread()
{
    stopReceiving();
    qInfo() << "[CANReceiveThread] 销毁接收线程";
}

/**
 * @brief 启动接收线程
 */
void CANReceiveThread::startReceiving()
{
    if (m_running.load() != 0)
    {
        qWarning() << "[CANReceiveThread] 接收线程已在运行";
        return;
    }
    
    m_running.store(1);
    start();  // 启动QThread
    
    qInfo() << "[CANReceiveThread] ✓ 独立接收线程已启动";
}

/**
 * @brief 停止接收线程
 */
void CANReceiveThread::stopReceiving()
{
    if (m_running.load() == 0)
    {
        return;
    }
    
    qInfo() << "[CANReceiveThread] 正在停止接收线程...";
    
    m_running.store(0);
    
    // 等待线程退出（最多3秒）
    if (!wait(3000))
    {
        qWarning() << "[CANReceiveThread] 线程未能及时退出，强制终止";
        terminate();
        wait();
    }
    
    qInfo() << "[CANReceiveThread] ✓ 接收线程已停止";
}

/**
 * @brief 线程运行函数（类似can_972.c的Can_Read_Thread）
 * 
 * 核心接收循环：
 *   1. 实时检查CAN设备是否有新帧
 *   2. 读取帧并添加到缓冲队列
 *   3. 发出信号通知上层
 *   4. 处理缓冲区溢出
 */
void CANReceiveThread::run()
{
    qInfo() << "[CANReceiveThread] 接收线程开始运行...";
    
    int consecutiveErrors = 0;
    const int MAX_CONSECUTIVE_ERRORS = 10;
    
    while (m_running.load() != 0)
    {
        if (!m_device)
        {
            msleep(100);
            continue;
        }
        
        // 检查是否有帧可读
        if (m_device->framesAvailable() > 0)
        {
            // 批量读取所有可用帧（提高效率）
            while (m_device->framesAvailable() > 0)
            {
                QCanBusFrame frame = m_device->readFrame();
                
                if (frame.isValid())
                {
                    consecutiveErrors = 0;  // 重置错误计数
                    m_receivedCount++;
                    
                    // 加锁添加到缓冲队列
                    m_bufferMutex.lock();
                    
                    // 检查缓冲区是否满
                    if (m_buffer.size() >= m_maxBufferSize)
                    {
                        // 缓冲区满，丢弃最旧的帧
                        m_buffer.dequeue();
                        m_droppedCount++;
                        
                        // 每丢弃100帧警告一次
                        if (m_droppedCount % 100 == 0)
                        {
                            qWarning() << "[CANReceiveThread] 缓冲区溢出，已丢弃" 
                                       << m_droppedCount << "帧";
                            emit bufferOverflow(m_droppedCount);
                        }
                    }
                    
                    // 添加新帧到队列
                    m_buffer.enqueue(frame);
                    
                    m_bufferMutex.unlock();
                    
                    // 发出信号通知（注意：这是在接收线程中发出）
                    emit frameReceived(frame);
                }
                else
                {
                    consecutiveErrors++;
                    if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS)
                    {
                        qCritical() << "[CANReceiveThread] 连续错误次数过多，暂停接收";
                        msleep(100);
                        consecutiveErrors = 0;
                    }
                }
            }
        }
        else
        {
            // 无数据，短暂休眠（类似can_972.c的usleep(10000)）
            // 10ms轮询间隔，平衡性能和CPU占用
            usleep(10000);  // 10ms
        }
    }
    
    qInfo() << "[CANReceiveThread] 接收线程退出";
    qInfo() << "  总接收: " << m_receivedCount << " 帧";
    qInfo() << "  总丢弃: " << m_droppedCount << " 帧";
}

/**
 * @brief 从缓冲区读取一帧
 */
QCanBusFrame CANReceiveThread::readFrame()
{
    QMutexLocker locker(&m_bufferMutex);
    
    if (m_buffer.isEmpty())
    {
        return QCanBusFrame();
    }
    
    return m_buffer.dequeue();
}

/**
 * @brief 从缓冲区读取所有帧
 */
QVector<QCanBusFrame> CANReceiveThread::readAllFrames()
{
    QMutexLocker locker(&m_bufferMutex);
    
    QVector<QCanBusFrame> frames;
    frames.reserve(m_buffer.size());
    
    while (!m_buffer.isEmpty())
    {
        frames.append(m_buffer.dequeue());
    }
    
    return frames;
}

/**
 * @brief 获取缓冲区帧数
 */
int CANReceiveThread::getBufferedFrameCount() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_buffer.size();
}

/**
 * @brief 清空缓冲区
 */
void CANReceiveThread::clearBuffer()
{
    QMutexLocker locker(&m_bufferMutex);
    m_buffer.clear();
    qDebug() << "[CANReceiveThread] 清空缓冲区";
}

/**
 * @brief 设置缓冲区最大大小
 */
void CANReceiveThread::setMaxBufferSize(int maxFrames)
{
    QMutexLocker locker(&m_bufferMutex);
    m_maxBufferSize = maxFrames;
    qInfo() << "[CANReceiveThread] 设置最大缓冲帧数:" << maxFrames;
}

// ========================================
// DriverCANHighPerf 实现
// ========================================

/**
 * @brief 构造函数
 */
DriverCANHighPerf::DriverCANHighPerf(const QString &interfaceName, QObject *parent)
    : DriverCAN(interfaceName, parent)
    , m_receiveThread(nullptr)
    , m_threadedReceiveEnabled(true)  // 默认启用独立线程
{
    qInfo() << "[DriverCANHighPerf] 创建高性能CAN驱动:" << interfaceName;
}

/**
 * @brief 析构函数
 */
DriverCANHighPerf::~DriverCANHighPerf()
{
    close();
    
    if (m_receiveThread)
    {
        m_receiveThread->stopReceiving();
        delete m_receiveThread;
        m_receiveThread = nullptr;
    }
    
    qInfo() << "[DriverCANHighPerf] 销毁高性能CAN驱动";
}

/**
 * @brief 打开CAN设备
 */
bool DriverCANHighPerf::open(quint32 bitrate)
{
    // 先调用基类打开
    if (!DriverCAN::open(bitrate))
    {
        return false;
    }
    
    // 如果启用了独立线程模式，创建接收线程
    if (m_threadedReceiveEnabled)
    {
        qInfo() << "[DriverCANHighPerf] 创建独立接收线程...";
        
        // 获取底层CAN设备
        QCanBusDevice *device = getCanDevice();
        if (!device)
        {
            qCritical() << "[DriverCANHighPerf] 无法获取CAN设备对象";
            return false;
        }
        
        // 创建接收线程
        m_receiveThread = new CANReceiveThread(device, this);
        
        // 连接线程信号到本对象（信号中转）
        connect(m_receiveThread, &CANReceiveThread::frameReceived,
                this, &DriverCANHighPerf::highPerfFrameReceived);
        
        connect(m_receiveThread, &CANReceiveThread::bufferOverflow,
                this, [](int dropped) {
            qWarning() << "[DriverCANHighPerf] 缓冲区溢出，丢弃" << dropped << "帧";
        });
        
        // 启动接收线程
        m_receiveThread->startReceiving();
        
        qInfo() << "[DriverCANHighPerf] ✓ 独立接收线程已启动";
    }
    
    return true;
}

/**
 * @brief 关闭CAN设备
 */
void DriverCANHighPerf::close()
{
    // 先停止接收线程
    if (m_receiveThread)
    {
        m_receiveThread->stopReceiving();
    }
    
    // 再调用基类关闭
    DriverCAN::close();
}

/**
 * @brief 从独立线程缓冲区读取一帧
 */
QCanBusFrame DriverCANHighPerf::readFrameFromThread()
{
    if (!m_receiveThread)
    {
        return QCanBusFrame();
    }
    
    return m_receiveThread->readFrame();
}

/**
 * @brief 从独立线程缓冲区读取所有帧
 */
QVector<QCanBusFrame> DriverCANHighPerf::readAllFramesFromThread()
{
    if (!m_receiveThread)
    {
        return QVector<QCanBusFrame>();
    }
    
    return m_receiveThread->readAllFrames();
}

/**
 * @brief 获取线程缓冲区帧数
 */
int DriverCANHighPerf::getThreadBufferCount() const
{
    if (!m_receiveThread)
    {
        return 0;
    }
    
    return m_receiveThread->getBufferedFrameCount();
}

/**
 * @brief 获取线程接收统计
 */
quint64 DriverCANHighPerf::getThreadReceivedCount() const
{
    if (!m_receiveThread)
    {
        return 0;
    }
    
    return m_receiveThread->getReceivedCount();
}

quint64 DriverCANHighPerf::getThreadDroppedCount() const
{
    if (!m_receiveThread)
    {
        return 0;
    }
    
    return m_receiveThread->getDroppedCount();
}

/**
 * @brief 设置线程优先级
 */
void DriverCANHighPerf::setThreadPriority(QThread::Priority priority)
{
    if (m_receiveThread && m_receiveThread->isRunning())
    {
        m_receiveThread->setPriority(priority);
        qInfo() << "[DriverCANHighPerf] 设置线程优先级:" << priority;
    }
}

/**
 * @brief 启用/禁用独立接收线程
 */
void DriverCANHighPerf::setThreadedReceiveEnabled(bool enable)
{
    m_threadedReceiveEnabled = enable;
    qInfo() << "[DriverCANHighPerf] 独立线程模式:" << (enable ? "启用" : "禁用");
}

/**
 * @brief 检查独立线程是否运行
 */
bool DriverCANHighPerf::isThreadedReceiveRunning() const
{
    return m_receiveThread && m_receiveThread->isRunning();
}

