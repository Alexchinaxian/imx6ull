/***************************************************************
 * Copyright: Alex
 * FileName: driver_serial.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 串口驱动类 - 基于QSerialPort
 *
 * 功能说明:
 *   封装Qt串口功能，提供统一的串口通信接口
 *   支持串口自动发现、配置、读写等功能
 *
 * 串口参数说明:
 *   - 波特率: 9600, 19200, 38400, 57600, 115200等
 *   - 数据位: 5, 6, 7, 8
 *   - 停止位: 1, 1.5, 2
 *   - 校验位: None, Even, Odd, Space, Mark
 *   - 流控制: None, Hardware, Software
 *
 * 使用示例:
 *   DriverSerial serial("/dev/ttyS1");
 *   serial.open(QSerialPort::ReadWrite);
 *   serial.setBaudRate(115200);
 *   serial.write("Hello");
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_DRIVERS_SERIAL_H
#define IMX6ULL_DRIVERS_SERIAL_H

#include <QObject>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

/***************************************************************
 * 类名: DriverSerial
 * 功能: 串口驱动类
 * 
 * 描述:
 *   基于QSerialPort实现的串口驱动，提供完整的串口通信功能
 *   支持多种波特率、数据格式、流控制等配置
 * 
 * 特性:
 *   1. 自动串口发现（基于QSerialPortInfo）
 *   2. 灵活的参数配置
 *   3. 异步读写支持
 *   4. 信号通知机制
 *   5. 错误处理和重连机制
 * 
 * 使用流程:
 *   1. 创建DriverSerial对象
 *   2. 配置串口参数（波特率、数据位等）
 *   3. open()打开串口
 *   4. write()/read()进行通信
 *   5. close()关闭串口
 ***************************************************************/
