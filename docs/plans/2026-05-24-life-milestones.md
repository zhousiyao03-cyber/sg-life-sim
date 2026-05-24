# Plan 13: Life Milestones（人生目标 / 主线脊柱）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6（身份/经济/职业/资产主线）

**Goal:** 游戏系统很厚，但缺一个让玩家知道「我该奔什么、还差多远」的主线 —— 沙盒变「有奔头的游戏」最关键的一块。本 plan 加一条有序的人生主线（落地新加坡的脊柱），随时在 HUD 显示当前目标 + 进度，达成时弹庆祝。

**七步主线：** 拿到第一份薪水 → 攒下 $5,000 → 升上中级工程师 → 买下自己的房 → 拿到 PR → 净资产 $100,000 → 成为公民。

---

## Task 序列

### Task 1: 纯逻辑核心 + 类型 ✅
`MilestoneTypes.h`（`EMilestone` 七步枚举 + `FMilestoneContext` 状态快照 + `FMilestoneProgress`）。`FMilestoneSystem`：`Evaluate`/`IsComplete`/`GetActive`（第一个未完成）/`CountCompleted`/`GetTitle`。零子系统依赖、可单测。

### Task 2: MilestoneSubsystem + HUD 目标行 ✅
`UMilestoneSubsystem`：`BuildContext` 从经济/职业/身份/资产/进度子系统聚合快照；`Refresh` 检测新达成并广播 `OnMilestoneCompleted`（首次静默 Prime 不为开局已满足项弹 toast）；`GetActiveObjectiveText` 给 HUD。订阅 `OnTimeAdvanced` 刷新。HUD 加 `ObjectiveText` 行（🎯 醒目金色）。玩家 BeginPlay 订阅完成 toast（🎉），DrawPrototypeHUD 每帧刷新目标行。

### Task 3: 测试 + 重建 + 文档 ✅
`SGLifeSim.Milestone.EvaluatesAndProgresses`：默认全未达成、各条件触达、目标按序推进、数值进度（$5k/$100k）正确、全达成返回 Count。

---

## 完成情况（2026-05-24）

全量重建成功，**全套 64 个 AutomationTest 全绿、零失败**（63 + 新增 1）。

**效果：** HUD 左列多一行「🎯 当前目标：攒下 $5,000（$3,200 / $5,000）· 已完成 1/7」，玩家随时知道下一步奔什么；达成弹「🎉 里程碑达成：…」。系统状态变化（发薪/升职/买房/PR）即时反映。

**设计：** 里程碑全部从其它子系统的状态**派生**（不自己存数据）；`GetActiveObjectiveText` 顺带 `Refresh` 保证 HUD 每帧检测达成、toast 及时。当前未持久化里程碑完成位（派生自已持久化状态），读档后会重新点亮——可能补弹 toast，留作后续小优化。

**留后续：** 里程碑完成位进存档（避免读档补弹 toast）；可选的「软目标/支线」轨道；庆祝音效。

> 注：随后项目转向**第一人称 + lo-fi 恐怖插入层**方向（大框架/系统不变）。本目标系统与视角无关，可直接复用为新方向的「日常目标」轨道。
