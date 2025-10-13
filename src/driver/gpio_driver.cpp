#include "gpio_driver.h"

GPIODriver::GPIODriver(const QVariantMap &config, QObject *parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
    , m_sysfsPath("/sys/class/gpio")
    , m_initialized(false)
    , m_running(false)
{
    // 从配置中获取GPIO引脚设置
    if (config.contains("pins")) {
        auto pinsList = config["pins"].toList();
        for (const auto &pinConfig : pinsList) {
            auto pinMap = pinConfig.toMap();
            int pin = pinMap["number"].toInt();
            bool isOutput = pinMap["direction"].toString() == "out";
            
            PinInfo info;
            info.number = pin;
            info.isOutput = isOutput;
            info.currentState = false;
            info.directionPath = getPinPath(pin, "direction");
            info.valuePath = getPinPath(pin, "value");
            info.edgePath = getPinPath(pin, "edge");
            
            m_pins[pin] = info;
        }
    }
    
    connect(m_pollTimer, &QTimer::timeout, this, &GPIODriver::pollPins);
    
    // 连接内部信号槽
    connect(this, &GPIODriver::setPinDirectionRequested,
            this, &GPIODriver::handleSetPinDirection);
    connect(this, &GPIODriver::writePinRequested,
            this, &GPIODriver::handleWritePin);
    connect(this, &GPIODriver::readPinRequested,
            this, &GPIODriver::handleReadPin);
}

GPIODriver::~GPIODriver()
{
    cleanup();
}

bool GPIODriver::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        qWarning() << "GPIO驱动已经初始化";
        return true;
    }
    
    qDebug() << "正在初始化GPIO驱动...";
    
    // 检查sysfs接口是否存在
    QDir sysfsDir(m_sysfsPath);
    if (!sysfsDir.exists()) {
        QString error = "未找到GPIO sysfs接口: " + m_sysfsPath;
        qCritical() << error;
        emit errorOccurred(error);
        return false;
    }
    
    // 导出并设置所有引脚
    for (auto it = m_pins.begin(); it != m_pins.end(); ++it) {
        if (!exportPin(it->number)) {
            QString error = QString("无法导出GPIO引脚 %1").arg(it->number);
            qCritical() << error;
            emit errorOccurred(error);
            return false;
        }
        
        if (!setupPin(it->number, it->isOutput)) {
            QString error = QString("无法设置GPIO引脚 %1").arg(it->number);
            qCritical() << error;
            emit errorOccurred(error);
            return false;
        }
        
        qDebug() << "GPIO引脚" << it->number << "初始化为" 
                 << (it->isOutput ? "输出" : "输入");
    }
    
    m_initialized = true;
    qDebug() << "GPIO驱动初始化成功";
    emit initialized(true);
    return true;
}

bool GPIODriver::start()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "GPIO驱动未初始化，无法启动";
        return false;
    }
    
    if (m_running) {
        qWarning() << "GPIO驱动已经在运行";
        return true;
    }
    
    m_running = true;
    qDebug() << "GPIO驱动启动";
    emit started(true);
    return true;
}

bool GPIODriver::stop()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_running) {
        return true;
    }
    
    m_running = false;
    m_pollTimer->stop();
    qDebug() << "GPIO驱动停止";
    emit stopped();
    return true;
}

void GPIODriver::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_running) {
        stop();
    }
    
    // 取消导出所有引脚
    for (auto it = m_pins.begin(); it != m_pins.end(); ++it) {
        unexportPin(it->number);
    }
    
    m_pins.clear();
    m_initialized = false;
    qDebug() << "GPIO驱动清理完成";
}

bool GPIODriver::setPinDirection(int pin, bool isOutput)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_pins.contains(pin)) {
        qWarning() << "未找到GPIO引脚:" << pin;
        return false;
    }
    
    if (setupPin(pin, isOutput)) {
        m_pins[pin].isOutput = isOutput;
        qDebug() << "GPIO引脚" << pin << "方向设置为" 
                 << (isOutput ? "输出" : "输入");
        return true;
    }
    
    return false;
}

bool GPIODriver::writePin(int pin, bool state)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_pins.contains(pin) || !m_pins[pin].isOutput) {
        qWarning() << "无法写入GPIO引脚:" << pin;
        return false;
    }
    
    QString value = state ? "1" : "0";
    if (writeSysfs(m_pins[pin].valuePath, value)) {
        m_pins[pin].currentState = state;
        emit pinStateChanged(pin, state);
        qDebug() << "GPIO引脚" << pin << "设置为" << state;
        return true;
    }
    
    return false;
}

bool GPIODriver::readPin(int pin)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_pins.contains(pin)) {
        qWarning() << "未找到GPIO引脚:" << pin;
        return false;
    }
    
    QString value = readSysfs(m_pins[pin].valuePath);
    if (!value.isEmpty()) {
        bool state = value.trimmed() == "1";
        m_pins[pin].currentState = state;
        return state;
    }
    
    return false;
}

