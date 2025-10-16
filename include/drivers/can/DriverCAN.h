/***************************************************************
 * Copyright: Alex
 * FileName: DriverCAN.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: CAN总线驱动类 - 基于Qt QCanBus
 *
 * 功能说明:
 *   封装Qt的QCanBus功能，提供统一的CAN总线通信接口
 *   支持SocketCAN（Linux标准CAN接口）
 *
 * CAN总线说明:
 *   - 波特率: 常用 125K, 250K, 500K, 1M
 *   - 帧类型: 标准帧(11位ID)、扩展帧(29位ID)
 *   - 传输: 数据帧、远程帧、错误帧
 *
 * 使用示例:
 *   DriverCAN can("can0");
 *   can.open(250000);  // 250K波特率
 *   can.writeFrame(0x123, QByteArray::fromHex("0102030405060708"));
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_DRIVERS_CAN_H
#define IMX6ULL_DRIVERS_CAN_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QCanBusDeviceInfo>

/***************************************************************
 * 类名: DriverCAN
 * 功能: CAN总线驱动类
 * 
 * 描述:
 *   基于Qt QCanBus实现的CAN总线驱动，支持Linux SocketCAN
 *   提供完整的CAN帧收发、过滤、错误处理功能
 * 
 * 特性:
 *   1. 支持标准帧和扩展帧
 *   2. 可配置波特率
 *   3. CAN帧过滤
 *   4. 异步接收
 *   5. 错误状态监控
 * 
 * 使用流程:
 *   1. 创建DriverCAN对象
 *   2. 配置波特率（可选）
 *   3. open()打开CAN设备
 *   4. writeFrame()发送帧
 *   5. 通过信号接收帧
 *   6. close()关闭设备
 ***************************************************************/
