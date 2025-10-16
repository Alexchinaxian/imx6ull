/***************************************************************
 * Copyright: Alex
 * FileName: WeatherService.cpp
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 天气服务实现
 ***************************************************************/

#include "services/weather/WeatherService.h"
#include "core/LogManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QProcess>

/***************************************************************
 * 构造函数
 ***************************************************************/
WeatherService::WeatherService(int32_t svr_id, int32_t svr_type, QObject *parent)
    : ISysSvrInterface(svr_id, svr_type, parent)
    , m_location("陕西省西安市雁塔区中建群贤汇")
    , m_apiKey("")
    , m_fetchIntervalMinutes(5)      // 每5分钟获取一次
    , m_publishIntervalSeconds(10)   // 每10秒输出一次
    , m_pSerial(nullptr)
{
    // 创建获取天气定时器（每5分钟）
    m_fetchTimer = new QTimer(this);
    m_fetchTimer->setInterval(m_fetchIntervalMinutes * 60 * 1000);
    
    // 创建发布天气定时器（每10秒）
    m_publishTimer = new QTimer(this);
    m_publishTimer->setInterval(m_publishIntervalSeconds * 1000);
    
    // 创建网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    // 连接定时器信号槽
    connect(m_fetchTimer, &QTimer::timeout, this, &WeatherService::onFetchTimer);
    connect(m_publishTimer, &QTimer::timeout, this, &WeatherService::onPublishTimer);
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &WeatherService::onNetworkReplyFinished);
    
    // 连接内部信号槽 - 自动打印天气信息
    connect(this, &WeatherService::weatherUpdated, this, &WeatherService::onWeatherUpdated);
    connect(this, &WeatherService::updateFailed, this, &WeatherService::onUpdateFailed);
    
    qInfo() << "[WeatherService] 天气服务创建";
}

/***************************************************************
 * 析构函数
 ***************************************************************/
WeatherService::~WeatherService()
{
    qInfo() << "[WeatherService] 天气服务销毁";
    SvrStop();
}

/***************************************************************
 * 服务初始化
 ***************************************************************/
bool WeatherService::SvrInit()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  天气服务初始化";
    qInfo() << "========================================";
    qInfo() << "📍 监测位置: " << m_location;
    qInfo() << "⏰ 获取间隔: " << m_fetchIntervalMinutes << " 分钟";
    qInfo() << "📢 输出间隔: " << m_publishIntervalSeconds << " 秒";
    qInfo() << "🔌 串口输出: " << (m_pSerial ? "启用" : "禁用");
    qInfo() << "🔑 API密钥: " << (m_apiKey.isEmpty() ? "未配置（模拟模式）" : "已配置");
    qInfo() << "========================================";
    qInfo() << "[WeatherService] ✓ 天气服务初始化成功";
    qInfo() << "";
    
    return true;
}

/***************************************************************
 * 服务启动
 ***************************************************************/
bool WeatherService::SvrStart()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  启动天气服务";
    qInfo() << "========================================";
    
    // 立即获取一次天气
    qInfo() << "🌐 执行首次天气获取...";
    performWeatherFetch();
    
    // 启动获取定时器（每5分钟获取新数据）
    m_fetchTimer->start();
    qInfo() << "⏱️  定时获取已启用（间隔：" << m_fetchIntervalMinutes << " 分钟）";
    
    // 启动发布定时器（每10秒输出数据）
    m_publishTimer->start();
    qInfo() << "📡 定时输出已启用（间隔：" << m_publishIntervalSeconds << " 秒）";
    
    qInfo() << "========================================";
    qInfo() << "[WeatherService] ✓ 天气服务启动成功";
    qInfo() << "";
    
    return true;
}

/***************************************************************
 * 服务停止
 ***************************************************************/
