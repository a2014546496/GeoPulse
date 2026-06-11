# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此仓库中工作时提供指导。

## 构建与测试命令

```bash
# 配置（使用 /home/pt/Qt/ 下的 Qt 6.7.3）
# 注意：如果之前构建过，先删除 CMakeCache.txt 确保使用正确的 Qt 版本
rm -f build/CMakeCache.txt
cmake -B build -DCMAKE_PREFIX_PATH=/home/pt/Qt/6.7.3/gcc_64

# 构建
cmake --build build -j$(nproc)

# 运行全部测试
cd build && ctest --output-on-failure

# 运行单个测试
./build/tests/test_nmea_parser

# 启动应用（qt.conf 已自动处理 QML/插件路径，只需 LD_LIBRARY_PATH）
export LD_LIBRARY_PATH=/home/pt/Qt/6.7.3/gcc_64/lib:$LD_LIBRARY_PATH
./build/src/app/GeoPulse

# VSCode：F5 一键编译+启动（.vscode/launch.json + tasks.json 已配置）
```

## 架构总览

GeoPulse 是一个基于 Qt6/C++ 的桌面 GIS 应用，用于实时 GPS 监控与追踪。采用分层架构，顶层为 QML 界面，底层为静态链接的 C++ 库。

### 分层结构图

```
┌────────────────────────────────────────────────┐
│  QML 界面层 (qml/)                               │
│  QtLocation Map 原生地图，仪表盘面板               │
│  通过 QQmlContext 属性绑定 C++ 对象               │
└──────────────────┬─────────────────────────────┘
                   │ QQmlContext 属性注入
┌──────────────────▼─────────────────────────────┐
│  应用层 (src/app/)                               │
│  main.cpp → QQmlApplicationEngine               │
│  GeoPulseApp：持有并串联所有核心管理器              │
└──────────────────┬─────────────────────────────┘
                   │ 创建 / 持有
┌──────────────────▼─────────────────────────────┐
│  核心业务层 (src/core/)                          │
│  GpsManager：驱动 + 解析器 协调中心               │
│  TrackRecorder：轨迹录制与 GPX/CSV 导出           │
│  GeofenceManager：多边形围栏进出检测             │
└──────┬────────────────────┬────────────────────┘
       │                    │
┌──────▼──────────┐  ┌──────▼──────────────────┐
│ libs/gpscomm     │  │ libs/gpsutils            │
│ GPS 设备驱动      │  │ NMEA解析·地理计算·        │
│ 串口/TCP/UDP     │  │ 日志·配置管理             │
└──────────────────┘  └─────────────────────────┘
```

### libs/gpscomm/ — GPS 通信驱动层

抽象接口 + 三种具体实现，负责从物理设备获取原始 NMEA 字节流。

- **`IGPSDeviceDriver`** — 抽象接口：`open(GpsDriverConfig)`、`close()`、`isOpen()`、`sendCommand()`。信号：`rawDataReady(QByteArray)`、`connectionStateChanged(bool)`、`errorOccurred(QString)`。
- **`GpsSerialDriver`** — 封装 `QSerialPort`。配置设备路径（如 `/dev/ttyUSB0`）和波特率，默认 9600-8N1。
- **`GpsTcpDriver`** — TCP 客户端，基于 `QTcpSocket`。地址格式：`host:port`。通过 `QTimer` 实现断线自动重连，间隔可配置。
- **`GpsUdpDriver`** — UDP 监听器，基于 `QUdpSocket`。绑定端口，接收广播数据报。
- **`DriverFactory`** — 工厂类，通过枚举（`Tcp/Udp/Serial`）或大小写不敏感的字符串创建驱动实例。

配置结构体 `GpsDriverConfig`：name（名称）、address（地址）、baudRate（波特率）、timeoutMs（超时）、reconnectIntervalMs（重连间隔）、autoReconnect（自动重连）。

### libs/gpsutils/ — GPS 工具层

独立的工具集，不依赖 gpscomm。