class DriverSerial : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param portName 串口名称（例如：/dev/ttyS1, COM1）
     * @param parent 父对象指针
     */
    explicit DriverSerial(const QString &portName, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * @note 自动关闭串口
     */
    ~DriverSerial();
    
    // ========== 串口打开和关闭 ==========
    
    /**
     * @brief 打开串口
     * @param mode 打开模式（ReadOnly, WriteOnly, ReadWrite）
     * @return true=成功, false=失败
     */
    bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);
    
    /**
     * @brief 关闭串口
     */
    void close();
    
    /**
     * @brief 检查串口是否打开
     * @return true=已打开, false=未打开
     */
    bool isOpen() const;
    
    // ========== 串口配置 ==========
    
    /**
     * @brief 设置波特率
     * @param baudRate 波特率（例如：9600, 115200）
     * @return true=成功, false=失败
     */
    bool setBaudRate(qint32 baudRate);
    
    /**
     * @brief 设置数据位
     * @param dataBits 数据位（5, 6, 7, 8）
     * @return true=成功, false=失败
     */
    bool setDataBits(QSerialPort::DataBits dataBits);
    
    /**
     * @brief 设置停止位
     * @param stopBits 停止位（OneStop, TwoStop, OneAndHalfStop）
     * @return true=成功, false=失败
     */
    bool setStopBits(QSerialPort::StopBits stopBits);
    
    /**
     * @brief 设置校验位
     * @param parity 校验位（NoParity, EvenParity, OddParity等）
     * @return true=成功, false=失败
     */
    bool setParity(QSerialPort::Parity parity);
    
    /**
     * @brief 设置流控制
     * @param flowControl 流控制（NoFlowControl, HardwareControl, SoftwareControl）
     * @return true=成功, false=失败
     */
    bool setFlowControl(QSerialPort::FlowControl flowControl);
    
    /**
     * @brief 快速配置串口（常用配置）
     * @param baudRate 波特率
     * @param dataBits 数据位，默认8
     * @param parity 校验位，默认无校验
     * @param stopBits 停止位，默认1位
     * @return true=成功, false=失败
     * @example configure(115200) 配置115200,8,N,1
     */
    bool configure(qint32 baudRate,
                   QSerialPort::DataBits dataBits = QSerialPort::Data8,
                   QSerialPort::Parity parity = QSerialPort::NoParity,
                   QSerialPort::StopBits stopBits = QSerialPort::OneStop);
    
    // ========== 数据读写 ==========
    
    /**
     * @brief 写入数据
     * @param data 要写入的数据
     * @return 实际写入的字节数，-1表示失败
     */
    qint64 write(const QByteArray &data);
    
    /**
     * @brief 写入字符串
     * @param data 要写入的字符串
     * @return 实际写入的字节数，-1表示失败
     */
    qint64 write(const QString &data);
    
    /**
     * @brief 读取所有可用数据
     * @return 读取到的数据
     */
    QByteArray readAll();
    
    /**
     * @brief 读取指定字节数
     * @param maxSize 最大读取字节数
     * @return 读取到的数据
     */
    QByteArray read(qint64 maxSize);
    
    /**
     * @brief 读取一行数据（直到换行符）
     * @return 读取到的一行数据
     */
    QByteArray readLine();
    
    /**
     * @brief 获取可读字节数
     * @return 可读字节数
     */
    qint64 bytesAvailable() const;
    
    /**
     * @brief 等待数据可读
     * @param msecs 等待时间（毫秒）
     * @return true=有数据, false=超时
     */
    bool waitForReadyRead(int msecs = 3000);
    
    /**
     * @brief 清空接收缓冲区
     */
    void clear();
    
    /**
     * @brief 刷新发送缓冲区（立即发送）
     */
    void flush();
    
    // ========== 缓冲区管理 ==========
    
    /**
     * @brief 设置读缓冲区大小
     * @param size 缓冲区大小（字节）
     */
    void setReadBufferSize(qint64 size);
    
    /**
     * @brief 获取读缓冲区中的数据大小
     * @return 缓冲区数据大小
     */
    qint64 getReadBufferSize() const;
    
    /**
     * @brief 获取写缓冲区中待发送的数据大小
     * @return 待发送数据大小
     */
    qint64 getWriteBufferSize() const;
    
    /**
     * @brief 清空读缓冲区
     */
    void clearReadBuffer();
    
    /**
     * @brief 清空写缓冲区
     */
    void clearWriteBuffer();
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 获取串口名称
     * @return 串口名称
     */
    QString getPortName() const;
    
    /**
     * @brief 获取波特率
     * @return 当前波特率
     */
    qint32 getBaudRate() const;
    
    /**
     * @brief 获取错误信息
     * @return 错误描述字符串
     */
    QString getErrorString() const;
    
    /**
     * @brief 获取底层QSerialPort对象
     * @return QSerialPort指针
     */
    QSerialPort* getSerialPort() const { return m_pSerialPort; }
    
    // ========== 静态辅助方法 ==========
    
    /**
     * @brief 获取系统中所有可用串口列表
     * @return 串口信息列表
     */
    static QList<QSerialPortInfo> getAvailablePorts();
    
    /**
     * @brief 打印所有可用串口信息
     */
    static void printAvailablePorts();
    
    /**
     * @brief 根据描述查找串口
     * @param description 串口描述关键字
     * @return 串口名称，未找到返回空字符串
     */
    static QString findPortByDescription(const QString &description);

signals:
    /**
     * @brief 数据接收信号
     * @param data 接收到的数据
     */
    void dataReceived(const QByteArray &data);
    
    /**
     * @brief 串口打开成功信号
     */
    void opened();
    
    /**
     * @brief 串口关闭信号
     */
    void closed();
    
    /**
     * @brief 错误信号
     * @param errorString 错误描述信息
     */
    void error(const QString &errorString);

private slots:
    /**
     * @brief 数据可读槽函数
     * @note 由QSerialPort::readyRead信号触发
     */
    void onReadyRead();
    
    /**
     * @brief 错误处理槽函数
     * @param serialError 串口错误类型
     */
    void onError(QSerialPort::SerialPortError serialError);

private:
    /**
     * @brief 处理写缓冲区数据（异步发送）
     */
    void processWriteBuffer();

private:
    QSerialPort *m_pSerialPort;  // Qt串口对象
    QString m_portName;          // 串口名称
    bool m_isConfigured;         // 配置状态标志
    
    // 缓冲区
    QByteArray m_readBuffer;     // 读缓冲区（累积接收的数据）
    QByteArray m_writeBuffer;    // 写缓冲区（待发送的数据）
    qint64 m_readBufferMaxSize;  // 读缓冲区最大大小
    bool m_isWriting;            // 是否正在发送数据
};

#endif // DRIVER_SERIAL_H


