/***************************************************************
 * Copyright: Alex
 * FileName: protocol_modbus_rtu.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus RTU协议驱动（串口）
 *
 * 功能说明:
 *   实现标准Modbus RTU协议，支持：
 *   - 功能码：01(读线圈), 02(读离散输入), 03(读保持寄存器),
 *            04(读输入寄存器), 05(写单个线圈), 06(写单个寄存器),
 *            15(写多个线圈), 16(写多个寄存器)
 *   - CRC16校验
 *   - 超时处理
 *   - 错误重试
 *
 * Modbus RTU帧格式:
 *   [从站地址(1字节)][功能码(1字节)][数据(N字节)][CRC16(2字节)]
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_PROTOCOLS_MODBUS_RTU_H
#define IMX6ULL_PROTOCOLS_MODBUS_RTU_H

#include "protocols/IProtocolInterface.h"
#include <QSerialPort>
#include <QTimer>

/***************************************************************
 * 枚举: ModbusFunctionCode
 * 功能: Modbus功能码定义
 ***************************************************************/
enum class ModbusFunctionCode : quint8 {
    ReadCoils = 0x01,                   // 读线圈状态
    ReadDiscreteInputs = 0x02,          // 读离散输入状态
    ReadHoldingRegisters = 0x03,        // 读保持寄存器
    ReadInputRegisters = 0x04,          // 读输入寄存器
    WriteSingleCoil = 0x05,             // 写单个线圈
    WriteSingleRegister = 0x06,         // 写单个寄存器
    WriteMultipleCoils = 0x0F,          // 写多个线圈
    WriteMultipleRegisters = 0x10       // 写多个寄存器
};

/***************************************************************
 * 类名: ProtocolModbusRTU
 * 功能: Modbus RTU协议驱动类
 * 
 * 使用示例:
 *   ProtocolModbusRTU modbus("/dev/ttyS1");
 *   modbus.setSlaveAddress(1);
 *   modbus.configure({{"baudrate", 9600}, {"parity", "N"}});
 *   modbus.connect();
 *   
 *   // 读取保持寄存器
 *   QVector<quint16> values = modbus.readHoldingRegisters(0x0000, 10);
 *   
 *   // 写入单个寄存器
 *   modbus.writeSingleRegister(0x0000, 1234);
 ***************************************************************/
class ProtocolModbusRTU : public IProtocolInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param portName 串口名称（如"/dev/ttyS1"）
     * @param parent 父对象指针
     */
    explicit ProtocolModbusRTU(const QString &portName, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ProtocolModbusRTU() override;
    
    // ========== 实现IProtocolInterface接口 ==========
    
    ProtocolType getProtocolType() const override {
        return ProtocolType::ModbusRTU;
    }
    
    QString getProtocolName() const override {
        return "Modbus RTU";
    }
    
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    bool configure(const QMap<QString, QVariant> &config) override;
    
    // ========== Modbus特定方法 ==========
    
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
    
    /**
     * @brief 设置响应超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setTimeout(int timeout);
    
    /**
     * @brief 读取线圈状态 (功能码0x01)
     * @param startAddress 起始地址
     * @param count 读取数量
     * @return 线圈状态列表（true=ON, false=OFF）
     */
    QVector<bool> readCoils(quint16 startAddress, quint16 count);
    
    /**
     * @brief 读取离散输入状态 (功能码0x02)
     * @param startAddress 起始地址
     * @param count 读取数量
     * @return 离散输入状态列表
     */
    QVector<bool> readDiscreteInputs(quint16 startAddress, quint16 count);
    
    /**
     * @brief 读取保持寄存器 (功能码0x03)
     * @param startAddress 起始地址
     * @param count 读取数量
     * @return 寄存器值列表
     */
    QVector<quint16> readHoldingRegisters(quint16 startAddress, quint16 count);
    
    /**
     * @brief 读取输入寄存器 (功能码0x04)
     * @param startAddress 起始地址
     * @param count 读取数量
     * @return 寄存器值列表
     */
    QVector<quint16> readInputRegisters(quint16 startAddress, quint16 count);
    
    /**
     * @brief 写单个线圈 (功能码0x05)
     * @param address 线圈地址
     * @param value 线圈值（true=ON, false=OFF）
     * @return true=成功, false=失败
     */
    bool writeSingleCoil(quint16 address, bool value);
    
    /**
     * @brief 写单个寄存器 (功能码0x06)
     * @param address 寄存器地址
     * @param value 寄存器值
     * @return true=成功, false=失败
     */
    bool writeSingleRegister(quint16 address, quint16 value);
    
    /**
     * @brief 写多个线圈 (功能码0x0F)
     * @param startAddress 起始地址
     * @param values 线圈值列表
     * @return true=成功, false=失败
     */
    bool writeMultipleCoils(quint16 startAddress, const QVector<bool> &values);
    
    /**
     * @brief 写多个寄存器 (功能码0x10)
     * @param startAddress 起始地址
     * @param values 寄存器值列表
     * @return true=成功, false=失败
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
     * @brief 串口数据接收槽函数
     */
    void onSerialDataReceived();
    
    /**
     * @brief 响应超时槽函数
     */
    void onResponseTimeout();

private:
    /**
     * @brief 发送Modbus请求并等待响应
     * @param request 请求帧
     * @return 响应帧（空表示失败）
     */
    QByteArray sendRequest(const QByteArray &request);
    
    /**
     * @brief 构建Modbus请求帧
     * @param functionCode 功能码
     * @param data 数据部分
     * @return 完整请求帧（含CRC）
     */
    QByteArray buildRequest(quint8 functionCode, const QByteArray &data);
    
    /**
     * @brief 验证响应帧
     * @param response 响应帧
     * @return true=有效, false=无效
     */
    bool validateResponse(const QByteArray &response);
    
    /**
     * @brief 计算CRC16校验码
     * @param data 数据
     * @return CRC16值
     */
    quint16 calculateCRC16(const QByteArray &data);
    
    /**
     * @brief 解析布尔值列表（用于线圈/离散输入）
     * @param data 数据字节
     * @param count 数量
     * @return 布尔值列表
     */
    QVector<bool> parseBooleans(const QByteArray &data, quint16 count);
    
    /**
     * @brief 解析寄存器值列表
     * @param data 数据字节
     * @return 寄存器值列表
     */
    QVector<quint16> parseRegisters(const QByteArray &data);

private:
    QSerialPort *m_serialPort;          // 串口对象
    QString m_portName;                 // 串口名称
    quint8 m_slaveAddress;              // 从站地址
    
    QTimer *m_responseTimer;            // 响应超时定时器
    int m_timeout;                      // 超时时间（毫秒）
    
    QByteArray m_receiveBuffer;         // 接收缓冲区
    bool m_waitingForResponse;          // 等待响应标志
    
    // 串口配置参数
    int m_baudrate;                     // 波特率
    QSerialPort::DataBits m_dataBits;   // 数据位
    QSerialPort::Parity m_parity;       // 校验位
    QSerialPort::StopBits m_stopBits;   // 停止位
};

#endif // PROTOCOL_MODBUS_RTU_H