bool WeatherService::SvrStop()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  停止天气服务";
    qInfo() << "========================================";
    
    m_fetchTimer->stop();
    qInfo() << "⏱️  定时获取已停止";
    
    m_publishTimer->stop();
    qInfo() << "📡 定时输出已停止";
    
    // 取消所有网络请求
    if (m_networkManager) {
        m_networkManager->deleteLater();
        m_networkManager = nullptr;
        qInfo() << "🌐 网络管理器已清理";
    }
    
    qInfo() << "========================================";
    qInfo() << "[WeatherService] ✓ 天气服务已停止";
    qInfo() << "";
    
    return true;
}

/***************************************************************
 * 设置串口驱动
 ***************************************************************/
void WeatherService::setSerialDriver(DriverSerial *serial)
{
    m_pSerial = serial;
    qInfo() << "[WeatherService] 串口驱动已设置";
}

/***************************************************************
 * 设置城市位置
 ***************************************************************/
void WeatherService::setLocation(const QString &location)
{
    m_location = location;
    qInfo() << "[WeatherService] 位置设置为:" << m_location;
}

/***************************************************************
 * 设置API密钥
 ***************************************************************/
void WeatherService::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
    
    if (apiKey.isEmpty()) {
        qWarning() << "[WeatherService] ⚠️ API密钥为空";
    } else {
        qInfo() << "[WeatherService] ✓ API密钥已设置";
        qInfo() << "  密钥长度: " << apiKey.length() << " 字符";
        qInfo() << "  密钥预览: " << apiKey.left(8) << "..." << apiKey.right(8);
        
        // 验证密钥格式（和风天气API密钥通常是32位十六进制字符串）
        if (apiKey.length() == 32) {
            bool isHex = true;
            for (const QChar &c : apiKey) {
                if (!c.isLetterOrNumber()) {
                    isHex = false;
                    break;
                }
            }
            if (isHex) {
                qInfo() << "  格式验证: ✓ 符合和风天气API密钥格式";
            } else {
                qWarning() << "  格式验证: ⚠️ 可能不是标准的API密钥格式";
            }
        } else {
            qWarning() << "  格式验证: ⚠️ 密钥长度异常（标准为32字符）";
        }
    }
}

/***************************************************************
 * 设置更新间隔
 ***************************************************************/
void WeatherService::setUpdateInterval(int minutes)
{
    m_fetchIntervalMinutes = minutes;
    m_fetchTimer->setInterval(minutes * 60 * 1000);
    qInfo() << "[WeatherService] 获取间隔设置为:" << minutes << "分钟";
}

/***************************************************************
 * 立即获取天气
 ***************************************************************/
void WeatherService::fetchWeatherNow()
{
    qInfo() << "[WeatherService] 手动触发天气获取...";
    performWeatherFetch();
}

/***************************************************************
 * 定时获取槽函数（获取新天气数据）
 ***************************************************************/
void WeatherService::onFetchTimer()
{
    qInfo() << "";
    qInfo() << "╔════════════════════════════════════════╗";
    qInfo() << "║       定时获取天气数据触发             ║";
    qInfo() << "╚════════════════════════════════════════╝";
    performWeatherFetch();
}

/***************************************************************
 * 定时输出槽函数（输出已有天气数据）
 ***************************************************************/
void WeatherService::onPublishTimer()
{
    // 如果有有效的天气数据，则输出
    if (m_currentWeather.isValid) {
        LOG_DEBUG("Weather", "📢 定时输出天气信息...");
        
        // 简化版输出到日志（不要太详细，避免日志过大）
        LOG_INFO("Weather", QString("当前天气: %1 | %2°C | 湿度%3% | %4 %5")
            .arg(m_currentWeather.location)
            .arg(m_currentWeather.temperature)
            .arg(m_currentWeather.humidity)
            .arg(m_currentWeather.weather)
            .arg(m_currentWeather.windDirection));
        
        // 详细信息还是输出到标准输出（供实时查看）
        publishToLog(m_currentWeather);
        publishToSerial(m_currentWeather);
    } else {
        LOG_DEBUG("Weather", "⚠️ 天气数据无效，跳过输出");
    }
}

