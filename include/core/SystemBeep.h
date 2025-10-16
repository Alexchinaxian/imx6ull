/***************************************************************
 * Copyright: Alex
 * FileName: SystemBeep.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 系统蜂鸣器提示管理器
 *
 * 功能说明:
 *   统一管理系统各种状态的蜂鸣器提示音
 *   提供标准化的提示音模式
 *
 * 提示音定义:
 *   - 初始化完成: 滴滴（2次短促）
 *   - 配置加载: 滴（1次短促）
 *   - 操作成功: 滴滴滴（3次快速）
 *   - 警告: 滴~滴~（2次中等）
 *   - 错误: 滴~~~（1次长响）
 *   - 关机: 滴滴滴滴（4次快速）
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef SYSTEMBEEP_H
#define SYSTEMBEEP_H

#include <QObject>
#include <QMutex>

// 前向声明
class DriverBeep;

/**
 * @brief 系统蜂鸣器管理类
 * 
 * 功能说明:
 *   提供统一的系统提示音管理
 *   支持标准化的提示音模式
 */
class SystemBeep : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 获取单例实例
     * @return SystemBeep* 单例指针
     */
    static SystemBeep* getInstance();
    
    /**
     * @brief 析构函数
     */
    ~SystemBeep();
    
    // ========== 标准提示音 ==========
    
    /**
     * @brief 播放初始化完成提示音
     * @note 滴滴（2次短促，100ms间隔）
     */
    void playInitComplete();
    
    /**
     * @brief 播放配置加载成功提示音
     * @note 滴（1次短促，150ms）
     */
    void playConfigLoaded();
    
    /**
     * @brief 播放成功提示音
     * @note 滴滴滴（3次快速，80ms间隔）
     */
    void playSuccess();
    
    /**
     * @brief 播放警告提示音
     * @note 滴~滴~（2次中等，200ms持续，200ms间隔）
     */
    void playWarning();
    
    /**
     * @brief 播放错误警告音
     * @note 滴~~~（1次长响，500ms持续）
     */
    void playError();
    
    /**
     * @brief 播放关机提示音
     * @note 滴滴滴滴（4次快速，80ms间隔）
     */
    void playShutdown();
    
    /**
     * @brief 播放自定义提示音
     * @param count 次数
     * @param duration 持续时间（ms）
     * @param interval 间隔时间（ms）
     */
    void playCustom(int count, int duration, int interval);
    
    // ========== 控制方法 ==========
    
    /**
     * @brief 启用/禁用蜂鸣器
     * @param enabled true=启用, false=禁用
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief 检查是否启用
     * @return true=启用, false=禁用
     */
    bool isEnabled() const { return m_enabled; }
    
    /**
     * @brief 检查蜂鸣器是否可用
     * @return true=可用, false=不可用
     */
    bool isAvailable() const;
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    explicit SystemBeep(QObject *parent = nullptr);
    
private:
    static SystemBeep *m_instance;      // 单例实例
    static QMutex m_instanceMutex;      // 实例锁
    
    DriverBeep *m_beepDriver;           // 蜂鸣器驱动
    bool m_enabled;                     // 是否启用
};

#endif // SYSTEMBEEP_H

