#ifndef IMX6ULL_DRIVERS_GPIO_H
#define IMX6ULL_DRIVERS_GPIO_H

#include <QObject>
#include <QString>

/***************************************************************
 * 类名: DriverGPIO
 * 功能: GPIO驱动类 - 基于sysfs接口方式
 * 
 * 描述: 
 *   通过Linux标准sysfs接口(/sys/class/gpio/)控制GPIO引脚
 *   无需直接操作寄存器，安全性高
 * 
 * 优点:
 *   1. 安全可靠 - 通过内核提供的标准接口
 *   2. 无需root权限 - 普通用户权限即可（需要正确的文件权限设置）
 *   3. 不与内核驱动冲突 - 不会与其他驱动争夺GPIO资源
 *   4. 跨平台性好 - 适用于所有支持sysfs的Linux系统
 * 
 * 使用流程:
 *   1. exportGPIO() - 导出GPIO到用户空间
 *   2. setDirection() - 设置GPIO方向（输入/输出）
 *   3. setValue()/getValue() - 设置/读取GPIO值
 *   4. unexportGPIO() - 取消导出（清理资源）
 * 
 * sysfs接口说明:
 *   /sys/class/gpio/export        - 导出GPIO
 *   /sys/class/gpio/unexport      - 取消导出
 *   /sys/class/gpio/gpioN/value   - GPIO值（0或1）
 *   /sys/class/gpio/gpioN/direction - GPIO方向（in/out）
 ***************************************************************/

class DriverGPIO : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief GPIO方向枚举
     */
    enum Direction {
        Input = 0,   // 输入模式 - 用于读取外部信号
        Output = 1   // 输出模式 - 用于控制外部设备
    };
    
    /**
     * @brief GPIO电平值枚举
     */
    enum Value {
        Low = 0,    // 低电平 - 0V
        High = 1    // 高电平 - 3.3V或5V（取决于硬件）
    };
    
    /**
     * @brief 构造函数
     * @param gpioNum GPIO编号（例如：4表示GPIO4）
     * @param parent 父对象指针
     */
    explicit DriverGPIO(int gpioNum, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * @note 自动调用unexportGPIO()释放GPIO资源
     */
    ~DriverGPIO();
    
    // ========== 初始化和清理 ==========
    
    /**
     * @brief 导出GPIO到用户空间
     * @return true=成功, false=失败
     * @note 向/sys/class/gpio/export写入GPIO编号
     */
    bool exportGPIO();
    
    /**
     * @brief 取消导出GPIO
     * @return true=成功, false=失败
     * @note 向/sys/class/gpio/unexport写入GPIO编号
     */
    bool unexportGPIO();
    
    // ========== GPIO配置 ==========
    
    /**
     * @brief 设置GPIO方向
     * @param dir GPIO方向（Input或Output）
     * @return true=成功, false=失败
     */
    bool setDirection(Direction dir);
    
    /**
     * @brief 设置GPIO输出值
     * @param val GPIO值（Low或High）
     * @return true=成功, false=失败
     * @note 只有在Output模式下才有效
     */
    bool setValue(Value val);
    
    /**
     * @brief 读取GPIO当前值
     * @return GPIO值（Low或High）
     * @note 可用于Input或Output模式
     */
    Value getValue();
    
    // ========== 便捷方法 ==========
    
    /**
     * @brief 设置GPIO为高电平
     * @return true=成功, false=失败
     */
    bool setHigh();
    
    /**
     * @brief 设置GPIO为低电平
     * @return true=成功, false=失败
     */
    bool setLow();
    
    /**
     * @brief 翻转GPIO状态（高变低，低变高）
     * @return true=成功, false=失败
     */
    bool toggle();
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 检查GPIO是否已导出
     * @return true=已导出, false=未导出
     */
    bool isExported() const;
    
    /**
     * @brief 获取GPIO编号
     * @return GPIO编号
     */
    int getGPIONumber() const { return m_gpioNum; }
    
signals:
    /**
     * @brief GPIO值变化信号
     * @param value 新的GPIO值（0或1）
     */
    void valueChanged(int value);
    
    /**
     * @brief 错误信号
     * @param errorString 错误描述信息
     */
    void error(const QString &errorString);
    
private:
    int m_gpioNum;          // GPIO编号
    bool m_exported;        // 导出状态标志
    Value m_currentValue;   // 当前GPIO值
    
    /**
     * @brief 写入数据到sysfs文件
     * @param path 文件路径
     * @param value 要写入的值
     * @return true=成功, false=失败
     */
    bool writeToFile(const QString &path, const QString &value);
    
    /**
     * @brief 从sysfs文件读取数据
     * @param path 文件路径
     * @return 读取到的字符串内容
     */
    QString readFromFile(const QString &path);
};

#endif // DRIVER_GPIO_H

