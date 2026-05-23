# ADR 0005: Use GameInstanceSubsystem for Cross-Scene State

- **Status**: Accepted
- **Date**: 2026-05-23
- **Decider**: Project owner (with Claude)

## Context

游戏架构需要在多个场景（出租屋 / 办公室 / 食阁 / ...）之间保持**持久状态**：

- 当前时间（年 / 月 / 日 / 时间块）
- 经济数据（现金 / 投资 / 房产）
- 关系网（NPC 好感度）
- 进度成就

UE5 提供几种存储这些状态的机制：

1. **GameMode**：每个 Level 一个，**场景切换时销毁**
2. **GameState**：每个 Level 一个，同样销毁
3. **PlayerController**：每个 Level 一个，可以通过 `bAllowTickBeforeBeginPlay` 等手段保留，但不是设计意图
4. **GameInstance**：整个游戏生命周期一个实例，**场景切换不销毁**
5. **GameInstanceSubsystem**：挂在 GameInstance 上的子系统，跟 GameInstance 同生命周期
6. **SaveGame**：序列化到磁盘，主要用于持久化存档而非运行时状态

## Decision

**所有跨场景的核心系统用 GameInstanceSubsystem 实现**。

具体（Plan 1 起步，逐步扩展）：

- `UTimeSubsystem`（Plan 1 实现）
- `UEconomySubsystem`（Plan 2 实现）
- `URelationshipSubsystem`（Plan 3 实现）
- `UProgressSubsystem`（Plan 3 实现）
- `USaveSubsystem`（Plan 2 实现，桥接到磁盘）

底层数据模型用**纯 C++ 类**（如 `FTimeSystem`），Subsystem 是 UCLASS 包装层负责 Blueprint 集成 + 事件广播。

## Alternatives Considered

| 选项 | 优点 | 缺点 |
|------|------|------|
| **GameInstanceSubsystem** ✅ | UE5 推荐模式；跨场景持久；自动生命周期；Blueprint 友好；可单元测试（核心逻辑放纯 C++） | 略多样板代码 |
| 全局单例（Singleton C++ 类） | 写起来直接 | 不被 UE5 GC 管理；Blueprint 难访问；测试难 |
| 把状态塞 GameMode | UE5 自带 | 场景切换销毁，状态全丢；不可行 |
| 全用 SaveGame 实时序列化 | 简单 | 每次访问都磁盘 IO 太慢；不适合实时数据 |
| Actor with bNetLoadOnClient | hack | 不是设计意图，行为奇怪 |

## Consequences

### 好处

- **场景切换不丢状态**。这是核心要求。
- **Blueprint 一行代码访问**：`Get Game Instance Subsystem (TimeSubsystem)` → 调用任意函数。
- **测试友好**：核心逻辑在纯 C++ 类（如 `FTimeSystem`），可单元测试，Subsystem 只是薄包装。
- **职责清晰**：每个 Subsystem 一个独立责任，可以独立开发 / 维护 / debug。
- **存档桥接简单**：`USaveSubsystem` 知道哪些 Subsystem 要序列化，全部走它一个接口。

### 代价

- **样板代码**：每个核心系统要写 `.h/.cpp` 两套（纯 C++ 类 + Subsystem 包装）。可接受。
- **GameInstance 生命周期里不能销毁**：到游戏退出才释放。但对一个 sim 类型游戏来说这正合适。

## Implementation Pattern

以 TimeSystem 为例（Plan 1 Task 2~3）：

```cpp
// 纯数据 + 推进逻辑 - 单元测试友好
class SGLIFESIM_API FTimeSystem {
    void AdvanceBlock();
    ETimeBlock GetCurrentBlock() const;
    int32 GetDayNumber() const;
    EWeekday GetWeekday() const;
    int32 GetTotalBlocks() const;
private:
    int32 TotalBlocksSinceStart = 0;
};

// UE 集成层 - Blueprint 调用 + 事件
UCLASS()
class SGLIFESIM_API UTimeSubsystem : public UGameInstanceSubsystem {
    UFUNCTION(BlueprintCallable) void AdvanceBlock();
    UPROPERTY(BlueprintAssignable) FOnTimeAdvanced OnTimeAdvanced;
private:
    FTimeSystem Time;
};
```

所有后续 Subsystem 走同样的"FXxxSystem 核心 + UXxxSubsystem 包装"模式。

## When to Revisit

- 如果系统数量增长到 10+ 个，可能需要引入一个 `USystemRegistry` 来管理依赖关系
- 如果需要联网功能（远期），Subsystem 模式仍然适用，但需要决定哪些状态走 GameInstance 哪些走 GameState 网络复制
- 如果做存档，由 `USaveSubsystem` 反射枚举所有要序列化的 Subsystem

## References

- 项目 spec §10.3：核心系统模块
- Plan 1 Task 2~3：[TimeSystem 实现](../plans/2026-05-23-engine-validation-prototype.md)
- UE5 文档：[Programming Subsystems](https://docs.unrealengine.com/5.6/en-US/programming-subsystems-in-unreal-engine/)
