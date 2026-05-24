# Plan 5: Dialogue Engine 实施计划

> **状态：** 进行中（2026-05-24 起草并开始实现）
> **前置：** Plan 1–4 ✅（系统 + 可玩循环 + 进阶/终局都已就绪）
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md §6.3（关系/对话）
> **关联 plan：** [Plan 2](2026-05-24-core-systems-skeleton.md)（曾推迟「完整对话树 DialogueSystem」到后续 plan）

**Goal:** 做一个**数据驱动的对话树引擎** —— 节点 / 选项 / 条件门控 / 效果应用，纯 C++ 可测，接关系/经济/进度/身份系统。这是 Plan 2 明确推迟的「对话树」。

**范围内：** 对话数据结构 + 运行时状态机（导航 + 按条件过滤选项 + 产出效果）+ Subsystem（把条件/效果接到真实系统）+ 一棵示例对话树 + 单元/集成测试。
**不做（推迟到「对话 UI」plan）：** 对话选择的 UMG 界面、把 NPC 交互流程改成开对话树、正式文案内容、配音。引擎先行，UI 后接——与 Plan 2→3 的拆法一致。

---

## 架构

沿用纯 C++ 核心 + Subsystem 薄壳。**条件判定 / 效果应用与具体系统解耦**：核心只懂「节点图 + 条件/效果是数据」，对外用注入的求值器（`TFunctionRef`）判定条件、把选中项的效果**返回**给 Subsystem 去应用。这样核心可用 lambda 求值器单测，Subsystem 才把条件/效果接到 relationship/economy/progress/residency。

## Task 序列

### Task 1: 对话数据类型 + FDialogueSystem 核心 + 测试
- `DialogueTypes.h`：`EDialogueConditionType`（None/MinAffinity/MaxAffinity/MinResidency/HasAchievement）、`EDialogueEffectType`（None/AddAffinity/AddMoneyCents/MarkAchievement/EndDialogue）、`FDialogueCondition`、`FDialogueEffect`、`FDialogueChoice`（文本+下一节点+条件+效果）、`FDialogueNode`（id/说话人/台词/选项）、`FDialogueTree`（id/根节点/节点表）。
- `FDialogueSystem`（纯 C++）：`Start(tree)` / `GetCurrentNode` / `IsActive` / `GetAvailableChoiceIndices(evaluator)`（按条件过滤）/ `TryChoose(index, evaluator, outEffect)`（校验+产出效果+跳转下一节点，无下一节点或 EndDialogue 则结束）。
- 测试：用 lambda 求值器测导航、条件过滤、非法选项、结束。

### Task 2: UDialogueSubsystem 薄壳 + 条件/效果接线
`UDialogueSubsystem`：内置对话树注册表（C++ 建示例树）+ UFUNCTION（`StartDialogue(treeId)` / `GetCurrentSpeaker` / `GetCurrentLine` / `GetChoiceTexts` / `ChooseOption(visibleIndex)` / `IsDialogueActive`）+ `OnDialogueChanged` 委托。求值器读 relationship/residency/progress；效果应用器写 relationship(加好感)/economy(钱)/progress(成就)。一棵示例树（如邻居 AhHua：选项含「送礼+好感」「闲聊」，高好感解锁特殊分支）。

### Task 3: 接存档 + 集成测试 + 文档
存档记录对话产生的持久效果（已通过既有系统存档间接保存；对话「访问过的节点/flag」如需持久再单列）。集成测试（`InitializeStandalone`）：开对话→选加好感选项→关系上升→高好感解锁分支可见→选项效果应用正确。更新文档/README/记忆。

---

## Definition of Done

对话树引擎有数据结构 + 运行时 + 条件门控 + 效果应用 + Subsystem 薄壳 + 单元/集成测试，且接通 relationship/economy/progress。全套测试保持全绿。对话 UI、正式文案、NPC 交互流程改造留「对话 UI」plan。
