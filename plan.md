# plan.md

## 任务列表

- [x] 决策确认：环境（Ubuntu 22.04 KVM/QEMU）、从 0 搭建、fork 仅参考
- [x] `readme.md` 需求文档（用户审阅确认，项目名 foray_sentry_nav）
- [ ] Phase 0：环境搭建（Ubuntu VM + ROS2 Humble + Gazebo）【用户侧执行】
- [ ] Phase 1：跑通参考仿真（fork 分支参考）
- [ ] Phase 2：项目骨架（readme / AGENTS / plan / tree / decision / gitignore / 知识索引）
- [ ] Phase 3：建图与定位
- [ ] Phase 4：导航调参
- [ ] Phase 5：实车迁移
- [ ] Phase 6：决策层

## 当前 Agent State

PLAN_READY

## Before Snapshot

commit hash:   5d11e19
branch:        dev-rz
modified files: readme.md, plan.md, AGENTS.md, tree.md, decision.md, .gitignore, docs/knowledge/*
risk level:    L0

## 模糊点与待确认项

- [ ] Mid360 实际安装位姿（倾斜或水平，影响外参标定，Phase 5 前确认）
- [ ] 先验点云来源（Phase 1 参考仿真需要，FlowUs 链接下载）
- [ ] KVM 虚拟机内跑 Gazebo 的性能与显示方案（GPU 透传 / 软件渲染）

## Vector Backend Status

Backend:      <待初始化>
Status:       <ready | degraded | failed>
Environment:  <待填写>
Index:        <待填写>
Initialization: <待填写>
Commit:       <待填写>

## Acceptance Criteria

- [ ] `readme.md` 通过用户确认
- [ ] 工程文档体系完整（AGENTS / plan / tree / decision / gitignore）
- [ ] 知识索引初始化并记录 Backend Status
- [ ] 提交历史符合 Conventional Commits
