# SGLifeSim

新加坡人生模拟经营游戏。一个外来程序员在新加坡用五年时间证明自己的故事 —— 你在 CBD 写代码、在组屋区吃饭、在地铁里幻想、在房贷计算器前发呆，最后决定这座岛是不是你的家。

## 状态

🚧 早期原型阶段（Engine Validation Prototype）—— **核心竖切片已可玩**

### 当前可玩内容（双击 uproject → Play 直接进出租屋）

| 操作 | 效果 |
|------|------|
| WASD | 主角在场景内移动（等距正交相机跟随，移动播行走动画、静止播 idle） |
| 走近 NPC | 屏幕显示「[E] 对话」提示 |
| E | 与最近的 NPC 对话（屏幕显示台词） |
| T | 推进一个时间块（HUD 显示 Day X · 周几 · 时间块，循环跨天跨周） |
| M | 在出租屋 ↔ 食阁两个关卡间切换（时间状态跨关卡保留） |

- 两个关卡：`L_Rental`（出租屋，暖色居家）/ `L_HawkerCenter`（食阁，开放摊位带桌凳）
- 核心系统在 C++（移动 / 等距相机 / locomotion / 交互 / 时间 / 场景切换），Blueprint 仅作薄壳
- HUD / 对话用**纯 C++ UMG**（`USGHudWidget`，控件树在 C++ 里构造，无需 BP widget 资产）：
  顶部状态行、底部交互提示 + 对话气泡。地点切换仍是 M 键直接跳转（原型未做弹出菜单）。
  详见 [引擎验证结果文档](docs/decisions/2026-05-23-engine-validation-outcome.md)

## 类型

- **核心**：Sims 式纯沙盒模拟经营 + 新加坡设定
- **视角**：等距俯视 45°（UE5 正交相机）
- **时间**：Persona 5 日历 + 时间块
- **玩法循环**：赚钱 → 攒钱 → 买东西 → 投资 → 阶级跃迁
- **调性**：治愈 + 少量沉重

## 文档

- 设计文档（spec）：[docs/specs/2026-05-23-sg-life-sim-design.md](docs/specs/2026-05-23-sg-life-sim-design.md)
- 当前实施计划：[docs/plans/2026-05-23-engine-validation-prototype.md](docs/plans/2026-05-23-engine-validation-prototype.md)
- 决策记录：[docs/decisions/](docs/decisions/)

## 技术栈

- **引擎**：UE5.6.1 LTS
- **语言**：C++17（核心系统）+ Blueprint（业务逻辑）
- **UI**：UMG
- **版本控制**：Git + Git LFS

## 开发环境

- Windows 11
- Visual Studio 2022 Community（含 "Game development with C++" + ".NET Desktop Development" workloads）
- UE5.6.1 LTS 通过 Epic Games Launcher 安装
- Git LFS

## 上手

```bash
# 1. 装 UE5.6.1 LTS + VS2022 + Git LFS
# 2. clone
git clone https://github.com/<your-user>/sg-life-sim.git SGLifeSim
cd SGLifeSim
git lfs pull

# 3. 双击 SGLifeSim.uproject 打开（首次编译约 5~10 分钟）

# 注：GitHub repo 名是 sg-life-sim（kebab-case，仓库命名习惯），
# 本地文件夹用 SGLifeSim（PascalCase，UE C++ 模块名规范）。
```

## 致谢

- 主角 + NPC 动画来自 [Mixamo](https://mixamo.com)（Adobe 免费）
- 部分场景资产来自 UE5 Fab Marketplace、Sketchfab CC0、Kenney.nl

## License

TBD（待定，当前仅开发期作品，未公开发售许可）

---

Built with [Claude Code](https://claude.com/claude-code).
