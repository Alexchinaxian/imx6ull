#ifndef IMX6ULL_DRIVERS_PWM_H
#define IMX6ULL_DRIVERS_PWM_H

#include <QObject>
#include <QString>

/***************************************************************
 * 类名: DriverPWM
 * 功能: PWM驱动类 - 基于sysfs接口方式
 * 
 * 描述:
 *   PWM（Pulse Width Modulation，脉宽调制）是一种通过改变脉冲宽度
 *   来控制模拟输出的技术。常用于：
 *   - LED亮度控制
 *   - 电机速度控制
 *   - 蜂鸣器音量/频率控制
 *   - 伺服电机角度控制
 * 
 * PWM关键参数：
 *   - Period（周期）: PWM波形的完整周期，单位：纳秒
 *   - Duty Cycle（占空比）: 高电平持续时间，单位：纳秒或百分比
 *   - Frequency（频率）: 1/Period，单位：Hz
 *   - Polarity（极性）: 正常或反相
 * 
 * 使用流程：
 *   1. exportPWM() - 导出PWM通道到用户空间
 *   2. setPeriod() - 设置PWM周期
 *   3. setDutyCycle() - 设置占空比
 *   4. setEnable(true) - 使能PWM输出
 *   5. unexportPWM() - 清理资源
 * 
 * sysfs接口说明：
 *   /sys/class/pwm/pwmchipN/export       - 导出PWM通道
 *   /sys/class/pwm/pwmchipN/pwmM/period  - PWM周期（纳秒）
 *   /sys/class/pwm/pwmchipN/pwmM/duty_cycle - 占空比（纳秒）
 *   /sys/class/pwm/pwmchipN/pwmM/enable  - 使能控制（0/1）
 *   /sys/class/pwm/pwmchipN/pwmM/polarity - 极性（normal/inversed）
 ***************************************************************/

class DriverPWM : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param chipNum PWM芯片编号（例如：0表示pwmchip0）
     * @param channelNum PWM通道编号（例如：0表示该芯片的第0个通道）
     * @param parent 父对象指针
     */
    explicit DriverPWM(int chipNum, int channelNum, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * @note 自动调用unexportPWM()释放PWM资源
     */
    ~DriverPWM();
    
    // ========== 初始化和清理 ==========
    
    /**
     * @brief 导出PWM通道到用户空间
     * @return true=成功, false=失败
     */
    bool exportPWM();
    
    /**
     * @brief 取消导出PWM通道
     * @return true=成功, false=失败
     */
    bool unexportPWM();
    
    // ========== PWM配置 ==========
    
    /**
     * @brief 设置PWM周期
     * @param period_ns 周期值，单位：纳秒（1秒=10亿纳秒）
     * @return true=成功, false=失败
     * @note 周期决定PWM频率：freq = 1,000,000,000 / period_ns
     * @example period_ns=1000000 表示周期1ms，频率1KHz
     */
    bool setPeriod(int period_ns);
    
    /**
     * @brief 设置PWM占空比（绝对值）
     * @param duty_ns 占空比值，单位：纳秒
     * @return true=成功, false=失败
     * @note duty_ns必须 <= period_ns
     * @example period=1000000, duty=500000 表示50%占空比
     */
    bool setDutyCycle(int duty_ns);
    
    /**
     * @brief 设置PWM占空比（百分比）
     * @param percent 占空比百分比（0.0 - 100.0）
     * @return true=成功, false=失败
     * @example percent=75.0 表示75%占空比
     */
    bool setDutyCyclePercent(float percent);
    
    /**
     * @brief 设置PWM极性
     * @param inverted true=反相, false=正常
     * @return true=成功, false=失败
     */
    bool setPolarity(bool inverted);
    
    /**
     * @brief 设置PWM使能状态
     * @param enable true=使能, false=禁用
     * @return true=成功, false=失败
     */
    bool setEnable(bool enable);
    
    // ========== 读取状态 ==========
    
    /**
     * @brief 读取当前PWM周期
     * @return 周期值（纳秒）
     */
    int getPeriod();
    
    /**
     * @brief 读取当前PWM占空比
     * @return 占空比值（纳秒）
     */
    int getDutyCycle();
    
    /**
     * @brief 读取当前PWM占空比百分比
     * @return 占空比百分比（0.0 - 100.0）
     */
    float getDutyCyclePercent();
    
    /**
     * @brief 检查PWM是否使能
     * @return true=已使能, false=未使能
     */
    bool isEnabled();
    
    // ========== 便捷方法 ==========
    
    /**
     * @brief 启动PWM输出
     * @return true=成功, false=失败
     * @note 等同于setEnable(true)
     */
    bool start();
    
    /**
     * @brief 停止PWM输出
     * @return true=成功, false=失败
     * @note 等同于setEnable(false)
     */
    bool stop();
    
    /**
     * @brief 设置PWM频率和占空比（便捷方法）
     * @param freq_hz 频率，单位：Hz
     * @param dutyCycle_percent 占空比百分比，默认50%
     * @return true=成功, false=失败
     * @example setFrequency(1000, 75.0) 设置1KHz频率，75%占空比
     */
    bool setFrequency(int freq_hz, float dutyCycle_percent = 50.0);
    
signals:
    /**
     * @brief PWM状态变化信号
     * @param enabled true=已使能, false=已禁用
     */
    void stateChanged(bool enabled);
    
    /**
     * @brief 错误信号
     * @param errorString 错误描述信息
     */
    void error(const QString &errorString);
    
private:
    int m_chipNum;        // PWM芯片编号
    int m_channelNum;     // PWM通道编号
    bool m_exported;      // 导出状态标志
    bool m_enabled;       // 使能状态标志
    int m_period;         // 当前周期（纳秒）
    int m_dutyCycle;      // 当前占空比（纳秒）
    
    QString m_basePath;   // PWM芯片基础路径（例如：/sys/class/pwm/pwmchip0）
    QString m_pwmPath;    // PWM通道路径（例如：/sys/class/pwm/pwmchip0/pwm0）
    
    /**
     * @brief 写入数据到sysfs文件
     * @param filename 文件名（相对于m_pwmPath）或完整路径
     * @param value 要写入的值
     * @return true=成功, false=失败
     */
    bool writeToFile(const QString &filename, const QString &value);
    
    /**
     * @brief 从sysfs文件读取数据
     * @param filename 文件名（相对于m_pwmPath）
     * @return 读取到的字符串内容
     */
    QString readFromFile(const QString &filename);
};

#endif // DRIVER_PWM_H

