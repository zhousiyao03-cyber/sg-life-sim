# Plan 22: Horror Dialogue Resonance（恐怖共鸣对话分支）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **方向：** docs/decisions/2026-05-24-first-person-horror-pivot.md
> **承接：** Plan 21 恐怖图鉴、Plan 5/6 对话引擎/UI

**Goal:** 把「亲历过的都市传说」反哺进对话——你真的在电梯里碰到过那个空楼层之后，才能向看了十几年门的 Uncle Lim 坦白。被一个信你的人郑重听见，是恐怖里难得的喘息：加好感、回一点理智。让恐怖经历不只是收集数字，而成为和 NPC 的共同语言，强化「日常底色 + 恐怖渗入」的基调。纯代码（对话内容数据化 + 新条件/效果），复用既有 ValidateTree。

---

## Task 序列

### Task 1: 新条件 + 新效果类型 ✅
`EDialogueConditionType::HasDiscoveredHorror`（`Value` = `EHorrorEvent` 的 int，`Target` 不用）。`EDialogueEffectType::AddSanity`（`Value` = 回多少理智）。对话核心 `FDialogueSystem` 不碰具体系统（条件经注入求值器），故无需改核心；`ValidateTree` 只看跳转完整性，也无需改。

### Task 2: 求值器/效果器接系统 ✅
`UDialogueSubsystem::EvaluateCondition` 加 `HasDiscoveredHorror` → 查 `UHorrorCodexSubsystem::HasDiscovered`。`ApplyEffect` 加 `AddSanity` → `USanitySubsystem::Restore`。

### Task 3: 共鸣对话内容 ✅
Uncle Lim 树加 `confide` 分支：root 选项门控 `HasDiscoveredHorror(ElevatorGhostFloor)`——未亲历不可见。进 `confide` 节点 Uncle 郑重回应「我就知道。你眼神不对……叔叔信你，不是你疯了」，两个结束选项分别给「回理智 +12（被人信着踏实了些）」与「好感 +6」。

### Task 4: 测试 + 重建 ✅

---

## 完成情况（2026-05-24）

全量重建成功，**全套 76 个 AutomationTest 全绿、零失败**（75 + 新增 1）。既有 9 个对话相关测试（含 `Dialogue.ContentValidates` 树完整性校验）全部无回归。

| 产出 | 测试 |
|---|---|
| `HasDiscoveredHorror` 条件 + `AddSanity` 效果接子系统；Uncle Lim 电梯空楼层共鸣分支 | `Integration.HorrorDialogueResonance`（亲历前坦白分支不可见 → 亲历 ElevatorGhostFloor 后可见 → 选它进 confide → 选「被人信着」回理智 +12 → 对话结束） |

**设计取舍：**
- 条件经注入求值器实现，核心零改动；新增条件/效果只动 enum + Subsystem + 内容，最小侵入。
- 复用既有 `ElevatorGhostFloor`（Uncle Lim 本就讲它的传闻）做共鸣点：传闻 → 亲历 → 坦白，三段闭环，最贴合角色。
- 一个选项目前只挂一个效果（结构限制），故把「回理智」与「加好感」拆成 confide 节点的两个并列结束选项，玩家二选一。
- 测试用「按文案子串找 visible index」而非硬编码下标，避免选项顺序变动导致脆裂。

**待 PIE 复核：** 共鸣分支在对话面板里的呈现；回理智后的反馈（数字跳动 / HUD）。

**留后续：** 更多 NPC × 传说的共鸣（食阁阿姨对鬼月禁忌、同事 Wei 对地铁倒影）；坦白后解锁该 NPC 的「七月特别叮嘱」；让「一个选项多个效果」成为对话结构的能力（当前只能挂一个）。