/***************************************************************
 * 执行天气获取
 ***************************************************************/
void WeatherService::performWeatherFetch()
{
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  天气数据获取";
    qInfo() << "========================================";
    qInfo() << "📍 目标位置: " << m_location;
    qInfo() << "⏰ 获取时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    // ========================================
    // 方式1: 使用真实天气API（需要API密钥）
    // ========================================
    if (!m_apiKey.isEmpty()) {
        qInfo() << "🔑 使用和风天气API获取真实数据";
        
        // 和风天气API - 城市查询
        // 西安的城市代码：101110101
        QString cityCode = "101110101";  // 西安市
        
        // 构建API请求URL（使用个性化域名）
        QString url = QString("https://ng3md8uy6u.re.qweatherapi.com/v7/weather/now?location=%1&key=%2")
                         .arg(cityCode).arg(m_apiKey);
        
        qInfo() << "🌐 API端点: https://ng3md8uy6u.re.qweatherapi.com/v7/weather/now";
        qInfo() << "🏙️  城市代码: " << cityCode;
        qInfo() << "🔑 API密钥: " << m_apiKey.left(8) << "..." << m_apiKey.right(8);
        qInfo() << "========================================";
        
        // 由于Qt的SSL支持问题，使用curl命令获取数据
        QProcess *curlProcess = new QProcess(this);
        
        // 连接完成信号
        connect(curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, curlProcess](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                QByteArray data = curlProcess->readAllStandardOutput();
                qInfo() << "✓ 收到API响应";
                qInfo() << "  响应大小: " << data.size() << " 字节";
                qInfo() << "  响应内容: " << data.left(200) << (data.size() > 200 ? "..." : "");
                
                // 解析数据
                WeatherData weather = parseWeatherData(data);
                if (weather.isValid) {
                    m_currentWeather = weather;
                    
                    // 首次获取时立即发布，后续由定时器发布
                    static bool firstFetch = true;
                    if (firstFetch) {
                        publishToLog(weather);
                        publishToSerial(weather);
                        firstFetch = false;
                    }
                    
                    emit weatherUpdated(weather);
                    qInfo() << "[WeatherService] ✓ 天气信息更新成功";
                } else {
                    qWarning() << "[WeatherService] ✗ 天气数据解析失败";
                    
                    // 标记数据为无效
                    m_currentWeather.isValid = false;
                    m_currentWeather.location = m_location;
                    m_currentWeather.dataSource = "解析失败";
                    m_currentWeather.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    
                    emit updateFailed("数据解析失败");
                }
            } else {
                qWarning() << "";
                qWarning() << "========================================";
                qWarning() << "  curl命令执行失败";
                qWarning() << "========================================";
                qWarning() << "✗ 退出码: " << exitCode;
                QByteArray errorData = curlProcess->readAllStandardError();
                if (!errorData.isEmpty()) {
                    qWarning() << "  错误信息: " << errorData;
                }
                qWarning() << "";
                qWarning() << "  可能的原因：";
                qWarning() << "  1. curl命令未安装";
                qWarning() << "  2. 网络连接失败";
                qWarning() << "  3. 服务器无响应";
                qWarning() << "  4. SSL证书验证失败";
                qWarning() << "========================================";
                qWarning() << "[WeatherService] ✗ 天气数据获取失败";
                qWarning() << "";
                
                // 标记数据为无效
                m_currentWeather.isValid = false;
                m_currentWeather.location = m_location;
                m_currentWeather.dataSource = "获取失败";
                m_currentWeather.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                
                emit updateFailed("curl执行失败");
            }
            curlProcess->deleteLater();
        });
        
        // 执行curl命令（-k跳过SSL验证，--compressed解压gzip）
        qInfo() << "📡 发送API请求（使用curl）...";
        curlProcess->start("curl", QStringList() << "-k" << "--compressed" << "-s" << url);
        return;
    }
    
    // ========================================
    // 方式2: 无API密钥时 - 不获取数据
    // ========================================
    qWarning() << "✗ 未配置API密钥，无法获取天气数据";
    qInfo() << "💡 提示：在ServiceManager.cpp中配置API密钥以获取真实天气";
    qInfo() << "   1. 访问 https://dev.qweather.com/";
    qInfo() << "   2. 注册并创建项目获取API密钥";
    qInfo() << "   3. 在ServiceManager.cpp中调用 pWeatherSvr->setApiKey(\"YOUR_KEY\")";
    qInfo() << "========================================";
    qInfo() << "[WeatherService] ⚠️ 天气数据获取失败：未配置API密钥";
    qInfo() << "";
    
    // 标记当前天气数据为无效
    m_currentWeather.isValid = false;
    m_currentWeather.location = m_location;
    m_currentWeather.dataSource = "未获取";
    m_currentWeather.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    emit updateFailed("未配置API密钥");
}

