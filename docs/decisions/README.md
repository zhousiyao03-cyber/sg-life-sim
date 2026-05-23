# 架构决策记录（ADR）

本目录记录项目里所有"非显而易见"的设计 / 工程决策 —— 为什么选这条路，放弃了哪些替代方案，未来什么情况下要重新评估。

## 格式

采用 [Michael Nygard 的 ADR 模板](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions)。每个 ADR 包含：

- **Status**：Proposed / Accepted / Deprecated / Superseded
- **Context**：当时面对的问题、约束
- **Decision**：选了什么
- **Consequences**：这个选择的好处 + 代价 + 风险

## 命名规则

`<seq>-<short-slug>.md`，例如 `0001-use-ue5-6-1-lts.md`。序号单调递增，永不重用，废弃也只是改 Status。

## ADR 列表

| # | 标题 | 状态 | 日期 |
|---|------|------|------|
| 0001 | [Use UE5.6.1 LTS over 5.7.4](0001-use-ue5-6-1-lts.md) | Accepted | 2026-05-23 |
| 0002 | [Defer meshy.ai to Phase 2+](0002-defer-meshy-ai.md) | Accepted | 2026-05-23 |
| 0003 | [Isometric 45° orthographic camera](0003-isometric-45-camera.md) | Accepted | 2026-05-23 |
| 0004 | [Pure sandbox MVP (no chapters)](0004-pure-sandbox-mvp.md) | Accepted | 2026-05-23 |
| 0005 | [GameInstanceSubsystem for cross-scene state](0005-gameinstancesubsystem-for-time.md) | Accepted | 2026-05-23 |

## 何时写新 ADR

- 选了某个引擎 / 框架 / 库 / 服务
- 砍掉了一个原本计划的功能
- 改了核心架构
- 推翻了之前的 ADR（写新 ADR + 把旧 ADR Status 改 Superseded）

**不需要 ADR 的**：
- 实现细节（哪个变量叫什么名字）
- 一次性 bug 修复
- UI 微调

## 决策怎么"撤回"

不要删 ADR。写一个新 ADR，状态写 Accepted，在 Context 里说"this supersedes ADR-NNNN"。把旧 ADR 状态改成 Superseded by ADR-NNNN。Git history 保留全部演进过程。