- **`SatelliteInfo`**（独立结构体）— 单颗卫星信息。字段：`prn`（编号）、`elevation`（仰角）、`azimuth`（方位角）、`snr`（信噪比 dB）、`system`（System 枚举：GPS/GLONASS/Galileo/BeiDou/QZSS/NavIC）。`operator==` 可用，支持 QList 直接比较。
- **`NMEAParser`**（QObject）— 带缓冲的 NMEA 0183 语句解析器。`feed(QByteArray)` 接收原始字节，按 `\n` 分割，去除 `$` 前缀，验证 XOR 校验和，分派到各语句解析函数。发送 `positionUpdated(GpsData)` 和 `rawSentence(QString)` 信号。卫星列表独立于定位数据存储于 `m_satellitesInView`，通过 `satellitesInView()` 查询。支持的语句类型：`GGA`（定位/精度/卫星数）、`RMC`（时间/日期/速度/航向）、`VTG`（速度向量）、`GSA`（DOP 值）、`GSV`（可见卫星，含星座识别）。
- **`GpsData`**（结构体）— 包含：经纬度/海拔、速度（节+km/h）、航向角、定位质量（0=无效, 1=GPS, 2=DGPS, 4=RTK固定解, 5=RTK浮点解）、卫星数、HDOP/PDOP/VDOP、磁偏角、isValid 标志。**注意：卫星列表已从 GpsData 中分离为独立的 `SatelliteInfo` 结构体**，`GpsData` 聚焦定位数据，避免每次信号拷贝都携带完整的卫星列表。
- **`GeoUtils`**（命名空间）— Haversine 球面距离、方位角计算、已知起点和方位求终点、射线法点在多边形内判定、球面多边形面积、Douglas-Peucker 轨迹抽稀。
- **`ConfigManager`**（单例 QObject）— 线程安全的 JSON 配置文件管理，支持点号分隔的嵌套键访问（如 `serial.baudrate`）。方法：`value(key, defaultValue)`、`setValue(key, val)`、`load/save`。
- **`Logger`**（单例 QObject）— 四级日志（Debug/Info/Warning/Error），文件超过指定大小自动滚动归档。发送 `logEntryAdded` 信号供 UI 订阅。

### src/core/ — 核心业务层

将底层库编排为应用逻辑。所有类均为 QObject，通过 Q_PROPERTY 暴露给 QML。

- **`GpsManager`** — 中央协调器。通过 `DriverFactory` 创建驱动，将驱动的 `rawDataReady` 连接到 `NMEAParser::feed()`。暴露 Q_PROPERTY：lat/lon/alt/speed/course/fixQuality/satelliteCount/hdop。QML 调用 `connectToSource(address, type, baudRate)` 建立连接。发送 `newPosition(GpsData)` 信号（不含卫星列表），供 TrackRecorder 和 GeofenceManager 消费。**卫星数据独立管理**：从解析器查询 `QList<SatelliteInfo>`，通过 300ms 防抖定时器等待 GSV 完整周期后再发射 `satelliteInfoChanged()` 通知 QML。`satellitesInView()` 按 SNR 降序排序后返回 QVariantList。
- **`TrackRecorder`** — 将定位点记录为有序列表。QML 调用 `startRecording()/stopRecording()`。`addPoint(GpsData)` 通过 GeoUtils 累积距离。导出：`exportToGpx()`（XML 格式）、`exportToCsv()`。`simplifiedTrack()` 返回 Douglas-Peucker 抽稀后的轨迹点，用于地图叠加显示。
- **`GeofenceManager`** — 管理命名的多边形围栏。QML 调用 `addGeofence(name, QVariantList [{lat,lon}...])` 添加围栏。`checkPosition(lat, lon)` 对每个激活的围栏调用 `GeoUtils::pointInPolygon()`，状态变化时发送 `enteredGeofence` / `exitedGeofence` 信号。

### src/app/ — 应用入口层

- **`main.cpp`** — 创建 `QGuiApplication`、`GeoPulseApp`、`QQmlApplicationEngine`。将 `App`、`GpsManager`、`TrackRecorder`、`GeofenceManager` 作为 QML 上下文属性暴露。加载 `qrc:/qml/main.qml`。
- **`GeoPulseApp`** — 持有 GpsManager/TrackRecorder/GeofenceManager 实例。串联信号：`GpsManager::newPosition` → `TrackRecorder::addPoint` 和 `GeofenceManager::checkPosition`。在初始化和退出时通过 ConfigManager 加载/保存连接设置。

### qml/ — QML 界面层

- **`main.qml`** — `ApplicationWindow`，暗色主题（`#1a1a2e`）。顶部工具栏：连接指示灯、驱动类型、定位质量标签、卫星数量。左右分栏：MapDisplay（65%）+ SwipeView 侧边面板（35%），含 Dashboard/Satellites/Track/Settings 四个标签页。底部状态栏显示坐标。
- **MapDisplay.qml** — 直接基于 QtLocation `Map` 类型（OSM 瓦片，Plugin "osm"）。位置标记 MapQuickItem，精度圈 MapCircle，轨迹线 MapPolyline，围栏 Instantiator + MapPolygon。跟随模式用 Binding 控制。覆盖层按钮：缩放、跟随、居中（PinchHandler/DragHandler/WheelHandler 处理手势交互）。Timer 每 500ms 用于轨迹增量记录检测。
- **`Dashboard.qml`** — 位置面板（经纬度/海拔）、运动面板（速度仪表条+航向方位）、信号质量面板（定位质量彩色标签、卫星数、HDOP 颜色编码）。
- **`SatellitePanel.qml`** — `Canvas` 绘制的天空图（同心仰角环+方位角，星座区分形状，SNR 颜色编码）和 SNR 柱状图（按信号降序）。卫星列表表格（星座前缀彩色 PRN）。卫星数据通过 `satellitePanel.satellites` 属性缓存，一次 C++ 查询供三个 UI 组件共享。
- **`TrackPanel.qml`** — 录制/停止/清除按钮，统计信息（点数、距离、已用时间、平均速度），通过 `FileDialog` 导出 GPX/CSV。
- **`SettingsPanel.qml`** — 连接类型选择器（Serial/TCP/UDP）、地址输入框、波特率下拉框、连接/断开按钮。地图图层选择器、日志级别、关于信息。

