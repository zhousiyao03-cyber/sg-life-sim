#pragma once

#include "CoreMinimal.h"
#include "Systems/MilestoneTypes.h"

/**
 * 里程碑纯逻辑核心。Plan 13。零 UE 子系统依赖 —— 给定状态快照即可评估，可单测。
 */
class SGLIFESIM_API FMilestoneSystem
{
public:
	/** 里程碑总数（不含 Count 哨兵）。 */
	static constexpr int32 Num() { return (int32)EMilestone::Count; }

	/** 显示标题（含主题前缀）。 */
	static FText GetTitle(EMilestone Milestone);

	/** 给定快照评估单个里程碑（达成 + 数值进度）。 */
	static FMilestoneProgress Evaluate(EMilestone Milestone, const FMilestoneContext& Ctx);

	/** 是否达成。 */
	static bool IsComplete(EMilestone Milestone, const FMilestoneContext& Ctx)
	{
		return Evaluate(Milestone, Ctx).bComplete;
	}

	/** 当前应奔的目标 = 第一个未完成的里程碑；全部完成则返回 EMilestone::Count。 */
	static EMilestone GetActive(const FMilestoneContext& Ctx);

	/** 已完成数量。 */
	static int32 CountCompleted(const FMilestoneContext& Ctx);
};
