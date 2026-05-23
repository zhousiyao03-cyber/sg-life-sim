# ADR 0001: Use UE5.6.1 LTS over UE5.7.4

- **Status**: Accepted
- **Date**: 2026-05-23
- **Decider**: Project owner (with Claude)

## Context

UE5.7.4 是 2026 年初的当前稳定主线版本；UE5.6.1 是长期支持（LTS）版本。装机时 Epic Launcher 同时提供两个版本可选。

项目背景：一人独立开发，预计 MVP 周期 6~12 个月，Phase 2 还要 6~12 个月。引擎选定后短期内不会换版本，但每年要决定要不要升级。

## Decision

用 **UE5.6.1 LTS**。

## Alternatives Considered

| 选项 | 优点 | 缺点 |
|------|------|------|
| **UE5.6.1 LTS** ✅ | Epic 维护到 2027+；社区教程铺得多；bug 修复优先 | 无新特性（如 5.7 的新 Nanite 增量更新） |
| UE5.7.4 | 最新功能；性能略好 | LTS 周期短；新版 bug；教程稀少；半年到一年又被新版本取代 |
| UE5.4 | 更老更稳；教程最多 | 已经不在主流；缺少 5.5+ 的关键改进（Niagara / 编辑器性能） |

## Consequences

### 好处

- **教程 / 文档全**。网上 UE5.4~5.6 的教程铺天盖地；遇到 bug 容易找到 stackoverflow / forum 答案。
- **bug 少**。LTS 是 Epic 选出来稳定支持的版本，bug 修复持续滚动到 2027+。
- **不会被半年后的新版本"抛弃"**。如果用 5.7，明年 Epic 出 5.8 后 5.7 就停止接收 bug 修复了。
- **一人开发友好**。少踩新版坑 = 少浪费时间在引擎问题上。

### 代价

- **错过 5.7 引入的新功能**（如果有）。但 MVP 阶段用到的核心系统（C++、Blueprint、UMG、Lumen、Niagara、SkeletalMesh、AnimBlueprint）都在 5.6 完全成熟。
- 升级到 5.7+ 的话需要做一次 migration（小工作量，UE 项目跨小版本兼容良好）。

## When to Revisit

- 如果 Phase 2 启动时 5.8 LTS 已发布，重新评估是否升级（升级一次跨两个 LTS 版本）。
- 如果发现 5.6 的某个关键 bug 是 5.7 才修的（极少见），考虑早期跳版。

## References

- 项目 spec §10.1：技术架构
- Plan 1：[Engine Validation Prototype](../plans/2026-05-23-engine-validation-prototype.md)
- 历史：曾在 commit `a6e81b5` 切到 5.7.4，又在 `fb184b6` 改回 5.6.1（下载 reset 后趁机切回 LTS）
