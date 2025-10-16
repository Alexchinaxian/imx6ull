/***************************************************************
 * Copyright: Alex
 * FileName: IProtocolInterface.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 协议层统一接口定义
 *
 * 功能说明:
 *   定义所有通信协议的统一接口，包括但不限于：
 *   - Modbus RTU/TCP
 *   - CANopen
 *   - MQTT
 *   - HTTP/REST
 *   - 自定义协议
 *
 * 设计理念:
 *   通过统一的接口，实现不同协议的标准化管理和调用
 *   支持协议的热插拔和动态加载
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_PROTOCOLS_INTERFACE_H
#define IMX6ULL_PROTOCOLS_INTERFACE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QMap>

/***************************************************************
 * 枚举: ProtocolType
 * 功能: 定义支持的协议类型
 ***************************************************************/
enum class ProtocolType {
    Unknown = 0,
    ModbusRTU,          // Modbus RTU协议（串口）
    ModbusTCP,          // Modbus TCP协议（网络）
    CANopen,            // CANopen协议
    MQTT,               // MQTT协议
    HTTP,               // HTTP协议
    WebSocket,          // WebSocket协议
    Custom              // 自定义协议
};

/***************************************************************
 * 枚举: ProtocolState
 * 功能: 协议状态定义
 ***************************************************************/
enum class ProtocolState {
    Disconnected = 0,   // 未连接
    Connecting,         // 连接中
    Connected,          // 已连接
    Error               // 错误状态
};

/***************************************************************
 * 类名: IProtocolInterface
 * 功能: 协议层统一接口（抽象基类）
 * 
 * 说明:
 *   所有协议驱动都必须实现此接口
 *   提供统一的连接、断开、读写、配置等方法
 * 
 * 使用示例:
 *   IProtocolInterface *protocol = new ProtocolModbusRTU("/dev/ttyS1");
 *   protocol->connect();
 *   protocol->writeData(address, data);
 *   QByteArray response = protocol->readData(address, length);
 *   protocol->disconnect();
 ***************************************************************/
class IProtocolInterface : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit IProtocolInterface(QObject *parent = nullptr)
        : QObject(parent), m_state(ProtocolState::Disconnected) {}
    
    /**
     * @brief 虚析构函数
     */
    virtual ~IProtocolInterface() {}
    
    // ========== 纯虚函数（必须实现） ==========
    
    /**
     * @brief 获取协议类型
     * @return 协议类型枚举
     */
    virtual ProtocolType getProtocolType() const = 0;
    
    /**
     * @brief 获取协议名称
     * @return 协议名称字符串
     */
    virtual QString getProtocolName() const = 0;
    
    /**
     * @brief 连接到设备/服务器
     * @return true=成功, false=失败
     */
    virtual bool connect() = 0;
    
    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief 检查是否已连接
     * @return true=已连接, false=未连接
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief 配置协议参数
     * @param config 配置参数映射表
     * @return true=成功, false=失败
     */
    virtual bool configure(const QMap<QString, QVariant> &config) = 0;
    
    // ========== 通用接口（可选实现） ==========
    
    /**
     * @brief 写入数据（通用方法）
     * @param address 目标地址
     * @param data 要写入的数据
     * @return true=成功, false=失败
     */
    virtual bool writeData(quint16 address, const QByteArray &data) {
        Q_UNUSED(address);
        Q_UNUSED(data);
        return false;
    }
    
    /**
     * @brief 读取数据（通用方法）
     * @param address 源地址
     * @param length 读取长度
     * @return 读取到的数据
     */
    virtual QByteArray readData(quint16 address, quint16 length) {
        Q_UNUSED(address);
        Q_UNUSED(length);
        return QByteArray();
    }
    
    /**
     * @brief 发送原始数据
     * @param data 原始数据
     * @return true=成功, false=失败
     */
    virtual bool sendRawData(const QByteArray &data) {
        Q_UNUSED(data);
        return false;
    }
    
    /**
     * @brief 获取协议状态
     * @return 当前状态
     */
    virtual ProtocolState getState() const {
        return m_state;
    }
    
    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    virtual QString getLastError() const {
        return m_lastError;
    }

signals:
    /**
     * @brief 连接成功信号
     */
    void connected();
    
    /**
     * @brief 断开连接信号
     */
    void disconnected();
    
    /**
     * @brief 数据接收信号
     * @param data 接收到的数据
     */
    void dataReceived(const QByteArray &data);
    
    /**
     * @brief 错误信号
     * @param error 错误信息
     */
    void error(const QString &error);
    
    /**
     * @brief 状态变化信号
     * @param state 新状态
     */
    void stateChanged(ProtocolState state);

protected:
    /**
     * @brief 设置协议状态
     * @param state 新状态
     */
    void setState(ProtocolState state) {
        if (m_state != state) {
            m_state = state;
            emit stateChanged(state);
        }
    }
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setError(const QString &error) {
        m_lastError = error;
        emit this->error(error);
    }

protected:
    ProtocolState m_state;      // 当前状态
    QString m_lastError;        // 最后的错误信息
};

#endif // IPROTOCOLINTERFACE_H

