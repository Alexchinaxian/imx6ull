/***************************************************************
 * 文件名: main.cpp
 * 项目名: IMX6ULL嵌入式Linux驱动框架
 * 功能: 主程序入口，负责系统初始化和驱动管理
 * 
 * 架构说明:
 *   本项目采用基于Qt的面向对象驱动架构，通过Linux标准
 *   sysfs接口控制硬件，具有良好的可移植性和可维护性。
 * 
 * 主要特性:
 *   1. 多线程驱动架构 - 温度监控在独立线程运行
 *   2. 信号槽机制 - 松耦合的事件驱动架构
 *   3. 优雅退出机制 - 响应SIGINT/SIGTERM信号
 *   4. 硬件接口扫描 - 自动发现系统硬件资源
 *   5. sysfs接口 - 安全可靠的用户空间驱动
 * 
 * 支持的硬件:
 *   - GPIO（通用输入输出）
 *   - PWM（脉宽调制）
 *   - LED（发光二极管）
 *   - Temperature（温度传感器）
 * 
 * 作者: Alex
 * 日期: 2025
 * 版本: V0.1.0
 ***************************************************************/

#include <QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <QTextCodec>
#ifdef Q_OS_LINUX
    #include <sys/signal.h>  // Linux信号处理
#endif

// 核心系统
#include "core/ServiceManager.h"
#include "core/LogManager.h"
#include "core/SystemBeep.h"

// ==========================================
// 全局常量和变量
// ==========================================

// 软件版本号（主版本.次版本.修订版本）
const uint8_t SoftwareVersion[3] = {1, 2, 0};       // V1.2.0

// 全局服务管理器指针（用于信号处理器访问）
static ServiceManager *g_serviceManager = nullptr;

// 全局系统蜂鸣器指针（用于信号处理器访问）
static SystemBeep *g_systemBeep = nullptr;

// ==========================================
// 信号处理机制
// ==========================================

/**
 * @brief 信号处理函数
 * @param sig 接收到的信号编号
 * 
 * 功能说明:
 *   处理Linux系统信号，实现优雅退出机制
 * 
 * 处理的信号:
 *   - SIGINT (2)  : Ctrl+C中断信号
 *   - SIGTERM (15): 终止信号（kill命令默认信号）
 *   - SIGPIPE     : 管道破裂信号（被忽略）
 * 
 * 退出流程:
 *   1. 停止所有正在运行的驱动（温度监控等）
 *   2. 释放硬件资源（GPIO、PWM等）
 *   3. 调用QCoreApplication::quit()退出事件循环
 * 
 * 注意事项:
 *   在信号处理器中只能调用异步安全的函数
 *   使用QMetaObject::invokeMethod以线程安全的方式调用槽函数
 */
void signalHandler(int sig)
{
    qWarning() << "Received signal:" << sig;
    
    switch (sig) {
        case SIGINT:   // Ctrl+C
        case SIGTERM:  // kill命令
            qInfo() << "Shutting down gracefully...";
            
            // 播放关机提示音
            if (g_systemBeep) {
                g_systemBeep->playShutdown();
            }
            
            // 停止所有服务（使用服务管理器）
            // 服务管理器会自动停止所有服务，包括：
            // - 温度监控服务（停止温度线程）
            // - Modbus从站服务（断开串口连接）
            if (g_serviceManager) {
                qInfo() << "Stopping all services...";
                g_serviceManager->SvrStop();
            }
            
            // 退出Qt事件循环
            QCoreApplication::quit();
            break;
            
        default:
            qCritical() << "Unexpected signal:" << sig;
            QCoreApplication::exit(1);  // 异常退出
            break;
    }
}

/**
 * @brief 注册信号处理器
 * 
 * 功能说明:
 *   为特定的Linux信号注册处理函数
 * 
 * 注册的信号:
 *   - SIGINT  -> signalHandler (捕获Ctrl+C)
 *   - SIGTERM -> signalHandler (捕获kill命令)
 *   - SIGPIPE -> SIG_IGN (忽略管道破裂信号)
 * 
 * 为什么需要忽略SIGPIPE:
 *   当向已关闭的socket或管道写入数据时，会产生SIGPIPE信号
 *   默认行为是终止程序，忽略它可以避免程序意外退出
 */
void registerSignalHandlers()
{
#ifdef Q_OS_LINUX
    signal(SIGINT, signalHandler);   // 捕获Ctrl+C
    signal(SIGTERM, signalHandler);  // 捕获终止信号
    signal(SIGPIPE, SIG_IGN);        // 忽略管道信号
#endif
}

