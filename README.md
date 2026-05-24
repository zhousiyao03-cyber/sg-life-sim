# SGLifeSim

新加坡人生模拟经营游戏。一个外来程序员在新加坡用五年时间证明自己的故事 —— 你在 CBD 写代码、在组屋区吃饭、在地铁里幻想、在房贷计算器前发呆，最后决定这座岛是不是你的家。

## 状态

🚧 早期原型阶段（Engine Validation Prototype）—— **核心竖切片已可玩**

### 当前可玩内容（双击 uproject → Play 直接进出租屋）

| 操作 | 效果 |
|------|------|
| WASD | 第一人称移动（相对视线方向） |
| 鼠标 | 第一人称转视角（平时锁定捕获；开菜单/对话时放出光标可点击） |
| 走近 NPC | 屏幕显示「[E] 对话」提示 |
| E | 与最近的 NPC 开对话面板（说话人 + 台词 + 可点选项；选项按好感门控、点击施加效果） |
| T | 推进一个时间块（HUD 显示 Day X · 周几 · 时间块，循环跨天跨周） |
| M | 弹出菜单：切换地点（出租屋 ↔ 食阁，时间跨关卡保留）/ 存读档 / 按揭买房·还贷 / 升职·跳槽 /「做点事…」开活动菜单 |

- 两个关卡：`L_Rental`（出租屋，暖色居家）/ `L_HawkerCenter`（食阁，开放摊位带桌凳）
- 4 个可对话 NPC（关卡加载时由 `USGWorldPopulatorSubsystem` 代码生成，免手摆 .umap）：出租屋有邻居 Ah Hua + 保安 Uncle Lim，食阁有阿姨 Ah Mei + 同事 Wei，各有独立性格 / 分支 / 好感奖励
- 核心系统在 C++（第一人称移动 / 相机 / 鼠标视角 / 交互 / 时间 / 场景切换），Blueprint 仅作薄壳
- **方向**：正转向**第一人称 + lo-fi 恐怖**（大框架/系统不变，恐怖按桥段插入农历七月/都市传说等）——见 [方向决策](docs/decisions/2026-05-24-first-person-horror-pivot.md)
- UI 全用**纯 C++ UMG**（控件树在 C++ 里构造，无需 BP widget 资产）：`USGHudWidget`（顶部状态行 +
  底部交互提示 + 对话气泡）、`USGLocationMenuWidget`（M 键弹出的可点击地点菜单）。
  详见 [引擎验证结果文档](docs/decisions/2026-05-23-engine-validation-outcome.md)

### 核心系统骨架（Plan 2，已完成，25 个单元/集成测试全绿）

spec §6 的五大系统都已落地为「纯 C++ 逻辑核心 + `UGameInstanceSubsystem` 薄壳 + AutomationTest」：

| 系统 | 核心类 / 子系统 | 要点 |
|------|----------------|------|
| 经济 | `FEconomySystem` / `UEconomySubsystem` | 现金/银行/CPF(OA/SA/MA) 钱包，金额以「分」存；月薪按 CPF 规则分账 |
| 职业 | `FCareerSystem` / `UCareerSubsystem` | 程序员升迁阶梯（初级→首席）：专业技能+在职时长可升职涨薪，或跳槽 +35%；薪资喂给月度发薪 |
| 时间 | `FTimeSystem` / `UTimeSubsystem` | 时间块/天/周/月推进；每月 1 号经事件自动发薪 + 扣房租水电交通 |
| 进度 | `FProgressSystem` / `UProgressSubsystem` | 软成就去重追踪 + 首解锁委托 |
| 关系 | `FRelationshipSystem` / `URelationshipSubsystem` | NPC 好感 0~100 + 六档关系等级（陌生→恋人） |
| 主角 | `FPlayerStats` / `UPlayerStateSubsystem` | 健康/心情/能量/专业/社交/见识 0~100，能量每日恢复 |
| 活动 | `FActivitySystem` / `UActivitySubsystem` | 时间块选活动（睡觉/学习/接私活/健身/吃饭/听八卦）换属性/钱，能量为约束；按地点过滤 |
| 进度 | `FProgressSystem` / `UProgressSubsystem` + `UAchievementDirector` | 软成就追踪；Director 订阅经济/关系自动解锁 |
| 身份 | `FResidencySystem` / `UResidencySubsystem` | EP/SP→申请PR→PR→公民 状态机（含被拒退回） |
| 资产 | `FAssetsSystem` / `UAssetsSubsystem` | 房/车 tier + 投资（月度复利回报）+ **按揭融资**（首付 25%/月供逐月自动扣/利息随余额递减/提前结清，未还本金计入净资产负债） |
| 事件 | `FEconomicEventSystem` / `UEconomicEventSubsystem` | 每月加权随机经济事件（行情涨跌/年终奖/政府红包/突发账单），可种子复现，弹 HUD toast |
| 恐怖 | `FHorrorEventSystem` / `UHorrorEventSubsystem` | 深夜随机降临的新加坡都市传说 + 农历七月（中元节）禁忌 + 异乡人心理恐怖；扣心情/健康/理智、弹阴森气泡；鬼月概率升温并解锁限定事件；**理智失常时出现分不清真假的幻觉**；可种子复现 |
| 理智 | `FSanitySystem` / `USanitySubsystem` | 理智 0~100（平静/不安/失常/濒临崩溃）：恐怖事件消耗理智，理智越低深夜越频越凶（恐惧螺旋）；每日缓慢恢复但鬼月无喘息；进存档 |
| 图鉴 | `FHorrorCodexSystem` / `UHorrorCodexSubsystem` | 都市传说收集：亲历过的恐怖事件记进图鉴（bitmask），未遇到的显示「？？？」；首条解锁/集齐有成就；进存档；HUD 显示「N / M」收集进度 |
| 终局 | `FEndingEvaluator` / `UEndingSubsystem` | 五终局：四软终局（扎根/兑现/心碎/漂着）+ 恐怖坏结局「被压垮」（理智耗尽，盖过一切）；评估倾向 + 主动选择 + 归零强制 |
| 对话 | `FDialogueSystem` / `UDialogueSubsystem` + `USGDialogueWidget` + `SGDialogueContent` | 数据驱动对话树：选项条件门控（好感/身份/成就）+ 效果（好感/钱/成就）；按 E 弹纯 C++ UMG 对话面板。内容数据化（邻居 Ah Hua 多分支故事弧 + 食阁阿姨 Ah Mei），含 `ValidateTree` 完整性校验器 |
| 存档 | `USGSaveGame` / `USaveGameSubsystem` | 聚合全部系统状态，`SaveGameToSlot`/`LoadGameFromSlot` |

