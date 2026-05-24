# Plan 10: Activities Loop（时间块活动循环）实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–3）
> **前置：** Plan 1–9 ✅
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.1（时间块选活动 = 时间推进）、§6.4（属性消耗/恢复）

**Goal:** 把「按 T 空推时间」升级为**有意义的玩法循环**——在地点里选活动消耗时间块，换取属性/钱/关系。这是把前 9 个 plan 堆的系统「盘活成玩」的关键一步：撸代码涨专业技能（→能升职）、学习涨见识、健身涨健康、吃饭/睡觉回能量、副业赚现金。让「过日子」有动作、有取舍（能量是稀缺资源）。

**范围内：** 纯 C++ `FActivitySystem`（活动定义表 + 能量门槛）+ `UActivitySubsystem`（消耗时间块、改属性/现金、广播）+ 活动菜单 UI（按当前地点列活动）+ 单元/集成测试。
**不做（留后续）：** 把活动绑到场景里的可交互物（书桌/床）、活动动画/演出、关系互动活动（约 NPC，已有对话）、活动结果随机性、按住房/属性解锁更多活动。

---

## 模型（数值为可调默认）

活动 `EActivityType`，每个 = 时间块数 + 各属性增减（含能量）+ 现金增减。**能量是稀缺资源**：消耗型活动要求做完能量不低于 0；恢复型（睡觉/吃饭）总可做。

| 活动 | 地点 | 时间块 | 效果 |
|---|---|---|---|
| 睡觉 | 出租屋 | 2 | 能量 +60，心情 +5 |
| 学习 | 出租屋 | 1 | 专业 +4，见识 +2，能量 −15 |
| 接私活（撸代码） | 出租屋 | 1 | 现金 +$300，专业 +2，能量 −20 |
| 健身 | 出租屋 | 1 | 健康 +5，心情 +3，能量 −10 |
| 食阁吃饭 | 食阁 | 1 | 能量 +20，心情 +5，健康 +1，现金 −$5 |
| 听八卦 | 食阁 | 1 | 见识 +3，社交 +3，心情 +2，能量 −5 |

属性改动经 `UPlayerStateSubsystem`（0~100 clamp），现金经 `UEconomySubsystem`，时间块经 `UTimeSubsystem::AdvanceBlock`（顺带触发已有的月度发薪/账单/投资/事件）。

## Task 序列

### Task 1: FActivitySystem 核心 + 单测
`ActivityTypes.h`（`EActivityType` + `FActivityDef`：标题/时间块/`AttrDelta[Count]`(含能量)/现金）+ `FActivitySystem`：静态 `GetActivityDef`、`CanPerform(Def, CurrentEnergy)`（做完能量 ≥ 0）。单测：定义数值、能量门槛挡住高耗活动、恢复型放行。

### Task 2: UActivitySubsystem + 接线 + 集成测试
`UActivitySubsystem::PerformActivity(type)`：校验能量 → 改属性 → 改现金 → 推时间块 ×N → 广播 `OnActivityPerformed(FText)`；`GetActivitiesForCurrentLevel()` 按关卡名过滤。集成测试（`InitializeStandalone`）：学习涨专业耗能推时间；连做到能量不足被挡，睡觉恢复后又能做；接私活赚现金。

### Task 3: 活动菜单 UI + 重建 + 测试 + 文档
`USGActivityMenuWidget`（纯 C++）列当前地点活动；从 location menu 加「做点事…」按钮打开；点活动 → `PerformActivity` → 反馈并关回游戏。全量重建 + headless 全套测试全绿。更新文档/README/记忆。

---

## Definition of Done

玩家能在地点里选活动，消耗时间块换属性/钱，能量是有意义的约束；活动推进时间会顺带触发月度结算。全套测试保持全绿、无回归。

---

## 完成情况（2026-05-24）

全部 3 个 Task 完成。全量重建成功，**全套 60 个 AutomationTest 全绿、零失败**（57 + 本 plan 新增 3）。

| Task | 产出 | 测试 |
|---|---|---|
| 1 | `ActivityTypes.h`（`EActivityType` + `FActivityDef`：标题/时间块/`AttrDelta[Count]`/现金）+ `FActivitySystem`（`GetActivityDef`/`CanPerform` 能量门槛） | `SGLifeSim.Activity.*` ×2 |
| 2 | `UActivitySubsystem`（`PerformActivity` 改属性/现金 + 推时间块 + 广播；`GetActivitiesForCurrentLevel` 按关卡过滤） | `Integration.ActivityLoop` ×1 |
| 3 | `USGActivityMenuWidget`（纯 C++，固定按钮 + 能量门槛禁用）；location menu 加「做点事…」按钮打开 | （重建 60 绿） |

**坑（重要设计交互）：活动推进时间块 → 跨天会触发 `FPlayerStats::RestoreEnergyDaily`（能量回满）。** 一天 = 5 个时间块，所以「靠连做活动把能量耗光」会被跨天回满打断——这是 spec §6.4「能量每天恢复」的正确行为，不是 bug。集成测试因此改为**直接 `SetAttribute(Energy, 低值)` 来验证能量门槛**，而非靠跨天累积耗能（后者会被每日恢复重置）。能量是「每天内」的稀缺资源。

**要点：** 活动效果数据化（`AttrDelta` 按属性索引，含能量）；`PerformActivity` 顺序 = 改属性/现金 → 推时间块；推时间块复用既有月度结算链（发薪/账单/投资/事件）。UI 用固定 6 按钮 + `CanPerform` 禁用（精力不够灰掉）。辅助 `ACDollars` 文件内唯一名避 unity ODR。

**留后续：** 活动绑场景可交互物（床/书桌按 E）、活动动画演出、关系互动活动、活动随机结果、按住房/属性解锁更多活动、活动消耗的时间块在 HUD 预告。
