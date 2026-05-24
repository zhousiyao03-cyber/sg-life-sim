#include "Systems/EndingEvaluator.h"

EEnding FEndingEvaluator::EvaluateLeaning(
	EResidencyStatus Status,
	bool bOwnsHome,
	int32 MaxAffinity,
	int64 NetWorthCents,
	int32 PRRejections)
{
	const bool bIsPRorCitizen =
		(Status == EResidencyStatus::PR) || (Status == EResidencyStatus::Citizen);

	// 1. 破产，或申请 PR 被拒过且至今仍没拿到 → 心碎离开。
	if (NetWorthCents < 0 || (PRRejections > 0 && !bIsPRorCitizen))
	{
		return EEnding::Heartbreak;
	}

	// 2. 拿到 PR/公民 + 有房 + 有稳定关系 → 扎根。
	if (bIsPRorCitizen && bOwnsHome && MaxAffinity >= StableRelationshipAffinity)
	{
		return EEnding::Rooted;
	}

	// 3. 攒够钱（未扎根）→ 兑现离开。
	if (NetWorthCents >= CashOutNetWorthCents)
	{
		return EEnding::CashOut;
	}

	// 4. 没 PR、租房、关系薄 —— 留下但漂着。
	return EEnding::Adrift;
}
