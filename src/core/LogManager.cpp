/***************************************************************
 * Copyright: Alex
 * FileName: LogManager.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 日志管理器实现
 ***************************************************************/

#include "core/LogManager.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

// 初始化静态成员
LogManager* LogManager::m_instance = nullptr;
QMutex LogManager::m_instanceMutex;

/***************************************************************
 * 获取单例实例
 ***************************************************************/
LogManager* LogManager::getInstance()
{
    if (m_instance == nullptr)
    {
        QMutexLocker locker(&m_instanceMutex);
        if (m_instance == nullptr)
        {
            m_instance = new LogManager();
        }
    }
    return m_instance;
}

/***************************************************************
 * 销毁单例
 ***************************************************************/
void LogManager::destroyInstance()
{
    QMutexLocker locker(&m_instanceMutex);
    if (m_instance != nullptr)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

/***************************************************************
 * 构造函数
 ***************************************************************/
LogManager::LogManager(QObject *parent)
    : QObject(parent)
    , m_flushTimer(nullptr)
{
    // 创建刷新定时器（每5秒刷新一次）
    m_flushTimer = new QTimer(this);
    m_flushTimer->setInterval(5000);
    connect(m_flushTimer, &QTimer::timeout, this, &LogManager::onFlushTimer);
}

/***************************************************************
 * 析构函数
 ***************************************************************/
LogManager::~LogManager()
{
    // 停止定时器
    if (m_flushTimer)
    {
        m_flushTimer->stop();
    }
    
    // 刷新并关闭所有文件
    flush();
    
    // 释放文件资源
    for (auto it = m_moduleFiles.begin(); it != m_moduleFiles.end(); ++it)
    {
        LogFileInfo *info = it.value();
        if (info)
        {
            if (info->stream)
            {
                delete info->stream;
            }
            if (info->file)
            {
                info->file->close();
                delete info->file;
            }
            delete info;
        }
    }
    m_moduleFiles.clear();
}

/***************************************************************
 * 初始化日志系统
 ***************************************************************/
void LogManager::initialize(const LogConfig &config)
{
    QMutexLocker locker(&m_logMutex);
    
    m_config = config;
    
    // 创建日志目录
    if (m_config.enableFile)
    {
        QDir logDir(m_config.logDir);
        if (!logDir.exists())
        {
            logDir.mkpath(".");
            qInfo() << "[LogManager] 日志目录创建:" << m_config.logDir;
        }
    }
    
    // 启动刷新定时器
    m_flushTimer->start();
    
    qInfo() << "[LogManager] 日志系统初始化完成";
    qInfo() << "  日志目录:" << m_config.logDir;
    qInfo() << "  控制台输出:" << (m_config.enableConsole ? "启用" : "禁用");
    qInfo() << "  文件输出:" << (m_config.enableFile ? "启用" : "禁用");
    qInfo() << "  最小级别:" << getLevelString(m_config.minLevel);
}

/***************************************************************
 * 记录日志
 ***************************************************************/
void LogManager::log(const QString &module, LogLevel level, const QString &message)
{
    // 检查日志级别
    LogLevel minLevel = m_config.minLevel;
    if (m_moduleLevels.contains(module))
    {
        minLevel = m_moduleLevels[module];
    }
    
    if (level < minLevel)
    {
        return;  // 级别太低，跳过
    }
    
    // 格式化消息
    QString formattedMessage = formatMessage(module, level, message);
    
    // 加锁保护
    QMutexLocker locker(&m_logMutex);
    
    // 输出到控制台
    if (m_config.enableConsole)
    {
        writeToConsole(level, formattedMessage);
    }
    
    // 输出到文件
    if (m_config.enableFile)
    {
        writeToFile(module, formattedMessage);
    }
    
    // 统计
    m_logCounts[module]++;
    
    // 发送信号
    emit newLog(module, static_cast<int>(level), message);
}

/***************************************************************
 * 设置模块日志级别
 ***************************************************************/
void LogManager::setModuleLevel(const QString &module, LogLevel level)
{
    QMutexLocker locker(&m_logMutex);
    m_moduleLevels[module] = level;
}

/***************************************************************
 * 刷新所有日志缓冲
 ***************************************************************/
void LogManager::flush()
{
    QMutexLocker locker(&m_logMutex);
    
    for (auto it = m_moduleFiles.begin(); it != m_moduleFiles.end(); ++it)
    {
        LogFileInfo *info = it.value();
        if (info && info->stream)
        {
            info->stream->flush();
        }
        if (info && info->file)
        {
            info->file->flush();
        }
    }
}

/***************************************************************
 * 获取日志统计信息
 ***************************************************************/
QString LogManager::getStatistics() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_logMutex));
    
    QString stats;
    stats += "========================================\n";
    stats += "  日志统计信息\n";
    stats += "========================================\n";
    
    for (auto it = m_logCounts.begin(); it != m_logCounts.end(); ++it)
    {
        stats += QString("  %1: %2 条\n").arg(it.key(), -20).arg(it.value());
    }
    
    stats += "========================================\n";
    return stats;
}

