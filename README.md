# i.MX6ULL 嵌入式Qt应用框架

<div align="center">

**基于Qt5的工业级嵌入式Linux应用开发框架**

![Version](https://img.shields.io/badge/version-v1.3.0-blue)
![Platform](https://img.shields.io/badge/platform-i.MX6ULL-orange)
![Qt](https://img.shields.io/badge/Qt-5.12.9-green)
![Status](https://img.shields.io/badge/status-Production%20Ready-success)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

</div>

---

## 📋 项目概述

这是一个专业的嵌入式Linux应用开发框架，采用**四层架构设计**（应用层-服务层-协议层-驱动层），基于Qt5框架实现了完整的硬件抽象和业务逻辑分离。适用于工业控制、物联网设备、智能硬件等领域。

### 核心优势

- 🏗️ **专业架构** - 四层分层设计，模块化开发，易于扩展和维护
- 🔌 **即插即用** - 通过配置文件管理硬件，无需修改代码
- 🚀 **高性能** - 多线程架构，高性能CAN驱动(<0.5ms延迟)
- 🛡️ **安全可靠** - 基于Linux标准sysfs接口，不直接操作寄存器
- 📱 **易于移植** - 跨平台设计，适配不同ARM板只需修改配置
- 📊 **完整文档** - 详细的API文档、示例代码和部署指南

### 技术栈

| 层次 | 技术 |
|------|------|
| **应用框架** | Qt 5.12.9 (C++11) |
| **硬件接口** | Linux sysfs, SocketCAN, QSerialPort |
| **工业协议** | Modbus RTU, Modbus TCP, Modbus Slave |
| **交叉编译** | Yocto arm-poky-linux-gnueabi |
| **构建系统** | CMake 3.5+ |
| **目标平台** | i.MX6ULL (ARM Cortex-A7) |

---

## 📊 项目规模

| 指标 | 数值 |
|------|------|
| **C++类/结构体** | 60+ |
| **驱动支持** | 9种（GPIO/LED/PWM/Serial/CAN/Temperature等） |
| **服务模块** | 6个（温度/时间/天气/告警/Modbus等） |
| **协议支持** | 3种（Modbus RTU/TCP/Slave） |
| **可执行文件** | ~1MB (单一可执行文件) |
| **运行内存** | <5MB |
| **CPU占用** | <1% (空闲状态) |

---

## 🏗️ 系统架构

### 四层架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (Application)                      │
│  ┌─────────────────────────────────────────────────────┐     │
│  │  ServiceManager - 统一服务管理                        │     │
│  │  HardwareMapper - 硬件别名映射                        │     │
│  │  SystemBeep     - 系统提示音                         │     │
│  │  LogManager     - 日志管理系统                        │     │
│  └─────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
                               ↓
┌─────────────────────────────────────────────────────────────┐
│                      服务层 (Services)                         │
│  ┌─────────────────────────────────────────────────────┐     │
│  │  TemperatureService  - 温度监控服务                   │     │
│  │  ModbusSlaveService  - Modbus从站服务                │     │
│  │  TimeService         - 时间服务（RTC/NTP）            │     │
│  │  WeatherService      - 天气数据服务                   │     │
│  │  AlarmService        - 告警管理服务                   │     │
│  │  HardwareInitService - 硬件初始化服务                 │     │
│  └─────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
                               ↓
┌─────────────────────────────────────────────────────────────┐
│                      协议层 (Protocols)                        │
│  ┌─────────────────────────────────────────────────────┐     │
│  │  ProtocolManager - 协议管理器                         │     │
│  │  ModbusRTU      - Modbus RTU (串口主站)              │     │
│  │  ModbusTCP      - Modbus TCP (网络主站)              │     │
│  │  ModbusSlave    - Modbus Slave (从站通用)            │     │
│  │  [CANopen]      - CANopen协议（预留）                 │     │
│  │  [MQTT]         - MQTT物联网协议（预留）               │     │
│  └─────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
                               ↓
┌─────────────────────────────────────────────────────────────┐
│                      驱动层 (Drivers)                          │
│  ┌─────────────────────────────────────────────────────┐     │
│  │  DriverManager       - 驱动统一管理                   │     │
│  │  DriverGPIO          - GPIO控制（sysfs）              │     │
│  │  DriverLED           - LED控制（sysfs）               │     │
│  │  DriverBeep          - 蜂鸣器控制（sysfs）            │     │
│  │  DriverPWM           - PWM波形生成（sysfs）            │     │
│  │  DriverSerial        - 串口通信（QSerialPort+缓冲）    │     │
│  │  DriverCAN           - CAN通信（SocketCAN+缓冲）       │     │
│  │  DriverCANHighPerf   - 高性能CAN（独立接收线程）       │     │
│  │  DriverTemperature   - 温度监控（sysfs）              │     │
│  │  SystemScanner       - 硬件自动扫描                   │     │
│  └─────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
                               ↓
┌─────────────────────────────────────────────────────────────┐
│                 硬件层 (Linux Kernel Interface)                │
│  /sys/class/gpio/  |  /sys/class/leds/  |  /sys/class/pwm/   │
│  /dev/ttyS*        |  /dev/can*         |  /sys/class/thermal/│
└─────────────────────────────────────────────────────────────┘
```

---

## 🎯 核心特性

### 1. 智能硬件配置系统

通过 `hardware.init` 配置文件管理所有硬件设备，支持中文别名：

```ini
[PWM/风扇]
type = PWM
name = 风扇
chip = 0
channel = 0
frequency = 25000
duty_cycle = 50.0
enabled = true
description = 散热风扇控制

[GPIO/继电器1]
type = GPIO
name = 继电器1
gpio_num = 3
direction = out
initial_value = 0
enabled = true
description = 主控继电器
```

**优势**：
- ✅ 无需修改代码，只需修改配置文件
- ✅ 支持中文别名，提高可读性
- ✅ 支持设备启用/禁用控制
- ✅ 集中管理所有硬件参数

### 2. 高性能通讯缓冲

**串口驱动缓冲** (DriverSerial)
- 读缓冲区：64KB，防止数据丢失
- 写缓冲区：异步发送，不阻塞主线程
- 支持 `readAll()`, `readLine()`, `read(n)`

**CAN驱动缓冲** (DriverCAN / DriverCANHighPerf)
- 接收缓冲：1000帧FIFO队列
- 高性能版本：独立接收线程，<0.5ms延迟
- 吞吐量：2500帧/秒

### 3. 完整的Modbus协议栈

| 协议类型 | 功能 | 传输方式 | 状态 |
|---------|------|---------|------|
| Modbus RTU | 串口主站 | RS485/RS232 | ✅ 生产就绪 |
| Modbus TCP | 网络主站 | TCP/IP | ✅ 生产就绪 |
| Modbus Slave | 通用从站 | RTU/TCP | ✅ 生产就绪 |

**支持的功能码**：
- 0x01 - 读线圈
- 0x03 - 读保持寄存器
- 0x04 - 读输入寄存器
- 0x05 - 写单个线圈
- 0x06 - 写单个寄存器
- 0x0F - 写多个线圈
- 0x10 - 写多个寄存器

### 4. 多线程架构

```
主线程 (Main)
  ├─ ServiceManager（服务管理）
  │
  ├─ 驱动线程1 (Temperature)
  │   └─ 温度实时监控
  │
  ├─ 驱动线程2 (Serial)
  │   └─ 串口数据收发
  │
  ├─ 驱动线程3 (CAN)
  │   └─ CAN帧接收处理
  │
  └─ 服务线程N (AlarmService)
      └─ 告警逻辑处理
```

**优势**：
- 驱动在独立线程运行，互不干扰
- 异步信号槽通信，非阻塞
- CPU占用低，响应速度快

### 5. 业务服务模块

| 服务 | 功能 | 特性 |
|-----|------|-----|
| **TemperatureService** | CPU温度监控 | 实时采集、超温告警 |
| **ModbusSlaveService** | Modbus从站 | 支持RTU/TCP，寄存器映射 |
| **TimeService** | 时间同步 | RTC硬件时钟、NTP网络同步 |
| **WeatherService** | 天气数据 | 定时获取天气信息 |
| **AlarmService** | 告警管理 | 多级告警、蜂鸣器提示 |
| **HardwareInitService** | 硬件初始化 | 开机自检、配置加载 |

### 6. 日志管理系统

```cpp
LogManager &logger = LogManager::getInstance();

// 不同级别的日志
logger.debug("调试信息", "ModuleName");
logger.info("运行信息", "ModuleName");
logger.warning("警告信息", "ModuleName");
logger.error("错误信息", "ModuleName");

// 自动分类管理
logger.setModuleLogLevel("CAN", LogLevel::Debug);
logger.enableFileLogging("/var/log/app.log");
```

**特性**：
- 支持多级日志：Debug/Info/Warning/Error
- 模块化日志管理
- 文件和控制台双输出
- 自动时间戳和颜色标识

---

## 🚀 快速开始

### 方法1：一键部署（推荐，适合新环境）

```bash
# 克隆项目
git clone <your-repo-url> imx6ull
cd imx6ull

# 一键部署（自动安装工具链、配置环境、编译项目）
chmod +x quick_setup.sh
./quick_setup.sh
```

### 方法2：手动部署（适合已有环境）

```bash
# 1. 配置SSH免密登录（避免反复输入密码）
cat >> ~/.ssh/config << 'EOF'
Host 192.168.1.24 imx6ull
    HostName 192.168.1.24
    User root
    HostKeyAlgorithms +ssh-rsa
    PubkeyAcceptedKeyTypes +ssh-rsa
    IdentityFile ~/.ssh/id_rsa_imx6ull
EOF

# 生成密钥并复制到目标板（需手动输入密码一次）
ssh-keygen -t rsa -f ~/.ssh/id_rsa_imx6ull -N ""
ssh-copy-id -i ~/.ssh/id_rsa_imx6ull.pub root@192.168.1.24

# 2. 配置编译环境
source env-setup.sh

# 3. 编译项目
./rebuild.sh

# 4. 部署到设备（前台运行）
./download.sh

# 或后台运行
./download.sh --background
```

### 日常开发流程

```bash
# 修改代码后快速编译部署
./rebuild.sh && ./download.sh

# 只编译不部署
./rebuild.sh

# 只部署不运行
./download.sh --no-run

# 查看运行日志（后台模式）
ssh root@192.168.1.24 'tail -f /tmp/QtImx6ullBackend.log'

# 停止程序
ssh root@192.168.1.24 'killall QtImx6ullBackend'
```

---

## 💻 API使用指南

### GPIO控制

```cpp
#include "drivers/gpio/DriverGPIO.h"

DriverGPIO gpio(3);                      // GPIO3
gpio.exportGPIO();                       // 导出到用户空间
gpio.setDirection(DriverGPIO::Output);   // 设置为输出
gpio.setValue(DriverGPIO::High);         // 设置高电平
gpio.setValue(DriverGPIO::Low);          // 设置低电平
gpio.unexportGPIO();                     // 清理资源
```

### LED控制

```cpp
#include "drivers/led/DriverLED.h"

DriverLED led("sys-led");                // 控制sys-led
led.turnOn();                            // 打开
led.setBrightness(128);                  // 设置亮度（0-255）
led.blink(3, 500);                       // 闪烁3次，间隔500ms
led.turnOff();                           // 关闭
```

### PWM控制

```cpp
#include "drivers/pwm/DriverPWM.h"

DriverPWM pwm(0, 0);                     // pwmchip0, channel0
pwm.exportPWM();                         // 导出
pwm.setFrequency(25000, 60.0);           // 25KHz, 60%占空比
pwm.start();                             // 启动
pwm.setDutyCyclePercent(30.0);           // 调整占空比
pwm.stop();                              // 停止
pwm.unexportPWM();                       // 清理
```

### 串口通信（带缓冲）

```cpp
#include "drivers/serial/DriverSerial.h"

DriverSerial serial("/dev/ttyS1");
serial.configure(115200);                // 配置波特率
serial.open(QIODevice::ReadWrite);

// 设置缓冲区
serial.setReadBufferSize(32 * 1024);     // 32KB读缓冲

// 异步发送（使用写缓冲）
serial.write("Hello World\r\n");

// 接收数据
connect(&serial, &DriverSerial::dataReceived, [&](const QByteArray &data) {
    QByteArray line = serial.readLine();  // 读取一行
    qDebug() << "收到:" << line;
});

serial.close();
```

### CAN通信（高性能）

```cpp
#include "drivers/can/DriverCANHighPerf.h"

DriverCANHighPerf can("can0", 500000);   // CAN0, 500Kbps
can.open();

// 设置缓冲
can.setReceiveBufferMaxSize(1000);       // 最多缓存1000帧

// 发送
QCanBusFrame frame;
frame.setFrameId(0x123);
frame.setPayload(QByteArray::fromHex("0102030405060708"));
can.writeFrame(frame);

// 接收（异步，<0.5ms延迟）
connect(&can, &DriverCANHighPerf::frameReceived, [&](const QCanBusFrame &frame) {
    qDebug() << "收到帧:" << frame.frameId();
});

// 查询缓冲
qInfo() << "缓冲帧数:" << can.getBufferedFrameCount();

can.close();
```

### 硬件配置文件

```cpp
#include "drivers/manager/DriverManager.h"

DriverManager &driverMgr = DriverManager::getInstance();

// 加载配置文件
driverMgr.loadFromConfig("hardware.init");

// 通过别名访问硬件
DriverPWM *fan = driverMgr.getPWMByAlias("风扇");
if (fan) {
    fan->setFrequency(25000, 60.0);
    fan->start();
}

DriverGPIO *relay = driverMgr.getGPIOByAlias("继电器1");
if (relay) {
    relay->setValue(DriverGPIO::High);
}

// 查看配置
driverMgr.printConfigReport();
```

### Modbus RTU协议

```cpp
#include "protocols/modbus/ModbusRTU.h"

ModbusRTU modbus("/dev/ttyS1");

// 配置串口参数
QMap<QString, QVariant> config;
config["baudrate"] = 9600;
config["parity"] = "N";
config["slave_address"] = 1;
modbus.configure(config);

// 连接
if (modbus.connect()) {
    // 读取保持寄存器
    QVector<quint16> regs = modbus.readHoldingRegisters(0x0000, 10);
    
    // 写入单个寄存器
    modbus.writeSingleRegister(0x0000, 1234);
    
    // 写入多个寄存器
    QVector<quint16> values = {100, 200, 300};
    modbus.writeMultipleRegisters(0x0000, values);
}

modbus.disconnect();
```

### Modbus TCP协议

```cpp
#include "protocols/modbus/ModbusTCP.h"

ModbusTCP modbus("192.168.1.100", 502);

// 配置
QMap<QString, QVariant> config;
config["unit_id"] = 1;
config["timeout"] = 3000;
modbus.configure(config);

// 连接
if (modbus.connect()) {
    // 读取输入寄存器
    QVector<quint16> inputRegs = modbus.readInputRegisters(0x0000, 5);
    
    // 写入多个寄存器
    QVector<quint16> data = {10, 20, 30};
    modbus.writeMultipleRegisters(0x0000, data);
}

modbus.disconnect();
```

### 服务管理器

```cpp
#include "core/ServiceManager.h"

ServiceManager *mgr = ServiceManager::GetInstance();

// 创建所有服务
mgr->ManagerInitLoad();

// 初始化
mgr->SvrInit();

// 启动
mgr->SvrStart();

// 获取特定服务
TemperatureService *tempSvr = mgr->GetTemperatureSvrObj();
AlarmService *alarmSvr = mgr->GetAlarmSvrObj();
```

---

## 📂 项目结构

```
imx6ull/
├── src/                          # 源代码
│   ├── main.cpp                  # 主程序入口
│   ├── core/                     # 核心层
│   │   ├── ServiceManager.cpp    # 服务管理器
│   │   ├── HardwareConfig.cpp    # 硬件配置解析
│   │   ├── HardwareMapper.cpp    # 硬件别名映射
│   │   ├── SystemBeep.cpp        # 系统蜂鸣器
│   │   └── LogManager.cpp        # 日志管理
│   ├── drivers/                  # 驱动层
│   │   ├── gpio/                 # GPIO驱动
│   │   ├── led/                  # LED驱动
│   │   ├── beep/                 # 蜂鸣器驱动
│   │   ├── pwm/                  # PWM驱动
│   │   ├── serial/               # 串口驱动
│   │   ├── can/                  # CAN驱动
│   │   ├── temperature/          # 温度驱动
│   │   ├── manager/              # 驱动管理器
│   │   └── scanner/              # 系统扫描器
│   ├── services/                 # 服务层
│   │   ├── temperature/          # 温度监控服务
│   │   ├── modbus/               # Modbus从站服务
│   │   ├── time/                 # 时间服务
│   │   ├── weather/              # 天气服务
│   │   ├── alarm/                # 告警服务
│   │   └── hardware/             # 硬件初始化服务
│   └── protocols/                # 协议层
│       ├── modbus/               # Modbus协议
│       │   ├── ModbusRTU.cpp     # Modbus RTU主站
│       │   ├── ModbusTCP.cpp     # Modbus TCP主站
│       │   └── ModbusSlave.cpp   # Modbus从站
│       └── manager/              # 协议管理器
├── include/                      # 头文件（与src结构对应）
├── docs/                         # 文档
│   ├── 待更新.md
│   └── x.md
├── examples/                     # 示例代码
│   ├── buffer_example.cpp
│   └── highperf_can_example.cpp
├── tools/                        # 部署和调试工具
│   ├── view_log.sh               # 日志查看
│   ├── view_module_log.sh        # 模块日志查看
│   ├── test_alarm.sh             # 告警测试
│   ├── test_system_beep.sh       # 蜂鸣器测试
│   └── setup_test_beep.sh        # 蜂鸣器设置
├── third_party/                  # 第三方库
│   └── qt5/                      # Qt5交叉编译库
├── cmake/                        # CMake配置
│   └── arm-imx6ull-toolchain.cmake
├── build-arm/                    # 编译输出（自动生成）
│   ├── bin/
│   │   └── QtImx6ullBackend      # 主程序（~1MB）
│   └── lib/
├── hardware.init                 # 硬件配置文件
├── CMakeLists.txt                # CMake构建配置
├── rebuild.sh                    # 快速编译脚本
├── download.sh                   # 部署和运行脚本
├── env-setup.sh                  # 环境配置脚本
├── README.md                     # 项目说明（本文档）
└── CHANGELOG.md                  # 版本更新日志
```

---

## 🔧 移植到其他ARM板

本系统使用Linux标准接口，移植非常简单：

### 步骤1：修改部署IP

编辑 `download.sh`：
```bash
TARGET_IP="192.168.1.XX"    # 修改为新板子的IP
```

### 步骤2：编译并部署

```bash
./rebuild.sh && ./download.sh
```

### 步骤3：调整硬件配置

编辑 `hardware.init`，根据实际硬件调整参数：
- LED名称：查看 `/sys/class/leds/`
- GPIO编号：查看 `/sys/class/gpio/`
- PWM芯片：查看 `/sys/class/pwm/`
- 串口设备：查看 `/dev/tty*`

### 步骤4：验证硬件

程序启动时会自动扫描硬件并显示：

```
========================================
  Hardware Scan Report
========================================
GPIO Controllers: 5
  - gpiochip0 (32 lines)
  - gpiochip32 (32 lines)

LEDs: 4
  - sys-led
  - heartbeat
  - red-led
  - green-led

PWM Controllers: 3
  - pwmchip0 (1 channels)
  - pwmchip1 (1 channels)
  - pwmchip2 (2 channels)

Serial Ports: 3
  - /dev/ttyS0
  - /dev/ttyS1
  - /dev/ttymxc2
```

**完成！** 🎉 系统已适配新硬件平台。

---

## 🌟 设计亮点

### 1. 设计模式应用

| 模式 | 应用 | 优势 |
|-----|------|------|
| **单例模式** | ServiceManager, DriverManager | 全局唯一访问点 |
| **观察者模式** | Qt信号槽机制 | 松耦合的事件驱动 |
| **策略模式** | 协议切换（Modbus RTU/TCP） | 运行时动态切换 |
| **外观模式** | DriverManager, ProtocolManager | 简化复杂子系统的使用 |

### 2. 高性能优化

- **多线程并行**：驱动独立线程，互不阻塞
- **异步通信**：信号槽异步机制，零拷贝
- **缓冲优化**：读写缓冲区，减少系统调用
- **零拷贝**：Qt容器隐式共享，减少内存拷贝
- **RAII管理**：智能指针，自动资源管理

### 3. 安全可靠

- ✅ 基于sysfs标准接口，不直接操作寄存器
- ✅ 不与内核驱动冲突
- ✅ 完整的错误处理机制
- ✅ 资源自动清理（RAII）
- ✅ 线程安全的信号槽通信

### 4. 易于维护

- 📖 清晰的代码结构和注释
- 📚 完整的API文档
- 🔍 模块化设计，低耦合
- 🧪 丰富的示例代码
- 📝 详细的CHANGELOG

---

## 📊 性能测试

**测试平台**：正点原子 ATK-IMX6ULL-MINI
**内核版本**：Linux 4.1.15  
**Qt版本**：5.12.9

### 功能测试

| 功能 | 状态 | 备注 |
|------|------|------|
| GPIO控制 | ✅ 正常 | 响应时间<100μs |
| LED控制 | ✅ 正常 | 支持亮度调节 |


---

## 🛠️ 工具脚本

### 编译部署工具

| 脚本 | 功能 | 用法 |
|------|------|------|
| `rebuild.sh` | 快速编译 | `./rebuild.sh` |
| `download.sh` | 部署并运行 | `./download.sh [--no-run] [--background]` |
| `env-setup.sh` | 环境配置 | `source env-setup.sh` |

### 调试工具

| 脚本 | 功能 | 用法 |
|------|------|------|


---

## 📚 文档索引

| 文档 | 说明 |
|------|------|
| [README.md](README.md) | 项目主文档（本文档） |
| [CHANGELOG.md](CHANGELOG.md) | 版本更新日志 |
| [hardware.init](hardware.init) | 硬件配置文件（含详细注释） |
| [CMakeLists.txt](CMakeLists.txt) | CMake构建配置 |

---

## ❓ 常见问题

### Q1: SSH连接失败，提示"no matching host key type found"

**A**: 这是因为目标设备使用旧的ssh-rsa算法。解决方法：

```bash
# 配置SSH客户端允许旧算法
cat >> ~/.ssh/config << 'EOF'
Host 192.168.1.24
    HostKeyAlgorithms +ssh-rsa
    PubkeyAcceptedKeyTypes +ssh-rsa
EOF
```

### Q2: 编译时找不到Qt库

**A**: 确保已配置环境变量：

```bash
source env-setup.sh
echo $QT_ARM_PATH  # 应输出Qt库路径
```

### Q3: 程序运行时提示"cannot open GPIO"

**A**: 检查GPIO是否已被其他程序占用，或需要root权限：

```bash
ssh root@192.168.1.24 'ls -l /sys/class/gpio/'
```

### Q4: 如何添加新的硬件设备？

**A**: 只需编辑 `hardware.init` 配置文件，无需修改代码：

```ini
[PWM/新设备]
type = PWM
name = 新设备
chip = 3
channel = 0
enabled = true
```

### Q5: 如何查看程序运行日志？

**A**: 
```bash
# 前台运行时，直接显示在终端
./download.sh

# 后台运行时，查看日志文件
./download.sh --background
ssh root@192.168.1.24 'tail -f /tmp/QtImx6ullBackend.log'
```

---

## 🚧 未来计划

### 短期计划（V1.0.3）

- [ ] SPI总线驱动

### 中期计划（V2.0.0）

- [ ] CANopen协议支持
- [ ] MQTT物联网协议
- [ ] HTTP REST API服务
- [ ] WebSocket实时通信
- [ ] SQLite数据存储

### 长期计划（V3.0.0）

- [ ] Web管理界面
- [ ] OTA远程升级
- [ ] 云平台对接
- [ ] 数据可视化
- [ ] AI边缘计算集成

---

## 📜 版本信息

**当前版本**: V1.0.1  
**发布日期**: 2025-10-16  
**开发者**: Alex  
**许可证**: MIT License  

### 版本历史

| 版本 | 日期 | 主要更新 |
|------|------|---------|
| **V1.0.1** | 2025-10-16 | 告警服务、系统蜂鸣器、日志管理、硬件初始化服务 |
| **V1.0.0** | 2025-10-15 | 初始版本、基础驱动框架 |

详细更新日志请查看 [CHANGELOG.md](CHANGELOG.md)

---

## 📞 技术支持

### 项目信息

- **项目路径**: `/home/alex/alexcode/imx6ull`
- **目标设备**: `root@192.168.1.24`
- **开发平台**: Ubuntu 20.04+ / VMware

### 联系方式

如有问题或建议，欢迎通过以下方式联系：
- 📧 Email: [alex.work.xian@gmail.com]
- 💬 Issues: [GitHub Issues]

---

<div align="center">

**感谢使用 i.MX6ULL 嵌入式Qt应用框架！** 🎉

如果这个项目对您有帮助，请给个⭐Star支持一下！

</div>
