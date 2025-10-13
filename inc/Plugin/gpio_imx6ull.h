#ifndef _GPIO_IMX6ULL_H_
#define _GPIO_IMX6ULL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>


// GPIO配置结构体
typedef struct {
    int fd;                  // 文件描述符
    char portNo[20];         // 端口号
    int timeout;             // 超时时间
    int direction;           // 方向: 0-输入, 1-输出
    int value;               // 当前值
    pthread_t threadid;      // 线程ID
    int bl_Exit;             // 退出标志
    void (*processData)(int, int);  // 数据处理回调函数
} GPIOCfg;

// 函数声明
#define HANDLE void *

#ifdef __cplusplus
extern "C" {
#endif

HANDLE HAL_CommOpen(char *pPortDescriptor, char *pOpenParams, int dwPortAttr, int nTimeout, int *pErrCode);
HANDLE HAL_CommAccept(HANDLE hPort);
int HAL_CommRead(HANDLE hPort, char *pBuffer, int nBytesToRead);
int HAL_CommWrite(HANDLE hPort, char *pBuffer, int nBytesToWrite);
int HAL_CommControl(HANDLE hPort, int nCmd, void *pBuffer, int nDataLength);
int HAL_CommClose(HANDLE hPort);
char *DLLInfo(void);

#ifdef __cplusplus
}
#endif

#endif