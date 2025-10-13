#include "driver_manager.h"

DriverManager::DriverManager(QObject *parent) 
    : QObject(parent)                           // 调用父类构造函数，设置父对象
    , m_configPath("./config/drivers.ini")      // 初始化配置文件路径为相对路径
{
    qDebug() << "DriverManager created\n驱动管理创建\n";
}

DriverManager::~DriverManager()
{
    cleanupAllDrivers();            // 销毁所有驱动
    qDebug() << "DriverManager destroyed\n驱动管理销毁\n";
}

DriverManager& DriverManager::getInstance()
{
    static DriverManager instance;
    return instance;
}

bool DriverManager::initializeAllDrivers()
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "Initializing all drivers...";

    bool allSuccess = true;

    // 初始化GPIO驱动
    //if (!initializeDriver(DRIVER_GPIO)) {
    //    qCritical() << "Failed to initialize GPIO driver";
    //    allSuccess = false;
    //}

    if (allSuccess) 
    {
        qDebug() << "All drivers initialized successfully\n所有驱动程序初始化成功\n";
        emit allDriversReady();
    }
}

bool DriverManager::initializeDriver(DriverType type, const QVariantMap &config)
{
    QMutexLocker locker(&m_mutex);      // 互斥锁
    if (m_drivers.contains(type)) 
    {
        qWarning() << "Driver already initialized:" << type;
        return true;
    }

    // 创建驱动线程
    if (!createDriverThread(type)) 
    {
        qCritical() << "Failed to create thread for driver:" << type;
        return false;
    }
    DriverInfo info;
    info.status = DRIVER_INITIALIZING;
    info.config = config;

    // 根据类型创建具体的驱动实例
    switch (type) {
    case DRIVER_GPIO:
        //info.driver = QSharedPointer<QObject>(new GPIODriver(config));
        break;
        
        default:
        qCritical() << "Unsupported driver type:" << type;
        return false;
    }

    // 移动到线程并设置连接
    info.driver->moveToThread(info.thread.data());
    setupDriverConnections(type, info.driver.data());

    m_drivers[type] = info;

    // 异步初始化驱动
    QMetaObject::invokeMethod(info.driver.data(), "initialize", Qt::QueuedConnection);
    
    qDebug() << "Driver initialization started:" << type;
    return true;
}
bool DriverManager::createDriverThread(DriverType type)
{
    QSharedPointer<QThread> thread(new QThread);
    
    connect(thread.data(), &QThread::started, [type]() {
        qDebug() << "Driver thread started:" << type;
    });
    
    connect(thread.data(), &QThread::finished, [type]() {
        qDebug() << "Driver thread finished:" << type;
    });
    
    thread->start();
    return thread->isRunning();
}

void DriverManager::setupDriverConnections(DriverType type, QObject *driver)
{
    // 连接通用驱动信号
    connect(driver, SIGNAL(initialized(bool)), 
            this, SLOT(handleDriverInitialized(DriverType)), 
            Qt::QueuedConnection);
    connect(driver, SIGNAL(started(bool)), 
            this, SLOT(handleDriverStarted(DriverType)), 
            Qt::QueuedConnection);
    connect(driver, SIGNAL(stopped()), 
            this, SLOT(handleDriverStopped(DriverType)), 
            Qt::QueuedConnection);
    
    connect(driver, SIGNAL(errorOccurred(QString)), 
            this, SLOT(handleDriverError(DriverType, QString)), 
            Qt::QueuedConnection);
    // 连接特定驱动的数据信号
    switch (type) 
    {
    case DRIVER_GPIO:
        connect(driver, SIGNAL(pinStateChanged(int, bool)),
                this, SIGNAL(gpioDataReceived(int, bool)),
                Qt::QueuedConnection);
        break;
        
    }
}

void DriverManager::handleDriverInitialized(DriverType type)
{
    QMutexLocker locker(&m_mutex);
    if (m_drivers.contains(type)) {
        m_drivers[type].status = DRIVER_READY;
        qDebug() << "Driver initialized:" << type;
        emit driverInitialized(type);
    }
}

void DriverManager::handleDriverStarted(DriverType type)
{
    QMutexLocker locker(&m_mutex);
    if (m_drivers.contains(type)) {
        m_drivers[type].status = DRIVER_RUNNING;
        qDebug() << "Driver started:" << type;
        emit driverStarted(type);
    }
}

void DriverManager::handleDriverStopped(DriverType type)
{
    QMutexLocker locker(&m_mutex);
    if (m_drivers.contains(type)) {
        m_drivers[type].status = DRIVER_STOPPED;
        qDebug() << "Driver stopped:" << type;
        emit driverStopped(type);
    }
}

void DriverManager::handleDriverError(DriverType type, const QString &error)
{
    QMutexLocker locker(&m_mutex);
    if (m_drivers.contains(type)) {
        m_drivers[type].status = DRIVER_ERROR;
        qCritical() << "Driver error:" << type << "-" << error;
        emit driverError(type, error);
    }
}

bool DriverManager::startAllDrivers()
{
    QMutexLocker locker(&m_mutex);
    bool allSuccess = true;
    
    for (auto it = m_drivers.begin(); it != m_drivers.end(); ++it) {
        if (it->status == DRIVER_READY) {
            QMetaObject::invokeMethod(it->driver.data(), "start", Qt::QueuedConnection);
        } else {
            qWarning() << "Driver not ready for start:" << it.key();
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool DriverManager::startDriver(DriverType type)
{
    QMutexLocker locker(&m_mutex);
    if (m_drivers.contains(type) && m_drivers[type].status == DRIVER_READY) {
        QMetaObject::invokeMethod(m_drivers[type].driver.data(), "start", Qt::QueuedConnection);
        return true;
    }
    return false;
}

void DriverManager::cleanupAllDrivers()
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "Cleaning up all drivers...";
    
    for (auto it = m_drivers.begin(); it != m_drivers.end(); ++it) {
        if (it->driver) {
            QMetaObject::invokeMethod(it->driver.data(), "cleanup", Qt::BlockingQueuedConnection);
            it->driver.clear();
        }
        
        if (it->thread && it->thread->isRunning()) {
            it->thread->quit();
            it->thread->wait(1000);
            if (it->thread->isRunning()) {
                it->thread->terminate();
            }
        }
    }
    
    m_drivers.clear();
    qDebug() << "All drivers cleaned up";
}