#pragma once

#include "CoreMinimal.h"
#include "Systems/EndingTypes.h"
#include "Systems/ResidencyTypes.h"

/**
 * 终局评估器。spec §6.5。
 *
 * 纯静态函数：从「身份 + 是否有房 + 最高好感 + 净资产 + PR 被拒次数」算当前最可能
 * 的软终局倾向。系统间不耦合——UEndingSubsystem 读各子系统状态后调它。
 */
class SGLIFESIM_API FEndingEvaluator
{
public:
	/** 「朋友」档好感阈值（与 ERelationshipTier::Friend 对齐 = 50）。 */
	static constexpr int32 StableRelationshipAffinity = 50;

	/** 「兑现离开」可触发的净资产阈值（分）：$300,000。 */
	static constexpr int64 CashOutNetWorthCents = 30000000;

	/** 理智低于此值视为「被压垮」（与 ESanityState::Breaking 对齐 = 15）。 */
	static constexpr int32 BreakdownSanityThreshold = 15;

	/**
	 * 评估当前终局倾向（不代表已结束，玩家可主动选）。
	 * 判定顺序（先到先得）：
	 *   0. 理智 < 15（濒临崩溃）→ 被压垮（最高优先级，盖过一切——心先垮了）
	 *   1. 净资产 < 0（破产）或 被拒过 PR 且仍非 PR → 心碎离开
	 *   2. (PR|公民) + 有房 + 有「朋友」以上关系 → 扎根
	 *   3. 净资产 ≥ $300k 且未扎根 → 兑现离开
	 *   4. 其余 → 留下漂着
	 */
	static EEnding EvaluateLeaning(
		EResidencyStatus Status,
		bool bOwnsHome,
		int32 MaxAffinity,
		int64 NetWorthCents,
		int32 PRRejections,
		int32 Sanity = 100);

	/** 结局标题（取枚举 DisplayName）。 */
	static FText GetEndingTitle(EEnding Ending);

	/** 结局收尾文案（演出 overlay 用）。None 返回空。 */
	static FText GetEndingFlavor(EEnding Ending);
};
