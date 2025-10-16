/***************************************************************
 * Copyright: Alex
 * FileName: protocol_modbus_tcp.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus TCP协议驱动（网络）
 *
 * 功能说明:
 *   实现标准Modbus TCP协议（基于以太网）
 *   支持与Modbus RTU相同的功能码，但使用TCP/IP传输
 *
 * Modbus TCP帧格式:
 *   [事务ID(2)][协议ID(2)][长度(2)][单元ID(1)][功能码(1)][数据(N)]
 *
 * 与Modbus RTU的区别:
 *   1. 使用TCP/IP传输，无需CRC校验
 *   2. 增加MBAP头（7字节）
 *   3. 支持多个客户端同时连接
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_PROTOCOLS_MODBUS_TCP_H
#define IMX6ULL_PROTOCOLS_MODBUS_TCP_H

#include "protocols/IProtocolInterface.h"
#include <QTcpSocket>
#include <QTimer>

/***************************************************************
 * 类名: ProtocolModbusTCP
 * 功能: Modbus TCP协议驱动类
 * 
 * 使用示例:
 *   ProtocolModbusTCP modbus("192.168.1.100", 502);
 *   modbus.setUnitId(1);
 *   modbus.connect();
 *   
 *   // 读取保持寄存器
 *   QVector<quint16> values = modbus.readHoldingRegisters(0x0000, 10);
 *   
 *   // 写入单个寄存器
 *   modbus.writeSingleRegister(0x0000, 1234);
 ***************************************************************/
class ProtocolModbusTCP : public IProtocolInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param host 服务器地址（IP或域名）
     * @param port 服务器端口（默认502）
     * @param parent 父对象指针
     */
    explicit ProtocolModbusTCP(const QString &host, quint16 port = 502, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ProtocolModbusTCP() override;
    
    // ========== 实现IProtocolInterface接口 ==========
    
    ProtocolType getProtocolType() const override {
        return ProtocolType::ModbusTCP;
    }
    
    QString getProtocolName() const override {
        return "Modbus TCP";
    }
    
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    bool configure(const QMap<QString, QVariant> &config) override;
    
    // ========== Modbus特定方法 ==========
    
    /**
     * @brief 设置单元ID（设备ID）
     * @param unitId 单元ID（0-255）
     */
    void setUnitId(quint8 unitId);
    
    /**
     * @brief 获取单元ID
     * @return 当前单元ID
     */
    quint8 getUnitId() const { return m_unitId; }
    
    /**
     * @brief 设置响应超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setTimeout(int timeout);
    
    /**
     * @brief 读取线圈状态 (功能码0x01)
     */
    QVector<bool> readCoils(quint16 startAddress, quint16 count);
    
    /**
     * @brief 读取离散输入状态 (功能码0x02)
     */
    QVector<bool> readDiscreteInputs(quint16 startAddress, quint16 count);
    
    /**
     * @brief 读取保持寄存器 (功能码0x03)
     */
    QVector<quint16> readHoldingRegisters(quint16 startAddress, quint16 count);
    
    /**
     * @brief 读取输入寄存器 (功能码0x04)
     */
    QVector<quint16> readInputRegisters(quint16 startAddress, quint16 count);
    
    /**
     * @brief 写单个线圈 (功能码0x05)
     */
    bool writeSingleCoil(quint16 address, bool value);
    
    /**
     * @brief 写单个寄存器 (功能码0x06)
     */
    bool writeSingleRegister(quint16 address, quint16 value);
    
    /**
     * @brief 写多个线圈 (功能码0x0F)
     */
    bool writeMultipleCoils(quint16 startAddress, const QVector<bool> &values);
    
    /**
     * @brief 写多个寄存器 (功能码0x10)
     */
    bool writeMultipleRegisters(quint16 startAddress, const QVector<quint16> &values);

signals:
    /**
     * @brief Modbus异常信号
     * @param exceptionCode 异常码
     */
    void modbusException(quint8 exceptionCode);

private slots:
    /**
     * @brief TCP连接成功槽函数
     */
    void onTcpConnected();
    
    /**
     * @brief TCP断开连接槽函数
     */
    void onTcpDisconnected();
    
    /**
     * @brief TCP数据接收槽函数
     */
    void onTcpDataReceived();
    
    /**
     * @brief TCP错误槽函数
     */
    void onTcpError(QAbstractSocket::SocketError error);
    
    /**
     * @brief 响应超时槽函数
     */
    void onResponseTimeout();

private:
    /**
     * @brief 发送Modbus TCP请求并等待响应
     */
    QByteArray sendRequest(const QByteArray &pdu);
    
    /**
     * @brief 构建Modbus TCP请求帧（含MBAP头）
     */
    QByteArray buildRequest(quint8 functionCode, const QByteArray &data);
    
    /**
     * @brief 验证响应帧
     */
    bool validateResponse(const QByteArray &response, quint16 expectedTransactionId);
    
    /**
     * @brief 解析布尔值列表
     */
    QVector<bool> parseBooleans(const QByteArray &data, quint16 count);
    
    /**
     * @brief 解析寄存器值列表
     */
    QVector<quint16> parseRegisters(const QByteArray &data);

private:
    QTcpSocket *m_tcpSocket;            // TCP套接字
    QString m_host;                     // 服务器地址
    quint16 m_port;                     // 服务器端口
    quint8 m_unitId;                    // 单元ID
    
    QTimer *m_responseTimer;            // 响应超时定时器
    int m_timeout;                      // 超时时间（毫秒）
    
    QByteArray m_receiveBuffer;         // 接收缓冲区
    bool m_waitingForResponse;          // 等待响应标志
    quint16 m_transactionId;            // 事务ID计数器
};

#endif // PROTOCOL_MODBUS_TCP_H

