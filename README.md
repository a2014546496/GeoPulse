# GeoPulse

**Real-time GPS Monitoring & Tracking Desktop Application**

GeoPulse 是一个基于 Qt6 + QML 的桌面 GIS 应用，支持实时 GPS 位置监控、轨迹记录和电子围栏功能。

## 功能特性

- 🗺 **实时地图显示** — 基于 Leaflet + OpenStreetMap 的地图，支持位置跟随
- 📡 **多源 GPS 接入** — 支持串口、TCP、UDP 三种方式连接 GPS 设备
- 🛰 **卫星信息** — 天空视图、信噪比图表、卫星列表
- 📍 **位置仪表盘** — 实时显示经纬度、海拔、速度、航向、DOP 值
- 🎯 **电子围栏** — 多边形围栏定义与进入/离开告警
- 🚶 **轨迹记录** — 支持开始/停止录制，导出 GPX/CSV 格式
- 🎨 **暗色主题** — 专业暗色地图界面
- ⚙️ **配置持久化** — JSON 配置文件，记住上次连接参数

## 系统架构

```
GeoPulse
├── libs/
│   ├── gpscomm/      GPS 通信驱动 (Serial/TCP/UDP)
│   └── gpsutils/     GPS 工具 (NMEA解析/地理计算/日志/配置)
├── src/
│   ├── core/         核心业务层 (GpsManager/TrackRecorder/GeofenceManager)
│   └── app/          应用层 (main/GeoPulseApp)
├── qml/              QML 界面
├── resources/        资源文件
├── sim/              模拟 NMEA 数据
└── tests/            单元测试
```

## 依赖

| 组件 | 说明 |
|------|------|
| Qt 6.2+ | Core, Quick, Qml, Positioning, SerialPort, Network, WebEngineQuick |
| CMake 3.16+ | 构建系统 |
| C++17 | 编译器 |
| Leaflet 1.9 | 地图库（通过 CDN 加载） |

### 安装依赖 (Ubuntu 22.04+)

```bash
sudo apt install qt6-base-dev qt6-declarative-dev qt6-positioning-dev \
                 libqt6serialport6-dev qt6-webengine-dev cmake g++
```

## 构建

```bash
cmake -B build
cmake --build build -j$(nproc)
```

## 运行

```bash
./build/src/app/GeoPulse
```

## 测试

```bash
cd build && ctest --output-on-failure
```

## 使用方式

### 连接 GPS 设备

1. 打开 Settings 面板
2. 选择连接类型 (Serial/TCP/UDP)
3. 填写设备地址：
   - Serial: `/dev/ttyUSB0` 或 `COM3`
   - TCP: `192.168.1.100:5000`
   - UDP: `5000`（监听端口）
4. 点击 Connect

### 轨迹录制

1. 在 Track 面板点击 Record 开始录制
2. 轨迹线实时显示在地图上
3. 点击 Stop 结束录制
4. 使用 Export GPX/CSV 导出轨迹文件

### 电子围栏

通过 `GeofenceManager` C++ 接口添加多边形围栏，当设备进入/离开围栏区域时触发告警。

## 项目文件

### 新增文件清单

```
CMakeLists.txt                          # 根构建文件
libs/gpscomm/CMakeLists.txt            # gpscomm 库
libs/gpscomm/driver_factory.cpp        # 驱动工厂实现
libs/gpsutils/CMakeLists.txt           # gpsutils 库
src/core/CMakeLists.txt                # 核心库
src/core/gps_manager.h/.cpp            # GPS 数据管理器
src/core/track_recorder.h/.cpp         # 轨迹录制器
src/core/geofence_manager.h/.cpp       # 电子围栏管理器
src/app/CMakeLists.txt                 # 应用
src/app/main.cpp                       # 入口
src/app/geopulse_app.h/.cpp            # 应用根对象
qml/main.qml                           # 主窗口
qml/MapView.qml                        # 地图视图
qml/map.html                           # Leaflet 地图页面
qml/Dashboard.qml                      # 仪表盘面板
qml/SatellitePanel.qml                 # 卫星面板
qml/TrackPanel.qml                     # 轨迹面板
qml/SettingsPanel.qml                  # 设置面板
resources/resources.qrc                # Qt 资源文件
resources/icons/*.svg                  # 图标 (8个)
sim/sample.nmea                        # 模拟数据
tests/CMakeLists.txt                   # 测试构建
tests/test_nmea_parser.cpp             # NMEA 解析器测试
tests/test_geo_utils.cpp               # 地理计算测试
tests/test_config_manager.cpp          # 配置管理器测试
tests/test_driver_factory.cpp          # 驱动工厂测试
README.md                              # 本文件
```

## 许可证

MIT License
