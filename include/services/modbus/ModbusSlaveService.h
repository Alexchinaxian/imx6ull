/***************************************************************
 * Copyright: Alex
 * FileName: ModbusSlaveService.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: Modbus从站服务适配器
 *
 * 功能说明:
 *   将Modbus从站功能封装为服务，在独立线程中运行
 *   实现实时串口监听和响应
 *
 * 设计模式:
 *   - 适配器模式: 将Protocol适配到服务接口
 *   - 多线程模式: 独立线程运行，保证实时性
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_SERVICES_MODBUS_H
#define IMX6ULL_SERVICES_MODBUS_H

#include "core/ISysSvrInterface.h"
#include "protocols/modbus/ModbusSlave.h"
#include "drivers/beep/DriverBeep.h"
#include <QThread>

/***************************************************************
 * 类名: ModbusSlaveService
 * 功能: Modbus从站服务适配器
 * 
 * 说明:
 *   在独立线程中运行Modbus从站，实时监听串口通信
 *   自动处理温度数据更新和蜂鸣器控制
 * 
 * 使用示例:
 *   ModbusSlaveService *modbusSvr = new ModbusSlaveService(
 *       SYS_SVR_ID_MODBUS_SLAVE,
 *       SYS_SVR_TYPE_MODBUS_SLAVE,
 *       "/dev/ttymxc2",  // 串口
 *       1                 // 从站地址
 *   );
 *   
 *   modbusSvr->SvrInit();   // 初始化
 *   modbusSvr->SvrStart();  // 启动（开始监听）
 ***************************************************************/
class ModbusSlaveService : public ISysSvrInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @param portName 串口名称
     * @param slaveAddress 从站地址
     * @param parent 父对象指针
     */
    explicit ModbusSlaveService(int32_t svr_id, int32_t svr_type, 
                                const QString &portName, 
                                quint8 slaveAddress = 1,
                                QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~ModbusSlaveService();
    
    // ========== 实现ISysSvrInterface接口 ==========
    
    virtual bool SvrInit() override;
    virtual bool SvrStart() override;
    virtual bool SvrStop() override;
    
    virtual QString GetSvrName() const override {
        return "ModbusSlaveService";
    }
    
    // ========== Modbus从站服务特有接口 ==========
    
    /**
     * @brief 获取Modbus从站对象
     * @return ProtocolModbusSlave* 返回从站指针
     */
    ProtocolModbusSlave* GetModbusSlave() const {
        return m_pModbusSlave;
    }
    
    /**
     * @brief 获取Beep驱动对象
     * @return DriverBeep* 返回驱动指针
     */
    DriverBeep* GetBeepDriver() const {
        return m_pBeep;
    }
    
    /**
     * @brief 设置温度数据
     * @param temperature 温度值（°C）
     */
    void updateTemperature(float temperature);
    
    /**
     * @brief 设置系统状态
     * @param status 状态值（0=正常，1=告警）
     */
    void updateSystemStatus(quint16 status);
    
    /**
     * @brief 配置Modbus参数
     * @param config 配置映射
     * @return true=成功, false=失败
     */
    bool configureModbus(const QMap<QString, QVariant> &config);

signals:
    /**
     * @brief Modbus读请求信号
     * @param address 寄存器地址
     * @param count 数量
     */
    void modbusReadRequest(quint16 address, quint16 count);
    
    /**
     * @brief Modbus写请求信号
     * @param address 寄存器地址
     * @param value 写入值
     */
    void modbusWriteRequest(quint16 address, quint16 value);
    
    /**
     * @brief 蜂鸣器控制信号
     * @param cmd 控制命令（0=OFF, 1=ON, 2=Alarm）
     */
    void beepCommand(int cmd);

private slots:
    /**
     * @brief Modbus写请求槽函数
     */
    void onModbusWriteRequest(quint8 fc, quint16 addr, quint16 value);
    
    /**
     * @brief Modbus读请求槽函数
     */
    void onModbusReadRequest(quint8 fc, quint16 addr, quint16 count);

private:
    /**
     * @brief 初始化Beep驱动
     * @return true=成功, false=失败
     */
    bool initBeepDriver();
    
    /**
     * @brief 连接Modbus信号
     */
    void connectModbusSignals();

private:
    ProtocolModbusSlave *m_pModbusSlave;  // Modbus从站对象
    DriverBeep *m_pBeep;                  // Beep驱动对象
    QThread *m_pThread;                    // 独立线程对象
    
    QString m_portName;                    // 串口名称
    quint8 m_slaveAddress;                 // 从站地址
    
    bool m_IsInitialized;                  // 初始化标志
    bool m_IsStarted;                      // 启动标志
};

#endif // MODBUSSLAVESERVICE_H

