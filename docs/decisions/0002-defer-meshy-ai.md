# ADR 0002: Defer meshy.ai to Phase 2+, Use Free Asset Sources for MVP

- **Status**: Accepted
- **Date**: 2026-05-23
- **Decider**: Project owner (with Claude)

## Context

Spec §11 原本规划 **meshy.ai** 作为 MVP 阶段的主要 3D 资产生成管线（道具 / 建筑 / 车辆 / 部分 NPC），原因是新加坡设定需要很多通用 marketplace 找不到的本土元素（ERP gantry / 榴莲 / 组屋阳台 / Kopi 杯 / EZ-Link 卡等）。

但 meshy.ai 实际使用要付费：
- 免费版：200 credits / 月（约能生成 5~10 个资产）
- Pro：$20 / 月（1000 credits）
- Max：$60 / 月（4000 credits）

MVP 阶段（Plan 1）只需要验证"资产能进 UE5"这件事，不需要真的开始生产新加坡特色资产。

## Decision

**MVP 阶段（Plan 1~2）改用免费资产源**：

1. UE5 Fab Marketplace 的免费资产（首选，原生集成）
2. Sketchfab CC0 区
3. Kenney.nl CC0 资产
4. UE5 Starter Content（占位）

**meshy.ai 推迟到 Plan 3+** 重新评估，触发条件：
- 项目进入真正铺新加坡特色道具的阶段
- 连续 3 次在免费源找不到需要的资产
- 找到的资产无法通过统一 toon shader 调到风格一致

## Alternatives Considered

| 选项 | 月成本 | 何时启用 |
|------|--------|---------|
| **免费源（Fab/Sketchfab/Kenney）** ✅ | $0 | MVP 启动即用 |
| meshy.ai 免费版 200 credits | $0 | 不够 prototype 阶段大规模生产 |
| meshy.ai Pro | $20/月 | Plan 3+ 重新评估 |
| 买 Synty / POLYGON Asia 资产包 | ~$50 一次 | Plan 3+ 评估，可能更合算 |
| 自己学 Blender 建模 | 时间成本巨大 | 不在一人开发可行范围 |

## Consequences

### 好处

- **MVP 阶段零美术成本**。
- **降低风险**：先验证整条游戏循环跑通，再决定花不花 $20/月。
- **资产风格选择更广**。免费源里可能找到比 AI 生成更好用的资产。

### 代价

- **缺新加坡特色道具**。Plan 1~2 的场景里不会出现榴莲 / 组屋 / ERP gantry，看起来更通用。MVP 阶段可接受（验证机制为先）。
- **风格统一更难**。多个免费源风格各异，需要更强的 UE5 后期 toon shader 来统一。

## When to Revisit

- Plan 3 启动时，由 MVP 验证数据决定：
  - 如果 MVP 跑通且玩家反馈说"缺新加坡味"，启用 meshy.ai
  - 如果 MVP 跑通且免费源 + 后期处理已经够，继续不花钱
  - 如果 MVP 失败/转向 Unity，整个 meshy 策略重订

## References

- 项目 spec §11：美术管线
- Plan 1 Task 7：[外部 3D 资产导入流程验证](../plans/2026-05-23-engine-validation-prototype.md)
- 历史：commit `a6e81b5` 完成此切换
