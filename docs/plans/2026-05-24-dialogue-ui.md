# Plan 6: Dialogue UI 实施计划

> **状态：** ✅ 完成（2026-05-24 起草并当天实现完毕，Task 1–3）
> **前置：** Plan 1–5 ✅（含 Plan 5 数据驱动对话引擎 `FDialogueSystem` / `UDialogueSubsystem`）
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.3（关系/对话）
> **关联 plan：** [Plan 5](2026-05-24-dialogue-engine.md)（明确推迟「对话 UI、E 交互改为开对话树」到本 plan）

**Goal:** 给 Plan 5 的对话引擎接上**玩家可操作的界面** —— 靠近 NPC 按 E 弹出对话面板（说话人 + 台词 + 可点选项），选项按条件门控显示、点击施加效果并推进，对话结束自动关闭。这是 Plan 5 明确推迟的「对话 UI」。

**范围内：** 纯 C++ UMG 对话控件 + E 交互流程改造（开对话树取代旧的「+5 好感气泡」）+ 输入模式切换（鼠标可点）+ 全量重建 + 回归测试。
**不做（留后续）：** 正式文案/剧情内容、配音、对话访问 flag 进存档、打字机/立绘等表现层美化、对话期间禁用移动。引擎(Plan 5)→UI(本 plan) 的拆法延续 Plan 2→3。

---

## 架构

沿用纯 C++ UMG（控件树在 `RebuildWidget()` 里用 `WidgetTree` 构造，无 BP widget 资产）—— 与 `USGHudWidget` / `USGLocationMenuWidget` 一致。对话**逻辑全在 Plan 5 的 `UDialogueSubsystem`**（GameInstance 层、跨关卡），UI 只是它的「视图」：拉 `GetCurrentSpeaker/GetCurrentLine/GetChoiceTexts`，把点击转成 `ChooseOption(VisibleIndex)`，订阅 `OnDialogueChanged` 自动刷新。视图无状态，逻辑/存档不受影响。

## Task 序列

### Task 1: USGDialogueWidget 纯 C++ UMG ✅
`USGDialogueWidget`（`UUserWidget` 子类）：底部对话面板（半透明黑底，锚定底边、左右留边避开窗口化 PIE 右边裁切坑）= 说话人文本 + 自动换行台词 + 一列选项按钮。预建固定 `MaxChoices=6` 个按钮，刷新时只改文本/可见性（结构稳定）。`OpenForTree(TreeId)`：`StartDialogue` → 订阅 `OnDialogueChanged` → 加 viewport(zorder 80) + `FInputModeGameAndUI` + 显示鼠标 → `Refresh()`。`Close()`：解绑 + 还 `FInputModeGameOnly` + 移出 viewport。

### Task 2: E 交互改为开对话树 ✅
`ASGPlayerCharacter::TryInteract`：靠近 NPC 按 E → 交谈耗能量(-5) → 若该 `NpcId` 有注册对话树则懒创建 `USGDialogueWidget` 并 `OpenForTree(NpcId)`（**好感不再在交互瞬间平加 +5，改由对话选项效果给** —— 闲聊+3/送礼+10）。没有对话树时兜底走旧的 HUD 气泡显示一句台词。示例树 id `AhHua` 已与 `ASGInteractableNPC` 默认 `NpcId` 对齐。

### Task 3: 全量重建 + 回归测试 + 文档 ✅
新 UCLASS 需全量重建刷反射（Build.bat，编辑器须关闭）。headless 跑全套自动化测试确认无回归。更新文档/README/记忆。

---

## Definition of Done

按 E 能弹出数据驱动的对话面板，选项按条件门控显示、点击推进并施加效果到真实系统，对话结束自动关闭交还输入。全套测试保持全绿、无回归。

---

## 完成情况（2026-05-24）

全部 3 个 Task 完成。全量重建成功，**全套 44 个 AutomationTest 仍全绿、零失败**（headless `UnrealEditor-Cmd ... -nullrhi`）。

| Task | 产出 |
|---|---|
| 1 | `Public/UI/SGDialogueWidget.h` + `Private/UI/SGDialogueWidget.cpp`（说话人/台词/6 选项按钮 + Open/Close + 订阅刷新） |
| 2 | `ASGPlayerCharacter::TryInteract` 改为开对话树；新增 `DialogueWidget` 成员；好感改由对话效果给 |
| 3 | 全量重建 + 44 测试回归绿；本文档 + README + 记忆 |

**实现要点 / 坑：**
- **`UButton::OnClicked` 是无参 dynamic multicast，`AddDynamic` 宏在编译期 stringify 函数名** —— 不能在循环里用运行时函数指针数组绑定。解法：循环建按钮，循环外逐个显式 `AddDynamic(this, &USGDialogueWidget::OnChoiceN)`（固定 6 个 `OnChoice0..5` → `Choose(N)`）。
- `ChooseOption` 内部 `Broadcast(OnDialogueChanged)` → `HandleDialogueChanged`：对话还活着就 `Refresh`，结束（`EndDialogue`/无下个节点）就 `Close`。所以「选最后一项→关面板」走的是同一条委托链，无需特判。
- `OpenForTree` 里 `StartDialogue` 的首次 broadcast 发生在订阅之前（无害），订阅后再手动 `Refresh()` 填首屏。
- 对话面板锚底边，复用 Plan 3 的教训：**右边/右上锚点在窗口化 PIE 会被视口裁切**，所以面板锚 `(0.04,1, 0.96,1)` 贴底拉宽。

**验证范围说明（诚实标注）：** 逻辑层（导航/门控/效果/存档）由 Plan 5 的 44 个自动化测试覆盖，本 plan 重建后全绿无回归；**UI 的 PIE 实时渲染本轮未做可视化复核**（headless `-nullrhi` 不构建 UMG，且 PIE 截图/前台抓屏此前已知不可靠、会干扰用户）。控件树构造与既有 HUD/菜单同一套已验证模式，编译链接通过。

**留后续：** 正式文案/剧情、对话访问 flag 进存档、对话期间禁移动、表现层美化（打字机/立绘/音效）。
