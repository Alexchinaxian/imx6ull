/***************************************************************
 * Copyright: Alex
 * FileName: driver_beep.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 蜂鸣器驱动类 - 基于sysfs接口方式
 *
 * 描述:
 *   通过Linux标准sysfs接口(/sys/class/leds/beep/)控制蜂鸣器
 *   蜂鸣器在系统中作为LED设备实现
 *
 * 使用示例：
 *   DriverBeep beep;
 *   beep.turnOn();              // 打开蜂鸣器
 *   beep.turnOff();             // 关闭蜂鸣器
 *   beep.alarm(3, 200);         // 报警3次，间隔200ms
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_DRIVERS_BEEP_H
#define IMX6ULL_DRIVERS_BEEP_H

#include <QObject>
#include <QString>
#include <QTimer>

/***************************************************************
 * 类名: DriverBeep
 * 功能: 蜂鸣器驱动类 - 基于sysfs接口
 * 
 * 说明:
 *   蜂鸣器通常通过LED子系统控制
 *   路径：/sys/class/leds/beep/brightness
 *   值：0=关闭，非0=打开
 ***************************************************************/
class DriverBeep : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param beepName 蜂鸣器名称（默认"beep"）
     * @param parent 父对象指针
     */
    explicit DriverBeep(const QString &beepName = "beep", QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DriverBeep();
    
    // ========== 基本控制接口 ==========
    
    /**
     * @brief 打开蜂鸣器
     * @return true=成功, false=失败
     */
    bool turnOn();
    
    /**
     * @brief 关闭蜂鸣器
     * @return true=成功, false=失败
     */
    bool turnOff();
    
    /**
     * @brief 切换蜂鸣器状态
     * @return true=成功, false=失败
     */
    bool toggle();
    
    /**
     * @brief 设置蜂鸣器强度
     * @param intensity 强度值（0-255，0=关闭）
     * @return true=成功, false=失败
     */
    bool setIntensity(int intensity);
    
    /**
     * @brief 获取当前状态
     * @return true=开启, false=关闭
     */
    bool isOn() const;
    
    // ========== 高级功能 ==========
    
    /**
     * @brief 蜂鸣N次（短促）
     * @param count 次数
     * @param intervalMs 间隔时间（毫秒）
     */
    void beep(int count, int intervalMs = 100);
    
    /**
     * @brief 报警模式（长鸣）
     * @param count 次数
     * @param durationMs 每次持续时间（毫秒）
     * @param intervalMs 间隔时间（毫秒）
     */
    void alarm(int count, int durationMs = 500, int intervalMs = 200);
    
    /**
     * @brief 停止所有定时操作
     */
    void stopAll();
    
    /**
     * @brief 检查蜂鸣器是否可用
     * @return true=可用, false=不可用
     */
    bool isAvailable() const;

signals:
    /**
     * @brief 状态变化信号
     * @param isOn true=开启, false=关闭
     */
    void stateChanged(bool isOn);
    
    /**
     * @brief 错误信号
     * @param error 错误信息
     */
    void error(const QString &error);
    
    /**
     * @brief 操作完成信号
     */
    void finished();

private slots:
    /**
     * @brief 定时器槽函数（用于beep和alarm）
     */
    void onTimerTimeout();

private:
    /**
     * @brief 写入brightness文件
     * @param value 亮度值
     * @return true=成功, false=失败
     */
    bool writeBrightness(int value);
    
    /**
     * @brief 读取brightness文件
     * @return 当前亮度值
     */
    int readBrightness() const;
    
    /**
     * @brief 检查蜂鸣器路径是否存在
     * @return true=存在, false=不存在
     */
    bool checkPath() const;

private:
    QString m_beepName;          // 蜂鸣器名称
    QString m_sysfsPath;         // sysfs路径
    
    QTimer *m_timer;             // 定时器（用于beep和alarm）
    int m_currentCount;          // 当前计数
    int m_targetCount;           // 目标计数
    int m_duration;              // 持续时间
    int m_interval;              // 间隔时间
    bool m_timerState;           // 定时器状态（true=开, false=关）
};

#endif // DRIVER_BEEP_H

