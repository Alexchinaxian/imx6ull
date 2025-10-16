/***************************************************************
 * Copyright: Alex
 * FileName: protocol_manager.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 协议管理器
 *
 * 功能说明:
 *   统一管理所有通信协议实例
 *   提供协议的创建、查找、销毁等功能
 *
 * 设计模式:
 *   - 单例模式：全局唯一的协议管理器
 *   - 工厂模式：统一创建各类协议实例
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_PROTOCOLS_MANAGER_H
#define IMX6ULL_PROTOCOLS_MANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include "protocols/IProtocolInterface.h"

/***************************************************************
 * 类名: ProtocolManager
 * 功能: 协议管理器类
 * 
 * 职责:
 *   1. 统一创建和管理所有协议实例
 *   2. 提供协议查找和获取接口
 *   3. 管理协议的生命周期
 * 
 * 使用示例:
 *   ProtocolManager *mgr = ProtocolManager::getInstance();
 *   
 *   // 创建Modbus RTU协议
 *   mgr->createModbusRTU("modbus_rtu_1", "/dev/ttyS1");
 *   
 *   // 获取协议实例
 *   IProtocolInterface *protocol = mgr->getProtocol("modbus_rtu_1");
 *   protocol->connect();
 *   
 *   // 销毁协议
 *   mgr->destroyProtocol("modbus_rtu_1");
 ***************************************************************/
class ProtocolManager : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 获取协议管理器的单例实例
     * @return ProtocolManager* 返回协议管理器指针
     */
    static ProtocolManager* getInstance();
    
    /**
     * @brief 析构函数
     */
    ~ProtocolManager();
    
    // ========== 协议创建接口 ==========
    
    /**
     * @brief 创建Modbus RTU协议实例
     * @param name 协议实例名称（唯一标识）
     * @param portName 串口名称
     * @return true=成功, false=失败
     */
    bool createModbusRTU(const QString &name, const QString &portName);
    
    /**
     * @brief 创建Modbus TCP协议实例
     * @param name 协议实例名称（唯一标识）
     * @param host 服务器地址
     * @param port 服务器端口（默认502）
     * @return true=成功, false=失败
     */
    bool createModbusTCP(const QString &name, const QString &host, quint16 port = 502);
    
    /**
     * @brief 创建Modbus RTU从站（Slave）实例
     * @param name 协议实例名称（唯一标识）
     * @param portName 串口名称
     * @param slaveAddress 从站地址（1-247）
     * @return true=成功, false=失败
     */
    bool createModbusSlave(const QString &name, const QString &portName, quint8 slaveAddress = 1);
    
    // ========== 协议查找接口 ==========
    
    /**
     * @brief 根据名称获取协议实例
     * @param name 协议实例名称
     * @return IProtocolInterface* 返回协议接口指针，未找到返回nullptr
     */
    IProtocolInterface* getProtocol(const QString &name);
    
    /**
     * @brief 根据类型获取所有协议实例
     * @param type 协议类型
     * @return QList<IProtocolInterface*> 协议实例列表
     */
    QList<IProtocolInterface*> getProtocolsByType(ProtocolType type);
    
    /**
     * @brief 获取所有协议实例
     * @return QList<IProtocolInterface*> 所有协议实例列表
     */
    QList<IProtocolInterface*> getAllProtocols() const;
    
    /**
     * @brief 检查协议是否存在
     * @param name 协议实例名称
     * @return true=存在, false=不存在
     */
    bool hasProtocol(const QString &name) const;
    
    // ========== 协议管理接口 ==========
    
    /**
     * @brief 销毁协议实例
     * @param name 协议实例名称
     * @return true=成功, false=失败
     */
    bool destroyProtocol(const QString &name);
    
    /**
     * @brief 销毁所有协议实例
     */
    void destroyAllProtocols();
    
    /**
     * @brief 获取协议数量
     * @return 当前协议实例数量
     */
    int getProtocolCount() const;
    
    /**
     * @brief 连接所有协议
     */
    void connectAll();
    
    /**
     * @brief 断开所有协议
     */
    void disconnectAll();

signals:
    /**
     * @brief 协议创建信号
     * @param name 协议名称
     * @param type 协议类型
     */
    void protocolCreated(const QString &name, ProtocolType type);
    
    /**
     * @brief 协议销毁信号
     * @param name 协议名称
     */
    void protocolDestroyed(const QString &name);
    
    /**
     * @brief 协议连接信号
     * @param name 协议名称
     */
    void protocolConnected(const QString &name);
    
    /**
     * @brief 协议断开信号
     * @param name 协议名称
     */
    void protocolDisconnected(const QString &name);
    
    /**
     * @brief 协议错误信号
     * @param name 协议名称
     * @param error 错误信息
     */
    void protocolError(const QString &name, const QString &error);

private:
    /**
     * @brief 构造函数（私有，单例模式）
     */
    explicit ProtocolManager(QObject *parent = nullptr);
    
    /**
     * @brief 拷贝构造函数（禁用）
     */
    ProtocolManager(const ProtocolManager&) = delete;
    
    /**
     * @brief 赋值操作符（禁用）
     */
    ProtocolManager& operator=(const ProtocolManager&) = delete;
    
    /**
     * @brief 注册协议实例
     * @param name 协议名称
     * @param protocol 协议实例指针
     * @return true=成功, false=失败
     */
    bool registerProtocol(const QString &name, IProtocolInterface *protocol);
    
    /**
     * @brief 连接协议信号
     * @param name 协议名称
     * @param protocol 协议实例指针
     */
    void connectProtocolSignals(const QString &name, IProtocolInterface *protocol);

private:
    static ProtocolManager* m_instance;             // 单例实例
    QMap<QString, IProtocolInterface*> m_protocols; // 协议实例映射表
};

#endif // PROTOCOL_MANAGER_H

