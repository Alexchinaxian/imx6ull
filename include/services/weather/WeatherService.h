/***************************************************************
 * Copyright: Alex
 * FileName: WeatherService.h
 * Author: Alex
 * Version: 1.0
 * Date: 2025-10-15
 * Description: 天气服务 - 获取和发布天气信息
 *
 * 功能说明:
 *   1. 定时获取天气信息（HTTP API）
 *   2. 日志输出天气信息
 *   3. 串口输出天气信息
 *   4. 天气数据缓存
 *
 * 设计模式:
 *   - 单例服务模式
 *   - 观察者模式（信号槽）
 *
 * History:
 *   1. 2025-10-15 创建文件
 ***************************************************************/

#ifndef IMX6ULL_SERVICES_WEATHER_H
#define IMX6ULL_SERVICES_WEATHER_H

#include "core/ISysSvrInterface.h"
#include "drivers/serial/DriverSerial.h"
#include <QTimer>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/***************************************************************
 * 天气数据结构
 ***************************************************************/
struct WeatherData {
    // 基本信息
    QString location;           // 位置
    QString weather;            // 天气状况（晴、多云、雨等）
    QString weatherCode;        // 天气代码
    
    // 温度相关
    float temperature;          // 温度（摄氏度）
    float feelsLike;            // 体感温度（摄氏度）
    
    // 风相关
    float windSpeed;            // 风速（km/h）
    QString windDirection;      // 风向
    QString windScale;          // 风力等级
    
    // 湿度和降水
    float humidity;             // 湿度（%）
    float precipitation;        // 降水量（mm）
    
    // 气压和能见度
    float pressure;             // 气压（hPa）
    float visibility;           // 能见度（km）
    
    // 云量和空气质量
    int cloudCover;             // 云量（%）
    int airQuality;             // 空气质量指数AQI
    
    // 时间信息
    QString obsTime;            // 观测时间
    QString updateTime;         // 数据更新时间
    
    // 数据源信息
    QString dataSource;         // 数据源（API/模拟）
    
    // 有效性标志
    bool isValid;               // 数据是否有效
    
    WeatherData() : temperature(0), feelsLike(0), windSpeed(0), 
                    humidity(0), precipitation(0), pressure(0), 
                    visibility(0), cloudCover(0), airQuality(0), 
                    isValid(false), dataSource("Unknown") {}
};

/***************************************************************
 * 类名: WeatherService
 * 功能: 天气服务
 * 
 * 说明:
 *   定时获取天气信息并通过日志和串口发布
 * 
 * 使用示例:
 *   WeatherService *weatherSvr = new WeatherService(
 *       SYS_SVR_ID_WEATHER_SVR,
 *       SYS_SVR_TYPE_WEATHER_SVR
 *   );
 *   weatherSvr->setSerialDriver(serialDriver);
 *   weatherSvr->setLocation("shanghai");
 *   weatherSvr->SvrInit();
 *   weatherSvr->SvrStart();
 ***************************************************************/
class WeatherService : public ISysSvrInterface
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param svr_id 服务ID
     * @param svr_type 服务类型
     * @param parent 父对象指针
     */
    explicit WeatherService(int32_t svr_id, int32_t svr_type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~WeatherService();
    
    // ========== 实现ISysSvrInterface接口 ==========
    
    virtual bool SvrInit() override;
    virtual bool SvrStart() override;
    virtual bool SvrStop() override;
    
    virtual QString GetSvrName() const override {
        return "WeatherService";
    }
    
    // ========== 天气服务特有接口 ==========
    
    /**
     * @brief 设置串口驱动
     * @param serial 串口驱动指针
     */
    void setSerialDriver(DriverSerial *serial);
    
    /**
     * @brief 设置城市位置
     * @param location 城市名称或代码
     */
    void setLocation(const QString &location);
    
    /**
     * @brief 设置API密钥
     * @param apiKey API密钥
     */
    void setApiKey(const QString &apiKey);
    
    /**
     * @brief 设置更新间隔
     * @param minutes 更新间隔（分钟）
     */
    void setUpdateInterval(int minutes);
    
    /**
     * @brief 立即获取天气
     */
    void fetchWeatherNow();
    
    /**
     * @brief 获取当前天气数据
     * @return WeatherData 天气数据
     */
    WeatherData getCurrentWeather() const { return m_currentWeather; }
    
    /**
     * @brief 检查数据是否有效
     * @return true=有效, false=无效
     */
    bool isDataValid() const { return m_currentWeather.isValid; }

signals:
    /**
     * @brief 天气更新成功信号
     * @param weather 天气数据
     */
    void weatherUpdated(const WeatherData &weather);
    
    /**
     * @brief 天气更新失败信号
     * @param error 错误信息
     */
    void updateFailed(const QString &error);

private slots:
    /**
     * @brief 定时获取槽函数（获取新天气数据）
     */
    void onFetchTimer();
    
    /**
     * @brief 定时输出槽函数（输出已有天气数据）
     */
    void onPublishTimer();
    
    /**
     * @brief HTTP请求完成槽函数
     */
    void onNetworkReplyFinished(QNetworkReply *reply);
    
    /**
     * @brief 天气更新成功内部处理（打印详细信息）
     */
    void onWeatherUpdated(const WeatherData &weather);
    
    /**
     * @brief 天气更新失败内部处理
     */
    void onUpdateFailed(const QString &error);

private:
    /**
     * @brief 执行天气获取
     */
    void performWeatherFetch();
    
    /**
     * @brief 解析天气数据（简化版本，使用模拟数据）
     * @param data JSON数据
     * @return WeatherData 解析后的天气数据
     */
    WeatherData parseWeatherData(const QByteArray &data);
    
    /**
     * @brief 发布天气信息到日志
     * @param weather 天气数据
     */
    void publishToLog(const WeatherData &weather);
    
    /**
     * @brief 发布天气信息到串口
     * @param weather 天气数据
     */
    void publishToSerial(const WeatherData &weather);
    
    /**
     * @brief 格式化天气信息为字符串
     * @param weather 天气数据
     * @return QString 格式化的字符串
     */
    QString formatWeatherString(const WeatherData &weather);

private:
    QTimer *m_fetchTimer;                   // 获取天气定时器（每5分钟）
    QTimer *m_publishTimer;                 // 发布天气定时器（每10秒）
    QNetworkAccessManager *m_networkManager; // 网络管理器
    
    QString m_location;                     // 位置
    QString m_apiKey;                       // API密钥
    int m_fetchIntervalMinutes;             // 获取间隔（分钟）
    int m_publishIntervalSeconds;           // 发布间隔（秒）
    
    WeatherData m_currentWeather;           // 当前天气数据
    
    DriverSerial *m_pSerial;                // 串口驱动指针
};

#endif // WEATHERSERVICE_H

