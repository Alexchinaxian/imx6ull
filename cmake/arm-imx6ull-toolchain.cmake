# 工具链文件 for iMX6ULL
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 编译器路径
set(CMAKE_C_COMPILER /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++)

# Sysroot 设置
set(CMAKE_SYSROOT /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi)

# 编译标志
set(CMAKE_C_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

# 搜索模式
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Qt 路径
set(QT_ARM_PATH "/home/alex/qt-everywhere-src-5.12.9/arm-qt")
set(CMAKE_PREFIX_PATH "${QT_ARM_PATH}/lib/cmake" CACHE STRING "Qt5 CMake prefix path")

# 包含目录 - 为 IntelliSense 提供明确的路径
include_directories(
    ${QT_ARM_PATH}/include
    ${QT_ARM_PATH}/include/QtCore
    ${CMAKE_SYSROOT}/usr/include
)

# 设置 pkg-config 环境
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
set(ENV{PKG_CONFIG_PATH} "${QT_ARM_PATH}/lib/pkgconfig")

message(STATUS "iMX6ULL 工具链配置完成")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "Sysroot: ${CMAKE_SYSROOT}")
message(STATUS "Qt路径: ${QT_ARM_PATH}")