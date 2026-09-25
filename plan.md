# 开发计划

## 任务列表

- [ ] 清理 fork：删除 terrain_analysis 等已定删除项
- [ ] 把导航参数抽出为 `shared.yaml` + `<robot>.yaml` 两级
- [ ] 接 `foray_localization` 的位姿与地图（替代当前直连）
- [ ] 接 `foray_vision` 的动态障碍（替代自建障碍源）
- [ ] 拆出 `foray_robots` 装配根，本仓只留算法

## 当前 Agent State

PLAN_READY

## Before Snapshot

commit hash:   （首次提交）
branch:        main
modified files: （首次初始化）
risk level:    L1

## 模糊点与待确认项

- 本仓接口尚未冻结，任务清单为**方向性**的，落地顺序以 `foray_docs/algorithm_structure.md` §9 演进阶段为准
- 依赖的自研仓尚未建齐，跨仓任务需等对方 `README.md` 明确接口后再启动

## Vector Backend Status

Backend: Markdown
Status:  ready
Environment: 仓库内 Markdown 文档（无外部向量后端）
Index: 本仓 `README.md` / `tree.md` / `decision.md`
Initialization: 2026-02-19
Commit: （首次提交）

## Acceptance Criteria

- [ ] 本仓能独立 `colcon build`（无代码时跳过）
- [ ] CI 绿灯
- [ ] 每个新增模块带独立可执行测试
