---
name: geopulse-bugfix
description: GeoPulse 项目 Bug 修复与修复记录。当需要修复 Bug 或查阅修复历史时加载。
category: project
---

# GeoPulse Bug 修复 Skill

GeoPulse 项目专用 Bug 诊断修复流程，以及带时间戳的修复记录。

## 执行流程

1. **理解 Bug 现象** — 明确复现步骤和预期 vs 实际行为
2. **定位根因** — 根据数据流路径（GPS设备 → 驱动 → NMEA解析 → GpsManager → QML）逐层排查
3. **设计修复** — 确保改动最小化且遵循项目现有模式（Q_PROPERTY / Q_INVOKABLE / QVariantList 桥接等）
4. **验证修复** — 构建 `cmake --build build -j$(nproc)` + 测试 `cd build && ctest --output-on-failure` + 启动验证
5. **记录修复** — 更新本文件底部修复记录

---

## 项目架构速查

```
GPS 设备 → driver.rawDataReady(bytes)
  → GpsManager.onRawDataReady → NMEAParser.feed()
    → NMEAParser::parseSentence() 逐行解析
    → 发送 positionUpdated(GpsData) + satelliteInfoChanged()
  → GpsManager.onPositionUpdated(data)
    → QML 属性绑定更新 + TrackRecorder.addPoint + GeofenceManager.checkPosition
```

### C++ → QML 桥接模式
- 上下文属性: `ctx->setContextProperty("GpsManager", ...)` → QML 中直接 `GpsManager.latitude`
- 列表数据: `Q_INVOKABLE QVariantList methodName()` 返回 `QVariantMap` 列表（参考 `TrackRecorder::simplifiedTrack()`、`GeofenceManager::fenceList()`）
- 枚举: `Q_ENUM(Level)` + `Q_INVOKABLE void setLogLevel(int level)` int 桥接

### 关键文件

| 层 | 文件 | 职责 |
|----|------|------|
| QML | `qml/main.qml` | 主窗口、TabBar/SwipeView、通知浮层 |
| QML | `qml/MapDisplay.qml` | 地图、定位标记、轨迹线、手势交互 |
| QML | `qml/SatellitePanel.qml` | 天空图/SNR柱状图/卫星列表 |
| QML | `qml/SettingsPanel.qml` | 连接配置、日志级别、自动跟随 |
| QML | `qml/Dashboard.qml` | 位置/速度/信号质量面板 |
| QML | `qml/TrackPanel.qml` | 轨迹录制/统计/导出 |
| Core | `src/core/gps_manager.h/.cpp` | GPS 数据管理中心，暴露 Q_PROPERTY |
| Core | `src/core/track_recorder.h/.cpp` | 轨迹录制与 GPX/CSV 导出 |
| Core | `src/core/geofence_manager.h/.cpp` | 围栏管理与进出检测 |
| App | `src/app/geopulse_app.h/.cpp` | 应用根对象，持有并串联核心管理器 |
| App | `src/app/main.cpp` | 入口 + QML 上下文属性暴露 |
| Lib | `libs/gpsutils/nmea_parser.h/.cpp` | NMEA 0183 解析器（GGA/RMC/VTG/GSA/GSV） |
| Lib | `libs/gpsutils/geo_utils.h/.cpp` | 地理计算工具 |
| Lib | `libs/gpsutils/config_manager.h/.cpp` | JSON 配置管理 |
| Lib | `libs/gpsutils/logger.h/.cpp` | 分级日志系统 |

---

## 常见 Bug 模式

### 1. QML 数据不显示
- 检查 C++ 是否通过 `Q_PROPERTY` 或 `Q_INVOKABLE` 暴露数据
- 检查 `NOTIFY` 信号是否正确触发
- 检查 QML 绑定表达式是否引用了正确的属性名

### 2. 列表数据重复/不刷新
- Python/JS list 在 QML 中是引用类型，`var x = list; x.push(item); list = x` 可能不触发变更通知
- 使用 `[...list, newItem]` 创建新引用确保触发更新
- C++ 侧检查是否存在累积追加（如 `data.list.append()` 而非先 `clear()`）

