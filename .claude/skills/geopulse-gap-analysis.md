---
name: geopulse-gap-analysis
description: 分析 GeoPulse 项目当前状态，识别与行业成熟 GPS 监控产品之间的功能差距，按优先级给出改进建议。
category: project
---

# GeoPulse 需求缺口分析 Skill

分析 GeoPulse 项目的功能完备度，识别缺失需求并按优先级给出改进路线图。

## 执行流程

1. 快速扫描项目关键文件确认当前基线（尤其是 QML 文件、core/ 头文件、测试目录）
2. 与下方记录的缺口清单对照，确认哪些已修复、哪些仍然存在
3. 输出分优先级的增量改进建议

---

## 项目当前基线（v1.0 MVP 已实现）

### 核心数据流 ✅
GPS 设备驱动（Serial/TCP/UDP）→ NMEA 0183 解析（GGA/RMC/VTG/GSA/GSV）→ GpsManager 协调 → TrackRecorder 录制 + GeofenceManager 围栏检测

### QML 界面（6 个面板）✅
- **main.qml** — 暗色主题主窗口，工具栏含连接状态/定位质量/卫星数
- **MapDisplay.qml** — QtLocation OSM 地图，位置标记/精度圈/轨迹线/围栏多边形，跟随模式，手势交互
- **Dashboard.qml** — 位置面板 + 速度仪表条 + 信号质量面板
- **SatellitePanel.qml** — 天空图 Canvas + SNR 柱状图 + 卫星列表（⚠ 占位数据）
- **TrackPanel.qml** — 录制控制 + 统计信息 + GPX/CSV 导出 FileDialog
- **SettingsPanel.qml** — 连接配置 + 地图图层（含 Satellite TODO）+ 日志级别 + 关于

### 底层库 ✅
- **libs/gpscomm** — IGPSDeviceDriver 抽象接口 + Serial/TCP/UDP 三种驱动
- **libs/gpsutils** — NMEAParser + GeoUtils + ConfigManager + Logger

### 测试 ✅
- test_nmea_parser（8 用例）、test_geo_utils（12 用例）、test_config_manager（6 用例）、test_driver_factory（8 用例）

---

## 缺口清单（按优先级分层）

### 🔴 P0 — 致命缺口（影响核心可用性）

| # | 缺口 | 现状 | 建议 |
|---|------|------|------|
| 1 | **卫星数据未接入 UI** | `GpsData::satellitesInView` 数据已解析但未通过 GpsManager 暴露给 QML；天空图/SNR 柱状图/卫星列表全部使用 `Math.random()` 占位数据（见 `SatellitePanel.qml:82` `TODO`） | 在 GpsManager 中新增 `Q_PROPERTY(QVariantList satellitesInView)`，或注册 `QAbstractListModel`；天空图用真实 azimuth/elevation 计算极坐标 |
| 2 | **轨迹无持久化** | TrackRecorder 仅内存存储，关闭即丢失；无自动保存、无历史轨迹恢复 | 增加 autoSave/ savePath 属性；退出时自动写 GPX 到 `~/.geopulse/tracks/`；支持打开历史轨迹 |
| 3 | **无告警通知系统** | GeofenceManager 有 enteredGeofence/exitedGeofence 信号但上层无人消费；无超速/断连/DOP超限/低电量告警；无声音/系统通知 | 新增 AlertManager 类；支持围栏/超速/断连/DOP 超限告警；声音+状态栏闪烁+系统托盘通知；告警历史列表 |

### 🟠 P1 — 重要缺口（限制产品化交付）

| # | 缺口 | 现状 | 建议 |
|---|------|------|------|
| 4 | **无轨迹导入** | 只能导出 GPX/CSV，不能导入；用户无法加载历史或第三方轨迹 | TrackRecorder 增加 `importFromGpx()` / `importFromCsv()` |
| 5 | **无轨迹回放** | 轨迹仅显示静态折线，无法按时间轴回放历史移动过程 | 新增 TrackPlayer 类，支持按时间戳逐点回放、可调速(1x/2x/5x/10x)、地图跟随 |
| 6 | **无航点管理** | 无法在地图上标记/命名/管理 Waypoint | 新增 WaypointManager，支持增删改查 + GPX waypoint 导入导出 |
| 7 | **无时间序列图表** | 有数值显示但无趋势图：高程剖面、速度曲线、卫星信噪比历史均缺失 | 可用 QtCharts 或 Canvas 实现折线图；Dashboard 增加图表切换 |
| 8 | **围栏无交互编辑** | 只能通过 C++ addGeofence() 接口添加；QML 侧无任何绘制/编辑工具；围栏定义无持久化 | 地图上支持点击绘制多边形；围栏顶点可拖拽；支持 GeoJSON 导入导出；配置文件自动保存恢复 |
| 9 | **无坐标格式切换** | 全部仅显示十进制度(DD)；缺少 DMS/UTM/MGRS | 新增 CoordinateFormatter 工具类；Dashboard 增加格式切换下拉框 |
| 10 | **录制参数不可配** | addPoint() 来者不拒，无最小间隔(时间/距离)、无精度过滤(HDOP阈值)、无定位质量过滤(fixQuality阈值) | TrackRecorder 增加 minInterval/maxHdop/minFixQuality 属性 |
| 11 | **窗口状态不保存** | 窗口大小/位置/面板分割比/当前标签页重启即重置 | 通过 ConfigManager 在退出时保存窗口几何，启动时恢复 |
| 12 | **无键盘快捷键** | 所有操作依赖鼠标 | Ctrl+R 录制、Ctrl+S 导出、Ctrl++/- 缩放、Space 跟随模式 |

