# Plan 12: World Population（代码驱动 NPC 入世）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **前置：** Plan 5/6（对话引擎 + UI）、Plan 11（对话内容 + 校验器）
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §7.2（可交互 NPC）

**Goal:** Plan 11 给 Ah Mei / 准备好的内容写了树，但世界里能对话的只有手摆的 Ah Hua 一个人——其余 NPC「有树没人」。本 plan 用**代码在关卡加载时生成 NPC**，让世界真的有人可交互，又不必手摆进二进制 `.umap`（内容可版本化、不开编辑器、即玩即见）。

**核心思路：** `USGWorldPopulatorSubsystem : UWorldSubsystem` 在 `OnWorldBeginPlay` 按关卡名取 roster，把 NPC 生成在 **PlayerStart 附近的地面**（锚定 PlayerStart → 不依赖各关卡布局也能落在可达地面），幂等跳过已存在的 NpcId（与手摆的 Ah Hua 共存）。

**范围内：** 填充子系统 + NPC 公有初始化 `ConfigureNpc` + 2 个新 NPC 对话树（保安 Uncle Lim / 同事 Wei）+ roster 交叉校验测试。
**不做：** NPC 视觉分化（暂复用主角骨骼网格作占位，留待美术）、精确摆位/坐姿动画、寻路。

---

## Task 序列

### Task 1: 新增 Uncle Lim + 同事 Wei 对话树 ✅
`SGDialogueContent::BuildUncleLimTree()`（出租屋保安，闲聊/打听八卦，好感≥60 时 Uncle 请喝 kopi +$1）+ `BuildColleagueWeiTree()`（食阁同事，吐槽/职场建议，好感≥50 解锁内推成就 `KnowColleague`），并入 `BuildAllTrees()`。`SGAchievementIds::KnowColleague()` 新增。

### Task 2: 代码驱动 NPC 填充 WorldSubsystem ✅
`USGWorldPopulatorSubsystem`（`DoesSupportWorldType` 只在 Game/PIE 生效）。`ASGInteractableNPC::ConfigureNpc(id, speaker, line, mesh)` 公有初始化。`GetRosterForLevel(LevelName)` 纯静态函数（按子串匹配，含 `RemovePIEPrefix`）返回 `FNpcSpawnSpec` 列表 —— L_Rental: UncleLim；L_HawkerCenter: AhMei + Wei。生成时锚定首个 PlayerStart 位置 + 偏移，复用 `SK_Player` 网格 + idle 动画作占位。

### Task 3: 测试 + 重建 + 文档 ✅

---

## 完成情况（2026-05-24）

全量重建成功，**全套 63 个 AutomationTest 全绿、零失败**（62 + 新增 1）。

| 产出 | 测试 |
|---|---|
| `USGWorldPopulatorSubsystem`（关卡加载按 roster 生成 NPC，锚定 PlayerStart，幂等）+ `ASGInteractableNPC::ConfigureNpc` + Uncle Lim / Wei 两棵树 + `KnowColleague` 成就 | `SGLifeSim.World.RosterNpcsHaveDialogueTrees`：**roster 里每个 NpcId 都必须有注册的对话树**（把世界内容和对话内容钉死，防以后摆 NPC 忘配树）+ 关卡成员断言 + PIE 前缀解析。新两棵树由既有 `Dialogue.ContentValidates` 自动覆盖校验 |

**效果：** 现在 2 个关卡里有 **4 个可对话 NPC**（出租屋：Ah Hua + Uncle Lim；食阁：Ah Mei + Wei），全部数据驱动、纯代码上线。这是第一次在不开编辑器的情况下把「世界里能交互的人」变多。

**设计选择：** 锚定 PlayerStart 地面位置而非写死世界坐标 —— 因为看不到关卡布局，这样能保证 NPC 落在玩家附近的可达地面，不会掉进墙里/悬空。幂等跳过已存在 NpcId，让代码生成与手摆 actor 安全共存。

**诚实标注：** NPC 现在都长得跟主角一样（复用 `SK_Player` 占位）。视觉分化（不同模型/服装/坐姿）需要美术资产 + 编辑器，留作后续。但**玩法上已经是 4 个有独立性格/分支/奖励的可交互角色**。偏移坐标是盲配的合理值，PIE 实测可能需要微调（下次开编辑器顺手核一下落点）。

**留后续：** NPC 模型分化、更多关卡与场景、NPC 日程（按时间块出现在不同地点）、对话访问 flag 进存档。