### sim/ — 模拟数据与工具

- **`sample.nmea`** — 40 条 NMEA 语句（GGA/RMC/VTG/GSA/GSV），模拟北京望京区域向东北方向的 GPS 移动轨迹。校验和已修正。
- **`nmea_server.py`** — Python TCP 服务器，读取 NMEA 文件循环发送。用法：`python3 sim/nmea_server.py [文件] [端口] [间隔秒]`（默认 `sample.nmea`、`5000`、`1.0`）。GeoPulse 中选 TCP 模式连接 `127.0.0.1:5000` 即可接收模拟数据。

### .vscode/ — VSCode 调试配置

- **`launch.json`** — `F5` 启动配置，预设 `LD_LIBRARY_PATH` 环境变量，preLaunchTask 自动编译。
- **`tasks.json`** — `cmake build` 任务，供 preLaunchTask 调用。

## 核心数据流

```
GPS 设备 → driver.rawDataReady(bytes)
  → GpsManager.onRawDataReady(bytes)
    → NMEAParser.feed(bytes)
      → parseSentence() 逐行解析
        → validateChecksum()
        → parseGPGGA / parseGPRMC / parseGPVTG / parseGPGSA  → GpsData 累积
        → parseGPGSV  → QList<SatelliteInfo> 累积 (独立于 GpsData)
          → 设置 sat.system = constellationFromTalkerId(talkerId)
      → emit positionUpdated(m_lastData)            ← 不含卫星列表！
    → GpsManager.onPositionUpdated(data)
      → emit positionChanged()                      ← QML 属性刷新
      → emit newPosition(data)                      ← TrackRecorder / GeofenceManager
      → m_satelliteDebounceTimer.start()            ← 防抖 300ms
  ... (GSV 序列完成、300ms 无新数据后)
  → GpsManager.onSatelliteDebounceTimeout()
    → 比对 parser.satellitesInView() != m_satellitesInView
    → 更新 m_satellitesInView，emit satelliteInfoChanged()
      → QML SatellitePanel: 一次查询 satellitesInView() → 缓存到 property
        ├─ SkyPlot Canvas   (读取缓存)
        ├─ SNR Bar Chart    (读取缓存)
        └─ ListModel        (读取缓存)
```

## 重要实现细节

- **校验和验证**：`NMEAParser::validateChecksum()` 对 `$` 和 `*` 之间的语句体计算 XOR 校验和。调用方（`parseSentence`）已先剥离 `$` 前缀，因此 `validateChecksum` 使用 `sentence.left(starIdx)` 而非 `mid(1)`。
- **跨语句数据合并**：`feed()` 以累积状态 `m_lastData` 为起点解析每条语句（`GpsData parsed = m_lastData`），各语句只更新自己负责的字段。`parseGPRMC` 的 `isValid = active && (fixQuality > 0 || satelliteCount > 0)` 依赖 GGA 先设的 fixQuality/satelliteCount，因此需按 GGA → RMC 顺序发送。
- **卫星数据分离与防抖**：`SatelliteInfo` 已从 `GpsData` 中独立，由 `NMEAParser` 和 `GpsManager` 各自持有缓存。GSV 语句通常是多段消息（3-5 条）描述完整卫星列表，`GpsManager` 使用 300ms 单次定时器防抖，等待 GSV 序列完整到达后再发射 `satelliteInfoChanged()` 通知 QML。这避免了 QML 在卫星列表不完整时就重绘天球图和 SNR 柱状图。`satellitesInView()` 输出时按 SNR 降序排列。
- **QML/C++ 桥接**：核心对象通过上下文属性（非类型注册）暴露。使用 `Q_PROPERTY` 配合 NOTIFY 信号实现响应式绑定。MapDisplay 通过原生 `Binding`/属性绑定直接响应 `GpsManager` 属性变化，无需 `runJavaScript()` 中转。
- **Qt 版本**：项目目标为 `/home/pt/Qt/6.7.3/gcc_64` 下的 Qt 6.7.3。使用模块：Core、Quick、Qml、Positioning、Location、SerialPort、Network。系统自带的 Qt 6.2.4 缺少 QtLocation 等关键模块，**构建前务必删除旧的 `CMakeCache.txt` 重新配置**。
- **qt.conf 机制**：CMake 构建时自动从 `src/app/qt.conf.in` 生成 `qt.conf` 并复制到可执行文件旁，告知 Qt 运行时资源位置（QML 模块、插件等）。用户只需设置 `LD_LIBRARY_PATH` 即可运行。
- **地图方案**：使用 QtLocation `Map` + OSM Plugin（`name: "osm"`）显示 OpenStreetMap 瓦片。`MapDisplay.qml` 封装 Map 并添加 PinchHandler/DragHandler/WheelHandler 手势交互和自定义覆盖控件。无需 API Key，无 WebEngine 内存开销。
