#include "drivers/gpio/DriverGPIO.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDebug>

/***************************************************************
 * 实现文件: driver_gpio.cpp
 * 功能: GPIO驱动的具体实现
 * 说明: 通过读写sysfs文件系统来控制GPIO
 ***************************************************************/

/**
 * @brief 构造函数 - 初始化GPIO驱动实例
 * @param gpioNum GPIO编号（例如：4表示GPIO4）
 * @param parent 父对象指针
 * 
 * 初始化成员变量：
 * - m_gpioNum: 保存GPIO编号，用于后续操作
 * - m_exported: 初始为false，表示GPIO未导出
 * - m_currentValue: 初始为Low（低电平）
 */
DriverGPIO::DriverGPIO(int gpioNum, QObject *parent)
    : QObject(parent)
    , m_gpioNum(gpioNum)
    , m_exported(false)
    , m_currentValue(Low)
{
}

/**
 * @brief 析构函数 - 清理GPIO资源
 * 
 * 自动调用unexportGPIO()，确保GPIO资源被正确释放
 * 这是RAII（Resource Acquisition Is Initialization）的体现
 */
DriverGPIO::~DriverGPIO()
{
    unexportGPIO();  // 释放GPIO资源
}

/**
 * @brief 写入数据到sysfs文件
 * @param path 文件路径（例如：/sys/class/gpio/export）
 * @param value 要写入的值（字符串格式）
 * @return true=写入成功, false=写入失败
 * 
 * 实现步骤：
 * 1. 打开指定的sysfs文件（只写模式）
 * 2. 使用QTextStream写入数据
 * 3. 关闭文件
 * 4. 如果失败，发送error信号
 */
bool DriverGPIO::writeToFile(const QString &path, const QString &value)
{
    QFile file(path);
    // 以只写+文本模式打开文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // 打开失败，发送错误信号
        emit error(QString("Failed to open %1: %2").arg(path, file.errorString()));
        return false;
    }
    
    // 使用QTextStream写入数据
    QTextStream out(&file);
    out << value;
    file.close();
    
    return true;
}

/**
 * @brief 从sysfs文件读取数据
 * @param path 文件路径
 * @return 读取到的字符串内容（已去除首尾空白）
 * 
 * 实现步骤：
 * 1. 打开指定的sysfs文件（只读模式）
 * 2. 使用QTextStream读取所有内容
 * 3. 去除首尾空白字符（trimmed）
 * 4. 关闭文件并返回内容
 */
QString DriverGPIO::readFromFile(const QString &path)
{
    QFile file(path);
    // 以只读+文本模式打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();  // 打开失败返回空字符串
    }
    
    // 读取所有内容并去除首尾空白
    QTextStream in(&file);
    QString content = in.readAll().trimmed();
    file.close();
    
    return content;
}

/**
 * @brief 导出GPIO到用户空间
 * @return true=导出成功, false=导出失败
 * 
 * 实现流程：
 * 1. 检查是否已经导出，避免重复操作
 * 2. 向/sys/class/gpio/export写入GPIO编号
 * 3. 等待内核创建对应的sysfs节点（/sys/class/gpio/gpioN/）
 * 4. 最多等待100ms（10次×10ms），超时则返回失败
 * 
 * sysfs路径示例：
 * - /sys/class/gpio/export -> 写入"4"
 * - /sys/class/gpio/gpio4/ -> 内核自动创建此目录
 */
bool DriverGPIO::exportGPIO()
{
    // 如果已经导出，直接返回成功
    if (m_exported) {
        return true;
    }
    
    // 检查GPIO目录是否已存在（可能被其他程序导出）
    QString gpioPath = QString("/sys/class/gpio/gpio%1").arg(m_gpioNum);
    if (QFile::exists(gpioPath)) {
        m_exported = true;
        qInfo() << "GPIO" << m_gpioNum << "already exported";
        return true;
    }
    
    // 向export文件写入GPIO编号，请求内核导出GPIO
    if (!writeToFile("/sys/class/gpio/export", QString::number(m_gpioNum))) {
        return false;
    }
    
    // 等待内核创建sysfs节点（异步操作，需要轮询等待）
    for (int i = 0; i < 10; i++) {
        if (QFile::exists(gpioPath)) {
            m_exported = true;
            qInfo() << "GPIO" << m_gpioNum << "exported successfully";
            return true;
        }
        QThread::msleep(10);  // 等待10ms后重试
    }
    
    // 超时仍未创建，导出失败
    emit error(QString("GPIO%1 export timeout").arg(m_gpioNum));
    return false;
}