/***************************************************************
 * HTTP请求完成槽函数
 ***************************************************************/
void WeatherService::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QString error = reply->errorString();
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        qWarning() << "";
        qWarning() << "========================================";
        qWarning() << "  网络请求失败";
        qWarning() << "========================================";
        qWarning() << "✗ 错误信息: " << error;
        qWarning() << "  HTTP状态码: " << httpStatus;
        
        if (httpStatus == 401 || httpStatus == 403) {
            qWarning() << "";
            qWarning() << "【API密钥错误】";
            qWarning() << "  可能的原因：";
            qWarning() << "  1. API密钥无效或已过期";
            qWarning() << "  2. API密钥格式错误";
            qWarning() << "  3. 超出API调用配额";
            qWarning() << "";
            qWarning() << "  解决方法：";
            qWarning() << "  1. 登录 https://console.qweather.com/";
            qWarning() << "  2. 检查项目状态和配额";
            qWarning() << "  3. 复制正确的32位KEY字符串";
            qWarning() << "  4. 在ServiceManager.cpp中更新密钥";
        } else if (httpStatus == 429) {
            qWarning() << "";
            qWarning() << "【请求频率过高】";
            qWarning() << "  免费版API限制：每天1000次，2次/秒";
            qWarning() << "  建议：增加获取间隔时间";
        } else if (httpStatus == 0) {
            qWarning() << "";
            qWarning() << "【网络连接失败】";
            qWarning() << "  请检查：";
            qWarning() << "  1. 网络连接是否正常";
            qWarning() << "  2. DNS解析是否正常";
            qWarning() << "  3. 防火墙设置";
        }
        
        qWarning() << "========================================";
        qWarning() << "[WeatherService] ✗ 天气数据获取失败";
        qWarning() << "";
        
        // 标记数据为无效
        m_currentWeather.isValid = false;
        m_currentWeather.location = m_location;
        m_currentWeather.dataSource = "获取失败";
        m_currentWeather.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        emit updateFailed(error);
        reply->deleteLater();
        return;
    }
    
    // 读取响应数据
    QByteArray data = reply->readAll();
    reply->deleteLater();
    
    qInfo() << "[WeatherService] 收到API响应:" << data.size() << "字节";
    
    // 解析天气数据
    WeatherData weather = parseWeatherData(data);
    
    if (weather.isValid) {
        m_currentWeather = weather;
        
        // 发布天气信息
        publishToLog(weather);
        publishToSerial(weather);
        
        // 发送信号
        emit weatherUpdated(weather);
        
        qInfo() << "[WeatherService] ✓ 天气信息更新成功";
    } else {
        qWarning() << "[WeatherService] ✗ 天气数据解析失败";
        
        // 标记数据为无效
        m_currentWeather.isValid = false;
        m_currentWeather.location = m_location;
        m_currentWeather.dataSource = "解析失败";
        m_currentWeather.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        emit updateFailed("数据解析失败");
    }
}

/***************************************************************
 * 解析天气数据
 ***************************************************************/
