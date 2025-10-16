/***************************************************************
 * 文件名: buffer_example.cpp
 * 功能: 串口和CAN通讯缓冲区使用示例
 * 说明: 演示如何使用缓冲区进行可靠的数据通信
 ***************************************************************/

#include "drivers/serial/DriverSerial.h"
#include "drivers/can/DriverCAN.h"
#include "drivers/manager/DriverManager.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

// ========================================
// 示例1：串口缓冲区基本使用
// ========================================

void example1_SerialBasicBuffer()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "示例1：串口缓冲区基本使用";
    qInfo() << "========================================";
    
    // 创建串口驱动
    DriverSerial serial("/dev/ttymxc2");
    serial.configure(9600);
    serial.open(QIODevice::ReadWrite);
    
    // 设置读缓冲区大小为16KB
    serial.setReadBufferSize(16 * 1024);
    
    // 发送数据（自动使用写缓冲）
    serial.write("Hello World\r\n");
    
    // 查询缓冲区状态
    qInfo() << "读缓冲区大小:" << serial.getReadBufferSize() << "字节";
    qInfo() << "写缓冲区待发送:" << serial.getWriteBufferSize() << "字节";
    qInfo() << "可读数据:" << serial.bytesAvailable() << "字节";
    
    serial.close();
}

// ========================================
// 示例2：串口协议解析（处理半包/粘包）
// ========================================

void example2_SerialProtocolParsing()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "示例2：串口协议解析（固定长度帧）";
    qInfo() << "========================================";
    
    DriverSerial serial("/dev/ttymxc2");
    serial.configure(9600);
    serial.open(QIODevice::ReadWrite);
    
    const int FRAME_SIZE = 10;  // 假设协议帧长度为10字节
    
    // 连接数据接收信号
    QObject::connect(&serial, &DriverSerial::dataReceived, [&serial](const QByteArray &data) {
        qInfo() << "接收到" << data.size() << "字节，读缓冲区总计:" << serial.bytesAvailable() << "字节";
        
        // 从读缓冲区提取完整帧
        while (serial.bytesAvailable() >= FRAME_SIZE) {
            QByteArray frame = serial.read(FRAME_SIZE);
            qInfo() << "解析到完整帧:" << frame.toHex(' ');
            
            // 处理帧...
        }
        
        // 剩余不完整的数据继续保留在缓冲区
        if (serial.bytesAvailable() > 0) {
            qInfo() << "剩余不完整数据:" << serial.bytesAvailable() << "字节，等待更多数据";
        }
    });
    
    // 模拟数据接收...
}

// ========================================
// 示例3：串口按行读取
// ========================================

void example3_SerialLineReading()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "示例3：串口按行读取";
    qInfo() << "========================================";
    
    DriverSerial serial("/dev/ttymxc2");
    serial.configure(115200);
    serial.open(QIODevice::ReadWrite);
    
    // 连接数据接收信号
    QObject::connect(&serial, &DriverSerial::dataReceived, [&serial](const QByteArray &data) {
        // 数据已累积到读缓冲区
        
        // 按行读取
        QByteArray line;
        while (!(line = serial.readLine()).isEmpty()) {
            qInfo() << "接收到一行:" << line.trimmed();
            
            // 处理行数据...
        }
    });
}

// ========================================
// 示例4：CAN帧缓冲区使用
// ========================================

void example4_CANBuffer()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "示例4：CAN帧缓冲区使用";
    qInfo() << "========================================";
    
    DriverCAN can("can0");
    can.setBitrate(500000);
    can.open();
    
    // 设置接收缓冲区最大1000帧
    can.setReceiveBufferMaxSize(1000);
    
    // 连接帧接收信号
    QObject::connect(&can, &DriverCAN::frameReceived, [](const QCanBusFrame &frame) {
        qInfo() << "接收到CAN帧，ID:" << QString::number(frame.frameId(), 16);
        // 帧已自动添加到接收缓冲区
    });
    
    // 定时处理缓冲区中的帧
    QTimer *timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [&can]() {
        int frameCount = can.getBufferedFrameCount();
        
        if (frameCount > 0) {
            qInfo() << "处理缓冲区中的" << frameCount << "帧";
            
            // 批量读取所有帧
            QVector<QCanBusFrame> frames = can.readAllFrames();
            
            for (const QCanBusFrame &frame : frames) {
                // 处理每一帧
                qDebug() << "处理帧 ID:" << QString::number(frame.frameId(), 16)
                         << "数据:" << frame.payload().toHex(' ');
            }
        }
    });
    timer->start(100);  // 每100ms处理一次
}

// ========================================
// 示例5：使用别名访问硬件
// ========================================

void example5_HardwareAliasWithBuffer()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "示例5：通过别名使用硬件缓冲区";
    qInfo() << "========================================";
    
    // 获取驱动管理器
    DriverManager &driverMgr = DriverManager::getInstance();
    
    // 加载硬件配置
    driverMgr.loadFromConfig("hardware.init");
    
    // 通过别名获取Modbus串口
    DriverSerial *modbusSerial = driverMgr.getSerialByAlias("Modbus串口");
    if (modbusSerial)
    {
        // 打开串口
        modbusSerial->open(QIODevice::ReadWrite);
        
        // 设置读缓冲区大小
        modbusSerial->setReadBufferSize(4096);
        
        // 发送Modbus请求
        QByteArray modbusRequest = QByteArray::fromHex("01 03 00 00 00 0A C5 CD");
        modbusSerial->write(modbusRequest);
        
        qInfo() << "已发送Modbus请求，写缓冲区待发送:" << modbusSerial->getWriteBufferSize() << "字节";
        
        // 等待响应
        if (modbusSerial->waitForReadyRead(1000))
        {
            qInfo() << "接收到响应，读缓冲区:" << modbusSerial->bytesAvailable() << "字节";
            QByteArray response = modbusSerial->readAll();
            qInfo() << "响应数据:" << response.toHex(' ');
        }
    }
    
    // 通过别名控制风扇PWM
    DriverPWM *fan = driverMgr.getPWMByAlias("风扇");
    if (fan)
    {
        fan->setFrequency(25000, 60.0);
        fan->start();
        qInfo() << "风扇PWM已启动";
    }
}

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qInfo() << "";
    qInfo() << "================================================";
    qInfo() << "  通讯缓冲区使用示例";
    qInfo() << "================================================";
    
    // 运行各个示例
    example1_SerialBasicBuffer();
    // example2_SerialProtocolParsing();
    // example3_SerialLineReading();
    // example4_CANBuffer();
    example5_HardwareAliasWithBuffer();
    
    qInfo() << "";
    qInfo() << "================================================";
    qInfo() << "  示例运行完成";
    qInfo() << "================================================";
    
    return 0;
}