/**
 * @brief 取消导出GPIO
 * @return true=取消成功, false=取消失败
 * 
 * 向/sys/class/gpio/unexport写入GPIO编号，
 * 通知内核释放该GPIO，删除对应的sysfs节点
 */
bool DriverGPIO::unexportGPIO()
{
    // 如果未导出，无需操作
    if (!m_exported) {
        return true;
    }
    
    // 向unexport文件写入GPIO编号
    if (!writeToFile("/sys/class/gpio/unexport", QString::number(m_gpioNum))) {
        return false;
    }
    
    m_exported = false;
    qInfo() << "GPIO" << m_gpioNum << "unexported";
    return true;
}

/**
 * @brief 设置GPIO方向（输入/输出）
 * @param dir GPIO方向（Input或Output）
 * @return true=设置成功, false=设置失败
 * 
 * 向/sys/class/gpio/gpioN/direction写入：
 * - "in"  表示输入模式
 * - "out" 表示输出模式
 */
bool DriverGPIO::setDirection(Direction dir)
{
    // 检查GPIO是否已导出
    if (!m_exported) {
        qWarning() << "GPIO" << m_gpioNum << "not exported";
        return false;
    }
    
    // 构造direction文件路径
    QString path = QString("/sys/class/gpio/gpio%1/direction").arg(m_gpioNum);
    // 转换枚举值为字符串
    QString dirStr = (dir == Output) ? "out" : "in";
    
    return writeToFile(path, dirStr);
}

/**
 * @brief 设置GPIO输出值
 * @param val GPIO值（Low或High）
 * @return true=设置成功, false=设置失败
 * 
 * 向/sys/class/gpio/gpioN/value写入：
 * - "0" 表示低电平
 * - "1" 表示高电平
 * 
 * 注意：只有在Output模式下此操作才有意义
 */
bool DriverGPIO::setValue(Value val)
{
    // 检查GPIO是否已导出
    if (!m_exported) {
        qWarning() << "GPIO" << m_gpioNum << "not exported";
        return false;
    }
    
    // 构造value文件路径
    QString path = QString("/sys/class/gpio/gpio%1/value").arg(m_gpioNum);
    
    // 写入值并更新状态
    if (writeToFile(path, QString::number(val))) {
        m_currentValue = val;  // 更新当前值
        emit valueChanged(val); // 发送值变化信号
        return true;
    }
    
    return false;
}

/**
 * @brief 读取GPIO当前值
 * @return GPIO值（Low或High）
 * 
 * 从/sys/class/gpio/gpioN/value读取值：
 * - "0" 表示低电平，返回Low
 * - "1" 表示高电平，返回High
 * 
 * 适用于Input和Output模式
 */
DriverGPIO::Value DriverGPIO::getValue()
{
    // 检查GPIO是否已导出
    if (!m_exported) {
        qWarning() << "GPIO" << m_gpioNum << "not exported";
        return Low;
    }
    
    // 读取value文件内容
    QString path = QString("/sys/class/gpio/gpio%1/value").arg(m_gpioNum);
    QString value = readFromFile(path);
    
    // 转换为枚举值
    return (value == "1") ? High : Low;
}

/**
 * @brief 设置GPIO为高电平
 * @return true=设置成功, false=设置失败
 * 
 * 便捷函数，等同于setValue(High)
 */
bool DriverGPIO::setHigh()
{
    return setValue(High);
}

/**
 * @brief 设置GPIO为低电平
 * @return true=设置成功, false=设置失败
 * 
 * 便捷函数，等同于setValue(Low)
 */
bool DriverGPIO::setLow()
{
    return setValue(Low);
}

/**
 * @brief 翻转GPIO状态
 * @return true=翻转成功, false=翻转失败
 * 
 * 根据当前状态进行翻转：
 * - 如果当前是High，则设置为Low
 * - 如果当前是Low，则设置为High
 */
bool DriverGPIO::toggle()
{
    Value newValue = (m_currentValue == High) ? Low : High;
    return setValue(newValue);
}

/**
 * @brief 检查GPIO是否已导出
 * @return true=已导出, false=未导出
 */
bool DriverGPIO::isExported() const
{
    return m_exported;
}

