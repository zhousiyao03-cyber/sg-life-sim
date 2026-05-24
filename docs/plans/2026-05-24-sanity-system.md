# Plan 16: Sanity System（理智 / 恐惧脊柱）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **方向：** docs/decisions/2026-05-24-first-person-horror-pivot.md
> **承接：** Plan 15 恐怖事件层

**Goal:** 把零散的恐怖事件变成有代价、有螺旋的玩法核心。引入「理智 0~100」——恐怖事件消耗理智；理智越低，深夜越容易、越凶地出事（恐惧螺旋）。不像能量每日回满（那会消解张力），只缓慢恢复，且农历七月没有喘息。

---

## Task 序列

### Task 1: 理智纯核心 + 类型 ✅
`SanityTypes.h`（`ESanityState` 平静≥70 / 不安 40-69 / 失常 15-39 / 濒临崩溃 <15）。`FSanitySystem`：`Clamp`、`GetState`、`ExtraDreadWeight`（平静0/不安10/失常25/崩溃45）。

### Task 2: SanitySubsystem + 恐怖耦合 + HUD + 存档 ✅
`USanitySubsystem`：理智 0~100；`Drain`/`Restore`（状态档变化时广播 `OnSanityChanged`）；订阅 `OnTimeAdvanced` 每天缓慢恢复 +8，但**农历七月不恢复**。耦合：`FHorrorEventDef` 加 `SanityCost`，`UHorrorEventSubsystem::ApplyEvent` 扣理智；`FHorrorEventSystem::PickEvent` 加 `DreadBonus`（低理智来的额外恐惧，从「无事」权重里扣、保底 5）。HUD 属性行显示「理智 N（状态）」。`SGSaveGame.Sanity` + SaveGameSubsystem gather/apply。

### Task 3: 测试 + 重建 + 文档 ✅

---

## 完成情况（2026-05-24）

全量重建成功，**全套 68 个 AutomationTest 全绿、零失败**（66 + 新增 2）。

| 产出 | 测试 |
|---|---|
| `FSanitySystem` + `USanitySubsystem`（理智状态机/每日恢复/鬼月不恢复/存档）+ 恐怖事件扣理智 + 低理智放大恐怖频率 + HUD 理智行 | `Sanity.StatesAndDread`（阈值/clamp/dread 单调）+ `Integration.SanitySpiralAndSave`（恐怖扣理智、低理智升恐惧、每日 +8 恢复、鬼月不恢复、存档 round-trip） |

**恐惧螺旋：** 恐怖事件 → 扣理智 → 理智档下降 → `ExtraDreadWeight` 升高 → 深夜「无事」权重被扣得更低 → 更频繁/更凶的事件 → 理智更低。农历七月叠加（基础 None 权重已更低 + 不恢复），把鬼月做成真正难熬的一段。

**设计取舍：** 理智独立于 PlayerStats（不走每日回满），自带恢复规则。心理恐怖一条（被遣返噩梦）同时扣健康 + 理智，呼应异乡人主题。

**待 PIE 复核：** 理智数值/状态在 HUD 的可读性、螺旋节奏手感。

**已接续：** 恢复手段（Plan 17 拜拜/睡觉）；理智为 0 的后果（Plan 18 被压垮结局）；低理智幻觉（Plan 20）。
**留后续：** 理智驱动 lo-fi 后处理强度（越低越扭曲，需编辑器）；找人倾诉式恢复（对话效果）。