WeatherData WeatherService::parseWeatherData(const QByteArray &data)
{
    WeatherData weather;
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "  解析API响应数据";
    qInfo() << "========================================";
    
    // 解析JSON数据（和风天气API格式）
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "✗ JSON解析失败";
        qWarning() << "  原始数据: " << data;
        qInfo() << "========================================";
        return weather;
    }
    
    QJsonObject root = doc.object();
    qInfo() << "✓ JSON解析成功";
    
    // 检查API响应状态
    QString code = root["code"].toString();
    QString updateTimeRaw = root["updateTime"].toString();
    
    qInfo() << "📋 API响应信息:";
    qInfo() << "  响应码: " << code;
    qInfo() << "  更新时间: " << updateTimeRaw;
    
    if (code != "200") {
        qWarning() << "✗ API返回错误";
        qWarning() << "  错误码: " << code;
        
        // 尝试读取错误信息
        if (root.contains("error")) {
            qWarning() << "  错误信息: " << root["error"].toString();
        }
        
        qInfo() << "========================================";
        return weather;
    }
    
    // 和风天气API JSON格式：
    // {
    //   "code": "200",
    //   "updateTime": "2025-10-15T15:00+08:00",
    //   "fxLink": "...",
    //   "now": {
    //     "obsTime": "2025-10-15T15:00+08:00",
    //     "temp": "18",
    //     "feelsLike": "16",
    //     "icon": "100",
    //     "text": "晴",
    //     "wind360": "135",
    //     "windDir": "东南风",
    //     "windScale": "3",
    //     "windSpeed": "15",
    //     "humidity": "45",
    //     "precip": "0.0",
    //     "pressure": "1012",
    //     "vis": "30",
    //     "cloud": "10",
    //     "dew": "8"
    //   }
    // }
    
    QJsonObject now = root["now"].toObject();
    if (now.isEmpty()) {
        qWarning() << "✗ API响应中没有'now'字段";
        qInfo() << "========================================";
        return weather;
    }
    
    qInfo() << "";
    qInfo() << "📊 解析天气字段:";
    
    // 基本信息
    weather.location = m_location;
    weather.dataSource = "和风天气API";
    
    // 天气状况
    weather.weather = now["text"].toString();
    weather.weatherCode = now["icon"].toString();
    qInfo() << "  天气状况: " << weather.weather << " (代码:" << weather.weatherCode << ")";
    
    // 温度
    weather.temperature = now["temp"].toString().toFloat();
    weather.feelsLike = now["feelsLike"].toString().toFloat();
    qInfo() << "  温度: " << weather.temperature << "°C (体感:" << weather.feelsLike << "°C)";
    
    // 风
    weather.windDirection = now["windDir"].toString();
    weather.windSpeed = now["windSpeed"].toString().toFloat();
    weather.windScale = now["windScale"].toString() + "级";
    qInfo() << "  风: " << weather.windDirection << " " << weather.windSpeed << " km/h (" << weather.windScale << ")";
    
    // 湿度和降水
    weather.humidity = now["humidity"].toString().toFloat();
    weather.precipitation = now["precip"].toString().toFloat();
    qInfo() << "  湿度: " << weather.humidity << "%";
    qInfo() << "  降水量: " << weather.precipitation << " mm";
    
    // 气压和能见度
    weather.pressure = now["pressure"].toString().toFloat();
    weather.visibility = now["vis"].toString().toFloat();
    qInfo() << "  气压: " << weather.pressure << " hPa";
    qInfo() << "  能见度: " << weather.visibility << " km";
    
    // 云量
    weather.cloudCover = now["cloud"].toString().toInt();
    qInfo() << "  云量: " << weather.cloudCover << "%";
    
    // 空气质量（和风天气API没有直接提供AQI，需要调用空气质量API）
    weather.airQuality = 0;  // 0表示未获取
    
    // 时间信息
    weather.obsTime = now["obsTime"].toString();
    weather.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qInfo() << "  观测时间: " << weather.obsTime;
    qInfo() << "  本地更新: " << weather.updateTime;
    
    weather.isValid = true;
    
    qInfo() << "";
    qInfo() << "========================================";
    qInfo() << "[WeatherService] ✓ API数据解析成功";
    qInfo() << "========================================";
    qInfo() << "";
    
    return weather;
}

