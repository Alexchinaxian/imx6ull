#include "gpio_imx6ull.h"

// 日志输出宏定义，实际使用时需替换为具体实现
#define AppLogOut(...) printf(__VA_ARGS__); printf("\n")

/*
 * 设备关闭
 */
int gpio_close(GPIOCfg *cfg)
{
    if (cfg->fd > 0)
    {
        close(cfg->fd);
    }

    cfg->fd = -1;
    return 1;
}

/*
 * GPIO 设置方向
 */
int gpio_set_direction(GPIOCfg *cfg, int direction)
{
    char path[50] = {0};
    int fd;
    
    // 构建方向文件路径
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", cfg->portNo);
    
    // 打开方向文件
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        AppLogOut("[%s] Can't open direction file for GPIO%s", __FUNCTION__, cfg->portNo);
        return -1;
    }
    
    // 设置方向
    if (direction == 0)  // 输入
    {
        write(fd, "in", 2);
        cfg->direction = 0;
    }
    else  // 输出
    {
        write(fd, "out", 3);
        cfg->direction = 1;
    }
    
    close(fd);
    return 0;
}

/*
 * GPIO 写(设置电平)
 */
int gpio_write(GPIOCfg *cfg, int value)
{
    char path[50] = {0};
    int fd;
    char buf[2];
    
    // 检查是否为输出模式
    if (cfg->direction != 1)
    {
        AppLogOut("[%s] GPIO%s is not set as output", __FUNCTION__, cfg->portNo);
        return -1;
    }
    
    // 构建值文件路径
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", cfg->portNo);
    
    // 打开值文件
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        AppLogOut("[%s] Can't open value file for GPIO%s", __FUNCTION__, cfg->portNo);
        return -1;
    }
    
    // 写入值
    snprintf(buf, sizeof(buf), "%d", value);
    write(fd, buf, 1);
    cfg->value = value;
    
    close(fd);
    return 1;
}

/*
 * GPIO 读(获取电平)
 */
int gpio_read(GPIOCfg *cfg, int *value)
{
    char path[50] = {0};
    int fd;
    char buf[2];
    int ret;
    
    // 构建值文件路径
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", cfg->portNo);
    
    // 打开值文件
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        AppLogOut("[%s] Can't open value file for GPIO%s", __FUNCTION__, cfg->portNo);
        return -1;
    }
    
    // 读取值
    ret = read(fd, buf, 1);
    if (ret > 0)
    {
        *value = buf[0] - '0';
        cfg->value = *value;
    }
    else
    {
        AppLogOut("[%s] Failed to read value for GPIO%s", __FUNCTION__, cfg->portNo);
        close(fd);
        return -1;
    }
    
    close(fd);
    return 1;
}

/*
 * GPIO 监听线程--注册了才会有
 */
void *GPIO_Monitor_Thread(void *arg)
{
    GPIOCfg *s = (GPIOCfg *)arg;
    char name[100] = {0};
    int last_value = -1;
    int current_value;
    
    snprintf(name, sizeof(name), "%s-monitor", s->portNo);
    // 假设setThreadName已实现
    // setThreadName(name);
    
    // 确保GPIO为输入模式
    if (s->direction != 0)
    {
        gpio_set_direction(s, 0);
    }
    
    while (!s->bl_Exit)
    {
        // 读取当前值
        if (gpio_read(s, &current_value) == 1)
        {
            // 如果值发生变化，调用回调函数
            if (current_value != last_value)
            {
                if (s->processData != NULL)
                {
                    s->processData(atoi(s->portNo), current_value);
                }
                last_value = current_value;
            }
        }
        
        // 短暂延时，避免CPU占用过高
        usleep(10000); // 10ms
    }
    
    return NULL;
}

/*
 * API接口打开设备
 */
HANDLE HAL_CommOpen(char *pPortDescriptor, char *pOpenParams, int dwPortAttr, int nTimeout, int *pErrCode)
{
    int i = 0;
    char gpio_num[20] = {0};
    char *list[10] = {0};
    char export_path[50] = "/sys/class/gpio/export";
    int export_fd;

    if (pOpenParams != NULL)
    {
        char *s = pOpenParams;
        char *buf = (char *)s;
        char *inner_ptr = NULL;

        // 解析参数
        while ((list[i] = strtok_r(buf, ",", &inner_ptr)) != NULL)
        {
            i++;
            buf = NULL;
            if (i >= 10)
            {
                break;
            }
        }
        
        // 获取GPIO编号
        strncpy(gpio_num, list[0], sizeof(gpio_num)-1);
    }
    else
    {
        AppLogOut("[%s] GPIO settings is null", __FUNCTION__);
        return NULL;
    }

    // 导出GPIO
    export_fd = open(export_path, O_WRONLY);
    if (export_fd < 0)
    {
        AppLogOut("[%s] Can't open export file", __FUNCTION__);
        return NULL;
    }
    
    write(export_fd, gpio_num, strlen(gpio_num));
    close(export_fd);
    
    // 等待导出完成
    usleep(100000); // 100ms

    GPIOCfg *s = (GPIOCfg *)calloc(1, sizeof(GPIOCfg));
    if (!s)
    {
        AppLogOut("[%s] Memory allocation failed", __FUNCTION__);
        return NULL;
    }

    // 初始化配置
    snprintf(s->portNo, sizeof(s->portNo), "%s", gpio_num);
    s->timeout = nTimeout;
    s->fd = -1;  // GPIO不需要持续打开文件描述符
    s->bl_Exit = 0;
    s->processData = NULL;
    
    // 默认设置为输入模式
    gpio_set_direction(s, 0);

    AppLogOut("[%s] Opened GPIO%s, TimeOut=[%d]",
          __FUNCTION__, s->portNo, nTimeout);
    
    return (HANDLE)s;
}

