# foray_sentry_nav

RoboMaster 哨兵自主导航栈（自研，从 0 搭建）。

## 1. 项目目标

基于 ROS 2 Humble + NAV2 导航框架，从 0 搭建 RoboMaster 哨兵机器人的自主导航栈，支持仿真与实车部署：

- 自主建图（SLAM）与导航（重定位 + 路径规划 + 路径跟踪）
- 障碍物与地形感知（上坡、绕坑）
- 云台自旋扫描时仍能稳定跟踪路径
- 同一套代码支撑仿真（Gazebo）与实车

## 2. 硬件平台

- LiDAR：Livox Mid360（安装方式不限定，倾斜或水平均可，以实际标定为准）
- IMU：与里程计算法（point_lio）配套
- 底盘：全向（麦轮）底盘 + 云台（gimbal_yaw 关节，可自旋扫描）
- 运动学：全向移动（平移 + 横移 + 自转）

## 3. 软件环境

- 开发环境：Ubuntu 22.04 虚拟机（KVM/QEMU），宿主为 Arch Linux
- ROS 发行版：ROS 2 Humble
- 导航框架：NAV2
- 仿真：rmu_gazebo_simulator（Gazebo 11）

## 4. 功能范围（v1 里程碑）

- [ ] 雷达点云接入（仿真：点云补 ring/time；实车：livox_ros_driver2）
- [ ] 里程计（point_lio + 坐标系转换）
- [ ] 建图模式（slam_toolbox 生成 2D 栅格地图）
- [ ] 定位模式（先验点云重定位）
- [ ] 地形代价感知（terrain_analysis 方案）
- [ ] NAV2 导航（全局规划 + 全向 PID 路径跟踪）
- [ ] 云台自旋补偿（伪坐标系方案）
- [ ] 手柄遥控（teleop）

## 5. 参考与借鉴

- 架构参考：SMBU-PolarBear-Robotics-Team/pb2025_sentry_nav（本仓库 fork 分支，仅作参考）
- 架构设计文档：`docs/nav_stack_architecture_design.md`
- 详细开发路线：`plan.md`

## 6. 分支策略

- `dev-rz`：开发分支（默认分支）
- `fork`：上游参考基线（只读参考）
- `main`：dev 完成基础工作后创建（集成分支）

## 7. 开发路线

Phase 0 环境搭建 → Phase 1 跑通参考仿真 → Phase 2 项目骨架 → Phase 3 建图与定位 → Phase 4 导航调参 → Phase 5 实车迁移 → Phase 6 决策层
