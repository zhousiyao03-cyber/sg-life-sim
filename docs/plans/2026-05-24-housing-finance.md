# Plan 7: Housing Finance（按揭购房融资）实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–3）
> **前置：** Plan 1–6 ✅
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.2（支出：房首付 $50k~$500k；经济事件：HDB 政策/利率变化）、§6.4（资产：房产阶梯）

**Goal:** 把买房从「一次性全现金扣款」升级为**真实的按揭融资循环** —— 首付 + 贷款本金 + 逐月还款（利息随余额递减）+ 提前结清。这是 README 主打的核心体感（"在房贷计算器前发呆…决定这座岛是不是你的家"），也让净资产/终局多出「负债」这一维真实焦虑。

**范围内：** 纯 C++ `FMortgage` 账本 + `FAssetsSystem` 接按揭 + `UAssetsSubsystem` 融资接线（首付扣款、月供自动扣、提前还清）+ 净资产计负债 + 存档 + HUD 房贷余额显示 + 游戏内入口（菜单按钮）+ 单元/集成测试。
**不做（留后续）：** 多笔按揭 / 再融资 / 利率随机事件（留「经济事件」plan）、CPF OA 抵首付与月供（先纯现金，后续可加）、卖房还贷。

---

## 模型（简化但有新加坡味，数值为可调默认）

- **首付比例 `DownPaymentPercent = 25`**（贷款价值比 LTV 75%，贴近 SG HDB/银行房贷）。
- **年利率 `MortgageAnnualRatePerMille = 26`（2.6%/年，HDB 优惠贷款档）。** 月利息 = 未还本金 × 26 / 12000（整数运算）。
- **贷款年限 `MortgageTenureMonths = 300`（25 年）。** 采用**等额本金（直线本金）**：每月还固定本金 `原始贷款 / 年限` + 当月利息（利息随余额递减，总月供逐月降）—— 比等额本息整数实现更干净、可精确断言。末月把不足一档的本金一次还清。
- **月供强制扣（`Charge`，允许现金变负=逾期/欠债）** —— 还不起会拖累净资产，正是「房贷焦虑」的体感，不做 hard fail（符合 spec §6.5）。
- **净资产**：房估值仍计入资产，但**减去未还本金（负债）** → `GetAssetNetWorthContribution` 扣掉 `OutstandingPrincipal`。终局 net worth 自动反映负债（`UEndingSubsystem` 已 `Eco净资产 + 资产贡献`）。
- **拥有房产**：按揭中的房算「自有」（`OwnsHome` 为真）—— 扎根终局认这套房，符合现实。

## Task 序列

### Task 1: FMortgage 核心 + FAssetsSystem 接按揭 + 单测
`FMortgage`（纯）：`OutstandingPrincipalCents` / `AnnualRatePerMille` / `MonthlyPrincipalCents`；`IsActive` / `InterestDueCents` / `PrincipalDueCents`(min(余额, 月供本金)) / `PaymentDueCents` / `PayScheduledMonth`(扣本金、返还应付现金) / `PayoffAmountCents`(余额+当月利息)。`FAssetsSystem` 加 `Mortgage` + `OpenMortgage(principal, ratePerMille, tenureMonths)`；`GetAssetNetWorthContribution` 减未还本金；`RestoreState` 扩展按揭字段。单测：开贷、净资产含负债、逐月利息递减/本金递减、末月清零、提前结清额。

### Task 2: UAssetsSubsystem 融资接线 + 存档 + 集成测试
`BuyHousingFinanced(Tier)`：首付从现金 `TryWithdraw`，成功则升 tier + `OpenMortgage(余额)`；现金不够首付返回 false。月初 `HandleTimeAdvanced` 的跨月循环里：有按揭则 `PayScheduledMonth` → `Eco->Charge(Cash, 应付, "Mortgage")`。`PrepayMortgage()`：`TryWithdraw(结清额)` 成功则清零。`SGSaveGame` 加 3 个按揭字段 + gather/apply。集成测试（`InitializeStandalone`）：按揭买房→现金只扣首付→推数月→月供逐月扣、本金递减→净资产含负债→提前结清→存读档复原。

### Task 3: 可玩入口 + 重建 + 测试 + 文档
HUD 钱包行有按揭时追加「房贷余额 $X」。地点菜单加「按揭买组屋」「提前还清房贷」按钮，让循环游戏内可达。全量重建（新 UCLASS/字段需刷反射）+ headless 全套测试全绿。更新文档/README/记忆。

---

## Definition of Done

按揭买房后只扣首付、月供逐月自动扣且利息随余额递减、可提前结清、未还本金计入净资产负债并影响终局；状态进存档；HUD/菜单游戏内可达。全套测试保持全绿、无回归。

---

## 完成情况（2026-05-24）

全部 3 个 Task 完成。全量重建成功，**全套 48 个 AutomationTest 全绿、零失败**（44 + 本 plan 新增 4）。

| Task | 产出 | 测试 |
|---|---|---|
| 1 | `FMortgage`（账本）+ `FAssetsSystem` 接按揭（OpenMortgage / 净资产减负债 / RestoreState 扩展） | `SGLifeSim.Mortgage.*` ×3 |
| 2 | `UAssetsSubsystem`：`BuyHousingFinanced`（首付 25%）/ 月初自动扣月供（强制、可欠债）/ `PrepayMortgage`；`SGSaveGame` 3 字段 + gather/apply | `Integration.MortgageFinancedPurchase` ×1 |
| 3 | HUD 钱包行显示房贷余额+月供；地点菜单加「按揭买组屋」「提前还清房贷」按钮 | （重建 48 绿） |

**模型实测数（默认值）：** 组屋 $400k → 首付 $100k、贷款 $300k；月供本金 $1000，首月利息 $650（30000000×26/12000）→ 月供 $1650，利息随余额递减。净资产里房产权益 = 估值 − 未还本金。

**坑/要点：**
- **跨月触发**：月供和投资回报都在 `HandleTimeAdvanced` 的 `while (CurrentMonth > LastReturnMonth)` 循环里逐月处理（一个月 = 28×5 = 140 block）；月供用 `Eco->GetEconomy().Charge`（强制扣、允许现金变负=逾期，不 hard fail）。
- **净资产负债**：`GetAssetNetWorthContribution` 减 `OutstandingPrincipal`，终局 `ComputeTotalNetWorth`（economy + 资产贡献）自动反映负债，无需改 Ending。
- `FMortgage` 是纯 struct（非 USTRUCT），存档存 3 个标量字段而非整个结构。
- 测试辅助函数 `MGDollars` 文件内唯一名，避开 unity build ODR（见 [[project-plan3-plan4]] 同款坑）。

**留后续：** CPF OA 抵首付/月供、利率随机事件（经济事件 plan）、再融资、卖房还贷、多笔按揭。
