# imx6ull_project

## 概述
> 该项目是用于学习**嵌入式QT**而做的项目。纯后台项目

## 项目目的
- 学习网络相关知识
- 学习QT线程
- 了解Linux 运行
- 了解槽信号机制等


## 项目
## 项目架构

```bash
.  
├── build-arm/   # 编译好的文件  
├── docs/        # 文档与材料  
├── inc/         # 头文件  
├── src/         # 源文件代码  
│   ├── managers/              # 管理类
│   ├── services/              # 服务类
│   ├── devices/               # 设备类
│   ├── common/                # 公共组件
│   └── Plugin/                # 插件
│      ├──  /               # 插件接口
│      ├── LED /               # LED GPIO接口
│      ├── Serial /            # 串口接口
│      └── CAN/                # Can 通讯接口
└── unittest/    # 单元测试与测试程序
```
## 使用

```sh
./rebuild.sh
```

## 功能概述
- 首先构建GPIO 的控制