/***************************************************************
 * 发布天气信息到日志
 ***************************************************************/
void WeatherService::publishToLog(const WeatherData &weather)
{
    qInfo() << "";
    qInfo() << "╔════════════════════════════════════════╗";
    qInfo() << "║          天气信息实时播报              ║";
    qInfo() << "╚════════════════════════════════════════╝";
    qInfo() << "";
    
    // 基本信息
    qInfo() << "【基本信息】";
    qInfo() << "📍 位置: " << weather.location;
    qInfo() << "☁️  天气: " << weather.weather;
    if (!weather.weatherCode.isEmpty()) {
        qInfo() << "🔢 天气代码: " << weather.weatherCode;
    }
    qInfo() << "📊 数据源: " << weather.dataSource;
    qInfo() << "";
    
    // 温度信息
    qInfo() << "【温度信息】";
    qInfo() << "🌡️  当前温度: " << QString::number(weather.temperature, 'f', 1) << "°C";
    if (weather.feelsLike != 0) {
        qInfo() << "🤚 体感温度: " << QString::number(weather.feelsLike, 'f', 1) << "°C";
        float diff = weather.temperature - weather.feelsLike;
        if (diff > 0) {
            qInfo() << "   (比实际温度低" << QString::number(diff, 'f', 1) << "°C)";
        } else if (diff < 0) {
            qInfo() << "   (比实际温度高" << QString::number(-diff, 'f', 1) << "°C)";
        }
    }
    qInfo() << "";
    
    // 风力信息
    qInfo() << "【风力信息】";
    qInfo() << "🧭 风向: " << weather.windDirection;
    qInfo() << "💨 风速: " << QString::number(weather.windSpeed, 'f', 1) << " km/h";
    if (!weather.windScale.isEmpty()) {
        qInfo() << "📏 风力等级: " << weather.windScale;
    }
    qInfo() << "";
    
    // 湿度和降水
    qInfo() << "【湿度与降水】";
    qInfo() << "💧 相对湿度: " << QString::number(weather.humidity, 'f', 0) << "%";
    
    QString humidityLevel;
    if (weather.humidity < 30) humidityLevel = "干燥";
    else if (weather.humidity < 60) humidityLevel = "舒适";
    else if (weather.humidity < 80) humidityLevel = "较湿";
    else humidityLevel = "潮湿";
    qInfo() << "   等级: " << humidityLevel;
    
    if (weather.precipitation > 0) {
        qInfo() << "🌧️  降水量: " << QString::number(weather.precipitation, 'f', 1) << " mm";
    } else {
        qInfo() << "🌧️  降水量: 无降水";
    }
    qInfo() << "";
    
    // 气压和能见度
    qInfo() << "【大气条件】";
    if (weather.pressure > 0) {
        qInfo() << "🔽 气压: " << QString::number(weather.pressure, 'f', 1) << " hPa";
        
        QString pressureLevel;
        if (weather.pressure < 1000) pressureLevel = "低气压";
        else if (weather.pressure < 1020) pressureLevel = "正常";
        else pressureLevel = "高气压";
        qInfo() << "   状态: " << pressureLevel;
    }
    
    if (weather.visibility > 0) {
        qInfo() << "👁️  能见度: " << QString::number(weather.visibility, 'f', 1) << " km";
        
        QString visibilityLevel;
        if (weather.visibility < 1) visibilityLevel = "很差";
        else if (weather.visibility < 5) visibilityLevel = "较差";
        else if (weather.visibility < 10) visibilityLevel = "一般";
        else if (weather.visibility < 20) visibilityLevel = "良好";
        else visibilityLevel = "极好";
        qInfo() << "   等级: " << visibilityLevel;
    }
    
    if (weather.cloudCover >= 0) {
        qInfo() << "☁️  云量: " << weather.cloudCover << "%";
        
        QString cloudLevel;
        if (weather.cloudCover < 10) cloudLevel = "晴朗";
        else if (weather.cloudCover < 30) cloudLevel = "少云";
        else if (weather.cloudCover < 60) cloudLevel = "多云";
        else if (weather.cloudCover < 90) cloudLevel = "阴天";
        else cloudLevel = "密云";
        qInfo() << "   描述: " << cloudLevel;
    }
    qInfo() << "";
    
    // 空气质量
    if (weather.airQuality > 0) {
        qInfo() << "【空气质量】";
        qInfo() << "🌫️  AQI指数: " << weather.airQuality;
        
        QString aqiLevel;
        QString aqiColor;
        if (weather.airQuality <= 50) {
            aqiLevel = "优";
            aqiColor = "绿色";
        } else if (weather.airQuality <= 100) {
            aqiLevel = "良";
            aqiColor = "黄色";
        } else if (weather.airQuality <= 150) {
            aqiLevel = "轻度污染";
            aqiColor = "橙色";
        } else if (weather.airQuality <= 200) {
            aqiLevel = "中度污染";
            aqiColor = "红色";
        } else if (weather.airQuality <= 300) {
            aqiLevel = "重度污染";
            aqiColor = "紫色";
        } else {
            aqiLevel = "严重污染";
            aqiColor = "褐红色";
        }
        qInfo() << "   等级: " << aqiLevel << " (" << aqiColor << ")";
        qInfo() << "";
    }
    
    // 时间信息
    qInfo() << "【时间信息】";
    if (!weather.obsTime.isEmpty()) {
        qInfo() << "⏱️  观测时间: " << weather.obsTime;
    }
    qInfo() << "🕐 更新时间: " << weather.updateTime;
    
    qInfo() << "";
    qInfo() << "════════════════════════════════════════";
    qInfo() << "";
}

