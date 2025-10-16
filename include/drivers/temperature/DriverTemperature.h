#ifndef IMX6ULL_DRIVERS_TEMPERATURE_H
#define IMX6ULL_DRIVERS_TEMPERATURE_H

#include <QObject>
#include <QTimer>
#include <QString>

/***************************************************************
 * 结构体: TemperatureInfo
 * 功能: 温度信息数据结构
 * 说明: 包含当前温度、最高温度、最低温度和传感器类型
 ***************************************************************/
struct TemperatureInfo {
    float currentTemp;    // 当前温度（°C）
    float maxTemp;        // 记录的最高温度（°C）
    float minTemp;        // 记录的最低温度（°C）
    QString sensorType;   // 传感器类型描述
};

/***************************************************************
 * 类名: DriverTemperature
 * 功能: 温度监控驱动类
 * 
 * 描述:
 *   通过读取Linux thermal zone获取CPU温度
 *   支持定时监控、温度报警、温度统计等功能
 * 
 * 温度报警机制：
 *   - 当温度超过阈值时，发送temperatureHigh信号
 *   - 当温度恢复正常时，发送temperatureNormal信号
 *   - 避免频繁报警（只在状态变化时发送信号）
 * 
 * 使用流程：
 *   1. 创建DriverTemperature对象
 *   2. 调用initialize()初始化
 *   3. 调用start()开始监控
 *   4. 连接信号接收温度变化通知
 * 
 * sysfs接口说明：
 *   /sys/class/thermal/thermal_zone0/temp - CPU温度（单位：毫摄氏度）
 *   例如：读取到45000表示45.000°C
 * 
 * 建议使用方式：
 *   在独立线程中运行，避免阻塞主线程
 ***************************************************************/

class DriverTemperature : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit DriverTemperature(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DriverTemperature();
    
    /**
     * @brief 获取温度信息
     * @return TemperatureInfo结构体，包含当前温度、最值等
     */
    TemperatureInfo getTemperatureInfo() const;
    
signals:
    /**
     * @brief 初始化完成信号
     */
    void initialized();
    
    /**
     * @brief 监控已启动信号
     */
    void started();
    
    /**
     * @brief 监控已停止信号
     */
    void stopped();
    
    /**
     * @brief 错误信号
     * @param errorString 错误描述信息
     */
    void error(const QString &errorString);
    
    /**
     * @brief 温度变化信号（每次读取都发送）
     * @param temperature 当前温度值（°C）
     */
    void temperatureChanged(float temperature);
    
    /**
     * @brief 高温报警信号（温度超过阈值时发送一次）
     * @param temperature 当前温度值（°C）
     */
    void temperatureHigh(float temperature);
    
    /**
     * @brief 温度恢复正常信号（从高温恢复时发送一次）
     * @param temperature 当前温度值（°C）
     */
    void temperatureNormal(float temperature);
    
public slots:
    /**
     * @brief 初始化温度传感器
     * @note 检查传感器文件是否存在，发送initialized信号
     */
    void initialize();
    
    /**
     * @brief 启动温度监控
     * @note 启动定时器，开始定期读取温度
     */
    void start();
    
    /**
     * @brief 停止温度监控
     * @note 停止定时器，停止读取温度
     */
    void stop();
    
private slots:
    /**
     * @brief 读取温度（定时器回调函数）
     * @note 从sysfs读取温度，更新统计数据，检查报警条件
     */
    void readTemperature();
    
private:
    QTimer *m_timer;           // 定时器，用于定期读取温度
    float m_currentTemp;       // 当前温度（°C）
    float m_maxTemp;           // 记录的最高温度（°C）
    float m_minTemp;           // 记录的最低温度（°C）
    float m_highThreshold;     // 高温报警阈值（°C），默认60°C
    bool m_isHighTemp;         // 高温状态标志（避免重复报警）
    QString m_sensorPath;      // 温度传感器文件路径
};

#endif // DRIVER_TEMPERATURE_H

