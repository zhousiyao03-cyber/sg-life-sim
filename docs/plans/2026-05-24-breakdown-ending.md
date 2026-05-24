# Plan 18: Breakdown Ending（精神崩溃终局 / 理智的赌注）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **承接：** Plan 16 理智系统、Plan 17 理智恢复；Plan 4 终局系统

**Goal:** 给恐惧螺旋一个真正的赌注 —— 理智耗尽会怎样？答案：第 5 种终局「**被压垮**」。理智失守压过一切人生成就（再有钱有房，心垮了就是垮了），把恐怖玩法接进既有终局框架，闭合「下沉 → 恢复 → 触底」的循环。

---

## 实现

- `EEnding` 加 `Breakdown`（被压垮，恐怖坏结局）。
- `FEndingEvaluator::EvaluateLeaning` 加 `int32 Sanity = 100`（默认参，旧 5 参调用向后兼容）。判定第 0 优先级：**理智 < 15（濒临崩溃）→ Breakdown，盖过破产/心碎/扎根/兑现/漂着一切**。
- `UEndingSubsystem::GetCurrentLeaning` 读 `USanitySubsystem` 理智传入 —— 理智一旦进 Breaking 档，HUD「走向」即显示「被压垮」作警示。
- `USanitySubsystem`：理智归零（SetSanity 落到 ≤0）→ 幂等强制 `EndingSubsystem->ChooseEnding(Breakdown)`（`bBrokenDown` 防重）。`RestoreFromSave` 走直接赋值不触发，避免读档误触。

---

## 完成情况（2026-05-24）

全量重建成功，**全套 72 个 AutomationTest 全绿、零失败**（70 + 新增 2）。

| 产出 | 测试 |
|---|---|
| `EEnding::Breakdown` + 终局评估器理智门（最高优先级）+ 理智归零强制结局 | `Ending.Breakdown`（低理智盖过一切、阈值边界、默认参向后兼容）+ `Integration.SanityBreakdownEnding`（理智<15 倾向 Breakdown、归零强制选定） |

**完整恐怖循环：** 深夜恐怖事件扣理智（Plan 15/16）→ 理智越低越频越凶（螺旋）→ 拜拜/睡觉可恢复（Plan 17）→ 撑不住归零则「被压垮」结局（Plan 18）。这是一个有风险、有对抗手段、有终局赌注的可玩管理循环。

**待 PIE / 后续：** 「被压垮」的专属 game-over 演出（黑屏/字幕/音效）目前只记录 ChosenEnding，没有结束界面；低理智的画面扭曲（lo-fi 后处理）；理智=0 前的最后警示。
