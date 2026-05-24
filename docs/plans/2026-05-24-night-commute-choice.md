# Plan 23: Night Commute Choice（鬼月夜归抉择 / 电梯禁忌）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **方向：** docs/decisions/2026-05-24-first-person-horror-pivot.md
> **承接：** Plan 15 恐怖事件层、Plan 16 理智、Plan 22 共鸣对话（Uncle Lim 的电梯警告）

**Goal:** 把恐怖从「被动播报」升级成「主动博弈」——这是恐怖游戏真正的张力来源：玩家明知有禁忌，却要在「贪图省事」和「招惹危险」之间亲手做选择，并承担后果。农历七月深夜回家，电梯自己停在 13 楼空楼层（正是 Uncle Lim 警告过的那个），玩家选：等下一趟（守规矩，安全慢）/ 赶紧进去（省事，但赌）/ 走楼梯（最稳最累）。纯代码闭环，UI 触发点留待 PIE。

---

## Task 序列

### Task 1: 抉择纯核心 ✅
`NightCommuteTypes.h`：`ENightCommuteChoice`（WaitForNext / StepIn / TakeStairs）+ `FNightCommuteOutcome`（文案 / 理智变化 / 精力变化 / 是否真出事）。`FNightCommuteSystem::Resolve(Choice, FRandomStream&)`：守规矩选项确定且安全（等下一趟理智 +2/耗精力 8；走楼梯理智 0/耗精力 20），「赶紧进去」是 55% 出事的赌局——赌输重扣理智 22 且 `bSomethingHappened=true`，赌赢也吓出冷汗轻扣 4。Stream 仅在 StepIn 时消费 → 可复现。

### Task 2: 子系统薄壳 ✅
`UNightCommuteSubsystem`：`IsAvailable`（农历七月 + 深夜，复用 `UHorrorEventSubsystem::IsGhostMonth` + `UTimeSubsystem` 时间块）；`MakeChoice` 把结算落到 `USanitySubsystem`（Restore/Drain）与 `UPlayerStateSubsystem`（Energy），广播 `OnResolved` 文案。注入种子可复现。

### Task 3: 测试 + 重建 ✅

---

## 完成情况（2026-05-24）

全量重建成功，**全套 78 个 AutomationTest 全绿、零失败**（76 + 新增 2）。

| 产出 | 测试 |
|---|---|
| `FNightCommuteSystem`（三选项结算 + 概率赌局 + 可复现）+ `UNightCommuteSubsystem`（鬼月深夜门控 + 落理智/精力 + 广播） | `Horror.NightCommuteChoice`（守规矩确定安全、StepIn 赌赢/赌输两分支都出现且后果一致、同种子可复现）+ `Integration.NightCommuteResolves`（开局不可触发、等下一趟回理智耗精力、赌输重扣 22、赌赢轻扣 4；用纯核心循环找种子避免硬编码） |

**设计取舍：**
- 独立子系统而非塞进活动系统：风险结算要注入 RandomStream、联动恐怖/理智/主角三系统，语义是「博弈」而非「日常活动」，混进去会污染活动表的纯静态可测性。
- 守规矩有代价（等=耗精力、走楼梯=很累），犯禁忌最省事——这才让「贪图省事」成为真实诱惑，否则没有取舍。
- 「赶紧进去」赌赢也轻扣理智：哪怕没出事，犯了禁忌本身就吓人，奖励的是省下的精力而非心安。
- 测试用纯核心循环探测「会赌输/赌赢的种子」，而非硬编码 magic seed，FRandomStream 算法变了也不脆裂。

**待 PIE 复核：** 触发入口——鬼月深夜回到出租屋时，在 M 菜单/HUD 弹出这个三选一抉择（已有纯 C++ UMG 菜单范式可复用）；结算文案的呈现与音效。

**留后续：** 更多七月禁忌博弈（半夜捡红包、夜里晾衣、被叫名回不回头）；把「赌输」接成真正触发一次 `UHorrorEventSubsystem` 恶性事件（现仅扣理智 + 文案）；夜归抉择的历史计入某种「这座岛的七月你过了几个」统计/成就。