/***************************************************************
 * 清理旧日志文件
 ***************************************************************/
void LogManager::cleanOldLogs()
{
    QMutexLocker locker(&m_logMutex);
    
    QDir logDir(m_config.logDir);
    if (!logDir.exists())
    {
        return;
    }
    
    // 获取所有日志文件
    QStringList filters;
    filters << "*.log";
    QFileInfoList fileList = logDir.entryInfoList(filters, QDir::Files, QDir::Time);
    
    // 删除超过最大数量的文件
    int deleteCount = fileList.size() - m_config.maxFileCount;
    if (deleteCount > 0)
    {
        for (int i = fileList.size() - 1; i >= fileList.size() - deleteCount; --i)
        {
            QFile::remove(fileList[i].filePath());
        }
    }
}

/***************************************************************
 * 格式化日志消息
 ***************************************************************/
QString LogManager::formatMessage(const QString &module, LogLevel level, const QString &message)
{
    QString formatted;
    
    // 添加时间戳
    if (m_config.enableTimestamp)
    {
        formatted += "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") + "] ";
    }
    
    // 添加级别
    formatted += "[" + getLevelString(level) + "] ";
    
    // 添加模块名
    formatted += "[" + module + "] ";
    
    // 添加线程ID
    if (m_config.enableThreadId)
    {
        formatted += QString("[T:%1] ").arg((quintptr)QThread::currentThreadId(), 0, 16);
    }
    
    // 添加消息
    formatted += message;
    
    return formatted;
}

/***************************************************************
 * 获取日志级别字符串
 ***************************************************************/
QString LogManager::getLevelString(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO ";
        case LogLevel::Warning:  return "WARN ";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT ";
        default:                 return "UNKNOWN";
    }
}

/***************************************************************
 * 获取或创建模块日志文件
 ***************************************************************/
LogFileInfo* LogManager::getModuleFile(const QString &module)
{
    // 如果已存在，直接返回
    if (m_moduleFiles.contains(module))
    {
        return m_moduleFiles[module];
    }
    
    // 创建新文件
    LogFileInfo *info = new LogFileInfo();
    
    QString filename = QString("%1/%2.log").arg(m_config.logDir).arg(module.toLower());
    info->file = new QFile(filename);
    
    if (info->file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text | QIODevice::Unbuffered))
    {
        info->stream = new QTextStream(info->file);
        info->stream->setCodec("UTF-8");  // 设置UTF-8编码
        info->stream->setAutoDetectUnicode(true);
        info->currentSize = info->file->size();
        m_moduleFiles[module] = info;
        return info;
    }
    else
    {
        qWarning() << "[LogManager] 无法打开日志文件:" << filename;
        delete info->file;
        delete info;
        return nullptr;
    }
}

/***************************************************************
 * 检查并轮转日志文件
 ***************************************************************/
void LogManager::rotateLogFile(const QString &module, LogFileInfo *info)
{
    if (!info || !info->file)
    {
        return;
    }
    
    // 检查文件大小
    if (info->currentSize < m_config.maxFileSize * 1024)
    {
        return;  // 未超过限制
    }
    
    // 关闭当前文件
    if (info->stream)
    {
        delete info->stream;
        info->stream = nullptr;
    }
    info->file->close();
    
    // 重命名当前文件
    QString oldName = info->file->fileName();
    QString newName = QString("%1.%2")
                      .arg(oldName)
                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QFile::rename(oldName, newName);
    
    // 重新打开文件
    if (info->file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text | QIODevice::Unbuffered))
    {
        info->stream = new QTextStream(info->file);
        info->stream->setCodec("UTF-8");  // 设置UTF-8编码
        info->stream->setAutoDetectUnicode(true);
        info->currentSize = 0;
        info->fileIndex++;
    }
    
    // 清理旧文件
    cleanOldLogs();
}

/***************************************************************
 * 写入到控制台
 ***************************************************************/
void LogManager::writeToConsole(LogLevel level, const QString &formattedMessage)
{
    // 根据级别选择不同的输出流
    if (level >= LogLevel::Warning)
    {
        qWarning().noquote() << formattedMessage;
    }
    else
    {
        qInfo().noquote() << formattedMessage;
    }
}

/***************************************************************
 * 写入到文件
 ***************************************************************/
void LogManager::writeToFile(const QString &module, const QString &formattedMessage)
{
    LogFileInfo *info = getModuleFile(module);
    if (!info || !info->stream)
    {
        return;
    }
    
    // 写入文件
    *info->stream << formattedMessage << "\n";
    
    // 立即刷新到磁盘（实现实时 tail -f）
    info->stream->flush();
    if (info->file) {
        info->file->flush();
    }
    
    info->currentSize += formattedMessage.length() + 1;
    
    // 检查是否需要轮转
    rotateLogFile(module, info);
}

/***************************************************************
 * 定时刷新日志
 ***************************************************************/
void LogManager::onFlushTimer()
{
    flush();
}

