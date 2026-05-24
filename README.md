# SGLifeSim

新加坡人生模拟经营游戏。一个外来程序员在新加坡用五年时间证明自己的故事 —— 你在 CBD 写代码、在组屋区吃饭、在地铁里幻想、在房贷计算器前发呆，最后决定这座岛是不是你的家。

## 状态

🚧 早期原型阶段（Engine Validation Prototype）—— **核心竖切片已可玩**

### 当前可玩内容（双击 uproject → Play 直接进出租屋）

| 操作 | 效果 |
|------|------|
| WASD | 主角在场景内移动（等距正交相机跟随，移动播行走动画、静止播 idle） |
| 走近 NPC | 屏幕显示「[E] 对话」提示 |
| E | 与最近的 NPC 开对话面板（说话人 + 台词 + 可点选项；选项按好感门控、点击施加效果） |
| T | 推进一个时间块（HUD 显示 Day X · 周几 · 时间块，循环跨天跨周） |
| M | 弹出地点菜单，点按钮在出租屋 ↔ 食阁间切换（时间状态跨关卡保留） |

- 两个关卡：`L_Rental`（出租屋，暖色居家）/ `L_HawkerCenter`（食阁，开放摊位带桌凳）
- 核心系统在 C++（移动 / 等距相机 / locomotion / 交互 / 时间 / 场景切换），Blueprint 仅作薄壳
- UI 全用**纯 C++ UMG**（控件树在 C++ 里构造，无需 BP widget 资产）：`USGHudWidget`（顶部状态行 +
  底部交互提示 + 对话气泡）、`USGLocationMenuWidget`（M 键弹出的可点击地点菜单）。
  详见 [引擎验证结果文档](docs/decisions/2026-05-23-engine-validation-outcome.md)

### 核心系统骨架（Plan 2，已完成，25 个单元/集成测试全绿）

spec §6 的五大系统都已落地为「纯 C++ 逻辑核心 + `UGameInstanceSubsystem` 薄壳 + AutomationTest」：

| 系统 | 核心类 / 子系统 | 要点 |
|------|----------------|------|
| 经济 | `FEconomySystem` / `UEconomySubsystem` | 现金/银行/CPF(OA/SA/MA) 钱包，金额以「分」存；月薪按 CPF 规则分账 |
| 时间 | `FTimeSystem` / `UTimeSubsystem` | 时间块/天/周/月推进；每月 1 号经事件自动发薪 + 扣房租水电交通 |
| 进度 | `FProgressSystem` / `UProgressSubsystem` | 软成就去重追踪 + 首解锁委托 |
| 关系 | `FRelationshipSystem` / `URelationshipSubsystem` | NPC 好感 0~100 + 六档关系等级（陌生→恋人） |
| 主角 | `FPlayerStats` / `UPlayerStateSubsystem` | 健康/心情/能量/专业/社交/见识 0~100，能量每日恢复 |
| 进度 | `FProgressSystem` / `UProgressSubsystem` + `UAchievementDirector` | 软成就追踪；Director 订阅经济/关系自动解锁 |
| 身份 | `FResidencySystem` / `UResidencySubsystem` | EP/SP→申请PR→PR→公民 状态机（含被拒退回） |
| 资产 | `FAssetsSystem` / `UAssetsSubsystem` | 房/车 tier + 投资（月度复利回报），买卖经经济扣款 |
| 终局 | `FEndingEvaluator` / `UEndingSubsystem` | spec §6.5 四软终局（扎根/兑现/心碎/漂着）评估 + 主动选择 |
| 对话 | `FDialogueSystem` / `UDialogueSubsystem` + `USGDialogueWidget` | 数据驱动对话树：选项条件门控（好感/身份/成就）+ 效果（好感/钱/成就）；按 E 弹纯 C++ UMG 对话面板 |
| 存档 | `USGSaveGame` / `USaveGameSubsystem` | 聚合全部系统状态，`SaveGameToSlot`/`LoadGameFromSlot` |

玩家操作已接入系统（Plan 3/4/6）：按 E 与 NPC 开对话面板（选项门控/施加好感等效果）+ 耗能量、推时间触发月度发薪/账单/投资回报、HUD 实时显示钱包/属性/身份/住房/终局倾向、菜单可存读档。

跑测试：`Automation RunTests SGLifeSim`（headless `UnrealEditor-Cmd ... -nullrhi`），当前 **44 个全绿**。

## 类型

- **核心**：Sims 式纯沙盒模拟经营 + 新加坡设定
- **视角**：等距俯视 45°（UE5 正交相机）
- **时间**：Persona 5 日历 + 时间块
- **玩法循环**：赚钱 → 攒钱 → 买东西 → 投资 → 阶级跃迁
- **调性**：治愈 + 少量沉重

## 文档

- 设计文档（spec）：[docs/specs/2026-05-23-sg-life-sim-design.md](docs/specs/2026-05-23-sg-life-sim-design.md)
- 实施计划：[Plan 1 引擎验证原型](docs/plans/2026-05-23-engine-validation-prototype.md)（✅）· [Plan 2 核心系统骨架](docs/plans/2026-05-24-core-systems-skeleton.md)（✅）· [Plan 3 系统接入可玩循环](docs/plans/2026-05-24-gameplay-integration.md)（✅）· [Plan 4 进阶与终局](docs/plans/2026-05-24-progression-and-endings.md)（✅）· [Plan 5 对话引擎](docs/plans/2026-05-24-dialogue-engine.md)（✅）· [Plan 6 对话 UI](docs/plans/2026-05-24-dialogue-ui.md)（✅）
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
