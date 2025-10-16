/***************************************************************
 * Copyright: Alex
 * FileName: HardwareInitService.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 硬件初始化服务 - 负责硬件扫描、配置加载、驱动初始化
 *
 * 功能说明:
 *   1. 硬件接口扫描（GPIO、I2C、SPI、CAN等）
 *   2. 从配置文件加载硬件配置
 *   3. 初始化硬件驱动（LED、PWM、串口等）
 *   4. 打印硬件状态报告
 *
 * 设计模式:
 *   - 单例服务模式
 *   - 一次性初始化服务（启动后完成初始化即可）
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_SERVICES_HARDWARE_INIT_H
#define IMX6ULL_SERVICES_HARDWARE_INIT_H

#include "core/ISysSvrInterface.h"

/***************************************************************
 * 类名: HardwareInitService
 * 功能: 硬件初始化服务
 * 
 * 说明:
 *   负责系统硬件的扫描、配置和初始化
 *   这是一次性服务，启动后完成初始化即停止
 ***************************************************************/
class HardwareInitService : public ISysSvrInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     */
    explicit HardwareInitService(int32_t svr_id, int32_t svr_type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~HardwareInitService();
    
    // ========== 实现ISysSvrInterface接口 ==========
    
    virtual bool SvrInit() override;
    virtual bool SvrStart() override;
    virtual bool SvrStop() override;
    
    virtual QString GetSvrName() const override {
        return "HardwareInitService";
    }
    
signals:
    /**
     * @brief 硬件初始化完成信号
     */
    void initCompleted();
    
    /**
     * @brief 硬件初始化失败信号
     */
    void initFailed(QString error);

private:
    /**
     * @brief 扫描系统硬件接口
     */
    bool scanHardware();
    
    /**
     * @brief 加载硬件配置文件
     */
    bool loadHardwareConfig();
    
    /**
     * @brief 初始化硬件设备
     */
    bool initHardwareDevices();

private:
    QString m_configFile;       // 配置文件路径
    bool m_scanCompleted;       // 扫描是否完成
    bool m_configLoaded;        // 配置是否加载
};

#endif // IMX6ULL_SERVICES_HARDWARE_INIT_H

