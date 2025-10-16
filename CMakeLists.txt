cmake_minimum_required(VERSION 3.5)
project(QtImx6ullBackend)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 修复 Qt5 配置警告
set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin")

# 设置交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 编译器设置
set(CMAKE_C_COMPILER /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++)

# Sysroot 设置
set(CMAKE_SYSROOT /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi)

# Qt 路径设置 - 使用项目内的Qt库
set(QT_ARM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/qt5")
set(CMAKE_PREFIX_PATH "${QT_ARM_PATH}/cmake")

# 编译标志
set(CMAKE_C_FLAGS "--sysroot=${CMAKE_SYSROOT} -mcpu=cortex-a7 -mfpu=neon -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# 设置库文件输出目录
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# 查找 Qt5 模块
find_package(Qt5 REQUIRED COMPONENTS Core Gui SerialBus Network SerialPort)

# 设置 AUTOMOC
set(CMAKE_AUTOMOC ON)

# ===========================================
# 源文件定义
# ===========================================

# 主程序
set(MAIN_SOURCES
    src/main.cpp
)

# 核心层
set(CORE_SOURCES
    src/core/ServiceManager.cpp
    src/core/HardwareConfig.cpp
    src/core/HardwareMapper.cpp
    src/core/SystemBeep.cpp
    src/core/LogManager.cpp
)

set(CORE_HEADERS
    include/core/ServiceManager.h
    include/core/ISysSvrInterface.h
    include/core/HardwareConfig.h
    include/core/HardwareMapper.h
    include/core/SystemBeep.h
    include/core/LogManager.h
)

# 驱动层
set(DRIVER_SOURCES
    src/drivers/temperature/DriverTemperature.cpp
    src/drivers/gpio/DriverGPIO.cpp
    src/drivers/led/DriverLED.cpp
    src/drivers/beep/DriverBeep.cpp
    src/drivers/pwm/DriverPWM.cpp
    src/drivers/serial/DriverSerial.cpp
    src/drivers/can/DriverCAN.cpp
    src/drivers/can/DriverCANHighPerf.cpp
    src/drivers/manager/DriverManager.cpp
    src/drivers/scanner/SystemScanner.cpp
)

set(DRIVER_HEADERS
    include/drivers/temperature/DriverTemperature.h
    include/drivers/gpio/DriverGPIO.h
    include/drivers/led/DriverLED.h
    include/drivers/beep/DriverBeep.h
    include/drivers/pwm/DriverPWM.h
    include/drivers/serial/DriverSerial.h
    include/drivers/can/DriverCAN.h
    include/drivers/can/DriverCANHighPerf.h
    include/drivers/manager/DriverManager.h
    include/drivers/scanner/SystemScanner.h
)

# 服务层
set(SERVICE_SOURCES
    src/services/temperature/TemperatureService.cpp
    src/services/modbus/ModbusSlaveService.cpp
    src/services/time/TimeService.cpp
    src/services/weather/WeatherService.cpp
    src/services/alarm/AlarmService.cpp
)

set(SERVICE_HEADERS
    include/services/temperature/TemperatureService.h
    include/services/modbus/ModbusSlaveService.h
    include/services/time/TimeService.h
    include/services/weather/WeatherService.h
    include/services/alarm/AlarmService.h
)

# 协议层
set(PROTOCOL_SOURCES
    src/protocols/modbus/ModbusRTU.cpp
    src/protocols/modbus/ModbusTCP.cpp
    src/protocols/modbus/ModbusSlave.cpp
    src/protocols/manager/ProtocolManager.cpp
)

set(PROTOCOL_HEADERS
    include/protocols/IProtocolInterface.h
    include/protocols/modbus/ModbusRTU.h
    include/protocols/modbus/ModbusTCP.h
    include/protocols/modbus/ModbusSlave.h
    include/protocols/manager/ProtocolManager.h
)

# ===========================================
# 创建可执行文件
# ===========================================
add_executable(QtImx6ullBackend
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${DRIVER_SOURCES}
    ${DRIVER_HEADERS}
    ${SERVICE_SOURCES}
    ${SERVICE_HEADERS}
    ${PROTOCOL_SOURCES}
    ${PROTOCOL_HEADERS}
)

# 包含目录
target_include_directories(QtImx6ullBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${QT_ARM_PATH}/include
    ${QT_ARM_PATH}/include/Qt
    ${QT_ARM_PATH}/include/QtCore
    ${QT_ARM_PATH}/include/QtGui
    ${QT_ARM_PATH}/include/QtSerialBus
    ${QT_ARM_PATH}/include/QtNetwork
    ${QT_ARM_PATH}/include/QtSerialPort
)

# 链接Qt库
target_link_libraries(QtImx6ullBackend 
    Qt5::Core 
    Qt5::Gui 
    Qt5::SerialBus
    Qt5::Network
    Qt5::SerialPort
)

# 设置链接标志
# RPATH设置为相对路径，使可执行文件能找到项目内的Qt库
set_target_properties(QtImx6ullBackend PROPERTIES
    LINK_FLAGS "--sysroot=${CMAKE_SYSROOT} -Wl,-rpath-link,${QT_ARM_PATH}/lib -Wl,-rpath,\\\$ORIGIN/../../third_party/qt5/lib:\\\$ORIGIN"
)

# ===========================================
# 源文件分组（IDE显示用）
# ===========================================
source_group("Main" FILES ${MAIN_SOURCES})
source_group("Core" FILES ${CORE_SOURCES} ${CORE_HEADERS})
source_group("Drivers" FILES ${DRIVER_SOURCES} ${DRIVER_HEADERS})
source_group("Services" FILES ${SERVICE_SOURCES} ${SERVICE_HEADERS})
source_group("Protocols" FILES ${PROTOCOL_SOURCES} ${PROTOCOL_HEADERS})

# ===========================================
# 安装规则
# ===========================================
install(TARGETS QtImx6ullBackend
    RUNTIME DESTINATION bin
)

# ===========================================
# 调试信息
# ===========================================
message(STATUS "==========================================")
message(STATUS "IMX6ULL Qt Backend Project")
message(STATUS "==========================================")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "目标文件: QtImx6ullBackend")
message(STATUS "Qt 版本: ${Qt5_VERSION}")
message(STATUS "Qt 路径: ${QT_ARM_PATH}")
message(STATUS "系统根目录: ${CMAKE_SYSROOT}")
message(STATUS "C++ 编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "可执行文件输出: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "")
message(STATUS "项目架构:")
message(STATUS "  - Core Layer: ${CMAKE_CURRENT_SOURCE_DIR}/include/core")
message(STATUS "  - Driver Layer: ${CMAKE_CURRENT_SOURCE_DIR}/include/drivers")
message(STATUS "  - Service Layer: ${CMAKE_CURRENT_SOURCE_DIR}/include/services")
message(STATUS "  - Protocol Layer: ${CMAKE_CURRENT_SOURCE_DIR}/include/protocols")
message(STATUS "==========================================")
