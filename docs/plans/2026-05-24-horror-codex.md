# Plan 21: Horror Codex（恐怖图鉴 / 都市传说收集）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **方向：** docs/decisions/2026-05-24-first-person-horror-pivot.md
> **承接：** Plan 15 恐怖事件层、Plan 20 低理智幻觉

**Goal:** 把一闪而过的恐怖事件文案沉淀成可回看、可收集的「都市传说图鉴」——恐怖游戏经典的探索回报机制。给玩家「我经历过什么、还有哪些没遇到」的钩子，和已有的进度/成就/存档系统天然咬合，纯代码即可落地（PIE 才需要做图鉴 UI 面板）。

---

## Task 序列

### Task 1: 图鉴纯核心 ✅
`FHorrorCodexSystem`：一个 `int64` bitmask 记录「亲历过哪些 `EHorrorEvent`」（事件 < 64，bit 索引 = 枚举值）。`MarkEncountered`（None 忽略，首次返回 true）/`HasEncountered`/`CountDiscovered`（`FMath::CountBits`）/`TotalCollectable`（所有非 None 事件数）/`IsComplete`/`GetMask`+`RestoreMask`（存档）。零 UE 依赖，可单测。

### Task 2: 子系统薄壳 + 事件耦合 + 成就 ✅
`UHorrorEventSubsystem` 加 `OnHorrorEventTyped`（带 `EHorrorEvent`，区别于只带文案的旧 `OnHorrorEvent`，HUD/图鉴各取所需）。`UHorrorCodexSubsystem` 订阅它，`RecordEncounter` 标记图鉴；首次发现解锁 `FirstUrbanLegend`、集齐解锁 `CompleteHorrorCodex`（推 `UProgressSubsystem`）；`GetEntries` 返回整本图鉴（未发现项隐藏文案显示「占位」），`GetProgressText`「都市传说 N / M」。

### Task 3: 存档 + HUD + 测试 ✅
`SGSaveGame.DiscoveredHorrors`（int64 bitmask）+ SaveGameSubsystem gather/apply。HUD 目标行在亲历≥1 条后追加「🕯 都市传说 N / M」。测试见下。

---

## 完成情况（2026-05-24）

全量重建成功，**全套 75 个 AutomationTest 全绿、零失败**（73 + 新增 2）。

| 产出 | 测试 |
|---|---|
| `FHorrorCodexSystem`（bitmask 收集/去重/计数/集齐/round-trip）+ `UHorrorCodexSubsystem`（订阅 typed 事件记录、首条/集齐成就、GetEntries 隐藏未发现文案、存档）+ HUD 图鉴进度行 | `Horror.CodexCollects`（标记/去重/计数/集齐/bitmask round-trip/None 不计）+ `Integration.HorrorCodexRecordsAndPersists`（施加事件→记录+首条成就、重复不增、None 不计、GetEntries 文案门控、集齐成就、存档 round-trip） |

**设计取舍：**
- 加 `OnHorrorEventTyped`（枚举）而非改旧委托——HUD 要文案、图鉴要类型，两个委托互不干扰，旧订阅者零改动。
- bitmask 而非 `TSet<EHorrorEvent>`：事件数远小于 64，bitmask 存档只占 8 字节、计数靠 popcount，更省更快。
- 未发现项在 `GetEntries` 里隐藏文案（UI 显示「？？？」），保留「还有什么没遇到」的悬念。
- 集齐判定基于「所有非 None 事件」，含鬼月限定与低理智幻觉——想集齐图鉴就得熬过鬼月、也得把自己逼到理智失常，收集欲与恐怖体验对齐。

**待 PIE 复核：** 图鉴 UI 面板（菜单里开一页列出已/未发现的传说，未发现显示「？？？」）；新解锁时的演出（toast / 音效）。

**已接续：** —
**留后续：** 图鉴 UI 面板（需编辑器/UMG，但已有纯 C++ UMG 范式可复用）；按传说类别分组（都市传说 / 鬼月 / 幻觉）；解锁某条传说后在对应 NPC 对话里开新分支（「你也遇到了？」）。
