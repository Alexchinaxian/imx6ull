/***************************************************************
 * Copyright: Alex
 * FileName: TemperatureService.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 温度监控服务适配器
 *
 * 功能说明:
 *   将DriverTemperature适配到ISysSvrInterface接口
 *   提供统一的服务管理接口
 *
 * 设计模式:
 *   - 适配器模式: 将Driver接口适配到服务接口
 *   - 代理模式: 代理DriverTemperature的功能
 *
 * History:
 *   1. 2025-10-15 创建文件，作为服务适配器示例
 ***************************************************************/

#ifndef IMX6ULL_SERVICES_TEMPERATURE_H
#define IMX6ULL_SERVICES_TEMPERATURE_H

#include "core/ISysSvrInterface.h"
#include "drivers/temperature/DriverTemperature.h"
#include <QThread>

/***************************************************************
 * 类名: TemperatureService
 * 功能: 温度监控服务适配器
 * 
 * 说明:
 *   继承ISysSvrInterface接口，内部封装DriverTemperature
 *   提供服务的生命周期管理，并转发温度驱动的信号
 * 
 * 使用示例:
 *   TemperatureService *tempSvr = new TemperatureService(
 *       SYS_SVR_ID_TEMPERATURE_SVR,
 *       SYS_SVR_TYPE_TEMPERATURE_SVR
 *   );
 *   
 *   tempSvr->SvrInit();   // 初始化温度驱动
 *   tempSvr->SvrStart();  // 启动温度监控
 *   
 *   DriverTemperature *driver = tempSvr->GetDriver(); // 获取底层驱动
 ***************************************************************/
class TemperatureService : public ISysSvrInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @param parent 父对象指针
     */
    explicit TemperatureService(int32_t svr_id, int32_t svr_type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * @note 自动清理温度驱动和线程资源
     */
    virtual ~TemperatureService();
    
    // ========== 实现ISysSvrInterface接口 ==========
    
    /**
     * @brief 服务初始化
     * @return true=成功, false=失败
     * @note 创建温度驱动对象和独立线程
     */
    virtual bool SvrInit() override;
    
    /**
     * @brief 服务启动
     * @return true=成功, false=失败
     * @note 启动温度监控线程
     */
    virtual bool SvrStart() override;
    
    /**
     * @brief 服务停止
     * @return true=成功, false=失败
     * @note 停止温度监控并清理线程
     */
    virtual bool SvrStop() override;
    
    /**
     * @brief 获取服务名称
     * @return QString 返回服务名称
     */
    virtual QString GetSvrName() const override
    {
        return "TemperatureService";
    }
    
    // ========== 温度服务特有接口 ==========
    
    /**
     * @brief 获取底层温度驱动对象
     * @return DriverTemperature* 返回驱动指针
     */
    DriverTemperature* GetDriver() const
    {
        return m_pTempDriver;
    }
    
    /**
     * @brief 获取当前温度
     * @return float 当前温度（°C）
     */
    float GetCurrentTemperature() const;
    
    /**
     * @brief 获取温度信息
     * @return TemperatureInfo 温度信息结构
     */
    TemperatureInfo GetTemperatureInfo() const;
    
    /**
     * @brief 设置高温报警阈值
     * @param threshold 阈值温度（°C）
     */
    void SetHighThreshold(float threshold);

signals:
    /**
     * @brief 温度变化信号
     * @param temperature 当前温度（°C）
     */
    void temperatureChanged(float temperature);
    
    /**
     * @brief 高温报警信号
     * @param temperature 当前温度（°C）
     */
    void temperatureHigh(float temperature);
    
    /**
     * @brief 温度恢复正常信号
     * @param temperature 当前温度（°C）
     */
    void temperatureNormal(float temperature);

private slots:
    /**
     * @brief 驱动初始化完成槽函数
     */
    void onDriverInitialized();
    
    /**
     * @brief 驱动启动完成槽函数
     */
    void onDriverStarted();
    
    /**
     * @brief 驱动错误槽函数
     * @param errorString 错误信息
     */
    void onDriverError(const QString &errorString);

private:
    DriverTemperature *m_pTempDriver;  // 温度驱动对象
    QThread *m_pThread;                 // 独立线程对象
    bool m_IsInitialized;               // 初始化标志
    bool m_IsStarted;                   // 启动标志
};

#endif // TEMPERATURESERVICE_H

