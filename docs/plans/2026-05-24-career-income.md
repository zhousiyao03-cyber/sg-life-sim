# Plan 8: Career & Income（职业与收入成长）实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–3）
> **前置：** Plan 1–7 ✅
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.2（主业月薪逐年增长、跳槽 +30~50%）、§6.4（软成就「第一次升职」、专业技能影响跳槽机会）

**Goal:** 给金钱循环装上**收入成长曲线** —— 月薪不再是写死的 $5000 常量，而是来自职业等级：靠专业技能 + 在职时长**升职**涨薪，或**跳槽** +35%。收入喂给已有的经济/按揭/投资循环，攒钱终于有了上升通道。

**范围内：** 纯 C++ `FCareerSystem`（等级/薪资/在职时长 + 升职/跳槽规则）+ `UCareerSubsystem`（把当前月薪推给 Economy、月初累计在职、读 PlayerState 专业技能判定升职）+ 「第一次升职」成就 + 存档 + HUD 职位/月薪显示 + 菜单入口 + 单元/集成测试。
**不做（留后续）：** 副业/外包接单、年终奖、被裁员事件、跳槽面试小游戏、按职业切换住房/通勤成本。

---

## 模型（数值为可调默认）

职业阶梯 `ECareerLevel`：Unemployed / Junior / Mid / Senior / Lead / Principal。税前月薪（分）按等级：

| 等级 | 月薪 | 升至此级需专业技能 |
|---|---|---|
| Junior（起点） | $5000 | — |
| Mid | $7500 | 50 |
| Senior | $11000 | 65 |
| Lead | $16000 | 80 |
| Principal | $24000 | 92 |

- **升职**：`CanPromote` = 未到顶 && 专业技能 ≥ 目标级门槛 && 在职 ≥ `MinMonthsForPromotion`(3)。升职后薪资 = `max(当前薪资, 目标级基薪)`（不砍掉此前跳槽涨上去的部分），在职清零。
- **跳槽**：薪资 ×(1+35%)，在职清零（换公司重新算）。模拟「跳槽比内部晋升涨得多」，但失去资历。
- **薪资是唯一真相**（不每次从等级反推）—— 跳槽能让薪资高于本级基薪。
- 玩家默认专业技能 60 → 入职 3 个月后可升 Mid（门槛 50）；要到 Senior 需把专业技能练到 65，把「升职」和属性成长绑起来。
- `UCareerSubsystem` 在任何薪资变化时把新月薪推给 `UEconomySubsystem`（新增 `SetMonthlyGrossSalary`），月度结算照常用它发薪（含 CPF 分账）。

## Task 序列

### Task 1: FCareerSystem 核心 + 单测
`ECareerLevel` + `FCareerSystem`：`Level`/`GrossSalaryCents`/`MonthsInRole`；`CanPromote(Professional)` / `Promote(Professional)` / `JobHop(RaisePercent=35)` / `AdvanceMonth()` / 静态 `BaseSalaryCents(Level)` / `PromotionProfessionalReq(TargetLevel)` / `RestoreState`。单测：起点薪资、在职不足不能升、达标升级涨薪、跳槽 +35%、顶级不能再升。

### Task 2: UCareerSubsystem + 接经济 + 存档 + 集成测试
`UCareerSubsystem`：Initialize 依赖 Economy/Time/PlayerState，把当前月薪推给 Economy；订阅 time 月初 `AdvanceMonth`；`TryPromote()` 读 PlayerState 专业技能→`Promote`，首次成功标成就 `FirstPromotion`；`JobHop()`。`UEconomySubsystem::SetMonthlyGrossSalary`。`SGAchievementIds::FirstPromotion`。存档加 `CareerLevel`/`CareerGrossSalaryCents`/`CareerMonthsInRole`（读档后重推薪资）。集成测试（`InitializeStandalone`）：拉高专业技能→推月→TryPromote 涨薪→月结到手变多→跳槽→存读档复原。

### Task 3: 可玩入口 + 重建 + 测试 + 文档
HUD 显示职位 + 月薪。地点菜单加「申请升职」「跳槽（+35%）」按钮（带反馈）。全量重建 + headless 全套测试全绿。更新文档/README/记忆。

---

## Definition of Done

月薪由职业等级驱动；满足条件可升职涨薪、可跳槽 +35%；薪资变化即时反映到月度发薪；首次升职解锁成就；状态进存档；HUD/菜单游戏内可达。全套测试保持全绿、无回归。

---

## 完成情况（2026-05-24）

全部 3 个 Task 完成。全量重建成功，**全套 53 个 AutomationTest 全绿、零失败**（48 + 本 plan 新增 5）。

| Task | 产出 | 测试 |
|---|---|---|
| 1 | `ECareerLevel` + `FCareerSystem`（等级/薪资/在职 + 升职/跳槽规则 + 静态薪资&门槛表） | `SGLifeSim.Career.*` ×4 |
| 2 | `UCareerSubsystem`（推薪资给 Economy / 月初累计在职 / TryPromote 读专业技能 + 首次升职成就 / JobHop）；`UEconomySubsystem::SetMonthlyGrossSalary`；`SGAchievementIds::FirstPromotion`；存档 3 字段 | `Integration.CareerPromotionRaisesIncome` ×1 |
| 3 | HUD 属性行显示职位+月薪+专业技能；地点菜单加「申请升职」「跳槽（+35%）」按钮 | （重建 53 绿） |

**实测数：** 起步初级 $5000；专业技能 70 + 在职 3 月 → 升中级 $7500（月度到手现金 +$4930 = 净薪 $6000 − 固定账单 $1070）；跳槽 +35% → $10125。

**坑/要点：**
- **UHT 不能解析 UFUNCTION 的非字面量默认参数** —— `JobHop(int32 RaisePercent = FCareerSystem::DefaultJobHopRaisePercent)` 报 `C++ Default parameter not parsed`，改成字面量 `= 35`。
- 薪资是唯一真相：`UCareerSubsystem` 在 Initialize / 升职 / 跳槽 / 读档后都调 `SyncSalaryToEconomy()` 把月薪推给 `UEconomySubsystem`（覆盖其默认 $5000 常量），月度结算照常用它发薪（含 CPF 分账）。
- 升职门槛绑定 `UPlayerStateSubsystem` 的专业技能属性 → 把「练专业技能」和「涨薪」串起来。
- 测试辅助函数 `CRDollars` 文件内唯一名，避 unity ODR。

**留后续：** 副业/外包接单、年终奖、被裁员/降薪事件、按职业切住房/通勤成本、跳槽面试 gating。
