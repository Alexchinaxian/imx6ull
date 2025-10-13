#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QDebug>

class GPIODriver : public QObject
{
    Q_OBJECT

public:
    explicit GPIODriver(const QVariantMap &config = QVariantMap(), QObject *parent = nullptr);
    ~GPIODriver();
    
    // 驱动控制
    bool initialize();
    bool start();
    bool stop();
    void cleanup();
    
    // GPIO操作
    bool setPinDirection(int pin, bool isOutput);
    bool writePin(int pin, bool state);
    bool readPin(int pin);
    bool setPinEdge(int pin, const QString &edge); // "rising", "falling", "both"
    
    // 批量操作
    QMap<int, bool> readAllPins();
    bool writeMultiplePins(const QMap<int, bool> &states);
    
    // 状态信息
    QString getStatus() const;
    QVariantMap getPinStates() const;

signals:
    void initialized(bool success);
    void started(bool success);
    void stopped();
    void errorOccurred(const QString &error);
    
    // 数据信号
    void pinStateChanged(int pin, bool state);
    void pinEdgeDetected(int pin, bool risingEdge);
    
    // 内部线程信号
    void setPinDirectionRequested(int pin, bool isOutput);
    void writePinRequested(int pin, bool state);
    void readPinRequested(int pin);

public slots:
    void enablePolling(bool enable, int intervalMs = 100);
    void setPollingPins(const QList<int> &pins);

private slots:
    void pollPins();
    void handleSetPinDirection(int pin, bool isOutput);
    void handleWritePin(int pin, bool state);
    void handleReadPin(int pin);

private:
    struct PinInfo {
        int number;
        bool isOutput;
        bool currentState;
        QString directionPath;
        QString valuePath;
        QString edgePath;
    };
    
    QMap<int, PinInfo> m_pins;
    QTimer *m_pollTimer;
    mutable QMutex m_mutex;
    QString m_sysfsPath;
    bool m_initialized;
    bool m_running;
    
    bool exportPin(int pin);
    bool unexportPin(int pin);
    bool setupPin(int pin, bool isOutput);
    QString getPinPath(int pin, const QString &file) const;
    bool writeSysfs(const QString &path, const QString &value);
    QString readSysfs(const QString &path) const;
};

#endif // GPIO_DRIVER_H