/***************************************************************
 * 发布天气信息到串口
 ***************************************************************/
void WeatherService::publishToSerial(const WeatherData &weather)
{
    if (!m_pSerial || !m_pSerial->isOpen()) {
        qDebug() << "[WeatherService] 串口未打开，跳过串口输出";
        return;
    }
    
    // 格式化天气信息
    QString weatherStr = formatWeatherString(weather);
    
    // 发送到串口
    QByteArray data = weatherStr.toUtf8();
    qint64 written = m_pSerial->write(data);
    
    if (written > 0) {
        qInfo() << "[WeatherService] ✓ 天气信息已发送到串口 (" << written << " 字节)";
    } else {
        qWarning() << "[WeatherService] ✗ 串口写入失败";
    }
}

/***************************************************************
 * 格式化天气信息为字符串
 ***************************************************************/
QString WeatherService::formatWeatherString(const WeatherData &weather)
{
    // 格式化为详细的字符串（适合串口传输）
    QString str;
    str += "╔═══════════════════════════════════╗\r\n";
    str += "║        WEATHER REPORT             ║\r\n";
    str += "╚═══════════════════════════════════╝\r\n";
    str += "\r\n";
    
    // 基本信息
    str += "[BASIC INFO]\r\n";
    str += QString("Location    : %1\r\n").arg(weather.location);
    str += QString("Weather     : %1\r\n").arg(weather.weather);
    str += QString("Data Source : %1\r\n").arg(weather.dataSource);
    str += "\r\n";
    
    // 温度
    str += "[TEMPERATURE]\r\n";
    str += QString("Current     : %1C\r\n").arg(weather.temperature, 0, 'f', 1);
    if (weather.feelsLike != 0) {
        str += QString("Feels Like  : %1C\r\n").arg(weather.feelsLike, 0, 'f', 1);
    }
    str += "\r\n";
    
    // 风
    str += "[WIND]\r\n";
    str += QString("Direction   : %1\r\n").arg(weather.windDirection);
    str += QString("Speed       : %1 km/h\r\n").arg(weather.windSpeed, 0, 'f', 1);
    if (!weather.windScale.isEmpty()) {
        str += QString("Scale       : %1\r\n").arg(weather.windScale);
    }
    str += "\r\n";
    
    // 湿度和降水
    str += "[HUMIDITY & PRECIPITATION]\r\n";
    str += QString("Humidity    : %1%%\r\n").arg(weather.humidity, 0, 'f', 0);
    if (weather.precipitation > 0) {
        str += QString("Precipitation: %1 mm\r\n").arg(weather.precipitation, 0, 'f', 1);
    }
    str += "\r\n";
    
    // 大气条件
    str += "[ATMOSPHERE]\r\n";
    if (weather.pressure > 0) {
        str += QString("Pressure    : %1 hPa\r\n").arg(weather.pressure, 0, 'f', 1);
    }
    if (weather.visibility > 0) {
        str += QString("Visibility  : %1 km\r\n").arg(weather.visibility, 0, 'f', 1);
    }
    if (weather.cloudCover >= 0) {
        str += QString("Cloud Cover : %1%%\r\n").arg(weather.cloudCover);
    }
    str += "\r\n";
    
    // 空气质量
    if (weather.airQuality > 0) {
        str += "[AIR QUALITY]\r\n";
        str += QString("AQI         : %1\r\n").arg(weather.airQuality);
        str += "\r\n";
    }
    
    // 时间
    str += "[TIME]\r\n";
    if (!weather.obsTime.isEmpty()) {
        str += QString("Observation : %1\r\n").arg(weather.obsTime);
    }
    str += QString("Update Time : %1\r\n").arg(weather.updateTime);
    str += "\r\n";
    str += "═══════════════════════════════════\r\n";
    
    return str;
}

