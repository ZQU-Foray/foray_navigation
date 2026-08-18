# 自研导航栈架构设计方案（借鉴 pb2025_sentry_nav）

> 版本：v1.0
> 面向：ZQU-Foray 战队导航组
> 前提：硬件与源仓库同款（Livox Mid360 + IMU + 全向底盘 + 云台）、仿真优先、队伍为 ROS2 完全新手
> 参考仓库：[SMBU-PolarBear-Robotics-Team/pb2025_sentry_nav](https://github.com/SMBU-PolarBear-Robotics-Team/pb2025_sentry_nav)（即本 fork 的上游）

---

## 0. 文档说明

本文档解决三个问题：

1. **看懂**：源仓库（pb2025_sentry_nav）的导航系统是怎么分层、怎么协作的；
2. **借鉴**：我们自己的导航栈应该抄哪些、改哪些、丢哪些、自己写哪些；
3. **落地**：从零基础到仿真跑通再到实车，分阶段怎么做，每阶段怎么验收。

> 本文档假定你手里有本仓库的完整代码（含已初始化的 submodule）。先读一遍仓库 `README.md` 再读本文档效果最好。

---

## 1. 总体目标与设计原则

### 1.1 目标

搭建一套"哨兵/云台机器人"自主导航栈，能力与源仓库对齐：

- 在 2D 栅格地图中自主导航（RViz 点目标 → 机器人走到目标点并避障）
- 支持两种模式：**建图模式**（SLAM 生成地图）与 **导航模式**（加载地图 + 重定位）
- 感知障碍物与地形（上坡、绕坑），不把坡当墙
- 云台自旋扫描时仍能稳定跟踪路径（伪坐标系方案）
- 同一套代码同时支撑仿真（Gazebo）与实车（仅配置不同）

### 1.2 设计原则（源仓库的精华）

| 原则 | 源仓库体现 | 我们的做法 |
|---|---|---|
| **分层解耦** | 感知/里程计/定位/地形/规划各是独立包 | 完全沿用，每个包一个职责 |
| **接口先行** | 包之间只通过 topic/TF 通信 | 先定接口（见 §4），再填实现 |
| **仿真=实车** | `config/simulation/` 与 `config/reality/` 两套参数，代码相同 | 沿用双配置目录 |
| **多机器人就绪** | 所有节点带 namespace | 沿用，即使目前只有一台车 |
| **免重编译开发** | `colcon build --symlink-install` | 沿用，launch/yaml 改动即时生效 |
| **先跑通再优化** | 默认参数即可动起来 | 路线图按"先跑通→再调好"排 |

---

## 2. 总体分层架构

```
┌────────────────────────────────────────────────────────────────────┐
│ 第 7 层  交互与装配层（launch 文件 / RViz / 手柄）                  │
│          pb2025_nav_bringup  ← 系统"总开关"，只做组装不写算法      │
├────────────────────────────────────────────────────────────────────┤
│ 第 6 层  运动接口层（让 NAV2 适应"会自旋的云台底盘"）               │
│          fake_vel_transform（伪坐标系）                            │
│          pb_omni_pid_pursuit_controller（全向 PID 跟踪插件）       │
│          pb_nav2_plugins（IntensityVoxelLayer / BackUpFreeSpace）  │
├────────────────────────────────────────────────────────────────────┤
│ 第 5 层  规划控制层（NAV2 标准栈，全部用官方组件）                  │
│          planner(Theta*) → smoother → controller(全向PID)          │
│          behavior(自旋/倒车) / bt_navigator(行为树) /              │
│          waypoint_follower / velocity_smoother / costmap×2         │
├────────────────────────────────────────────────────────────────────┤
│ 第 4 层  环境感知层（把"能不能走"变成代价）                         │
│          terrain_analysis（≤4m，局部代价）                         │
│          terrain_analysis_ext（>4m，全局代价）                     │
│          pointcloud_to_laserscan（仅建图模式：点云→2D扫描）        │
├────────────────────────────────────────────────────────────────────┤
│ 第 3 层  定位层（我在哪）                                          │
│          导航模式：small_gicp_relocalization（先验点云重定位）     │
│          建图模式：slam_toolbox（2D SLAM，生成栅格地图）           │
├────────────────────────────────────────────────────────────────────┤
│ 第 2 层  状态估计层（我怎么动的）                                   │
│          point_lio（雷达+IMU 里程计，内部用 lidar_odom 系）        │
│          loam_interface（lidar_odom → odom 坐标系转换）            │
│          sensor_scan_generation（发布 odom→底盘 TF + 底盘里程计）  │
├────────────────────────────────────────────────────────────────────┤
│ 第 1 层  驱动层（原始数据从哪来）                                   │
│          实车：livox_ros_driver2（Mid360 驱动）                    │
│          仿真：Gazebo 点云 → ign_sim_pointcloud_tool（补 ring/time）│
└────────────────────────────────────────────────────────────────────┘
```

### 2.1 导航模式完整数据流

```
[仿真] Gazebo 点云(livox/lidar) ──► ign_sim_pointcloud_tool ──► velodyne_points
[实车] Mid360 ──► livox_ros_driver2 ─────────────────────────► velodyne_points
                                                                     │
                                                    point_lio（里程计，lidar_odom 系）
                                                                     │ cloud_registered / aft_mapped_to_init
                                                    loam_interface（转换到 odom 系）
                                                                     │ registered_scan / lidar_odometry
                                                    sensor_scan_generation（TF+里程计）
                                                                     │ sensor_scan
                              ┌──────────────────────────────────────┴──────────────┐
                              ▼                                                     ▼
                terrain_analysis（≤4m）                                terrain_analysis_ext（>4m）
                              │ terrain_map                                      │ terrain_map_ext
                              └───────────────────┬──────────────────────────────┘
                                                  ▼
                    local_costmap（IntensityVoxelLayer）   global_costmap（IntensityVoxelLayer）
                                                  │
                    small_gicp 重定位 ──► map→odom TF（导航模式）/ 静态 TF（建图模式）
                                                  │
                    NAV2：bt_navigator → planner → smoother → controller（全向PID）
                                                  │ cmd_vel（基于 gimbal_yaw_fake 伪坐标系）
                                                  ▼
                    fake_vel_transform（变换回真实 gimbal_yaw 系，可叠加 cmd_spin 自旋）
                                                  │ cmd_vel
                                                  ▼
                                                 底盘驱动
```

### 2.2 建图模式数据流（差异部分）

```
sensor_scan ──► terrain_analysis_ext ──► terrain_map_ext ──► pointcloud_to_laserscan
                                                                   │ obstacle_scan
                                                                   ▼
                                           slam_toolbox（2D SLAM，静态发布 map→odom）
                                                                   │
                                             保存地图：nav2_map_server map_saver_cli
```

> 核心差异：建图模式**不启动** small_gicp（没有先验地图），改用 slam_toolbox 实时建 2D 栅格地图；定位模式**不启动** slam_toolbox，用先验 PCD + small_gicp 重定位。这个开关就是 launch 参数 `slam:=True/False`。

---

## 3. 坐标系与 TF 设计（最重要的一章）

源仓库花了最多精力解决坐标系问题，这也是新手最容易崩的地方。

### 3.1 Frame 一览表

| Frame | 父系 | 谁发布 | 作用 |
|---|---|---|---|
| `map` | —（根） | 重定位/静态 TF | 世界系（地图系），全局规划用 |
| `odom` | `map` | 重定位节点/slam | 里程计系，**底盘真实的运动基准** |
| `lidar_odom` | —（内部） | point_lio 内部 | 雷达自己的里程计原点，**与 odom 有隐式偏移**（雷达倾斜侧装） |
| `base_footprint` | `odom` | sensor_scan_generation | 底盘原点（chassis），nav2 默认 base_frame |
| `gimbal_yaw` | `base_footprint` | robot_state_publisher / 实车串口 | 云台偏航关节系，**会自旋** |
| `gimbal_yaw_fake` | `gimbal_yaw` | fake_vel_transform | **伪坐标系**：位置同 gimbal_yaw，但 yaw 固定不随云台转，给 NAV2 用 |
| `front_mid360` | `gimbal_yaw` | robot_state_publisher | 雷达安装系（雷达装在云台上，随云台转） |

### 3.2 TF 树

```
map
 └── odom                     ← small_gicp（导航）/ slam_toolbox 静态（建图）
      └── base_footprint      ← sensor_scan_generation（由 lidar_odom 推算）
           └── gimbal_yaw     ← robot_state_publisher（关节）
                ├── gimbal_yaw_fake   ← fake_vel_transform（yaw 锁死）
                └── front_mid360      ← 静态外参（雷达安装位姿）

（point_lio 内部另有独立子树：lidar_odom → front_mid360，不参与主树）
```

### 3.3 三个必须理解的坑（源仓库为什么这么设计）

1. **`lidar_odom` ≠ `odom`**：point_lio 把雷达原点当里程计原点，但雷达是**倾斜侧装**在底盘上的，两者原点不重合。`loam_interface` 负责用 `base_footprint→front_mid360` 的静态外参，把 point_lio 输出变换到真正的 `odom` 系。**新手常见的错误**：直接用 point_lio 的 TF，导致底盘"飘"。

2. **`odom → base_footprint` 由谁发**：不是 point_lio 直接发，而是 `sensor_scan_generation` 时间同步订阅（里程计+点云）后，用 `T(odom→lidar) * T(lidar→chassis)` 推算并发布，同时给出底盘里程计（`odometry` 话题，NAV2 的 odom 源）。它还把点云从雷达系变换到 `odom` 系发布为 `sensor_scan`。

3. **为什么要 `gimbal_yaw_fake`**：NAV2 局部规划器假设"机器人朝向 = 路径前进方向"。云台自旋时 `gimbal_yaw` 的方向乱转，NAV2 会认为机器人在打转、无法前进。方案：给 NAV2 一个 yaw 固定的假坐标系 `gimbal_yaw_fake` 作为 `robot_base_frame`（costmap、bt_navigator、behavior_server 都指它），NAV2 输出的速度指令再由 `fake_vel_transform` 变换回真实的 `gimbal_yaw` 系下发给底盘。自旋速度通过 `cmd_spin` 话题叠加。

> 调试口诀：**TF 树不对，一切白搭**。任何阶段先确认 `ros2 run rqt_tf_tree rqt_tf_tree`（或 `view_frames`）里这棵树是完整的、无跳变。

---

## 4. 话题与接口清单（数据接口契约）

所有话题都在 namespace 下（默认 `/red_standard_robot1/`）。下表是"接口契约"——我们的栈必须维持这些接口不变，包内部随便改。

### 4.1 感知 / 里程计

| 话题 | 类型 | 发布者 | 订阅者 |
|---|---|---|---|
| `velodyne_points` | PointCloud2 | 驱动 / ign_sim_pointcloud_tool | point_lio |
| `cloud_registered` | PointCloud2 | point_lio | loam_interface、terrain_analysis_ext |
| `aft_mapped_to_init` | Odometry | point_lio | loam_interface |
| `registered_scan` | PointCloud2 | loam_interface | sensor_scan_generation、terrain_analysis(_ext) |
| `lidar_odometry` | Odometry | loam_interface | sensor_scan_generation、terrain_analysis(_ext) |
| `sensor_scan` | PointCloud2 | sensor_scan_generation | terrain_analysis / terrain_analysis_ext |
| `odometry` | Odometry | sensor_scan_generation | NAV2（odom 源）、fake_vel_transform |

### 4.2 地形 / 代价

| 话题 | 类型 | 发布者 | 订阅者 |
|---|---|---|---|
| `terrain_map` | PointCloud2 | terrain_analysis | local_costmap（observation_sources） |
| `terrain_map_ext` | PointCloud2 | terrain_analysis_ext | global_costmap、pointcloud_to_laserscan |
| `obstacle_scan` | LaserScan | pointcloud_to_laserscan | slam_toolbox（仅建图模式） |

### 4.3 导航 / 控制

| 话题 | 类型 | 发布者 | 订阅者 |
|---|---|---|---|
| `cmd_vel_controller` | Twist | controller_server | velocity_smoother |
| `cmd_vel_nav2_result` | Twist | velocity_smoother / behavior_server | fake_vel_transform、bt_navigator |
| `cmd_vel` | Twist | fake_vel_transform | 底盘 |
| `cmd_spin` | Float32 | 决策层（自定义） | fake_vel_transform（叠自旋） |
| `local_plan` | Path | controller_server | fake_vel_transform（取时间戳） |
| `joy` | Joy | joy_node | teleop、terrain_analysis（清地图） |

### 4.4 Action 接口（NAV2 对外服务）

| Action | 类型 | 说明 |
|---|---|---|
| `navigate_to_pose` | NavigateToPose | RViz Nav2 Goal 走的接口 |
| `navigate_through_poses` | NavigateThroughPoses | 多目标点 |
| `follow_waypoints` | FollowWaypoints | waypoint_follower 巡线 |

> 我们自己的决策层（比如哨兵巡逻逻辑）未来就是调用 `navigate_to_pose` / `follow_waypoints`，所以**这一层接口一定要稳定**。

---

## 5. 借鉴什么 / 简化什么 / 自己写什么

### 5.1 直接复用（与源仓库完全同款硬件，几乎不用改）

| 包 | 用途 | 备注 |
|---|---|---|
| `livox_ros_driver2` | 雷达驱动（实车） | submodule |
| `ign_sim_pointcloud_tool` | 仿真点云补 ring/time | 本地代码 |
| `point_lio` | 里程计 | submodule，注意用 RM2025_SMBU_auto_sentry 分支 |
| `loam_interface` | lidar_odom→odom | 本地代码，100 行，值得精读 |
| `sensor_scan_generation` | TF+底盘里程计 | 本地代码，160 行，值得精读 |
| `terrain_analysis` / `terrain_analysis_ext` | 地形代价 | 本地代码（源自北理 RMUA 开源） |
| `small_gicp_relocalization` | 重定位 | submodule |
| `pb_omni_pid_pursuit_controller` | 全向 PID 跟踪 | submodule，控制核心 |
| `pb_nav2_plugins` | costmap 插件 + 倒车行为 | submodule |
| `fake_vel_transform` | 伪坐标系 | 本地代码，30 行注释文档，值得精读 |
| `pb_teleop_twist_joy` | 手柄 | submodule |
| `slam_toolbox` | 建图 | ROS 官方包 |

### 5.2 必须自己做的（无法从仓库拷）

| 事项 | 说明 | 什么时候做 |
|---|---|---|
| **地图** | 用建图模式自己扫（rmuc/rmul 的图是别人家的，规则每届变） | Phase 3 |
| **先验点云** | 重定位用的 PCD，自己扫场景生成 | Phase 3 |
| **外参标定** | 雷达相对云台/底盘的安装位姿（`front_mid360` 静态 TF） | 上实车前 |
| **调参** | terrain/costmap/控制器参数按自己车调 | Phase 4 |
| **launch 定制** | 换 namespace、地图路径、传感器配置 | Phase 2 |
| **决策逻辑** | 哨兵巡逻/攻击/回补等高层逻辑（不在本仓库内） | Phase 6 |

### 5.3 可以延后/砍掉的

| 内容 | 原因 |
|---|---|
| 多机器人 launch（`rm_multi_navigation_simulation_launch.py`） | 官方标注实验性，单机先跑通 |
| prior_pcd 先验建图优化 | README 注明"大场景效果不好，容易飘"，先用不带先验的纯 point_lio |
| 复杂恢复行为（BackUpFreeSpace 深度调参） | 先保证直线/弯道跟踪，再调恢复 |

---

## 6. 推荐工作空间结构（复制改造方案）

> 结论先行：**不要从零写**。你们硬件与源仓库同款，最理性的路径是把这个 fork 作为基底复制改造——这正好也是你们 fork 它的意义。等跑通、改熟之后，再逐步用自己代码替换子模块。

```
~/ros_ws/
├── src/
│   └── YOUR_TEAM_nav/              # 你们的导航栈（从本 fork 复制后改名）
│       ├── YOUR_TEAM_nav_bringup/  # 启动文件+地图+参数（改名、换 namespace）
│       ├── loam_interface/         # 保留（坐标系转换）
│       ├── sensor_scan_generation/ # 保留（TF）
│       ├── terrain_analysis/       # 保留（地形）
│       ├── terrain_analysis_ext/   # 保留（地形扩展）
│       ├── fake_vel_transform/     # 保留（伪坐标系）
│       ├── ign_sim_pointcloud_tool/# 保留（仿真点云）
│       ├── point_lio/              # submodule 保留
│       ├── livox_ros_driver2/      # submodule 保留
│       ├── small_gicp_relocalization/ # submodule 保留
│       ├── pb_omni_pid_pursuit_controller/ # submodule 保留
│       ├── pb_nav2_plugins/        # submodule 保留
│       ├── pb_teleop_twist_joy/    # submodule 保留
│       └── pointcloud_to_laserscan/# submodule 保留（建图用）
└── install/ build/ log/            # colcon 输出
```

**改名清单（Phase 2 做）**：

| 项目 | 原值 | 改成 |
|---|---|---|
| 元包名 | `pb2025_sentry_nav` | `YOUR_TEAM_nav` |
| bringup 包名 | `pb2025_nav_bringup` | `YOUR_TEAM_nav_bringup` |
| namespace | `red_standard_robot1` | 你们自己的（如 `sentry1`） |
| launch 文件名 | `rm_navigation_*` | 按自己习惯 |
| config 目录 | `simulation/reality` | 保留双配置结构 |

> 注意：包改名要同步改 `package.xml`、`CMakeLists.txt`、所有 launch 里的包名引用、`nav2_params.yaml` 里 `$(find-pkg-share ...)` 路径。

---

## 7. 技术选型与备选方案

| 功能 | 本仓库选择 | 备选 | 建议 |
|---|---|---|---|
| 里程计 | point_lio（紧耦合 LiDAR-IMU） | fast-lio2、liom、cartographer | **沿用 point_lio**（与仓库配套、效果验证过） |
| 建图 | slam_toolbox（2D 栅格） | 点云建图（lio-sam） | 沿用；比赛用 2D 图足够，且与 NAV2 无缝 |
| 重定位 | small_gicp（点云全局配准） | AMCL（2D 激光）、PCL NDT | 沿用 small_gicp（与点云链路一致） |
| 全局规划 | nav2_theta_star（Theta*） | navfn、SmacPlanner | 沿用 Theta*（可穿越斜角、代价可控） |
| 局部控制 | pb_omni_pid_pursuit（全向 PID） | DWB、Regulated Pure Pursuit | **沿用全向 PID**（麦轮全向移动的标配，官方控制器不支持横移） |
| costmap 障碍层 | IntensityVoxelLayer（自定义） | 默认 ObstacleLayer | 沿用（地形代价必须靠 intensity 传递） |
| 地形分析 | terrain_analysis(_ext) | 纯 2D 激光避障 | 沿用（哨兵要上坡，地形代价是刚需） |
| 速度接口 | fake_vel_transform | 直接发 cmd_vel | 沿用（云台自旋是哨兵刚需） |
| 行为树 | 仓库自带两个 XML | 官方默认 BT | 沿用仓库版本 |

> 一句话：**只要硬件同款，全链路沿用源仓库选型；将来想替换任何一环，接口已经隔离好了（§4 的表），换包不换接口。**

---

## 8. 分阶段开发路线图（新手向）

> 每阶段都有明确的"验收标准"——完成才算过，不要跳阶段。

### Phase 0：环境与 ROS2 基础（约 2 周）

**目标**：有一台能跑 ROS2 Humble 的电脑/服务器，具备基本 ROS2 操作能力。

- [ ] 安装 Ubuntu 22.04 + ROS2 Humble（或配置 Docker：`ghcr.io/smbu-polarbear-robotics-team/pb2025_sentry_nav:1.3.2`）
- [ ] 学完 ROS2 入门：节点/话题/服务/参数/TF（资源见 §9）
- [ ] 装 Gazebo + `rmu_gazebo_simulator` 仿真包
- [ ] 装 small_gicp 依赖库（`sudo apt install libeigen3-dev libomp-dev` + 编译 koide3/small_gicp）

**验收**：能用 `ros2 run turtlesim` 跑通 demo；能用 `ros2 topic echo` 看到话题数据；TF 概念能讲清楚。

### Phase 1：跑通源仓库仿真（约 1 周，不动代码）

**目标**：原封不动跑起来，建立"系统全貌"的直觉。

- [ ] `git submodule update --init --recursive`（⚠️ 你们 fork 的 submodule 目前全是空的）
- [ ] 下载先验点云 PCD（README 里的 FlowUs 链接）放到 `pcd/` 目录
- [ ] `rosdep install` → `colcon build --symlink-install`
- [ ] `ros2 launch pb2025_nav_bringup rm_navigation_simulation_launch.py world:=rmuc_2025 slam:=False`
- [ ] RViz 里用 Nav2 Goal 点目标，看车自己走
- [ ] 逐个看：`ros2 topic list`、`rqt_graph`、`rqt_tf_tree`、`rviz2` 里关掉/打开各层显示，理解"哪个话题对应哪条数据链"

**验收**：能解释"从雷达点云到 cmd_vel"的完整链路；能说出 6 个以上关键话题的发布者/订阅者。

### Phase 2：复制改造（约 1 周）

**目标**：拥有自己的命名空间和包结构。

- [ ] 复制仓库到 `src/YOUR_TEAM_nav/`，按 §6 改名清单执行
- [ ] 换 namespace 为你们自己的，验证 TF 树/话题前缀全部更新
- [ ] 删掉不需要的实验性文件（如多机器人 launch，可留到以后）
- [ ] 重新 build + 跑通仿真（此时系统行为应与 Phase 1 完全一致）

**验收**：`ros2 topic list | grep 你们的namespace` 全部带新前缀；仿真照常跑。

### Phase 3：建图与定位（约 2 周）

**目标**：自己生成地图，并能在上面重定位。

- [ ] `slam:=True` 建图模式，手柄/遥控走遍仿真场地 → `map_saver_cli` 保存栅格地图
- [ ] 在自定义场地上重复，确认地图质量（墙壁清晰、无重影）
- [ ] 生成先验点云 PCD（point_lio 自动保存 `PCD/` 目录，或手动保存）
- [ ] `slam:=False` 导航模式加载自己的地图 + PCD，验证重定位成功（RViz 中地图与点云对齐）

**验收**：换一张新地图场景，从建图到重定位全流程 30 分钟内完成。

### Phase 4：导航调参与地形（持续，约 3-4 周）

**目标**：从"能走"到"走得好"。

- [ ] 调 `pb_omni_pid_pursuit_controller`：lookahead、曲率减速、PID 增益（`config/simulation/nav2_params.yaml` 的 `FollowPath` 段）
- [ ] 调 costmap：inflation 半径、IntensityVoxelLayer 的 min/max intensity、车体半径
- [ ] 调 terrain_analysis：`useSorting`（上坡关键）、`vehicleHeight`、`minRelZ/maxRelZ`
- [ ] 验证：上坡不误判为墙、绕坑、窄通道能过、自旋扫描时路径不崩

**验收**：在 3 个不同场景下，导航成功率 ≥ 90%，无原地打转/撞墙。

### Phase 5：实车迁移（上实车前 2-3 周）

**目标**：从仿真平滑切到实车。

- [ ] 外参标定：雷达安装位姿（`front_mid360` 静态 TF）、IMU 方向
- [ ] 实车驱动接通：`livox_ros_driver2` + 底盘 `cmd_vel` 订阅
- [ ] 云台关节数据接入（`robot_state_publisher` 或你们电控的 TF 源）
- [ ] 用 `reality/` 配置：`use_sim_time:=False`、`use_robot_state_pub:=True`（无完整机器人系统时）
- [ ] 小范围低速试跑 → 逐步放开速度

**验收**：实车在场地上完成"建图→保存→重定位→导航"闭环。

### Phase 6：进阶（赛季中持续）

- [ ] 哨兵决策层：巡逻路线（`follow_waypoints`）、目标切换、回补
- [ ] 云台自旋策略：`cmd_spin` 与导航的协同
- [ ] 动态障碍物应对、恢复行为调优
- [ ] 多机器人（namespace 架构已就绪，补 `map→odom` 初始化即可）

---

## 9. 学习路线与资源清单（新手必看）

**按顺序学，不要跳：**

1. **ROS2 基础（2 周）**：[ROS2 Humble 官方 Tutorials](https://docs.ros.org/en/humble/Tutorials.html)——重点：CLI 工具、话题/服务/动作、tf2、launch
2. **NAV2（1 周）**：[NAV2 官方文档](https://docs.nav2.org/) 的 Concepts 部分——重点：costmap、planner、controller、behavior tree、lifecycle
3. **本仓库**：
   - [B 站视频：谁说在家不能调车！？更适合新手宝宝的 RM 导航仿真](https://www.bilibili.com/video/BV12qcXeHETR)（北极熊战队的保姆级仿真教程）
   - 仓库 [Wiki](https://github.com/SMBU-PolarBear-Robotics-Team/pb2025_sentry_nav/wiki)（实车部署细节）
   - 代码精读顺序：`fake_vel_transform`（30 行）→ `loam_interface`（100 行）→ `sensor_scan_generation`（160 行）→ `pb_omni_pid_pursuit_controller` → `terrain_analysis`
4. **配套仿真**：[rmu_gazebo_simulator](https://github.com/SMBU-PolarBear-Robotics-Team/rmu_gazebo_simulator)

**推荐调试工具**：`rqt_graph`（节点图）、`rqt_tf_tree`/`view_frames`（TF）、`rviz2`（可视化）、`ros2 doctor`、`ros2 topic hz`（看频率）、`plotjuggler`（画曲线调 PID）。

---

## 10. 团队分工建议（4~6 人导航组）

| 角色 | 职责 | 对应阶段 |
|---|---|---|
| 环境/基建 | Docker、编译、CI、仿真环境 | Phase 0/1 |
| 感知/里程计 | point_lio、loam_interface、sensor_scan_generation、外参标定 | Phase 1/3/5 |
| 定位/建图 | slam_toolbox、small_gicp、地图与 PCD 管理 | Phase 3 |
| 规划/控制 | NAV2 调参、全向 PID、costmap、terrain 参数 | Phase 4 |
| 决策/集成 | 哨兵巡逻逻辑、launch 装配、实车联调 | Phase 5/6 |

> 建议所有人**都过一遍 Phase 1**（跑通+看懂数据流），再按角色深入。这样开会时能对同一套话题/TF 说话。

---

## 11. 风险与注意事项（踩坑预警）

| 风险 | 说明 | 对策 |
|---|---|---|
| **submodule 未初始化** | 当前 fork 的 7 个 submodule 目录全空，直接 build 必失败 | 先 `git submodule update --init --recursive`（需有访问战队仓库的权限） |
| **先验点云缺失** | 不在 git 里，重定位会失败 | Phase 1 就下载好；或先跑 `slam:=True` 模式（不需要 PCD） |
| **仿真/实车传感器差异** | 仿真里是 32 线 velodyne 式扫描（`lidar_type: 2`），实车 Mid360 是非重复扫描（`lidar_type: 1`） | 换配置别换代码；注意 `config/simulation` vs `config/reality` 的 `point_lio.preprocess` 段 |
| **TF 时间戳错位** | Twist（无时间戳）与 odometry 无法对齐 | 这就是 fake_vel_transform 订阅 `local_plan` 取时间戳的原因，别乱改 |
| **prior_pcd 易飘** | README 注明大场景效果不佳 | 先用不带先验的纯 point_lio |
| **版本匹配** | 必须是 Ubuntu 22.04 + Humble + 对应分支的 submodule | 严格按 README 2.2 节操作 |
| **命名空间遗漏** | 新写节点忘了带 namespace | 所有节点用 `PushRosNamespace` 或在 launch 里加 `namespace` 参数 |

---

## 12. 建议里程碑（时间线）

```
第 1-2 周    Phase 0  环境 + ROS2 入门                → 里程碑 A：环境就绪
第 3 周      Phase 1  跑通源仓库仿真                  → 里程碑 B：能走
第 4 周      Phase 2  复制改造（自己的 namespace）    → 里程碑 C：自己的栈能走
第 5-6 周    Phase 3  建图 + 重定位                   → 里程碑 D：闭环
第 7-10 周   Phase 4  调参（地形/控制）               → 里程碑 E：走得好
第 11-13 周  Phase 5  实车迁移                        → 里程碑 F：实车闭环
第 14 周起   Phase 6  决策层 + 赛季迭代
```

---

## 附：与源仓库的对照阅读索引

| 想弄懂的问题 | 读什么 |
|---|---|
| 系统怎么启动 | `pb2025_nav_bringup/launch/rm_navigation_simulation_launch.py` → `bringup_launch.py` |
| slam 和导航模式怎么切换 | `bringup_launch.py` 里 `IfCondition(slam)` 分支 |
| 每个节点什么参数 | `config/simulation/nav2_params.yaml`（实车看 `config/reality/`） |
| 坐标系转换 | `loam_interface/src/loam_interface.cpp` |
| TF 发布与底盘里程计 | `sensor_scan_generation/src/sensor_scan_generation.cpp` |
| 云台自旋补偿 | `fake_vel_transform/README.md` + `src/fake_vel_transform.cpp` |
| 地形代价怎么算 | `terrain_analysis/src/terrainAnalysis.cpp`（参数注释在 yaml 里） |
| 控制器怎么跟路径 | `pb_omni_pid_pursuit_controller`（submodule） |
| 速度指令链路 | `navigation_launch.py` 里的 cmd_vel remap 链 |
