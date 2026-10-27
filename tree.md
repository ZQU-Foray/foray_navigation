# 目录结构说明

## 目录结构

```
.
├── README.md                本仓定位、接口、上下游
├── AGENTS.md                AI 协作规范
├── plan.md                  开发计划与进度
├── decision.md              关键工程决策日志
├── tree.md                  本文件
├── CODEOWNERS               Review 自动分派
├── .foray-layer             所属层与依赖边界（CI 校验）
├── .gitignore
├── docs/
│   └── nav_architecture.md  寻路框架设计：契约、代价图、语义层、通行档案、验收
├── include/foray/nav/       公共接口（header-only，不依赖 ROS）
│   ├── types.hpp            几何/阵营/能力/目标/状态/错误码/路径/指令
│   ├── costmap_view.hpp     代价图只读快照接口 + 代价编码
│   ├── planner.hpp          全局规划器接口
│   └── controller.hpp       局部控制器接口
├── CMakeLists.txt           接口库 foray_navigation（INTERFACE，C++17）
├── package.xml              build_type=cmake，供 colcon 识别
└── .github/
    └── workflows/
        └── ci.yml           复制自组织 CI 模板
```

> 本仓源码目录尚未建立，当前只有设计文档（`docs/nav_architecture.md`）。
> 随代码落地同步更新本文件。

## 记录约束

MUST NOT 记录以下内容：

- `build/`、`install/`、`log/`、`.git/` 等依赖与产物目录
- 临时文件、缓存文件、日志文件
