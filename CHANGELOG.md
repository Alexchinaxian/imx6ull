# 更新日志 (Changelog)

## V1.2.0 - 2025-10-15

### 🆕 新增功能

#### 1. 硬件配置文件系统
- ✅ 创建 `hardware.init` 配置文件，支持INI格式
- ✅ 支持中文设备别名（如"风扇"、"继电器1"）
- ✅ 可配置参数：波特率、频率、占空比、GPIO方向等
- ✅ 支持设备启用/禁用控制

#### 2. DriverManager配置加载
- ✅ 新增 `loadFromConfig()` 方法从配置文件加载硬件
- ✅ 新增别名映射表（PWM、GPIO、LED、Serial）
- ✅ 新增 `getPWMByAlias()`、`getGPIOByAlias()` 等方法
- ✅ 新增 `printConfigReport()` 打印设备别名映射
- ✅ 自动初始化配置的硬件设备

#### 3. 通讯缓冲区机制

**串口驱动缓冲区 (DriverSerial)**
- ✅ 读缓冲区（默认64KB）
  - 自动累积接收数据
  - 防止数据丢失
  - 支持 `readAll()`、`read(n)`、`readLine()`
  - 溢出自动保护
- ✅ 写缓冲区（动态大小）
  - 异步发送机制
  - 分块发送（每次4KB）
  - 不阻塞主线程
- ✅ 缓冲区管理方法
  - `setReadBufferSize()` - 设置读缓冲大小
  - `getReadBufferSize()` - 查询读缓冲大小
  - `getWriteBufferSize()` - 查询写缓冲待发送数据
  - `clearReadBuffer()` - 清空读缓冲
  - `clearWriteBuffer()` - 清空写缓冲

**CAN驱动缓冲区 (DriverCAN)**
- ✅ 接收帧缓冲区（默认1000帧）
  - FIFO队列，先进先出
  - 防止帧丢失
  - 溢出自动丢弃旧帧
- ✅ 缓冲区管理方法
  - `readFrame()` - 读取一帧
  - `readAllFrames()` - 读取所有帧
  - `getBufferedFrameCount()` - 查询缓冲帧数
  - `clearReceiveBuffer()` - 清空缓冲区
  - `setReceiveBufferMaxSize()` - 设置最大帧数

### 📝 配置文件示例

```ini
[PWM/风扇]
type = PWM
name = 风扇
chip = 0
channel = 0
enabled = true
frequency = 25000
duty_cycle = 50.0
description = 散热风扇控制

[GPIO/继电器1]
type = GPIO
name = 继电器1
gpio_num = 3
direction = out
initial_value = 0
enabled = true
description = 主控继电器

[Serial/Modbus串口]
type = Serial
name = Modbus串口
device = /dev/ttymxc2
baudrate = 9600
databits = 8
parity = N
stopbits = 1
enabled = true
description = Modbus RTU通信
```

### 💡 使用示例

```cpp
// 1. 加载配置文件
DriverManager &driverMgr = DriverManager::getInstance();
driverMgr.loadFromConfig("hardware.init");

// 2. 通过别名访问硬件
DriverPWM *fan = driverMgr.getPWMByAlias("风扇");
fan->start();

DriverSerial *modbus = driverMgr.getSerialByAlias("Modbus串口");
modbus->open(QIODevice::ReadWrite);

// 3. 使用缓冲区
modbus->write("Hello");  // 异步发送
qInfo() << "待发送:" << modbus->getWriteBufferSize() << "字节";

// 4. 读取数据
QByteArray data = modbus->readAll();  // 从读缓冲读取
```

### 🔧 改进优化

- 提高串口通信可靠性（防止半包/粘包问题）
- 提高通讯性能（异步发送，不阻塞）
- 降低CPU占用（批量处理，减少系统调用）
- 简化协议解析（数据自动累积）

### 📚 文档更新

- 新增 `docs/BUFFER_USAGE.md` - 缓冲区详细使用指南
- 新增 `examples/buffer_example.cpp` - 缓冲区示例代码
- 更新 README.md - 添加配置文件和缓冲区说明

---

## V1.1.0 - 2025-10-15

### 🆕 新增功能
- ✅ 新增协议层架构
- ✅ 实现Modbus RTU协议（串口）
- ✅ 实现Modbus TCP协议（网络）
- ✅ 实现ProtocolManager协议管理器
- ✅ 支持多协议实例管理

---

## V1.0.0 - 2025-10-15

### 🆕 初始版本
- ✅ 完整的Qt驱动框架
- ✅ 基于sysfs标准接口
- ✅ 支持GPIO、LED、PWM、Serial、Temperature
- ✅ 实现ServiceManager服务管理
- ✅ 实现SystemScanner硬件扫描
- ✅ 多线程驱动架构
- ✅ 信号槽事件机制
- ✅ 优雅退出机制

