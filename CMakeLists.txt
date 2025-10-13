cmake_minimum_required(VERSION 3.5)
project(QtImx6ullBackend)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 修复 Qt5 配置警告
set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin")

# 设置交叉编译工具链:cite[3]:cite[7]
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 编译器设置
set(CMAKE_C_COMPILER /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++)

# Sysroot 设置
set(CMAKE_SYSROOT /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi)

# Qt 路径设置
set(QT_ARM_PATH "/home/alex/qt-everywhere-src-5.12.9/arm-qt")
set(CMAKE_PREFIX_PATH "${QT_ARM_PATH}/lib/cmake")

# 编译标志
set(CMAKE_CXX_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7")

# 查找 Qt5 模块:cite[2]:cite[10]
find_package(Qt5 REQUIRED COMPONENTS Core Gui SerialBus Network SerialPort )

# 设置 AUTOMOC（处理 Q_OBJECT 宏）:cite[10]
set(CMAKE_AUTOMOC ON)

# 源文件分组
set(DRIVER_SOURCES
    src/driver/driver_manager.cpp   # 驱动管理
    src/driver/gpio_driver.cpp      # IO驱动
)

set(DRIVER_HEADERS
    inc/driver/driver_manager.h     # 驱动管理
    inc/driver/gpio_driver.h        # IO驱动
)

set(CONNECTION_SOURCES
    src/connection/led_controller.cpp #led 连接层
)

set(CONNECTION_HEADERS
    inc/connection/led_controller.h #led 连接层
)

set(BUSINESS_SOURCES
    src/business/led_manager.cpp
)
set(BUSINESS_HEADERS
    inc/business/led_manager.h
)

set(MAIN_SOURCES
    src/main.cpp
)

# 添加可执行文件
add_executable(QtImx6ullBackend
    ${DRIVER_SOURCES}
    ${DRIVER_HEADERS}
    ${CONNECTION_SOURCES}
    ${CONNECTION_HEADERS}
    ${BUSINESS_SOURCES}
    ${BUSINESS_HEADERS}
    ${MAIN_SOURCES}
)

# 包含目录
target_include_directories(QtImx6ullBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/driver
    ${QT_ARM_PATH}/include
    ${QT_ARM_PATH}/include/Qt
    ${QT_ARM_PATH}/include/QtCore
    ${QT_ARM_PATH}/include/QtGui
    ${QT_ARM_PATH}/include/QtSerialBus
    ${QT_ARM_PATH}/include/QtNetwork
    ${QT_ARM_PATH}/include/QtSerialPort  # 添加 SerialPort 头文件路径
)

# 查找 zlib 库（推荐方法）:cite[1]
find_library(ZLIB_LIBRARY 
    NAMES z zlib
    PATHS ${CMAKE_SYSROOT}/usr/lib ${CMAKE_SYSROOT}/lib
    NO_DEFAULT_PATH
)

if(NOT ZLIB_LIBRARY)
    # 如果找不到，使用直接路径
    set(ZLIB_LIBRARY "${CMAKE_SYSROOT}/usr/lib/libz.so")
endif()


# 链接 Qt 库和其他必要库:cite[1]
target_link_libraries(QtImx6ullBackend 
    Qt5::Core 
    Qt5::Gui 
    Qt5::SerialBus
    Qt5::Network        # 添加Network模块
    Qt5::SerialPort     # 添加 SerialPort 库链接
    )

# 设置链接标志:cite[7]
set_target_properties(QtImx6ullBackend PROPERTIES
    LINK_FLAGS "--sysroot=${CMAKE_SYSROOT} -Wl,-rpath-link,${QT_ARM_PATH}/lib"
)

# 源文件分组显示
source_group("Driver Layer" FILES ${DRIVER_SOURCES} ${DRIVER_HEADERS})
source_group("Connection Layer" FILES ${CONNECTION_SOURCES} ${CONNECTION_HEADERS})
source_group("Business Layer" FILES ${BUSINESS_SOURCES} ${BUSINESS_HEADERS})
source_group("Main" FILES ${MAIN_SOURCES})

# 调试信息
message(STATUS "==========================================")
message(STATUS "项目配置信息")
message(STATUS "==========================================")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "目标文件: QtImx6ullBackend")
message(STATUS "Qt 版本: ${Qt5_VERSION}")
message(STATUS "Qt 路径: ${QT_ARM_PATH}")
message(STATUS "系统根目录: ${CMAKE_SYSROOT}")
message(STATUS "C++ 编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "包含目录: ${CMAKE_CURRENT_SOURCE_DIR}/inc")
message(STATUS "Qt 模块: Core Gui SerialBus")
message(STATUS "zlib 库路径: ${ZLIB_LIBRARY}")
message(STATUS "==========================================")