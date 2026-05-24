# Plan 2: Core Systems Skeleton 实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–8 全绿）
> **前置：** Plan 1（Engine Validation Prototype）已完成，GO 决策（用户主观项待勾，但工程上继续推进）
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6, §10.3
> **关联 plan：** docs/plans/2026-05-23-engine-validation-prototype.md

**Goal:** 把 spec §10.3 的核心系统从"只有 TimeSystem"扩成一组可测、数据驱动、Blueprint 可调用的 C++ 系统骨架，验证 spec §6 的经济/关系/进度机制能落地。沿用 Plan 1 验证过的架构：**纯 C++ 逻辑核心（零 UE GameplayFramework 依赖，方便 TDD）+ UGameInstanceSubsystem 薄壳暴露给 Blueprint**。

**范围（本 plan 做什么）：** 系统的**数值/逻辑骨架 + 单元测试 + Subsystem 薄壳**。不含正式 UI、不含真实内容文案、不含美术。内容（NPC 文案、经济平衡参数）后续数据驱动填充。

**不做（明确推迟）：** 完整对话树 DialogueSystem（spec §6.3 关系系统先做好感数值，对话树留后）、SaveSystem 的磁盘序列化（先做内存状态聚合，磁盘存档单列一个 task）、所有 UI 美化。

---

## 架构原则（沿用 Plan 1 经验）

1. **逻辑核心纯 C++**：每个系统一个 `F<Name>System` 普通 C++ 类（非 UObject），零 UE Actor/World 依赖，可被 AutomationTest 直接 new 出来测。
2. **Subsystem 薄壳**：`U<Name>Subsystem : UGameInstanceSubsystem` 持有 `F<Name>System`，用 `UFUNCTION(BlueprintCallable/Pure)` 暴露，用 `DECLARE_DYNAMIC_MULTICAST_DELEGATE` 广播变化。跨关卡状态保留（GameInstance 级，已由 TimeSubsystem 验证）。
3. **TDD**：先写 `IMPLEMENT_SIMPLE_AUTOMATION_TEST`，红→绿→提交。
4. **数据驱动**：可平衡的数值（工资、利率、账单）走 `USTRUCT` + 常量表，后续可换 DataTable，不硬编码散落各处。
5. **新增 UCLASS/UPROPERTY 需关编辑器完整 rebuild**（Live Coding 不刷新反射）——我自己管编辑器生命周期。

---

## 完成情况（2026-05-24）

全部 8 个 Task 当天实现完毕，**25 个 AutomationTest 全绿**（headless `Automation RunTests SGLifeSim`），含一条真实跑子系统的端到端集成测试。

| Task | 内容 | 产出 | 测试 | commit |
|---|---|---|---|---|
| 1 | EconomySystem 钱包+CPF | `EconomyTypes.h` / `EconomySystem.{h,cpp}` | `SGLifeSim.Economy.*` ×7 | 395763a |
| 2 | EconomySubsystem BP 壳 | `EconomySubsystem.{h,cpp}` | （UHT 编译验证 BP 可见） | e097c78 |
| 3 | 时间↔经济月度结算 | TimeSystem 加月份；`Charge`；`FMonthlyFinance`；Subsystem 订阅 `OnTimeAdvanced` | `TimeSystem.MonthRollover` / `Economy.MonthlySettlement` 等 | 699bf08 |
| 4 | ProgressSystem 软成就 | `ProgressSystem.{h,cpp}` | `SGLifeSim.Progress.*` ×3 | bb77679 |
| 5 | RelationshipSystem 好感 | `RelationshipTypes.h` / `RelationshipSystem.{h,cpp}` | `SGLifeSim.Relationship.*` ×4 | 2445221 |
| 6 | PlayerStateSubsystem 属性 | `PlayerStatsTypes.h` / `PlayerStats.{h,cpp}` / `PlayerStateSubsystem.{h,cpp}` | `SGLifeSim.PlayerStats.*` ×4 | 666660a |
| 7 | SaveSystem 存读档 | `SGSaveGame.h` / `SaveGameSubsystem.{h,cpp}` + Progress/Relationship 子系统壳 | `SGLifeSim.Save.RoundTrip` | 9d7570b |
| 8 | 集成验证 | `CoreSystemsIntegrationTest.cpp` | `SGLifeSim.Integration.MonthlyLoopAndSave` | （本提交） |

**关键设计落点 / 与原计划的偏差：**
- 金额统一以「分」(int64) 存，规避浮点误差。CPF 费率：雇员 20% + 雇主 17%，进 OA/SA/MA 按 620/160/220 千分比，OA 兜底余数保证不丢分。
- 「月」定义为 **28 天（4 周）**，让每月固定从周一开始、月度数学干净（spec 只说「每月 1 号发薪」，未规定月长）。
- 集成走 **`UGameInstance::InitializeStandalone()`** 在 headless 下拉起真实子系统，验证 `OnTimeAdvanced` 绑定、`InitializeDependency` 顺序、跨月自动结算、每日能量恢复、存读档回灌——这些是子系统**连线**层面的正确性，纯核心单测覆盖不到。
- 原计划只为 Time/Economy/PlayerState 建子系统；Task 7 聚合存档时补建了 **`UProgressSubsystem` / `URelationshipSubsystem`** 薄壳（成就/好感也需要一个能被 gameplay 改、能被存档读的家），把原生委托桥接成 BP 动态委托。

