/***************************************************************
 * Copyright: Alex
 * FileName: LogManager.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 日志管理器 - 分模块、分线程的日志系统
 *
 * 功能说明:
 *   1. 按模块分类记录日志（温度、天气、Modbus等）
 *   2. 支持多线程安全
 *   3. 日志分级（Debug/Info/Warning/Error）
 *   4. 自动日志轮转（按大小或时间）
 *   5. 控制台和文件双输出
 *
 * 使用示例:
 *   LogManager::getInstance()->log("Weather", LogLevel::Info, "天气更新成功");
 *   LOG_INFO("Weather", "天气更新成功");
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_CORE_LOGMANAGER_H
#define IMX6ULL_CORE_LOGMANAGER_H

#include <QObject>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMap>
#include <QThread>
#include <QTimer>

/***************************************************************
 * 日志级别枚举
 ***************************************************************/
enum class LogLevel {
    Debug = 0,      // 调试信息
    Info = 1,       // 一般信息
    Warning = 2,    // 警告
    Error = 3,      // 错误
    Critical = 4    // 严重错误
};

/***************************************************************
 * 日志配置结构
 ***************************************************************/
struct LogConfig {
    bool enableConsole;         // 是否输出到控制台
    bool enableFile;            // 是否输出到文件
    QString logDir;             // 日志目录
    int maxFileSize;            // 单文件最大大小（KB）
    int maxFileCount;           // 最大文件数量
    LogLevel minLevel;          // 最小日志级别
    bool enableTimestamp;       // 是否添加时间戳
    bool enableThreadId;        // 是否添加线程ID
    
    LogConfig()
        : enableConsole(true)
        , enableFile(true)
        , logDir("/tmp/imx6ull_logs")
        , maxFileSize(1024)     // 1MB
        , maxFileCount(10)
        , minLevel(LogLevel::Info)
        , enableTimestamp(true)
        , enableThreadId(true)
    {}
};

/***************************************************************
 * 日志文件信息
 ***************************************************************/
struct LogFileInfo {
    QFile *file;
    QTextStream *stream;
    qint64 currentSize;
    int fileIndex;
    
    LogFileInfo()
        : file(nullptr)
        , stream(nullptr)
        , currentSize(0)
        , fileIndex(0)
    {}
};

/***************************************************************
 * 类名: LogManager
 * 功能: 日志管理器（单例）
 ***************************************************************/
class LogManager : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 获取单例实例
     */
    static LogManager* getInstance();
    
    /**
     * @brief 销毁单例
     */
    static void destroyInstance();
    
    /**
     * @brief 初始化日志系统
     * @param config 日志配置
     */
    void initialize(const LogConfig &config = LogConfig());
    
    /**
     * @brief 记录日志
     * @param module 模块名称（如"Weather", "Temperature"）
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(const QString &module, LogLevel level, const QString &message);
    
    /**
     * @brief 设置模块的日志级别
     * @param module 模块名称
     * @param level 最小日志级别
     */
    void setModuleLevel(const QString &module, LogLevel level);
    
    /**
     * @brief 刷新所有日志缓冲
     */
    void flush();
    
    /**
     * @brief 获取日志统计信息
     */
    QString getStatistics() const;
    
    /**
     * @brief 清理旧日志文件
     */
    void cleanOldLogs();
    
signals:
    /**
     * @brief 新日志信号（用于实时监控）
     */
    void newLog(QString module, int level, QString message);

private:
    explicit LogManager(QObject *parent = nullptr);
    ~LogManager();
    
    // 禁止拷贝
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;
    
    /**
     * @brief 格式化日志消息
     */
    QString formatMessage(const QString &module, LogLevel level, const QString &message);
    
    /**
     * @brief 获取日志级别字符串
     */
    QString getLevelString(LogLevel level) const;
    
    /**
     * @brief 获取或创建模块日志文件
     */
    LogFileInfo* getModuleFile(const QString &module);
    
    /**
     * @brief 检查并轮转日志文件
     */
    void rotateLogFile(const QString &module, LogFileInfo *info);
    
    /**
     * @brief 写入到控制台
     */
    void writeToConsole(LogLevel level, const QString &formattedMessage);
    
    /**
     * @brief 写入到文件
     */
    void writeToFile(const QString &module, const QString &formattedMessage);

private slots:
    /**
     * @brief 定时刷新日志
     */
    void onFlushTimer();

private:
    static LogManager *m_instance;
    static QMutex m_instanceMutex;
    
    QMutex m_logMutex;                          // 日志互斥锁
    LogConfig m_config;                         // 日志配置
    QMap<QString, LogFileInfo*> m_moduleFiles;  // 模块日志文件映射
    QMap<QString, LogLevel> m_moduleLevels;     // 模块日志级别
    QTimer *m_flushTimer;                       // 刷新定时器
    
    // 统计信息
    QMap<QString, int> m_logCounts;             // 各模块日志计数
};

/***************************************************************
 * 便捷宏定义
 ***************************************************************/
#define LOG_DEBUG(module, msg)    LogManager::getInstance()->log(module, LogLevel::Debug, msg)
#define LOG_INFO(module, msg)     LogManager::getInstance()->log(module, LogLevel::Info, msg)
#define LOG_WARNING(module, msg)  LogManager::getInstance()->log(module, LogLevel::Warning, msg)
#define LOG_ERROR(module, msg)    LogManager::getInstance()->log(module, LogLevel::Error, msg)
#define LOG_CRITICAL(module, msg) LogManager::getInstance()->log(module, LogLevel::Critical, msg)

#endif // IMX6ULL_CORE_LOGMANAGER_H

