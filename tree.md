# tree.md

## 目录结构

```
.
├── readme.md                 # 项目需求（唯一需求来源）
├── AGENTS.md                 # AI 开发规范
├── plan.md                   # 开发计划与进度（含 Agent State / Before Snapshot）
├── tree.md                   # 目录结构说明
├── decision.md               # 关键工程决策日志
├── LICENSE                   # 开源协议（WTFPL）
├── .gitignore
├── .gitattributes            # 发布归档剔除 AI 工作流文件（export-ignore）
├── .github/
│   └── workflows/
│       └── guard-main-ai-files.yml   # CI 守卫：阻止 AI 工作流文件进入 main
├── scripts/
│   └── index_knowledge.py    # 知识索引脚本（Vector Backend Adapter，chromadb）
└── docs/
    ├── nav_stack_architecture_design.md   # 架构设计文档（分层 / TF / 数据流 / 路线图，参考）
    └── knowledge/
        ├── folder_summary.md  # 目录职责摘要
        └── file_summary.md    # 文件职责摘要
```

> `src/`（ROS2 功能包）等源码目录将在 Phase 3 增量开发时创建并补充到本文件。

## 记录约束

MUST NOT 记录以下内容：

- `node_modules/`、`build/`、`dist/`、`.git/` 等依赖与产物目录
- 临时文件、缓存文件、日志文件
- `.venv/`（项目内 Python 虚拟环境，仅供知识索引使用）
- `.vector-index/`（知识索引本地数据，由脚本生成）