/***************************************************************
 * 天气更新成功内部处理（打印详细信息）
 ***************************************************************/
void WeatherService::onWeatherUpdated(const WeatherData &weather)
{
    LOG_INFO("Weather", "");
    LOG_INFO("Weather", "========================================");
    LOG_INFO("Weather", "  🌤️ 天气信息更新成功");
    LOG_INFO("Weather", "========================================");
    LOG_INFO("Weather", QString("📍 位置: %1").arg(weather.location));
    LOG_INFO("Weather", QString("🌡️  温度: %1°C (体感%2°C)").arg(weather.temperature).arg(weather.feelsLike));
    LOG_INFO("Weather", QString("☁️  天气: %1").arg(weather.weather));
    LOG_INFO("Weather", QString("💧 湿度: %1%").arg(weather.humidity));
    LOG_INFO("Weather", QString("💨 风速: %1km/h %2 风力%3").arg(weather.windSpeed).arg(weather.windDirection).arg(weather.windScale));
    LOG_INFO("Weather", QString("🌧️  降水: %1mm").arg(weather.precipitation));
    LOG_INFO("Weather", QString("🔍 能见度: %1km").arg(weather.visibility));
    LOG_INFO("Weather", QString("📊 气压: %1hPa").arg(weather.pressure));
    LOG_INFO("Weather", QString("🏭 空气质量: %1").arg(weather.airQuality));
    LOG_INFO("Weather", QString("⏰ 观测时间: %1").arg(weather.obsTime));
    LOG_INFO("Weather", QString("🔄 更新时间: %1").arg(weather.updateTime));
    LOG_INFO("Weather", QString("📡 数据源: %1").arg(weather.dataSource));
    LOG_INFO("Weather", "========================================");
    LOG_INFO("Weather", "");
}

/***************************************************************
 * 天气更新失败内部处理
 ***************************************************************/
void WeatherService::onUpdateFailed(const QString &error)
{
    LOG_WARNING("Weather", QString("⚠️ 天气信息更新失败: %1").arg(error));
}

