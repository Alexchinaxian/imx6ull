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

# Qt 路径设置
set(QT_ARM_PATH "/home/alex/qt-everywhere-src-5.12.9/arm-qt")
set(CMAKE_PREFIX_PATH "${QT_ARM_PATH}/lib/cmake")

# 编译标志
set(CMAKE_C_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fPIC")  # 添加 -fPIC 用于动态库

# 设置库文件输出目录
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# 查找 Qt5 模块
find_package(Qt5 REQUIRED COMPONENTS Core Gui SerialBus Network SerialPort)

# 设置 AUTOMOC
set(CMAKE_AUTOMOC ON)

# ===========================================
# 1. 创建 Plugin 动态库
# ===========================================
set(PLUGIN_SOURCES
    src/Plugin/gpio_imx6ull.c
    # 可以添加更多 Plugin 源文件
)

set(PLUGIN_HEADERS
    inc/Plugin/gpio_imx6ull.h
    # 可以添加更多 Plugin 头文件
)

# 创建共享库
add_library(Imx6ullBoardPlugin SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# 设置动态库版本
set_target_properties(Imx6ullBoardPlugin PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    OUTPUT_NAME "Imx6ullBoardPlugin"
)

# Plugin 包含目录
target_include_directories(Imx6ullBoardPlugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/Plugin
)

# Plugin 编译选项
target_compile_options(Imx6ullBoardPlugin PRIVATE
    -fPIC
    -Wall
)

# Plugin 链接选项
target_link_libraries(Imx6ullBoardPlugin)
# 如果需要链接其他库，可以在这里添加

# ===========================================
# 2. 主程序可执行文件
# ===========================================
set(MAIN_SOURCES
    src/main.cpp
)

# 其他层的源文件（如果需要）
set(CONNECTION_SOURCES
    #src/connection/led_controller.cpp
)

set(CONNECTION_HEADERS
    #inc/connection/led_controller.h
)

set(BUSINESS_SOURCES
    #src/business/led_manager.cpp
)

set(BUSINESS_HEADERS
    #inc/business/led_manager.h
)

# 添加可执行文件
add_executable(QtImx6ullBackend
    ${MAIN_SOURCES}
    ${CONNECTION_SOURCES}
    ${CONNECTION_HEADERS}
    ${BUSINESS_SOURCES}
    ${BUSINESS_HEADERS}
)

# 主程序包含目录
target_include_directories(QtImx6ullBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/driver
    ${QT_ARM_PATH}/include
    ${QT_ARM_PATH}/include/Qt
    ${QT_ARM_PATH}/include/QtCore
    ${QT_ARM_PATH}/include/QtGui
    ${QT_ARM_PATH}/include/QtSerialBus
    ${QT_ARM_PATH}/include/QtNetwork
    ${QT_ARM_PATH}/include/QtSerialPort
)

# 链接库：先链接 Plugin 库，再链接 Qt 库
target_link_libraries(QtImx6ullBackend 
    Imx6ullBoardPlugin  # 链接我们创建的 Plugin 库
    Qt5::Core 
    Qt5::Gui 
    Qt5::SerialBus
    Qt5::Network
    Qt5::SerialPort
)

# 设置链接标志
set_target_properties(QtImx6ullBackend PROPERTIES
    LINK_FLAGS "--sysroot=${CMAKE_SYSROOT} -Wl,-rpath-link,${QT_ARM_PATH}/lib -Wl,-rpath,\\\$ORIGIN/../lib"
)

# ===========================================
# 3. 源文件分组显示
# ===========================================
source_group("Plugin Library" FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
source_group("Connection Layer" FILES ${CONNECTION_SOURCES} ${CONNECTION_HEADERS})
source_group("Business Layer" FILES ${BUSINESS_SOURCES} ${BUSINESS_HEADERS})
source_group("Main" FILES ${MAIN_SOURCES})

# ===========================================
# 4. 安装规则（可选）
# ===========================================
# 安装动态库
install(TARGETS Imx6ullBoardPlugin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装可执行文件
install(TARGETS QtImx6ullBackend
    RUNTIME DESTINATION bin
)

# 安装头文件（如果需要）
install(DIRECTORY inc/Plugin/
    DESTINATION include/Plugin
    FILES_MATCHING PATTERN "*.h"
)

# ===========================================
# 5. 调试信息
# ===========================================
message(STATUS "==========================================")
message(STATUS "项目配置信息")
message(STATUS "==========================================")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "目标文件: QtImx6ullBackend")
message(STATUS "动态库: Imx6ullBoardPlugin")
message(STATUS "Qt 版本: ${Qt5_VERSION}")
message(STATUS "Qt 路径: ${QT_ARM_PATH}")
message(STATUS "系统根目录: ${CMAKE_SYSROOT}")
message(STATUS "C++ 编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "库输出目录: ${LIBRARY_OUTPUT_PATH}")
message(STATUS "可执行文件输出目录: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "==========================================")