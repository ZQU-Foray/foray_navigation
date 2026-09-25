# foray_navigation

> **L4** · owner 角色：导航

静态地图规划 + 动态障碍规避 + 全向控制器

基于机内预置地图规划，**不用在线 SLAM**。

---

## 快速开始

> 本仓处于**初始化状态**，尚无源码。以下流程随 P0/P1 落地逐步可用。

```bash
# 1. 拉取（仓齐备后改为从元仓 foray_ws 一键拉取）
git clone git@github.com:ZQU-Foray/foray_navigation.git
cd foray_navigation

# 2. 依赖安装
rosdep install -y -r --from-paths . --ignore-src --rosdistro humble

# 3. 构建
colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release

# 4. 运行
# （待装配根就绪后补充）
```

## 依赖

| 类型 | 依赖 |
|---|---|
| **自研仓** | X2、L1、L2、L3、L4 |
| **第三方** | Nav2 · `pb_omni_pid_pursuit_controller`（pin）· `pb_nav2_plugins`（pin）· BehaviorTree.CPP |

## 接口

| 产出 | 内容 |
|---|---|
| 底盘指令 | `cmd_vel`（全向） |
| 导航状态 | 到点 / 失败 / 恢复 |
| 局部决策 | 导航 recovery（内嵌 L4，不上 L5） |

## 现状



本仓由原 `pb2025_sentry_nav`（哨兵导航栈）**改名而来**，当前 `fork` 分支保有工作基线。

按 `foray_docs/repository_structure.md` §8 的 Strangler Fig 路径逐步替换（**不推倒重来**）。



**命名理由**（判据 C5）：原 `foray_sentry_nav` 的语义是「兵种_功能」，隐含「哨兵专用」——

而**导航恰恰要复用给步兵**。兵种差异移到 `foray_robots/sentry_bringup`。



## 保留与删除



| 处置 | 对象 |

|---|---|

| **保留** | `point_lio` → `loam_interface` → `sensor_scan_generation` |

| **保留** | `fake_vel_transform`（云台自旋伪坐标系，哨兵必需） |

| **保留** | Nav2 规划控制 + `pb_omni_pid_pursuit_controller` |

| **保留** | `ign_sim_pointcloud_tool`（仿真喂点云给 LIO） |

| **删除** | `terrain_analysis` / `terrain_analysis_ext` / `IntensityVoxelLayer` |

| **删除** | `pointcloud_to_laserscan` / 在线 `slam_toolbox` |

| **删除** | `rm_multi_navigation_simulation_launch.py` |



## 纪律



- **激光不参与避障/地形/代价地图**——激光只用于定位

- 不用在线 SLAM；地图是离线资产

- 动态障碍来自 `foray_vision` 的 `WorldSnapshot`，本仓不自行检测

## 上下游

| 方向 | 对象 |
|---|---|
| **上游**（本仓依赖谁） | `foray_interfaces` · `foray_platform` · `foray_localization` |
| **下游**（谁依赖本仓） | `foray_robots` · `foray_ws` |

## 参考

- [算法结构](https://github.com/ZQU-Foray/foray_docs/blob/main/algorithm_structure.md)
- [仓库结构](https://github.com/ZQU-Foray/foray_docs/blob/main/repository_structure.md)
- [组织贡献指南](https://github.com/ZQU-Foray/.github/blob/main/CONTRIBUTING.md)