---

## Task 序列

### Task 1: EconomySystem 骨架 + 单元测试（最独立，先做）✅

spec §6.2。一个纯 C++ 钱包 + 收支记账 + 月度结算。

**Files:**
- `Source/SGLifeSim/Public/Systems/EconomyTypes.h` — `ECurrencyAccount`（Cash/Bank/CPF_OA/CPF_SA/CPF_MA）枚举、`FMoneyTransaction` 结构
- `Source/SGLifeSim/Public/Systems/EconomySystem.h` — `FEconomySystem`
- `Source/SGLifeSim/Private/Systems/EconomySystem.cpp`
- `Source/SGLifeSim/Private/Tests/EconomySystemTest.cpp`

**核心 API（最小集）：**
- `int64 GetBalance(ECurrencyAccount) const` — 以「分」存（避免浮点误差），SGD cents
- `void Deposit(ECurrencyAccount, int64 cents, FName reason)`
- `bool TryWithdraw(ECurrencyAccount, int64 cents, FName reason)` — 不足返回 false
- `void ApplyMonthlySalary(int64 grossCents)` — 按 CPF 规则分账（自付 20% → OA/SA/MA，雇主 17% 进 CPF），税暂记账
- `int64 GetNetWorth() const` — 各账户汇总
- 交易流水 `TArray<FMoneyTransaction>`（存档/UI 用）

**测试：** 初始余额为 0；存取款；取超额失败且余额不变；发薪后 CPF 分账比例正确、现金到手 = gross - 自付 CPF；净资产 = 各账户和。

**验收：** 编译 + 3~5 个 AutomationTest 全绿。

---

### Task 2: EconomySubsystem（Blueprint 薄壳）

把 `FEconomySystem` 包成 `UEconomySubsystem : UGameInstanceSubsystem`，UFUNCTION 暴露余额/存取/发薪，`FOnBalanceChanged` 委托广播。验证 Blueprint 可见 + 跨关卡保留（与 TimeSubsystem 同模式）。

---

### Task 3: 时间↔经济联动（月度账单/发薪触发）

TimeSubsystem 推进到「每月 1 号」时，EconomySubsystem 自动发薪 + 扣固定账单（房租/水电/交通）。用 TimeSubsystem 的 `OnTimeAdvanced` 委托驱动，验证 spec §5.3 周/月循环。需要 TimeSystem 增加「跨月」检测（GetMonthNumber / 当前是否月初）。

---

### Task 4: ProgressSystem 骨架（软成就追踪）

spec §6.4。`FProgressSystem` 追踪软成就节点（第一笔 $10k、第一次升职…），用一个 `TSet<FName>` 记已达成 + `MarkAchieved/HasAchieved` + `FOnAchievementUnlocked` 委托。数据驱动：成就定义表。单元测试覆盖去重 + 查询。

---

### Task 5: RelationshipSystem 骨架（好感数值）

spec §6.3。`FRelationshipSystem`：`TMap<FName, int32>` 存 NPC 好感 0~100，`AddAffinity/GetAffinity/GetTier`（陌生/认识/熟悉/朋友/知己/恋人），`FOnRelationshipChanged` 委托。对话树留后续 plan。单元测试覆盖 clamp + 等级阈值。

---

### Task 6: PlayerStateSubsystem（主角属性聚合）

spec §6.4 主角属性（健康/心情/能量/专业/社交/见识 0~100）。一个聚合 Subsystem 持有这些数值 + clamp + 委托。能量每日恢复（接 TimeSubsystem 跨天事件）。

---

### Task 7: SaveSystem（内存状态聚合 → 磁盘）

把 Time/Economy/Progress/Relationship/PlayerState 的状态聚合成一个 `USaveGame` 子类，`UGameplayStatics::SaveGameToSlot/LoadGameFromSlot`。先做基础存读档，New Game+ 留后。

---

### Task 8: 完整集成验证 + 文档

PIE 跑一遍：发薪→记账→推时间→月度结算→成就解锁→存档→读档，全部经 Subsystem 验证（日志/状态查询，不依赖 UI）。更新本文件勾选。

---

## Definition of Done

Task 1–8 完成 = spec §6 的五大系统都有 C++ 骨架 + 单元测试 + Blueprint 薄壳 + 基础存档，且至少一条「发薪→账单→成就」联动链路在 PIE 验证通过。UI/内容/平衡留给后续 plan。

**✅ 已达成（2026-05-24）：** 五大系统（Economy/Time/Progress/Relationship/PlayerState）均有纯 C++ 核心 + AutomationTest + GameInstanceSubsystem 薄壳 + 统一 USaveGame 存档；「推时间→月度发薪+账单→能量恢复→成就→存档→读档」整条链路由 `SGLifeSim.Integration.MonthlyLoopAndSave` 在 headless GameInstance 上验证通过（比 PIE 更轻、可 CI）。25/25 测试全绿。UI/内容/数值平衡按计划留给后续 plan。