玩家操作已接入系统（Plan 3/4/6/7/8）：按 E 与 NPC 开对话面板（选项门控/施加好感等效果）+ 耗能量、推时间触发月度发薪/账单/投资回报/房贷月供、HUD 实时显示职位月薪/钱包/属性/身份/住房/房贷/终局倾向、菜单可存读档 + 按揭买房/还贷 + 升职/跳槽。

跑测试：`Automation RunTests SGLifeSim`（headless `UnrealEditor-Cmd ... -nullrhi`），当前 **75 个全绿**。

## 类型

- **核心**：Sims 式纯沙盒模拟经营 + 新加坡设定
- **视角**：等距俯视 45°（UE5 正交相机）
- **时间**：Persona 5 日历 + 时间块
- **玩法循环**：赚钱 → 攒钱 → 买东西 → 投资 → 阶级跃迁
- **调性**：治愈 + 少量沉重

## 文档

- 设计文档（spec）：[docs/specs/2026-05-23-sg-life-sim-design.md](docs/specs/2026-05-23-sg-life-sim-design.md)
- 实施计划：[Plan 1 引擎验证原型](docs/plans/2026-05-23-engine-validation-prototype.md)（✅）· [Plan 2 核心系统骨架](docs/plans/2026-05-24-core-systems-skeleton.md)（✅）· [Plan 3 系统接入可玩循环](docs/plans/2026-05-24-gameplay-integration.md)（✅）· [Plan 4 进阶与终局](docs/plans/2026-05-24-progression-and-endings.md)（✅）· [Plan 5 对话引擎](docs/plans/2026-05-24-dialogue-engine.md)（✅）· [Plan 6 对话 UI](docs/plans/2026-05-24-dialogue-ui.md)（✅）· [Plan 7 按揭购房融资](docs/plans/2026-05-24-housing-finance.md)（✅）· [Plan 8 职业与收入成长](docs/plans/2026-05-24-career-income.md)（✅）· [Plan 9 随机经济事件](docs/plans/2026-05-24-economic-events.md)（✅）· [Plan 10 时间块活动循环](docs/plans/2026-05-24-activities-loop.md)（✅）· [Plan 11 剧情内容扩充](docs/plans/2026-05-24-dialogue-content.md)（✅）· [Plan 12 代码驱动 NPC 入世](docs/plans/2026-05-24-world-population.md)（✅）· [Plan 13 人生目标主线](docs/plans/2026-05-24-life-milestones.md)（✅）· [Plan 14 第一人称改造](docs/decisions/2026-05-24-first-person-horror-pivot.md)（✅）· [Plan 15 恐怖事件层](docs/plans/2026-05-24-horror-event-layer.md)（✅）· [Plan 16 理智系统](docs/plans/2026-05-24-sanity-system.md)（✅）· [Plan 17 理智恢复（拜拜）](docs/plans/2026-05-24-sanity-system.md)（✅）· [Plan 18 精神崩溃终局](docs/plans/2026-05-24-breakdown-ending.md)（✅）· [Plan 21 恐怖图鉴](docs/plans/2026-05-24-horror-codex.md)（✅）
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
