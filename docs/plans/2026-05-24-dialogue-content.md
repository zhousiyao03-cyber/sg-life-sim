# Plan 11: Dialogue Content Depth（剧情内容扩充）实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕）
> **前置：** Plan 1–10 ✅（对话引擎 Plan 5 + 对话 UI Plan 6）
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.3（关系/对话/NPC 故事弧）

**Goal:** 用户审视后指出「剧情几乎没有」（1 个 NPC、4 节点小树）。本 plan 在**纯代码、可测、立即可玩**的范围内补剧情深度：把已在世界里的邻居 Ah Hua 扩成多分支故事弧（展示全部条件/效果类型），建对话树**完整性校验器**让内容能安全扩张，并把内容从子系统里抽出为可测的数据模块。

**范围内：** `FDialogueSystem::ValidateTree` 校验器 + `SGDialogueContent` 数据模块（Ah Hua 多分支弧 + 食阁阿姨 Ah Mei）+ 校验/集成测试。
**不做（需编辑器，留作后续）：** 把新 NPC（Ah Mei 等）摆进关卡 `.umap`、搭更多场景、配音。**新 NPC 的对话树已就绪，等编辑器摆放 actor 即可触达；Ah Hua 的扩写即时可玩（它已在 L_Rental 里）。**

---

## Task 序列

### Task 1: 校验器 + 内容静态构建器重构 ✅
`FDialogueSystem::ValidateTree(tree, &err)`：根节点必须存在；每个选项若指定 NextNodeId（非空且非 EndDialogue 效果）必须解析到存在的节点。新建 `SGDialogueContent::BuildAllTrees()` 把内容集中为数据，`UDialogueSubsystem::BuildSampleTrees` 改为遍历注册（内容与子系统解耦、可单测）。

### Task 2: 扩写 Ah Hua 多分支 + 第二个 NPC + 集成测试 ✅
Ah Hua 弧：闲聊(+3)/送礼(+10)/[好感≥50]交心/[好感≥70]问当年故事(解锁成就 `KnowNeighborStory`)/[身份≥PR]报喜/告辞 —— 覆盖 MinAffinity、MinResidency 条件与 AddAffinity、MarkAchievement、EndDialogue 效果。食阁阿姨 Ah Mei 树（买鸡饭花 $3.5 / 唠两句加好感，SG 文案）。

### Task 3: 重建 + 测试 + 文档 ✅

---

## 完成情况（2026-05-24）

全量重建成功，**全套 62 个 AutomationTest 全绿、零失败**（60 + 本 plan 新增 2）。

| 产出 | 测试 |
|---|---|
| `FDialogueSystem::ValidateTree` + `SGDialogueContent`（Ah Hua 多分支弧 + Ah Mei）+ 子系统重构 + `SGAchievementIds::KnowNeighborStory` | `SGLifeSim.Dialogue.ContentValidates`（含「校验器能抓悬空引用」自检）+ `Integration.DialogueStoryAchievement`（好感 70 → 故事分支可见 → 选它解锁成就） |

**顺手修了一个潜伏 bug（重要）：Plan 9 的随机经济事件用时间戳种子、每月必抽，导致任何「推月断言精确现金」的测试非确定性**（`MonthlyLoopAndSave` 这次抽中年终奖 +$7500 → $2930 变 $10430；之前几轮是靠运气抽中 None 才过的）。修法：`UEconomicEventSubsystem` 加 `SetEventsEnabled(bool)`，`MonthlyLoopAndSave` / `CareerPromotionRaisesIncome` 两个断言精确月结的测试显式关掉事件。

**另一处回归修复：** 扩写 Ah Hua 后根节点选项变多（加了无条件的「先走了」+ 更多门控分支），`DialogueAffinityUnlock` 里写死的「2/3 个可见选项」过期 → 更新为「3/4」以匹配新内容。

**坑：** `TestNotEqual(int32, INDEX_NONE)` 因 `INDEX_NONE` 是匿名枚举而有歧义，改用 `TestTrue(x != INDEX_NONE)`。

**诚实标注：** 真正把「剧情/场景丰富度」拉上来仍需编辑器工作（摆放更多 NPC、搭更多场景）——这是下一步的「编辑器内容会话」。本 plan 把可纯代码做的剧情深度 + 内容管线（数据化 + 校验）做了。

**留后续：** 编辑器摆放 Ah Mei / Uncle 保安 / PR 顾问 / 同事等 NPC 进关卡、更多 NPC 故事弧、对话访问 flag 进存档、配音/立绘。
