# ADR 0003: Use Isometric 45° Orthographic Camera

- **Status**: Accepted
- **Date**: 2026-05-23
- **Decider**: Project owner (with Claude)

## Context

游戏初步设想是"新加坡 GTA"风的开放世界，但经过 brainstorm 一系列约束分析后，得出：

- **一人独立开发**，没有 3D 建模 / 动画师配合
- **想要开放世界感**但工程量必须可控
- 用户原话：**"先不做 Z 轴"** —— 接受没有立体感

这立刻把镜头选择压缩到俯视角范围内。俯视角有三种：

1. **纯顶视 90°**（Hotline Miami / GTA1）
2. **等距俯视 45°**（Disco Elysium / Stardew Valley / Cult of the Lamb）
3. **第三人称跟随但视角偏高**（Pokemon Sword/Shield）

## Decision

**用等距俯视 45° 正交相机**（UE5 Camera Component → Projection Mode: Orthographic + Spring Arm Rotation: Pitch=-45, Yaw=-45）。

## Alternatives Considered

| 选项 | 优点 | 缺点 |
|------|------|------|
| **等距 45° 正交** ✅ | 能展现建筑立面 + 街道布局；新加坡地标可识别；meshy / 外部资产从这角度拓扑瑕疵不易看出；UE5 设置简单 | 不能"欣赏景观"（CBD 天际线无法表现） |
| 纯顶视 90° | 画面最干净；最易开发 | 看不到建筑立面，新加坡识别度归零 |
| 第三人称跟随 | 最接近 GTA 手感 | 需要主角动画集 + 街道细节 + NPC 行为 → 对一人开发是地狱 |
| 第一人称车内 + 第三人称走路 | 适合 Grab 司机题材 | 但项目题材已转向程序员人生 sim，不再需要 |

## Consequences

### 好处

- **大幅降低美术工作量**（~70%）：不用做精细面部动画、不用做车外世界 LOD、不用做电影机位过场
- **meshy / 外部资产的拓扑瑕疵在等距视角下不易察觉**
- **城市规模可以做大 5~10 倍**（不需要每栋楼都细节高）
- **风格化路线友好**：Cult of the Lamb / Stardew Valley 都是这个视角的范本
- **跟治愈调性匹配**：俯视角自带"上帝视角观察人生"的疏离感，强化主题

### 代价

- **无法欣赏景观**：滨海湾天际线、樟宜机场夜景在等距视角下基本消失。需通过 UI / 音乐 / 偶尔过场镜头补偿
- **驾驶体验受限**：如果 Phase 2 真要加开车，等距视角下开车手感弱（无法看远）。届时可能需要为驾驶单独做一个"接近顶视"的子镜头模式
- **不适合战斗 / 平台跳跃**类玩法（但都不在 spec 范围）

## Implementation Notes

- Camera Component: **Orthographic**, Ortho Width ≈ 1500 单位（可调）
- Spring Arm: Pitch=-45°, Yaw=-45°, Length=1200, Lag Speed=10（平滑跟随）
- 关键参数（Pitch / Yaw / Length / Ortho Width）作为 Camera Rig 蓝图的 default values，Plan 1 Task 5 实现

## When to Revisit

- Phase 2 加街道开车体验时，决定是否需要"驾驶子镜头"
- 如果用户测试反馈"看不出场景"，调 Ortho Width / Pitch 角度

## References

- 项目 spec §9.1：视角
- Plan 1 Task 5：[等距俯视 45° 摄像机](../plans/2026-05-23-engine-validation-prototype.md)