HANDLE HAL_CommAccept(HANDLE hPort)
{
    // GPIO不需要接受连接，返回NULL
    return NULL;
}

int HAL_CommRead(HANDLE hPort, char *pBuffer, int nBytesToRead)
{
    if (hPort != NULL && pBuffer != NULL && nBytesToRead >= 1)
    {
        GPIOCfg *p = (GPIOCfg *)hPort;
        int value;
        
        if (gpio_read(p, &value) == 1)
        {
            pBuffer[0] = value + '0';
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

int HAL_CommWrite(HANDLE hPort, char *pBuffer, int nBytesToWrite)
{
    if (hPort != NULL && pBuffer != NULL && nBytesToWrite >= 1)
    {
        GPIOCfg *p = (GPIOCfg *)hPort;
        int value = pBuffer[0] - '0';
        
        // 确保是输出模式
        if (p->direction != 1)
        {
            gpio_set_direction(p, 1);
        }
        
        if (gpio_write(p, value) == 1)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

int HAL_CommControl(HANDLE hPort, int nCmd, void *pBuffer, int nDataLength)
{
    if (hPort != NULL)
    {
        GPIOCfg *p = (GPIOCfg *)hPort;
        
        switch(nCmd)
        {
            case 0x1000:  // 注册回调函数，启动监听线程
                p->processData = (void(*)(int, int))pBuffer;
                if (p->processData != NULL)
                {
                    pthread_create(&p->threadid, NULL, GPIO_Monitor_Thread, p);
                }
                break;
                
            case 0x1001:  // 退出监听线程
                p->bl_Exit = 1;
                p->processData = NULL;
                break;
                
            case 0x2000:  // 设置为输入模式
                gpio_set_direction(p, 0);
                break;
                
            case 0x2001:  // 设置为输出模式
                gpio_set_direction(p, 1);
                break;
                
            case 0x3000:  // 获取当前方向
                if (pBuffer != NULL && nDataLength >= sizeof(int))
                {
                    memcpy(pBuffer, &p->direction, sizeof(int));
                }
                break;
                
            default:
                AppLogOut("[%s] Unknown command: 0x%x", __FUNCTION__, nCmd);
                return 0;
        }
        
        return 1;
    }
    else
    {
        return 0;
    }
}

int HAL_CommClose(HANDLE hPort)
{
    if (hPort != NULL)
    {
        GPIOCfg *p = (GPIOCfg *)hPort;
        char unexport_path[50] = "/sys/class/gpio/unexport";
        int unexport_fd;
        
        // 停止线程
        p->bl_Exit = 1;
        if (p->threadid != 0)
        {
            pthread_join(p->threadid, NULL);
        }
        
        // 关闭设备
        gpio_close(p);
        
        // 取消导出GPIO
        unexport_fd = open(unexport_path, O_WRONLY);
        if (unexport_fd >= 0)
        {
            write(unexport_fd, p->portNo, strlen(p->portNo));
            close(unexport_fd);
        }
        
        free(p);
        return 1;
    }
    
    return 0;
}

// 版本信息
static char Info[] =
{
    "描述:用于imx6ull平台的GPIO控制库  \n"
    "参数列表：GPIO编号,方向(0-输入,1-输出)\n"
    "程序设计者:肖何鑫   \n"
    "开发日期：2023.1.5   \n"
    "本程序最后编译时间:\n"    // 请勿在此后增加信息!
    "                              "    // 请保留此行(分配内存用)
};

/*****************************************************************/
// 函数名称：DLLInfo();
// 功能描述：动态库版本中将信息包 Info 输出，以作版本信息等标志。
// 输入参数：Info--版本信息数组
// 输出参数：
// 返回：    版本信息数组
// 其他：
/*****************************************************************/
char *DLLInfo(void)
{
    int nStrLen = strlen(Info);
    char buf[30] = {0};
    sprintf(buf, "%s %s\n", __DATE__, __TIME__);
    int len = strlen(buf);
    sprintf(Info + nStrLen - len - 1, "%s ", buf);
    return Info;
}