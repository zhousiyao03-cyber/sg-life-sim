# Plan 3: Gameplay Integration 实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–5）
> **前置：** Plan 1（可玩竖切片）✅ + Plan 2（核心系统骨架，五大 Subsystem + 存档）✅
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §5（玩法循环）, §6（系统）
> **关联 plan：** [Plan 1](2026-05-23-engine-validation-prototype.md) · [Plan 2](2026-05-24-core-systems-skeleton.md)

**Goal:** 把 Plan 2 的六大 GameInstance Subsystem 接到 Plan 1 的可玩竖切片上 —— 让玩家的实际操作驱动系统、HUD 实时反映系统状态、软成就在条件满足时自动解锁、能从菜单存读档。跑出**第一条完整可玩的「赚钱→社交→成长」微循环**。

**为什么做这个：** Plan 2 的系统目前是「孤岛」—— 玩家走动 / 交互 NPC / 推时间还没真正驱动经济 / 关系 / 属性，HUD 也不显示这些。本 plan 是从「系统骨架」到「可玩循环」的桥接。纯工程、边界清晰、可自主验证（PIE + 桌面截图），不依赖美术 / 主观内容。

**范围内：** HUD 扩展显示系统状态；玩家操作驱动关系 / 能量；软成就自动解锁链；存读档接入菜单；PIE 集成验证。
**不做（推迟）：** 对话树分支、数值平衡、New Game+、美术、正式 UI 美化。

---

## 架构原则（沿用前两 plan）

- 系统逻辑已在 Plan 2 的 C++ 核心 + Subsystem，本 plan 只做**接线**：玩家 / UI 读写 Subsystem，Subsystem 之间不新增耦合（跨系统协调走「Director」订阅委托）。
- UI 继续用**纯 C++ UMG**（`USGHudWidget` / `USGLocationMenuWidget` 扩展），不碰 BP 控件树。
- 切关卡（OpenLevel）销毁玩家 + HUD，但所有系统在 GameInstance 层跨关卡保留；HUD 每次 `BeginPlay` 重新订阅 Director 委托。
- 新增 UCLASS/UPROPERTY 需关编辑器完整 rebuild（Live Coding 不刷新反射）。

---

## Task 序列

### Task 1: HUD 扩展 + NPC 稳定 Id
- `USGHudWidget` 加三块：**钱包行**（右上：现金 + 净资产）、**属性行**（左下：能量/心情/健康）、**成就 toast**（顶部居中，几秒后消失）。新方法 `SetWalletText` / `SetStatsText` / `ShowAchievementToast`。
- `ASGInteractableNPC` 加 `FName NpcId`（关系系统的 key）+ `GetNpcId()`；默认给两个现有 NPC 各设 Id（如 `AhHua` / `Auntie`）。

### Task 2: 玩家操作驱动系统
- `ASGPlayerCharacter::DrawPrototypeHUD` 扩展：读 `UEconomySubsystem`（现金/净资产）+ `UPlayerStateSubsystem`（属性）填 HUD 钱包/属性行。
- `TryInteract`：交互 NPC → `URelationshipSubsystem::AddAffinity(NpcId, +5)` + 消耗能量（`ModifyAttribute(Energy, -5)`）；对话气泡追加「（好感 +5 · 当前：<等级>）」。

### Task 3: AchievementDirector（软成就自动解锁）
- 新 `UAchievementDirector : UGameInstanceSubsystem`：`Initialize` 订阅 `UEconomySubsystem::OnBalanceChanged` 与 `URelationshipSubsystem::OnRelationshipChanged`，按规则判断并调 `UProgressSubsystem::MarkAchieved`。
- 软成就（spec §6.4）首批：`FirstSalary`（发过薪：现金因 Salary 流水首次 >0）、`NetWorth10k`（净资产 ≥ $10k）、`FirstFriend`（任一 NPC 好感达「朋友」档）。
- 成就 Id 集中在 `Systems/SGAchievementIds.h`（FName 常量）。判断规则做成可测纯函数 + 集成测试（`InitializeStandalone` 跑真实子系统验证解锁）。
- HUD 接 `UProgressSubsystem::OnAchievementUnlocked` → `ShowAchievementToast`。

### Task 4: 存读档接入菜单
- `USGLocationMenuWidget` 加「存档」「读档」两个按钮 → `USaveGameSubsystem::SaveToSlot/LoadFromSlot(DefaultSlot)`；标题改「菜单」。存/读后 HUD toast 提示。

### Task 5: 集成验证 + 文档
- PIE 跑一遍：走动→交互加好感→推时间发薪→看 HUD 钱/属性变化→成就 toast→存档→改动→读档复原。桌面截图确认 HUD。更新本文件勾选。

---

## Definition of Done

玩家在 PIE 里的实际操作能驱动经济/关系/属性并实时反映在 HUD；至少一条软成就（如 FirstSalary）在满足条件时自动 toast 解锁；菜单存读档可用且经 PIE 验证复原。新增逻辑有 AutomationTest 覆盖，全套测试保持全绿。UI 美化/内容/平衡留给后续 plan。

---

## 完成情况（2026-05-24）

| Task | 内容 | commit |
|---|---|---|
| 1+2 | HUD 显示钱包/属性/成就 toast；NPC NpcId；玩家交互驱动好感+耗能量、对话气泡显示等级 | 17eb0d2 |
| 3 | `UAchievementDirector` 订阅经济/关系委托自动解锁软成就（FirstSalary/NetWorth10k/FirstFriend） | （Task3 commit） |
| 4 | 菜单加存档/读档按钮 → `USaveGameSubsystem` | 1c6fbd2 |
| 5 | PIE 验证 + 文档 | （本提交） |

**验证：**
- **自动化**：全套 **26 个 AutomationTest 全绿**，含两条 headless `InitializeStandalone` 集成测试——`MonthlyLoopAndSave`（推时间→月结→能量→**成就自动解锁**→存档→读档复原）与 `AchievementUnlocks`（发薪/净资产/好感分别触发三个软成就）。这覆盖了全部接线逻辑与跨子系统连线。
- **PIE 实机**：编辑器内 Play，桌面抓屏确认 HUD 在等距视口渲染——状态行（`Day 1 · 周一 · 早 · [E]/[T]/[M]`）+ 属性行（`能量 100 · 心情 60 · 健康 80`）。钱包行改为与属性行同样的**左列**布局（右上角锚点在编辑器窗口化 PIE 视口里被裁剪，左列锚点稳定可见，且单列信息更易读）。

**偏差/经验：**
- HUD 原设计钱包行在右上角；编辑器窗口化 PIE 下右锚点 widget 会被视口右边裁掉，改成左列堆叠（状态/钱包/属性）——shipping 全屏下右锚点本可用，但左列更稳、可读性更好。
- **PIE 里无法用 Python 驱动 GameInstance 子系统**：`SubsystemBlueprintLibrary` 未暴露给 Python、`GameInstance.get_subsystem` 也没有；加上输入注入（LocalPlayer）受限，所以「玩家操作→系统」的运行时链路靠 26 个自动化测试覆盖，PIE 截图只验证渲染。
- 加 `UAchievementDirector` 后，老的 `MonthlyLoopAndSave` 测试里手动 `MarkAchieved("FirstSalary")` 变成 Director 已自动解锁 → 改为断言 `HasAchieved`（反而强化了测试）。
