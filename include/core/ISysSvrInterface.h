/***************************************************************
 * Copyright: Alex
 * FileName: ISysSvrInterface.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 系统服务接口基类，所有服务都必须继承此接口
 *
 * 功能说明:
 *   定义了服务的标准接口，包括生命周期管理、标识符查询等
 *   所有具体的服务类都应该继承此接口并实现虚函数
 *
 * 设计模式:
 *   - 接口模式: 定义统一的服务接口
 *   - 模板方法模式: 定义服务生命周期的标准流程
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_CORE_ISYSSVR_INTERFACE_H
#define IMX6ULL_CORE_ISYSSVR_INTERFACE_H

#include <QObject>
#include <QString>
#include <stdint.h>

/***************************************************************
 * 类名: ISysSvrInterface
 * 功能: 系统服务接口基类
 * 
 * 说明:
 *   这是一个抽象基类，定义了所有服务必须实现的接口
 *   包括初始化、启动、停止等生命周期管理方法
 * 
 * 继承此类的步骤:
 *   1. 继承ISysSvrInterface
 *   2. 实现所有纯虚函数
 *   3. 在构造函数中设置服务ID和类型
 *   4. 实现具体的业务逻辑
 * 
 * 生命周期:
 *   构造 -> SvrInit() -> SvrStart() -> [运行] -> SvrStop() -> 析构
 ***************************************************************/
class ISysSvrInterface : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @param parent 父对象指针
     */
    explicit ISysSvrInterface(int32_t svr_id, int32_t svr_type, QObject *parent = nullptr)
        : QObject(parent)
        , m_SvrId(svr_id)
        , m_SvrType(svr_type)
    {
    }
    
    /**
     * @brief 虚析构函数
     * @note 确保派生类的析构函数被正确调用
     */
    virtual ~ISysSvrInterface() {}
    
    // ========== 生命周期管理接口（纯虚函数） ==========
    
    /**
     * @brief 服务初始化
     * @return true=成功, false=失败
     * @note 派生类必须实现此方法，进行资源分配、配置加载等操作
     */
    virtual bool SvrInit() = 0;
    
    /**
     * @brief 服务启动
     * @return true=成功, false=失败
     * @note 派生类必须实现此方法，启动服务的主要功能
     */
    virtual bool SvrStart() = 0;
    
    /**
     * @brief 服务停止
     * @return true=成功, false=失败
     * @note 派生类可选实现，停止服务运行
     */
    virtual bool SvrStop() { return true; }
    
    // ========== 服务标识符查询接口 ==========
    
    /**
     * @brief 获取服务ID
     * @return int32_t 返回服务ID
     */
    virtual int32_t GetSvrId() const
    {
        return m_SvrId;
    }
    
    /**
     * @brief 获取服务类型
     * @return int32_t 返回服务类型
     */
    virtual int32_t GetSvrType() const
    {
        return m_SvrType;
    }
    
    /**
     * @brief 检查是否为指定的服务ID
     * @param svr_id 要检查的服务ID
     * @return true=是, false=否
     */
    virtual bool IsYesSvrId(int32_t svr_id) const
    {
        return (m_SvrId == svr_id);
    }
    
    /**
     * @brief 检查是否为指定的服务ID和类型
     * @param svr_id 要检查的服务ID
     * @param svr_type 要检查的服务类型
     * @return true=是, false=否
     */
    virtual bool IsSvrIdAndType(int32_t svr_id, int32_t svr_type) const
    {
        return ((m_SvrId == svr_id) && (m_SvrType == svr_type));
    }
    
    /**
     * @brief 获取服务名称
     * @return QString 返回服务名称
     * @note 派生类可以重写此方法返回有意义的名称
     */
    virtual QString GetSvrName() const
    {
        return QString("Service_%1").arg(m_SvrId);
    }
    
    /**
     * @brief 获取服务描述信息
     * @return QString 返回服务描述
     */
    virtual QString GetSvrDescription() const
    {
        return QString("Service ID: %1, Type: %2").arg(m_SvrId).arg(m_SvrType);
    }

signals:
    /**
     * @brief 服务状态变化信号
     * @param svrId 服务ID
     * @param status 状态描述
     */
    void statusChanged(int svrId, const QString &status);
    
    /**
     * @brief 服务错误信号
     * @param svrId 服务ID
     * @param errorMsg 错误消息
     */
    void error(int svrId, const QString &errorMsg);

protected:
    int32_t m_SvrId;      // 服务ID
    int32_t m_SvrType;    // 服务类型
};

#endif // ISYSSVRINTERFACE_H


