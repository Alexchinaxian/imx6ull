#ifndef IMX6ULL_DRIVERS_LED_H
#define IMX6ULL_DRIVERS_LED_H

#include <QObject>
#include <QString>

/***************************************************************
 * 类名: DriverLED
 * 功能: LED驱动类 - 基于sysfs接口方式
 * 
 * 描述:
 *   通过Linux标准sysfs接口(/sys/class/leds/)控制LED灯
 *   支持亮度控制和触发器模式（如心跳、定时器等）
 * 
 * LED控制方式：
 *   1. 直接控制 - 设置brightness值（0=关闭，max=最亮）
 *   2. 触发器模式 - 使用内核提供的自动闪烁模式
 *      - none: 手动控制
 *      - timer: 定时闪烁
 *      - heartbeat: 心跳闪烁
 *      - default-on: 默认打开
 * 
 * 使用示例：
 *   DriverLED led("sys-led");
 *   led.turnOn();              // 打开LED
 *   led.setBrightness(128);    // 设置50%亮度
 *   led.blink(3, 500);         // 闪烁3次，间隔500ms
 * 
 * sysfs接口说明：
 *   /sys/class/leds/led-name/brightness     - 亮度值（0~max_brightness）
 *   /sys/class/leds/led-name/max_brightness - 最大亮度值（只读）
 *   /sys/class/leds/led-name/trigger        - 触发器模式
 ***************************************************************/

class DriverLED : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param ledName LED名称（例如："sys-led"、"heartbeat"）
     * @param parent 父对象指针
     * @note LED名称对应/sys/class/leds/下的目录名
     */
    explicit DriverLED(const QString &ledName, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DriverLED();
    
    // ========== LED亮度控制 ==========
    
    /**
     * @brief 设置LED亮度
     * @param brightness 亮度值（0~max_brightness）
     * @return true=成功, false=失败
     * @note 0=关闭，max_brightness=最亮
     */
    bool setBrightness(int brightness);
    
    /**
     * @brief 获取当前LED亮度
     * @return 当前亮度值
     */
    int getBrightness();
    
    /**
     * @brief 获取LED最大亮度值
     * @return 最大亮度值（通常为1或255）
     */
    int getMaxBrightness();
    
    // ========== 触发器控制 ==========
    
    /**
     * @brief 设置LED触发器模式
     * @param trigger 触发器名称（如"none"、"timer"、"heartbeat"）
     * @return true=成功, false=失败
     * @note 设置为"none"可恢复手动控制
     */
    bool setTrigger(const QString &trigger);
    
    /**
     * @brief 获取当前触发器
     * @return 当前激活的触发器名称
     */
    QString getCurrentTrigger();
    
    /**
     * @brief 获取所有可用的触发器
     * @return 触发器名称列表
     */
    QStringList getAvailableTriggers();
    
    // ========== 便捷方法 ==========
    
    /**
     * @brief 打开LED（设置为最大亮度）
     * @return true=成功, false=失败
     */
    bool turnOn();
    
    /**
     * @brief 关闭LED（设置亮度为0）
     * @return true=成功, false=失败
     */
    bool turnOff();
    
    /**
     * @brief 切换LED状态（开变关，关变开）
     * @return true=成功, false=失败
     */
    bool toggle();
    
    /**
     * @brief LED闪烁指定次数
     * @param times 闪烁次数
     * @param interval_ms 闪烁间隔（毫秒）
     * @return true=成功, false=失败
     * @note 这是阻塞操作，会占用当前线程
     */
    bool blink(int times, int interval_ms);
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 获取LED名称
     * @return LED名称
     */
    QString getName() const { return m_ledName; }
    
signals:
    /**
     * @brief 亮度变化信号
     * @param brightness 新的亮度值
     */
    void brightnessChanged(int brightness);
    
    /**
     * @brief 错误信号
     * @param errorString 错误描述信息
     */
    void error(const QString &errorString);
    
private:
    QString m_ledName;           // LED名称
    QString m_basePath;          // LED sysfs基础路径
    int m_maxBrightness;         // 最大亮度值
    int m_currentBrightness;     // 当前亮度值
    
    /**
     * @brief 写入数据到sysfs文件
     * @param filename 文件名（相对于m_basePath）
     * @param value 要写入的值
     * @return true=成功, false=失败
     */
    bool writeToFile(const QString &filename, const QString &value);
    
    /**
     * @brief 从sysfs文件读取数据
     * @param filename 文件名（相对于m_basePath）
     * @return 读取到的字符串内容
     */
    QString readFromFile(const QString &filename);
};

#endif // DRIVER_LED_H