class DriverCAN : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief CAN帧类型枚举
     */
    enum FrameType {
        DataFrame = 0,          // 数据帧
        RemoteRequestFrame = 1, // 远程请求帧
        ErrorFrame = 2,         // 错误帧
        UnknownFrame = 3        // 未知帧类型
    };
    Q_ENUM(FrameType)
    
    /**
     * @brief CAN错误类型枚举
     */
    enum CANError {
        NoError = 0,           // 无错误
        ReadError = 1,         // 读取错误
        WriteError = 2,        // 写入错误
        ConnectionError = 3,   // 连接错误
        ConfigurationError = 4,// 配置错误
        UnknownError = 5       // 未知错误
    };
    Q_ENUM(CANError)
    
    /**
     * @brief 构造函数
     * @param interfaceName CAN接口名称（例如：can0, can1）
     * @param parent 父对象指针
     */
    explicit DriverCAN(const QString &interfaceName, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * @note 自动关闭CAN设备
     */
    ~DriverCAN();
    
    // ========== CAN设备打开和关闭 ==========
    
    /**
     * @brief 打开CAN设备
     * @param bitrate 波特率（可选，0表示使用系统配置）
     * @return true=成功, false=失败
     * @note 常用波特率: 125000, 250000, 500000, 1000000
     */
    virtual bool open(quint32 bitrate = 0);
    
    /**
     * @brief 关闭CAN设备
     */
    virtual void close();
    
    /**
     * @brief 检查CAN设备是否已打开
     * @return true=已打开, false=未打开
     */
    bool isOpen() const;
    
    // ========== CAN配置 ==========
    
    /**
     * @brief 设置波特率
     * @param bitrate 波特率（bps）
     * @return true=成功, false=失败
     * @note 必须在设备关闭时设置
     */
    bool setBitrate(quint32 bitrate);
    
    /**
     * @brief 获取当前波特率
     * @return 当前波特率
     */
    quint32 getBitrate() const;
    
    /**
     * @brief 设置接收过滤器
     * @param filterId 过滤ID
     * @param filterMask 过滤掩码
     * @return true=成功, false=失败
     */
    bool setFilter(quint32 filterId, quint32 filterMask);
    
    /**
     * @brief 清除所有过滤器
     */
    void clearFilters();
    
    // ========== CAN帧发送 ==========
    
    /**
     * @brief 发送标准数据帧
     * @param frameId 帧ID（11位标准帧）
     * @param data 数据（最多8字节）
     * @return true=成功, false=失败
     */
    bool writeFrame(quint32 frameId, const QByteArray &data);
    
    /**
     * @brief 发送扩展数据帧
     * @param frameId 帧ID（29位扩展帧）
     * @param data 数据（最多8字节）
     * @return true=成功, false=失败
     */
    bool writeExtendedFrame(quint32 frameId, const QByteArray &data);
    
    /**
     * @brief 发送远程请求帧
     * @param frameId 帧ID
     * @param dlc 数据长度代码（0-8）
     * @return true=成功, false=失败
     */
    bool writeRemoteFrame(quint32 frameId, quint8 dlc);
    
    /**
     * @brief 发送QCanBusFrame对象
     * @param frame CAN帧对象
     * @return true=成功, false=失败
     */
    bool writeFrame(const QCanBusFrame &frame);
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 获取CAN接口名称
     * @return 接口名称
     */
    QString getInterfaceName() const;
    
    /**
     * @brief 获取错误信息
     * @return 错误描述字符串
     */
    QString getErrorString() const;
    
    /**
     * @brief 获取设备状态
     * @return 状态字符串
     */
    QString getStateString() const;
    
    /**
     * @brief 检查总线是否处于错误状态
     * @return true=错误, false=正常
     */
    bool hasBusError() const;
    
    /**
     * @brief 获取接收帧计数
     * @return 已接收帧数量
     */
    quint64 getReceivedFrameCount() const;
    
    /**
     * @brief 获取发送帧计数
     * @return 已发送帧数量
     */
    quint64 getSentFrameCount() const;
    
    /**
     * @brief 获取底层CAN设备对象（供子类使用）
     * @return QCanBusDevice指针
     */
    QCanBusDevice* getCanDevice() const { return m_canDevice; }
    
    // ========== 缓冲区管理 ==========
    
    /**
     * @brief 从接收缓冲区读取一帧
     * @return CAN帧，缓冲区为空返回无效帧
     */
    QCanBusFrame readFrame();
    
    /**
     * @brief 从接收缓冲区读取所有帧
     * @return CAN帧列表
     */
    QVector<QCanBusFrame> readAllFrames();
    
    /**
     * @brief 获取接收缓冲区中的帧数量
     * @return 帧数量
     */
    int getBufferedFrameCount() const;
    
    /**
     * @brief 清空接收缓冲区
     */
    void clearReceiveBuffer();
    
    /**
     * @brief 设置接收缓冲区最大帧数
     * @param maxFrames 最大帧数
     */
    void setReceiveBufferMaxSize(int maxFrames);
    
    // ========== 静态辅助方法 ==========
    
    /**
     * @brief 获取系统中所有可用CAN接口
     * @return CAN接口名称列表
     */
    static QStringList getAvailableInterfaces();
    
    /**
     * @brief 打印所有可用CAN接口信息
     */
    static void printAvailableInterfaces();
    
    /**
     * @brief 检查CAN接口是否存在
     * @param interfaceName 接口名称
     * @return true=存在, false=不存在
     */
    static bool interfaceExists(const QString &interfaceName);
    
    /**
     * @brief CAN帧转字符串（用于调试）
     * @param frame CAN帧对象
     * @return 帧信息字符串
     */
    static QString frameToString(const QCanBusFrame &frame);
    
signals:
    /**
     * @brief CAN帧接收信号
     * @param frame 接收到的CAN帧
     */
    void frameReceived(const QCanBusFrame &frame);
    
    /**
     * @brief CAN设备打开成功信号
     */
    void opened();
    
    /**
     * @brief CAN设备关闭信号
     */
    void closed();
    
    /**
     * @brief 错误信号
     * @param error 错误类型
     * @param errorString 错误描述信息
     */
    void error(CANError error, const QString &errorString);
    
    /**
     * @brief 状态变化信号
     * @param state 新的设备状态
     */
    void stateChanged(QCanBusDevice::CanBusDeviceState state);
    
private slots:
    /**
     * @brief 帧接收就绪槽函数
     * @note 由QCanBusDevice::framesReceived信号触发
     */
    void onFramesReceived();
    
    /**
     * @brief 错误处理槽函数
     * @param error CAN设备错误类型
     */
    void onError(QCanBusDevice::CanBusError error);
    
    /**
     * @brief 状态变化槽函数
     * @param state 新的设备状态
     */
    void onStateChanged(QCanBusDevice::CanBusDeviceState state);
    
private:
    QCanBusDevice *m_canDevice;      // Qt CAN设备对象
    QString m_interfaceName;         // CAN接口名称
    quint32 m_bitrate;               // 波特率
    bool m_isOpen;                   // 打开状态标志
    quint64 m_receivedFrameCount;    // 接收帧计数
    quint64 m_sentFrameCount;        // 发送帧计数
    QString m_lastError;             // 最后的错误信息
    
    // 接收缓冲区
    QVector<QCanBusFrame> m_receiveBuffer;  // 接收帧缓冲队列
    int m_receiveBufferMaxSize;             // 接收缓冲区最大帧数
    
    /**
     * @brief 创建CAN设备对象
     * @return true=成功, false=失败
     */
    bool createDevice();
    
    /**
     * @brief 连接设备信号
     */
    void connectDeviceSignals();
};

#endif // IMX6ULL_DRIVERS_CAN_H