### 3. UI 状态不同步
- TabBar ↔ SwipeView：需要双向绑定 `currentIndex`
- 启动自动重连后 SettingsPanel 不匹配：需要在 `onConnectedChanged` 中同步控件状态
- 跨组件状态：考虑提升到 `main.qml` 根属性或通过 `Connections` 监听信号

### 4. QML 类型未识别
- `property coordinate` → 使用 `property var`（`coordinate` 非值类型）
- `QtPositioning.coordinate(lat, lon)` 创建坐标值
- QtLocation 的 `Map` 类型可通过 `import QtLocation` 直接使用

### 5. 命名冲突
- QML 文件名与模块类型同名会冲突（如 `MapView.qml` vs `QtLocation.MapView`）
- 重命名本地文件消除冲突

---

## 修复记录

### 2026-06-10
| Bug | 根因 | 修复 |
|-----|------|------|
| 定位点与轨迹无法在地图显示 | ① `MapView.qml` 与 `QtLocation.MapView` 命名冲突 ② `MapQuickItem`/`MapCircle`/`MapPolyline` 作为 `Item` 子项而非底层 `Map` 子项，不被地图渲染 | 重命名为 `MapDisplay.qml`，直接使用 `Map {}` 使覆盖物自动挂载为 Map 子项；添加 PinchHandler/DragHandler/WheelHandler 替代 MapView 手势 |

### 2026-06-11
| Bug | 根因 | 修复 |
|-----|------|------|
| SatellitePanel 显示假数据（Math.random） | `GpsData::satellitesInView` 已解析但 `GpsManager` 无属性暴露；卫星列表/天空图/SNR全部使用占位随机数 | GpsManager 新增 `Q_INVOKABLE satellitesInView()` → QVariantList；SatellitePanel 三处 Canvas + Repeater 改用真实数据 |
| saveConfig 只保存 type 不保存 address/baudRate | `GeoPulseApp::saveConfig()` 仅写入 `connection.type`，导致重启无法自动重连 | GeoPulseApp 新增 `Q_INVOKABLE connectToSource()` → 同时写入 ConfigManager 三字段 + 存储成员变量 |
| 围栏进出无用户通知 | `GeofenceManager::enteredGeofence/exitedGeofence` 信号无人监听 | main.qml 添加 Toast 浮层 + Timer 3s 自动消失 + Connections 监听 |
| SettingsPanel 控件无实际效果 | 自动跟随/日志级别/NMEA控制台三个控件未接线 | 自动跟随绑定 `mapDisplay.followMode`；Logger 暴露到 QML + `setLogLevel(int)`；NMEA控制台绑定 `console.log` |
| 取消跟随后定位点消失 | `Binding { delayed: true }` 生命周期竞态导致切换时地图中心异常 | 移除 Binding，在 Timer 中直接 `if (followMode) qtMap.center = ...` |
| 轨迹线不刷新 | `var p = trackLine.path; p.push(...); trackLine.path = p` 相同引用不触发 QML 属性通知 | 改用 `trackLine.path = [...currentPath, newCoord]` 创建新引用 |
| 卫星数据重复累积 | `parseGPGSV` 中 `data.satellitesInView.append(sat)` 但从不 clear，每轮 GSV 周期追加而非覆盖 | `messageNumber == 1` 时先 `data.satellitesInView.clear()` |
| TabBar 滑动后按钮未同步 | SwipeView → TabBar 单向绑定 | `onCurrentIndexChanged: sideTabBar.currentIndex = sideStack.currentIndex` |
| 启动自动重连后 SettingsPanel 不同步 | `loadConfig()` 连接成功但 UI 仍显示默认串口模式 | GpsManager 新增 `currentAddress`/`currentBaudRate` Q_PROPERTY；SettingsPanel 在 `onConnectedChanged` 中同步 driverTypeCombo/addressField/baudCombo |
