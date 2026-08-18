# AGENTS.md

## 项目技术栈

- ROS 2 Humble + NAV2，C++（算法节点）/ Python（launch 脚本）
- 构建：`colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release`
- 依赖：`rosdep install -r --from-paths src --ignore-src --rosdistro humble -y`
- 测试/检查：`colcon test`、`ament_lint_auto`
- 参考架构：`fork` 分支（SMBU-PolarBear-Robotics-Team/pb2025_sentry_nav，仅参考）

## AI 行为约束

遵循 `project-bootstrap-workflow` / `feature-change-workflow` Skill 的 Base Protocol：

- **唯一需求来源**：`readme.md`，不得臆造需求；需求模糊时转入 WAIT_USER
- 遵循 Agent State 状态机，状态转换记录到 `plan.md`
- 修改前在 `plan.md` 记录 Before Snapshot（commit hash / branch / 修改文件 / 风险等级）
- 按逻辑单元小步提交，MUST NOT 一次性提交全部代码
- 遵循 Navigation Protocol：L1 `tree.md` → L2 `file_summary` → L3 源码
- 维护 `folder_summary` / `file_summary` 知识索引，增量同步
- 提交前执行提交自检（构建/语法/tree.md/plan.md/无敏感信息）

## 项目约定

- 分支：`dev-rz` 开发（默认），`fork` 只读参考，`main` 待基础工作完成后创建（集成）
- 功能开发：`feature/<功能简称>` 分支，验证后合入 dev-rz
- 提交格式：`<type>: <简短描述>`（type: feat / fix / docs / refactor / test / chore）
- 包命名：小写 + 下划线（ROS2 规范）；launch 文件用 launch.py 后缀
- 文档同步：`readme.md` / `plan.md` / `tree.md` / `decision.md` 保持一致

## 明确禁止

- 敏感信息入库（密钥、Token、密码、个人信息）
- `git reset --hard` / `git push --force` / 非必要 `git rebase`
- 修改 `.env` 等环境配置
- 删除用户提供的源材料（`readme.md` 等）
- 引入无必要依赖（新增依赖需在 plan/decision 记录理由）
