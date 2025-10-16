#ifndef IMX6ULL_DRIVERS_SCANNER_H
#define IMX6ULL_DRIVERS_SCANNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>

/***************************************************************
 * 结构体: InterfaceInfo
 * 功能: 系统接口信息数据结构
 * 说明: 存储系统中各类硬件接口的详细信息
 ***************************************************************/
struct InterfaceInfo {
    QString type;           // 接口类型 (GPIO, I2C, SPI, CAN, PWM, LED, etc.)
    QString name;           // 接口名称 (例如：gpiochip0, i2c-0)
    QString path;           // 完整sysfs路径
    QString description;    // 接口描述信息
    QMap<QString, QString> attributes;  // 接口属性键值对
};

/***************************************************************
 * 类名: SystemScanner
 * 功能: 系统硬件接口扫描器
 * 
 * 描述:
 *   扫描Linux系统中的各类硬件接口，包括：
 *   - GPIO控制器
 *   - I2C总线
 *   - SPI总线
 *   - CAN总线
 *   - PWM控制器
 *   - 温度传感器（Thermal zones）
 *   - LED设备
 *   - 输入设备（按键、触摸屏等）
 *   - 网络接口
 * 
 * 使用场景：
 *   1. 系统启动时自动检测可用硬件资源
 *   2. 驱动开发时快速了解硬件配置
 *   3. 生成系统硬件报告
 * 
 * 使用方法：
 *   SystemScanner scanner;
 *   scanner.scanAll();              // 扫描所有接口
 *   scanner.printReport();          // 打印报告
 *   auto gpios = scanner.getInterfacesByType("GPIO");
 * 
 * 扫描原理：
 *   遍历/sys/class/下的各类设备目录，读取设备属性
 ***************************************************************/

class SystemScanner : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit SystemScanner(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SystemScanner();
    
    // ========== 扫描接口 ==========
    
    /**
     * @brief 扫描所有硬件接口
     * @note 调用所有单独的扫描函数，生成完整的硬件列表
     */
    void scanAll();
    
    /**
     * @brief 扫描GPIO控制器
     * @note 扫描/sys/class/gpio/下的gpiochip*设备
     */
    void scanGPIO();
    
    /**
     * @brief 扫描I2C总线
     * @note 扫描/sys/class/i2c-adapter/和/sys/class/i2c-dev/
     */
    void scanI2C();
    
    /**
     * @brief 扫描SPI总线
     * @note 扫描/sys/class/spi_master/
     */
    void scanSPI();
    
    /**
     * @brief 扫描CAN总线
     * @note 扫描/sys/class/net/下type为280的设备
     */
    void scanCAN();
    
    /**
     * @brief 扫描PWM控制器
     * @note 扫描/sys/class/pwm/下的pwmchip*设备
     */
    void scanPWM();
    
    /**
     * @brief 扫描温度传感器
     * @note 扫描/sys/class/thermal/下的thermal_zone*设备
     */
    void scanThermal();
    
    /**
     * @brief 扫描LED设备
     * @note 扫描/sys/class/leds/下的所有LED
     */
    void scanLED();
    
    /**
     * @brief 扫描输入设备
     * @note 扫描/sys/class/input/下的event*设备
     */
    void scanInput();
    
    /**
     * @brief 扫描网络接口
     * @note 扫描/sys/class/net/下的所有网络接口
     */
    void scanNetwork();
    
    /**
     * @brief 扫描串口设备
     * @note 使用QSerialPortInfo扫描所有可用串口
     */
    void scanSerialPorts();
    
    // ========== 获取扫描结果 ==========
    
    /**
     * @brief 获取所有扫描到的接口
     * @return 接口信息列表
     */
    QVector<InterfaceInfo> getAllInterfaces() const;
    
    /**
     * @brief 根据类型获取接口列表
     * @param type 接口类型（如"GPIO"、"I2C"）
     * @return 该类型的接口信息列表
     */
    QVector<InterfaceInfo> getInterfacesByType(const QString &type) const;
    
    // ========== 报告生成 ==========
    
    /**
     * @brief 生成格式化的扫描报告
     * @return 报告字符串
     */
    QString generateReport() const;
    
    /**
     * @brief 打印扫描报告到控制台
     */
    void printReport() const;
    
signals:
    /**
     * @brief 扫描开始信号
     */
    void scanStarted();
    
    /**
     * @brief 扫描进度信号
     * @param current 当前进度
     * @param total 总数
     * @param item 当前扫描项
     */
    void scanProgress(int current, int total, const QString &item);
    
    /**
     * @brief 扫描完成信号
     * @param totalFound 发现的接口总数
     */
    void scanCompleted(int totalFound);
    
    /**
     * @brief 发现接口信号
     * @param type 接口类型
     * @param name 接口名称
     */
    void interfaceFound(const QString &type, const QString &name);
    
private:
    QVector<InterfaceInfo> m_interfaces;  // 存储所有扫描到的接口信息
    
    // ========== 辅助函数 ==========
    
    /**
     * @brief 扫描目录（通用方法）
     * @param basePath 基础路径
     * @param type 接口类型
     */
    void scanDirectory(const QString &basePath, const QString &type);
    
    /**
     * @brief 读取sysfs文件内容
     * @param path 文件路径
     * @return 文件内容（去除首尾空白）
     */
    QString readSysFile(const QString &path);
    
    /**
     * @brief 检查路径是否存在
     * @param path 路径
     * @return true=存在, false=不存在
     */
    bool pathExists(const QString &path);
    
    /**
     * @brief 列出目录下的所有条目
     * @param path 目录路径
     * @return 条目名称列表
     */
    QStringList listDirectory(const QString &path);
};

#endif // SYSTEM_SCANNER_H

