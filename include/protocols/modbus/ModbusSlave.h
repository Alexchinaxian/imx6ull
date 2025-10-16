/***************************************************************
 * Copyright: Alex
 * FileName: protocol_modbus_slave.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus RTU从站（Slave）协议驱动
 *
 * 功能说明:
 *   实现Modbus RTU从站功能，响应主站的读写请求
 *   支持将系统数据（如温度）映射到Modbus寄存器
 *
 * 应用场景:
 *   - 将嵌入式设备作为Modbus从站
 *   - 对外提供温度、状态等数据
 *   - 接收主站的控制命令
 *
 * Modbus寄存器映射:
 *   保持寄存器（Holding Registers）:
 *     0x0000: 系统温度（整数部分）
 *     0x0001: 系统温度（小数部分x100）
 *     0x0002: 系统状态（0=正常，1=告警）
 *     0x0003-0x000F: 预留
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_PROTOCOLS_MODBUS_SLAVE_H
#define IMX6ULL_PROTOCOLS_MODBUS_SLAVE_H

#include "protocols/IProtocolInterface.h"
#include <QSerialPort>
#include <QTimer>
#include <QVector>

/***************************************************************
 * 类名: ProtocolModbusSlave
 * 功能: Modbus RTU从站协议类
 * 
 * 使用示例:
 *   ProtocolModbusSlave slave("/dev/ttymxc2", 1);  // 从站地址=1
 *   slave.configure({{"baudrate", 9600}});
 *   slave.connect();
 *   
 *   // 设置温度数据
 *   slave.setTemperature(45.5);
 *   
 *   // 自动响应主站请求
 ***************************************************************/
class ProtocolModbusSlave : public IProtocolInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param portName 串口名称
     * @param slaveAddress 从站地址（1-247）
     * @param parent 父对象指针
     */
    explicit ProtocolModbusSlave(const QString &portName, quint8 slaveAddress = 1, 
                                  QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ProtocolModbusSlave() override;
    
    // ========== 实现IProtocolInterface接口 ==========
    
    ProtocolType getProtocolType() const override {
        return ProtocolType::ModbusRTU;
    }
    
    QString getProtocolName() const override {
        return "Modbus RTU Slave";
    }
    
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    bool configure(const QMap<QString, QVariant> &config) override;
    
public slots:
    // ========== Modbus从站特定方法 ==========
    
    /**
     * @brief 设置温度值
     * @param temperature 温度（°C）
     * @note 自动映射到寄存器0x0000和0x0001
     */
    void setTemperature(float temperature);
    
    /**
     * @brief 设置系统状态
     * @param status 状态值（0=正常，1=告警）
     */
    void setSystemStatus(quint16 status);

public:
    
    /**
     * @brief 设置保持寄存器值
     * @param address 寄存器地址
     * @param value 寄存器值
     * @return true=成功, false=失败（地址超出范围）
     */
    bool setHoldingRegister(quint16 address, quint16 value);
    
    /**
     * @brief 获取保持寄存器值
     * @param address 寄存器地址
     * @return 寄存器值
     */
    quint16 getHoldingRegister(quint16 address) const;
    
    /**
     * @brief 设置从站地址
     * @param address 从站地址（1-247）
     */
    void setSlaveAddress(quint8 address);
    
    /**
     * @brief 获取从站地址
     * @return 当前从站地址
     */
    quint8 getSlaveAddress() const { return m_slaveAddress; }

signals:
    /**
     * @brief 接收到读请求信号
     * @param functionCode 功能码
     * @param address 起始地址
     * @param count 数量
     */
    void readRequest(quint8 functionCode, quint16 address, quint16 count);
    
    /**
     * @brief 接收到写请求信号
     * @param functionCode 功能码
     * @param address 起始地址
     * @param value 写入值
     */
    void writeRequest(quint8 functionCode, quint16 address, quint16 value);

private slots:
    /**
     * @brief 串口数据接收槽函数
     */
    void onSerialDataReceived();

private:
    /**
     * @brief 处理接收到的Modbus请求
     * @param request 请求帧
     */
    void processRequest(const QByteArray &request);
    
    /**
     * @brief 处理读保持寄存器请求（功能码0x03）
     * @param request 请求帧
     */
    void handleReadHoldingRegisters(const QByteArray &request);
    
    /**
     * @brief 处理读输入寄存器请求（功能码0x04）
     * @param request 请求帧
     */
    void handleReadInputRegisters(const QByteArray &request);
    
    /**
     * @brief 处理写单个寄存器请求（功能码0x06）
     * @param request 请求帧
     */
    void handleWriteSingleRegister(const QByteArray &request);
    
    /**
     * @brief 处理写多个寄存器请求（功能码0x10）
     * @param request 请求帧
     */
    void handleWriteMultipleRegisters(const QByteArray &request);
    
    /**
     * @brief 发送响应帧
     * @param response 响应数据（不含CRC）
     */
    void sendResponse(const QByteArray &response);
    
    /**
     * @brief 发送异常响应
     * @param functionCode 功能码
     * @param exceptionCode 异常码
     */
    void sendException(quint8 functionCode, quint8 exceptionCode);
    
    /**
     * @brief 计算CRC16校验码
     * @param data 数据
     * @return CRC16值
     */
    quint16 calculateCRC16(const QByteArray &data);
    
    /**
     * @brief 验证CRC
     * @param data 数据（含CRC）
     * @return true=有效, false=无效
     */
    bool verifyCRC(const QByteArray &data);

private:
    QSerialPort *m_serialPort;          // 串口对象
    QString m_portName;                 // 串口名称
    quint8 m_slaveAddress;              // 从站地址
    
    QByteArray m_receiveBuffer;         // 接收缓冲区
    QTimer *m_frameTimer;               // 帧间隔定时器
    
    // 串口配置参数
    int m_baudrate;                     // 波特率
    QSerialPort::DataBits m_dataBits;   // 数据位
    QSerialPort::Parity m_parity;       // 校验位
    QSerialPort::StopBits m_stopBits;   // 停止位
    
    // Modbus数据寄存器
    QVector<quint16> m_holdingRegisters; // 保持寄存器（可读写）
    QVector<quint16> m_inputRegisters;   // 输入寄存器（只读）
    
    static const int MAX_REGISTERS = 256; // 最大寄存器数量
};

#endif // PROTOCOL_MODBUS_SLAVE_H