// ==========================================
// 系统初始化函数
// ==========================================

/**
 * @brief 初始化系统配置（极简版）
 * @return true=成功, false=失败
 */
bool initSystemConfig()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    
    LOG_INFO("System", QString("软件版本: V%1.%2.%3 | 编译: %4 %5 | Qt: %6")
        .arg(SoftwareVersion[0])
        .arg(SoftwareVersion[1])
        .arg(SoftwareVersion[2])
        .arg(__DATE__)
        .arg(__TIME__)
        .arg(QT_VERSION_STR));
    
    return true;
}

/**
 * @brief 初始化服务管理器
 * @return true=成功, false=失败
 */
bool initServiceManager()
{
    g_serviceManager = ServiceManager::GetInstance();
    if (!g_serviceManager)
    {
        LOG_CRITICAL("System", "获取服务管理器实例失败");
        return false;
    }
    
    // 创建、初始化、启动所有服务
    if (!g_serviceManager->ManagerInitLoad() ||
        !g_serviceManager->SvrInit() ||
        !g_serviceManager->SvrStart())
    {
        LOG_CRITICAL("System", "服务管理器启动失败");
        return false;
    }
    
    LOG_INFO("System", QString("✓ 服务管理器启动成功 (%1个服务)")
        .arg(g_serviceManager->GetServiceCount()));
    
    return true;
}

// ==========================================
// 主程序入口
// ==========================================

/**
 * @brief 主函数 - 程序入口（极简版）
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码（0=正常退出，其他值=异常退出）
 * 
 * 程序启动流程（极简化）：
 *   1. 创建Qt应用程序对象
 *   2. 初始化日志系统
 *   3. 注册信号处理器
 *   4. 初始化系统配置
 *   5. 启动服务管理器（自动管理所有服务）
 *   6. 播放启动提示音
 *   7. 进入Qt事件循环
 * 
 * 设计理念：
 *   main.cpp 只负责基础设施初始化和服务容器启动
 *   所有业务逻辑（硬件扫描、温度监控、天气获取等）由服务自己管理
 */
int main(int argc, char *argv[])
{
    // ==========================================
    // 创建Qt应用程序对象
    // ==========================================
    QCoreApplication app(argc, argv);
    
    // 设置全局编码为UTF-8（解决中文乱码）
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
    // 设置应用程序名称（用于日志、配置文件等）
    app.setApplicationName("QtImx6ullBackend");
    
    // 设置应用程序版本号
    app.setApplicationVersion(QString("V%1.%2.%3")
                              .arg(SoftwareVersion[0])
                              .arg(SoftwareVersion[1])
                              .arg(SoftwareVersion[2]));
    
    // ==========================================
    // 步骤0：初始化日志系统
    // ==========================================
    LogConfig logConfig;
    logConfig.enableConsole = true;
    logConfig.enableFile = true;
    logConfig.logDir = "/tmp/imx6ull_logs";
    logConfig.maxFileSize = 1024;
    logConfig.maxFileCount = 10;
    logConfig.minLevel = LogLevel::Info;
    logConfig.enableTimestamp = true;
    logConfig.enableThreadId = true;
    LogManager::getInstance()->initialize(logConfig);
    
    // ==========================================
    // 步骤1：注册信号处理器
    // ==========================================
    registerSignalHandlers();
    
    // ==========================================
    // 步骤2：初始化系统配置
    // ==========================================
    if (!initSystemConfig()) {
        LOG_CRITICAL("System", "系统配置初始化失败");
        return -1;
    }
    
    // ==========================================
    // 步骤3：启动服务管理器
    // ==========================================
    if (!initServiceManager()) {
        LOG_CRITICAL("System", "服务管理器启动失败");
        return -1;
    }
    
    // ==========================================
    // 步骤4：系统就绪
    // ==========================================
    LOG_INFO("System", "========================================");
    LOG_INFO("System", "系统启动完成 - 按 Ctrl+C 退出");
    LOG_INFO("System", QString("服务总数: %1 | 日志目录: %2")
        .arg(g_serviceManager->GetServiceCount())
        .arg(logConfig.logDir));
    LOG_INFO("System", "========================================");
    
    // 播放启动完成提示音
    g_systemBeep = SystemBeep::getInstance();
    if (g_systemBeep && g_systemBeep->isAvailable())
    {
        g_systemBeep->playInitComplete();
    }
    
    // ==========================================
    // 进入事件循环
    // ==========================================
    // 启动Qt事件循环，处理信号、槽、定时器等事件
    // 程序将阻塞在这里，直到调用quit()或exit()
    return app.exec();
}
