#include "drivers/led/DriverLED.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDebug>

/***************************************************************
 * 实现文件: driver_led.cpp
 * 功能: LED驱动的具体实现
 * 说明: 通过读写sysfs文件系统来控制LED
 ***************************************************************/

/**
 * @brief 构造函数 - 初始化LED驱动实例
 * @param ledName LED名称
 * @param parent 父对象指针
 * 
 * 初始化流程：
 * 1. 构造LED的sysfs路径（/sys/class/leds/LED名称/）
 * 2. 读取最大亮度值
 * 3. 读取当前亮度值
 */
DriverLED::DriverLED(const QString &ledName, QObject *parent)
    : QObject(parent)
    , m_ledName(ledName)
    , m_basePath(QString("/sys/class/leds/%1").arg(ledName))
    , m_maxBrightness(255)  // 默认最大亮度255
    , m_currentBrightness(0)
{
    // 读取实际的最大亮度值（可能是1或255等）
    int maxBr = getMaxBrightness();
    if (maxBr > 0) {
        m_maxBrightness = maxBr;
    }
    
    // 读取LED当前状态
    m_currentBrightness = getBrightness();
}

DriverLED::~DriverLED()
{
}

bool DriverLED::writeToFile(const QString &filename, const QString &value)
{
    QString path = m_basePath + "/" + filename;
    QFile file(path);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit error(QString("Failed to open %1: %2").arg(path, file.errorString()));
        return false;
    }
    
    QTextStream out(&file);
    out << value;
    file.close();
    
    return true;
}

QString DriverLED::readFromFile(const QString &filename)
{
    QString path = m_basePath + "/" + filename;
    QFile file(path);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    QString content = in.readAll().trimmed();
    file.close();
    
    return content;
}

bool DriverLED::setBrightness(int brightness)
{
    if (brightness < 0 || brightness > m_maxBrightness) {
        qWarning() << "Invalid brightness:" << brightness;
        return false;
    }
    
    if (writeToFile("brightness", QString::number(brightness))) {
        m_currentBrightness = brightness;
        emit brightnessChanged(brightness);
        return true;
    }
    
    return false;
}

int DriverLED::getBrightness()
{
    QString value = readFromFile("brightness");
    bool ok;
    int brightness = value.toInt(&ok);
    return ok ? brightness : 0;
}

int DriverLED::getMaxBrightness()
{
    QString value = readFromFile("max_brightness");
    bool ok;
    int maxBr = value.toInt(&ok);
    return ok ? maxBr : 255;
}

bool DriverLED::setTrigger(const QString &trigger)
{
    return writeToFile("trigger", trigger);
}

QString DriverLED::getCurrentTrigger()
{
    QString triggers = readFromFile("trigger");
    // 当前trigger在方括号中，例如: [none] timer heartbeat
    int start = triggers.indexOf('[');
    int end = triggers.indexOf(']');
    if (start >= 0 && end > start) {
        return triggers.mid(start + 1, end - start - 1);
    }
    return QString();
}

QStringList DriverLED::getAvailableTriggers()
{
    QString triggers = readFromFile("trigger");
    // 移除方括号
    triggers.remove('[').remove(']');
    return triggers.split(' ', QString::SkipEmptyParts);
}

bool DriverLED::turnOn()
{
    return setBrightness(m_maxBrightness);
}

bool DriverLED::turnOff()
{
    return setBrightness(0);
}

bool DriverLED::toggle()
{
    if (m_currentBrightness > 0) {
        return turnOff();
    } else {
        return turnOn();
    }
}

/**
 * @brief LED闪烁指定次数
 * @param times 闪烁次数
 * @param interval_ms 闪烁间隔（毫秒）
 * @return true=成功, false=失败
 * 
 * 实现原理：
 * 1. 循环指定次数
 * 2. 每次循环：打开LED -> 等待半个间隔 -> 关闭LED -> 等待半个间隔
 * 3. 最后一次闪烁后不等待
 * 
 * 注意：这是阻塞操作，会占用当前线程
 * 例如：blink(3, 500) 表示闪烁3次，每次亮250ms，灭250ms
 */
bool DriverLED::blink(int times, int interval_ms)
{
    for (int i = 0; i < times; i++) {
        turnOn();  // 打开LED
        QThread::msleep(interval_ms / 2);  // 等待半个间隔
        turnOff();  // 关闭LED
        
        // 如果不是最后一次，等待另外半个间隔
        if (i < times - 1) {
            QThread::msleep(interval_ms / 2);
        }
    }
    return true;
}