### 🟡 P2 — 增强缺口（影响用户体验和国际化）

| # | 缺口 | 现状 | 建议 |
|---|------|------|------|
| 13 | **轨迹无速度着色** | 轨迹线单一橙色 `#ff9800`，不区分速度 | 按速度分段着色（绿→黄→红），直观展示快慢段 |
| 14 | **无视觉罗盘** | 航向仅以文字显示角度和方位 | Dashboard 增加圆形罗盘刻度盘，指针指向当前航向 |
| 15 | **无主题切换** | 仅暗色主题 `#1a1a2e`，户外强光下可读性差 | 支持亮色/暗色/自动跟随系统主题 |
| 16 | **无国际化 i18n** | 所有 UI 文本硬编码中文 | 使用 `qsTr()` + `QTranslator`，至少中/英文 |
| 17 | **卫星图层未实现** | SettingsPanel 标注 `"Satellite (TODO)"` | 接入 ESRI World Imagery 或用户自定义 XYZ 瓦片 URL |
| 18 | **无离线地图** | 完全依赖在线 OSM 瓦片，无网络不可用 | 集成 MBTiles 或本地瓦片缓存 |
| 19 | **无计量单位切换** | 全部硬编码公制（km/h、米） | 支持 Metric/Imperial/Nautical，Dashboard/TrackPanel 统一响应 |
| 20 | **无 NMEA 日志记录** | "Show raw NMEA" 仅控制台输出，无法保存到文件 | Logger 增加 NMEA 专用通道，按文件大小自动滚动 |
| 21 | **无连接预设** | 每次启动手动输入地址/端口 | ConfigManager 保存多个连接预设，SettingsPanel 增加预设下拉选择 |

### 🔵 P3 — 进阶缺口（长期路线图）

| # | 缺口 | 说明 |
|---|------|------|
| 22 | 多设备同时接入 | 同一界面显示多个 GPS 源（车队管理） |
| 23 | 蓝牙 GPS 驱动 | 通过 Qt Bluetooth 模块连接蓝牙 GNSS 接收器 |
| 24 | WebSocket/MQTT 数据源 | 支持 IoT 协议，适配现代 GPS 追踪器 |
| 25 | NTRIP RTK 校正客户端 | 接收差分改正数据实现厘米级定位 |
| 26 | NMEA 文件回放驱动 | 从 NMEA 日志文件读取数据作为虚拟驱动源 |
| 27 | 轨迹编辑工具 | 裁剪、分割、合并、删除异常点 |
| 28 | 围栏动作自动化 | 进出围栏时执行动作（HTTP 请求、脚本） |
| 29 | 语音播报 | 状态变化时 TTS 语音提示 |
| 30 | A-GPS 辅助星历 | 通过网络下载星历加速首次定位 |
| 31 | 路线规划与导航 | A→B 路径规划 + 逐向导航 |
| 32 | 3D 地形视图 | 使用高程数据渲染 3D 地形 |
| 33 | 系统托盘最小化 | 后台运行 + 气泡通知 |
| 34 | 远程遥测服务端 | 作为 TCP/HTTP 服务端供远程客户端订阅位置数据 |

### ⚪ 测试债务

| # | 缺口 | 说明 |
|---|------|------|
| 35 | 核心层 0 测试 | GpsManager / TrackRecorder / GeofenceManager / GeoPulseApp 完全无单元测试 |
| 36 | QML UI 无测试 | 无任何 UI 自动化测试 |

---

## 改进路线图建议

### 第一阶段 → 可交付内部试用版
修复 #1（卫星数据接入 UI）+ #2（轨迹持久化）+ #3（告警通知系统）

### 第二阶段 → 可对外发布
#4（轨迹导入）+ #5（轨迹回放）+ #7（高程/速度图表）+ #8（围栏地图编辑）+ #9（坐标格式）+ #10（录制参数配置）

### 第三阶段 → 完善产品体验
#6（航点管理）+ #11（窗口状态恢复）+ #12（键盘快捷键）+ #16（i18n）+ #18（离线地图）+ #15（主题切换）

### 第四阶段 → 行业标杆
#22-#34 全部 P3 项 + #35-#36 测试补充

---

## 使用方式

当用户询问"项目还缺什么"、"有哪些改进空间"、"下一步做什么"等问题时，按以下步骤执行：

1. **快速扫描**：Read 关键文件确认当前基线有无变化（优先检查：SatellitePanel.qml、GpsManager.h、TrackRecorder.h、GeofenceManager.h、SettingsPanel.qml、tests/CMakeLists.txt）
2. **对照缺口清单**：逐项检查 P0/P1 缺口是否已被修复，更新缺口状态
3. **输出增量建议**：仅列出仍未修复的缺口，按 P0→P1→P2→P3 优先级排列，给出具体的文件级修改建议
4. **如果用户无明确范围**：默认输出 P0/P1 层级；如果用户表示要做"全面"分析，则输出全部五层
