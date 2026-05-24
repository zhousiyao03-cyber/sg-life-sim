# Plan 9: Random Economic Events（随机经济事件）实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–3）
> **前置：** Plan 1–8 ✅
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.2（经济事件：币圈崩盘/大涨、公司发奖金/削减预算、利率变化）、§3（风险与焦虑的体感）

**Goal:** 给确定性的世界加入**变数与风险**。每月小概率触发一个经济事件——行情大涨/大跌冲击投资持仓、公司发年终奖、政府红包、突发账单——让攒钱之路有惊喜也有焦虑（spec 强调的「看到币圈崩盘会心痛」体感）。

**范围内：** 纯 C++ `FEconomicEventSystem`（事件枚举 + 效果表 + 注入 `FRandomStream` 的加权抽取，可确定性测试）+ `UEconomicEventSubsystem`（月初抽取并把效果应用到 Economy/Assets，广播事件供 HUD toast）+ HUD 事件提示 + 单元/集成测试。
**不做（留后续）：** 利率事件改房贷月供、裁员事件改职业（需求职循环）、事件链/剧情事件、玩家可选应对（事件分支）、按持有的具体投资工具区分波动（当前投资是单一本金）。

---

## 模型（权重/幅度为可调默认）

事件 `EEconomicEvent` + 效果 `EEventEffectType`（InvestmentReturnPerMille / CashDeltaCents / CashBonusSalaryMonthsX10）。月度加权抽取，**None 权重最高（多数月平静）**：

| 事件 | 效果 | 权重 |
|---|---|---|
| None（平静） | — | 70 |
| 股市回暖 | 持仓 +15% | 8 |
| 股市回调 | 持仓 −20% | 6 |
| 币圈大涨 | 持仓 +80% | 2 |
| 币圈崩盘 | 持仓 −50% | 3 |
| 年终奖 | 现金 +1.5 个月税前薪 | 3 |
| 政府红包（GST voucher / SG60） | 现金 +$600 | 4 |
| 突发账单（看牙/修车） | 现金 −$400 | 4 |

合计权重 100。投资效果走既有 `FAssetsSystem::AccrueInvestmentReturn(permille)`；现金走 Economy 存/扣；年终奖按 Economy 当前月薪算。

## Task 序列

### Task 1: FEconomicEventSystem 核心 + 单测
`EconomicEventTypes.h`（`EEconomicEvent` / `EEventEffectType` / `FEconomicEventDef`）+ `FEconomicEventSystem`：静态效果&权重表、`PickEvent(FRandomStream&)` 加权抽取、`GetEventDef(EEconomicEvent)`。单测：同种子可复现、抽取只返回合法事件、None 最常见（大样本）、效果定义数值正确。

### Task 2: UEconomicEventSubsystem + 接经济/资产 + 集成测试
`UEconomicEventSubsystem`：持 `FRandomStream`（`SetSeed`）；`ApplyEvent(EEconomicEvent)` 把效果应用到 Economy（存/扣现金、年终奖按月薪）与 Assets（投资涨跌），非 None 则广播 `OnEconomicEvent(FText)`；订阅 time 月初 `PickEvent`→`ApplyEvent`；`GetLastEvent`。集成测试（`InitializeStandalone`）：`ApplyEvent` 直接驱动已知事件验证现金/持仓改动正确 + 固定种子推月，`GetLastEvent` == 同种子离线抽取结果（证明接线）。

### Task 3: HUD toast + 重建 + 测试 + 文档
玩家角色订阅 `OnEconomicEvent` → HUD 弹事件 toast（复用成就 toast）。全量重建 + headless 全套测试全绿。更新文档/README/记忆。

---

## Definition of Done

每月按权重抽取经济事件，效果正确应用到投资/现金，事件可复现（同种子），非平静事件弹 HUD toast。全套测试保持全绿、无回归。

---

## 完成情况（2026-05-24）

全部 3 个 Task 完成。全量重建成功，**全套 57 个 AutomationTest 全绿、零失败**（53 + 本 plan 新增 4）。

| Task | 产出 | 测试 |
|---|---|---|
| 1 | `EconomicEventTypes.h`（`EEconomicEvent`/`EEventEffectType`/`FEconomicEventDef`）+ `FEconomicEventSystem`（效果&权重表 / `PickEvent(FRandomStream&)` 加权抽取） | `SGLifeSim.EconomicEvent.*` ×3 |
| 2 | `UEconomicEventSubsystem`（持 FRandomStream + SetSeed / `ApplyEvent` 接 Economy+Assets / 月初 PickEvent / 广播 `OnEconomicEvent`） | `Integration.EconomicEventAffectsWallet` ×1 |
| 3 | 玩家角色订阅 `OnEconomicEvent` → HUD 📰 toast | （重建 57 绿） |

**要点：**
- 抽取确定性：核心 `PickEvent` 注入 `FRandomStream`，同种子复现 → 既可单测，又支持将来 New Game+「固定剧本」。子系统默认用时间戳种子（每局不同），`SetSeed` 覆盖。
- `ApplyEvent` 月度抽取与脚本/测试共用同一入口；投资效果复用 `FAssetsSystem::AccrueInvestmentReturn(permille)`，年终奖按 `UEconomySubsystem::GetMonthlyGrossSalary()` 算（自动吃 Plan 8 的职业薪资）。
- 测试辅助函数 `EVDollars` 文件内唯一名避 unity ODR。
- 事件不进存档：效果已即时落到 economy/assets（两者都存档），事件历史本身无需持久。

**留后续：** 利率事件改房贷月供（mutate `FMortgage.AnnualRatePerMille`）、裁员事件改职业（需求职循环）、事件链/剧情事件、玩家可选应对分支、按具体投资工具区分波动。
