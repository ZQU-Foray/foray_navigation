# plan.md

## 任务列表

- [x] 决策确认：环境（Ubuntu 22.04 KVM/QEMU）、从 0 搭建、fork 仅参考
- [x] `readme.md` 需求文档（用户审阅确认，项目名 foray_sentry_nav）
- [ ] Phase 0：环境搭建（Ubuntu VM + ROS2 Humble + Gazebo）【用户侧执行】
- [ ] Phase 1：跑通参考仿真（fork 分支参考）
- [x] Phase 2：项目骨架（readme / AGENTS / plan / tree / decision / gitignore / 知识索引）
- [ ] Phase 3：建图与定位
- [ ] Phase 4：导航调参
- [ ] Phase 5：实车迁移
- [ ] Phase 6：决策层

## 当前 Agent State

PREPARE（脚手架与知识索引就绪，等待 Phase 0/1 环境就绪后进入 Phase 3 增量开发）

## Before Snapshot

commit hash:   e7bf7c1
branch:        dev-rz
modified files: LICENSE, .gitattributes, .github/workflows/guard-main-ai-files.yml,
                decision.md, AGENTS.md, tree.md, docs/knowledge/file_summary.md, plan.md
risk level:    L0

## 模糊点与待确认项

- [ ] Mid360 实际安装位姿（倾斜或水平，影响外参标定，Phase 5 前确认）
- [ ] 先验点云来源（Phase 1 参考仿真需要，FlowUs 链接下载）
- [ ] KVM 虚拟机内跑 Gazebo 的性能与显示方案（GPU 透传 / 软件渲染）

## Vector Backend Status

Backend:      Python（chromadb 1.5.9，项目 .venv 隔离环境）
Status:       ready
Environment:  Python 3.14.7（.venv），chromadb PersistentClient，纯 numpy TF-IDF 嵌入（无需模型下载）
Index:        .vector-index/（8 个文档，gitignore 排除）
Initialization: 2026-08-18
Commit:       26d3572（脚本随本阶段提交）

## Acceptance Criteria

- [x] `readme.md` 通过用户确认
- [x] 工程文档体系完整（AGENTS / plan / tree / decision / gitignore）
- [x] 知识索引初始化并记录 Backend Status（write → query → verify 通过）
- [x] 提交历史符合 Conventional Commits
