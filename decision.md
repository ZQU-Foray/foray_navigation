# decision.md

## Decision

Date: 2026-08-18

Context: AI 工作流文件（AGENTS.md / plan.md / tree.md / decision.md / docs/knowledge/ / scripts/ 等）不应进入 main 分支；main 应为干净的生产/集成分支。

Decision: 采用三层组合方案：
1. **orphan 创建 main**：`git checkout --orphan main` 后只检出生产文件（readme.md、LICENSE、.gitattributes、.gitignore、.github/、src/），AI 文件从根上不进入 main 的树；
2. **CI 守卫**：`.github/workflows/guard-main-ai-files.yml` 阻止 AI 文件通过 PR / push 合入 main；
3. **发布物剔除**：`.gitattributes` 中 `export-ignore` 使 GitHub 下载的 zip/tar 归档不包含 AI 文件。

Reason: git 分支机制无法按分支控制"文件是否被跟踪"（.gitignore 只作用于未跟踪文件，merge 会把文件带过去）；orphan 从根上隔离最干净，CI 防止将来误合，export-ignore 保证发布物干净。

Alternatives: 独立 meta 分支/仓库存放 AI 文件；仅靠 .gitignore 排除。

Rejected:
- 独立 meta 分支/仓库：当前 AI 文件已入库，迁移成本高且开发时引用不便
- 仅 .gitignore：对已跟踪文件无效，无法阻止 merge 带入

## Decision

Date: 2026-08-18

Context: 借鉴 SMBU-PolarBear pb2025_sentry_nav 架构搭建自研哨兵导航栈，需确定项目基础决策。

Decision: 项目名 `foray_sentry_nav`；开发环境为 Ubuntu 22.04 虚拟机（KVM/QEMU，宿主 Arch Linux）；在 `dev-rz` 分支从 0 搭建；`fork` 分支仅作参考；`main` 分支待 dev 完成基础工作后创建（集成分支）。

Reason: 硬件与参考仓库同款，架构可直接借鉴但代码从 0 掌控；VM 隔离宿主 Arch 环境，与 ROS2 Humble 官方支持（Ubuntu 22.04）保持一致。

Alternatives: Docker 容器方案（官方镜像已存在）；Arch 原生安装 Humble。

Rejected:
- Docker：镜像版本受限、图形/GPU 透传复杂，且希望完整理解构建过程
- Arch 原生装 Humble：ROS2 Humble 官方不支持 Arch，依赖地狱

## Decision

Date: 2026-08-18

Context: LiDAR 安装方式影响外参标定与坐标系（TF）设计。

Decision: Livox Mid360 安装方式不限定（倾斜或水平均可），以实际标定为准。

Reason: 倾斜侧装是参考仓库的特定做法；本项目坐标系设计需兼容两种安装方式，外参在 Phase 5 实车标定。

Alternatives: 直接沿用参考仓库的倾斜侧装。

Rejected: 无（保留安装灵活性）。

## Decision

Date: 2026-08-18

Context: 技术选型（里程计 / 建图 / 重定位 / 控制）是否沿用参考仓库。

Decision: 技术选型沿用参考仓库方案：point_lio（里程计）、slam_toolbox（建图）、small_gicp（重定位）、terrain_analysis（地形代价）、全向 PID 路径跟踪、fake_vel_transform（云台自旋伪坐标系）。

Reason: 硬件同款、方案经过实战验证；包间接口已解耦，后续可按接口逐个替换。

Alternatives: fast-lio2 / liom（里程计）、AMCL（重定位）、DWB（控制器）等。

Rejected: 无必要更换，验证过的方案优先，降低从 0 搭建的风险。

## Decision

Date: 2026-08-18

Context: 分支策略与参考代码组织。

Decision: `dev-rz` 为默认开发分支（当前仓库默认分支）；`fork` 分支保留上游完整代码作只读参考；`main` 在 dev 完成基础工作后创建并作为集成分支。

Reason: 从 0 搭建期间不需要 main 作为基座；参考代码与开发代码分离，避免相互污染。

Alternatives: 以 main 为开发基座。

Rejected: 与"从 0 搭建 + fork 仅参考"的既定策略不符。
