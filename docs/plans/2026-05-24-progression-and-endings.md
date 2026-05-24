# Plan 4: Progression & Endings 实施计划

> **状态：** 进行中（2026-05-24 起草并开始实现）
> **前置：** Plan 1 可玩竖切片 ✅ · Plan 2 核心系统骨架 ✅ · Plan 3 系统接入可玩循环 ✅
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §5.1（阶级跃迁）, §6.4（资产/身份）, §6.5（终局）
> **关联 plan：** [Plan 2](2026-05-24-core-systems-skeleton.md) · [Plan 3](2026-05-24-gameplay-integration.md)

**Goal:** 在经济/关系/属性骨架上加 spec 的「进阶 + 终局」层，让游戏第一次**有奔头、有结局**：身份阶梯（EP→PR→公民）、资产（房/车/投资）、以及 §6.5 的四种软终局评估。这是把一堆系统变成「一段有意义的人生」的关键一层。

**架构：** 沿用前三 plan —— 纯 C++ 逻辑核心（`F<Name>System`，零 UE 依赖、可单测）+ `U<Name>Subsystem` 薄壳 + TDD + 数据驱动。终局/资产汇总走「读各系统状态的纯函数」，系统间不新增耦合。

**范围内：** 身份状态机、资产 tier + 投资、投资月度回报（接 time）、四软终局评估 + 主动选择、接入存档 + HUD。
**不做（推迟）：** 对话树、剧情事件脚本、New Game+「上周目记忆」、随机经济事件、UI 美化、数值平衡。

---

## Task 序列

### Task 1: ResidencySystem（身份阶梯）
spec §6.4 身份。`FResidencySystem` 纯 C++ 状态机：`EResidencyStatus`（EP / SP / PR_Applying / PR / Citizen）。API：`GetStatus` / `ApplyForPR`（EP|SP → PR_Applying）/ `ResolvePRApplication(bool bApproved)`（→ PR 或退回原准证 + 记一次被拒）/ `Naturalize`（PR → Citizen）/ `GetPRRejectionCount`。非法转换被拒。单元测试覆盖合法/非法转换 + 被拒计数。

### Task 2: AssetsSystem（房 / 车 / 投资持仓）
spec §6.4 资产。`EHousingTier`（无/出租屋/整租组屋/自购HDB/自购公寓/多套）、`EVehicleTier`（无/Grab月卡/二手车/新车/豪车）。`FAssetsSystem`：当前房/车 tier + 投资本金（分）。API：`SetHousingTier` / `SetVehicleTier` / `AddInvestment(cents)` / `WithdrawInvestment(cents)` / `GetInvestmentValue` / `AccrueInvestmentReturn(permilleRate)`（月度复利，千分比）/ `GetAssetNetWorthContribution`（房/车估值 + 投资）。单元测试覆盖 tier 设置、投资增值、估值汇总。

### Task 3: Residency + Assets 的 Subsystem 薄壳 + 接线
`UResidencySubsystem` / `UAssetsSubsystem`（UFUNCTION 暴露 + 委托）。Assets 接 economy：买房/买车经 `UEconomySubsystem::TryWithdraw` 扣钱后才改 tier；投资从现金转入。`UAssetsSubsystem` 订阅 `TimeSubsystem.OnTimeAdvanced`，月初给投资计回报（与 Economy 月结同节奏）。

### Task 4: EndingSystem（四软终局评估）
spec §6.5。`EEnding`（None/Rooted/CashOut/Heartbreak/Adrift）。`FEndingEvaluator`：纯函数 `EvaluateLeaning(身份, 是否有房, 最高好感, 净资产)` → 当前最可能的软终局：
- 破产（净资产<0）或 PR 被拒过且仍非 PR → **心碎离开**
- PR/公民 + 有房 + 有「朋友」以上关系 → **扎根**
- 净资产 ≥ 兑现阈值（如 $300k）且未扎根 → **兑现离开**（可主动选）
- 其余（无 PR、租房、关系薄） → **留下漂着**
`UEndingSubsystem`：`GetCurrentLeaning()`（读各子系统状态算）+ `ChooseEnding(EEnding)`（玩家主动结束，记录最终结局）。单元测试覆盖四种倾向的判定边界。

### Task 5: 接入存档 + HUD + 集成测试
存档：`USGSaveGame` 加身份 / 房车 tier / 投资本金 / 已选结局字段，`USaveGameSubsystem` 采集/回灌。HUD：状态行或新行显示「身份 · 房产 · 终局倾向」。集成测试（`InitializeStandalone`）：买房→申请 PR 通过→拉好感到朋友→评估倾向=扎根→存读档复原。

### Task 6: 文档 + README + 记忆
更新本文件勾选、README 系统表、记忆。

---

## Definition of Done

身份阶梯、资产（房/车/投资 + 月度回报）、四软终局评估都有 C++ 骨架 + 单元测试 + Subsystem 薄壳 + 存档；一条「买房+PR+关系→扎根」链路在 headless 集成测试验证通过；HUD 能看到身份/房产/终局倾向。全套测试保持全绿。对话树/剧情/平衡/美化留后续 plan。
