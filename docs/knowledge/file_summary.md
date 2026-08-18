# file_summary.md

| 文件 | 单一职责 | 对外接口 |
|---|---|---|
| `readme.md` | 项目需求（唯一需求来源） | 需求基线，规划与验收依据 |
| `AGENTS.md` | AI 开发规范与行为约束 | 开发约束，引用 Base Protocol |
| `plan.md` | 开发计划、进度、状态 | Agent State / Before Snapshot / 待确认项 |
| `tree.md` | 目录结构说明 | 定位协议 L1 |
| `decision.md` | 关键工程决策日志 | 决策回溯 |
| `.gitignore` | 忽略构建产物与敏感文件 | git 跟踪边界 |
| `LICENSE` | 开源协议（WTFPL） | 授权声明 |
| `.gitattributes` | 发布归档剔除 AI 工作流文件（export-ignore） | GitHub zip/tar 归档内容 |
| `.github/workflows/guard-main-ai-files.yml` | CI 守卫：阻止 AI 工作流文件进入 main | PR/push 到 main 的合并检查 |
| `scripts/index_knowledge.py` | 知识索引脚本（chromadb 全量索引 / 查询验证） | 全量索引 + 语义查询（`.venv/bin/python scripts/index_knowledge.py [--query ...]`） |
| `docs/nav_stack_architecture_design.md` | 架构设计（分层 / TF / 数据流 / 路线图） | 架构参考（fork 借鉴） |
| `docs/knowledge/folder_summary.md` | 目录职责摘要 | 定位协议 L1 |
| `docs/knowledge/file_summary.md` | 文件职责摘要 | 定位协议 L2 |

更新时机：文件职责变化时增量更新，MUST NOT 全量重写。
