#ifndef DRIVER_MANAGER_H
#define DRIVER_MANAGER_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QVariant>
#include <qdebug.h>

enum DriverType {
    DRIVER_GPIO = 0,            // gpio
};

enum DriverStatus {
    DRIVER_UNINITIALIZED = 0,     //   
    DRIVER_INITIALIZING,
    DRIVER_READY,
    DRIVER_RUNNING,
    DRIVER_ERROR,
    DRIVER_STOPPED
};

class DriverManager : public QObject
{
    Q_OBJECT
public:
    static DriverManager& getInstance();        
    
    // 驱动生命周期管理
    bool initializeAllDrivers();
    bool startAllDrivers();         
    bool stopAllDrivers();
    void cleanupAllDrivers();

    // 单个驱动管理
    bool initializeDriver(DriverType type, const QVariantMap &config = QVariantMap());
    bool startDriver(DriverType type);
    bool stopDriver(DriverType type);
    DriverStatus getDriverStatus(DriverType type) const;

    // 驱动实例获取
    template<typename T>
    QSharedPointer<T> getDriver(DriverType type) const;

    // 配置管理
    void loadDriverConfigs(const QString &configPath);
    void saveDriverConfigs(const QString &configPath);
    
    // 系统信息
    QMap<DriverType, QVariantMap> getAllDriverInfo() const;
    QString getDriverInfoString() const;

signals:
    void driverInitialized(DriverType type);
    void driverStarted(DriverType type);
    void driverStopped(DriverType type);
    void driverError(DriverType type, const QString &error);
    void allDriversReady();
    
    // 驱动数据转发信号
    void gpioDataReceived(int pin, bool state);
    void serialDataReceived(const QByteArray &data);

private slots:
    void handleDriverInitialized(DriverType type);
    void handleDriverStarted(DriverType type);
    void handleDriverStopped(DriverType type);
    void handleDriverError(DriverType type, const QString &error);

private:
    explicit DriverManager(QObject *parent = nullptr);
    ~DriverManager();
    
    struct DriverInfo {
        QSharedPointer<QObject> driver;
        QSharedPointer<QThread> thread;
        DriverStatus status;
        QVariantMap config;
    };
    
    QMap<DriverType, DriverInfo> m_drivers;
    mutable QMutex m_mutex;
    QString m_configPath;
    
    void setupDriverConnections(DriverType type, QObject *driver);
    bool createDriverThread(DriverType type);
};
#endif