bool GPIODriver::setPinEdge(int pin, const QString &edge)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_pins.contains(pin)) {
        qWarning() << "未找到GPIO引脚:" << pin;
        return false;
    }
    
    if (m_pins[pin].isOutput) {
        qWarning() << "输出引脚不支持边缘检测:" << pin;
        return false;
    }
    
    QString edgePath = getPinPath(pin, "edge");
    if (writeSysfs(edgePath, edge)) {
        qDebug() << "GPIO引脚" << pin << "边缘检测设置为" << edge;
        return true;
    }
    
    return false;
}

QMap<int, bool> GPIODriver::readAllPins()
{
    QMutexLocker locker(&m_mutex);
    QMap<int, bool> states;
    
    for (auto it = m_pins.begin(); it != m_pins.end(); ++it) {
        states[it->number] = it->currentState;
    }
    
    return states;
}

bool GPIODriver::writeMultiplePins(const QMap<int, bool> &states)
{
    QMutexLocker locker(&m_mutex);
    bool success = true;
    
    for (auto it = states.begin(); it != states.end(); ++it) {
        if (!writePin(it.key(), it.value())) {
            success = false;
        }
    }
    
    return success;
}

QString GPIODriver::getStatus() const
{
    QMutexLocker locker(&m_mutex);
    
    QString status = QString("GPIO驱动状态: %1, 运行: %2, 引脚数量: %3")
                    .arg(m_initialized ? "已初始化" : "未初始化")
                    .arg(m_running ? "是" : "否")
                    .arg(m_pins.size());
    
    return status;
}

QVariantMap GPIODriver::getPinStates() const
{
    QMutexLocker locker(&m_mutex);
    QVariantMap states;
    
    for (auto it = m_pins.begin(); it != m_pins.end(); ++it) {
        states[QString::number(it->number)] = it->currentState;
    }
    
    return states;
}

void GPIODriver::enablePolling(bool enable, int intervalMs)
{
    QMutexLocker locker(&m_mutex);
    
    if (enable) {
        if (!m_pollTimer->isActive()) {
            m_pollTimer->start(intervalMs);
            qDebug() << "GPIO轮询启动，间隔:" << intervalMs << "毫秒";
        }
    } else {
        m_pollTimer->stop();
        qDebug() << "GPIO轮询停止";
    }
}

void GPIODriver::setPollingPins(const QList<int> &pins)
{
    QMutexLocker locker(&m_mutex);
    
    // 这里可以添加逻辑来限制只轮询特定的引脚
    // 当前实现会轮询所有输入引脚
    Q_UNUSED(pins)
    qDebug() << "设置轮询引脚:" << pins;
}

bool GPIODriver::exportPin(int pin)
{
    QString exportPath = m_sysfsPath + "/export";
    return writeSysfs(exportPath, QString::number(pin));
}

bool GPIODriver::unexportPin(int pin)
{
    QString unexportPath = m_sysfsPath + "/unexport";
    return writeSysfs(unexportPath, QString::number(pin));
}

bool GPIODriver::setupPin(int pin, bool isOutput)
{
    QString directionPath = getPinPath(pin, "direction");
    QString direction = isOutput ? "out" : "in";
    
    if (!writeSysfs(directionPath, direction)) {
        return false;
    }
    
    // 如果是输入引脚，设置边缘检测
    if (!isOutput) {
        QString edgePath = getPinPath(pin, "edge");
        writeSysfs(edgePath, "both"); // 检测上升沿和下降沿
    }
    
    return true;
}

QString GPIODriver::getPinPath(int pin, const QString &file) const
{
    return QString("%1/gpio%2/%3").arg(m_sysfsPath).arg(pin).arg(file);
}

bool GPIODriver::writeSysfs(const QString &path, const QString &value)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << value;
        file.close();
        return true;
    }
    
    qWarning() << "无法写入文件:" << path << "错误:" << file.errorString();
    return false;
}

QString GPIODriver::readSysfs(const QString &path) const
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString value = in.readAll();
        file.close();
        return value;
    }
    
    qWarning() << "无法读取文件:" << path << "错误:" << file.errorString();
    return QString();
}

void GPIODriver::handleSetPinDirection(int pin, bool isOutput)
{
    setPinDirection(pin, isOutput);
}

void GPIODriver::handleWritePin(int pin, bool state)
{
    writePin(pin, state);
}

void GPIODriver::handleReadPin(int pin)
{
    readPin(pin);
}

void GPIODriver::pollPins()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_running) {
        return;
    }
    
    for (auto it = m_pins.begin(); it != m_pins.end(); ++it) {
        if (!it->isOutput) { // 只轮询输入引脚
            QString value = readSysfs(it->valuePath);
            if (!value.isEmpty()) {
                bool newState = value.trimmed() == "1";
                if (it->currentState != newState) {
                    bool risingEdge = newState && !it->currentState;
                    it->currentState = newState;
                    emit pinStateChanged(it->number, newState);
                    if (risingEdge) {
                        emit pinEdgeDetected(it->number, true);
                    } else {
                        emit pinEdgeDetected(it->number, false);
                    }
                }
            }
        }
    }
}