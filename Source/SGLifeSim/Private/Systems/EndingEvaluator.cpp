#include "Systems/EndingEvaluator.h"

EEnding FEndingEvaluator::EvaluateLeaning(
	EResidencyStatus Status,
	bool bOwnsHome,
	int32 MaxAffinity,
	int64 NetWorthCents,
	int32 PRRejections,
	int32 Sanity)
{
	// 0. 理智垮了 —— 盖过一切。再有钱有房，人垮了就是被压垮。
	if (Sanity < BreakdownSanityThreshold)
	{
		return EEnding::Breakdown;
	}

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

FText FEndingEvaluator::GetEndingTitle(EEnding Ending)
{
	return UEnum::GetDisplayValueAsText(Ending);
}

FText FEndingEvaluator::GetEndingFlavor(EEnding Ending)
{
	switch (Ending)
	{
	case EEnding::Rooted:
		return FText::FromString(TEXT("红登记到手，钥匙在手，楼下有人喊你的名字。\n这座岛，认了你。"));
	case EEnding::CashOut:
		return FText::FromString(TEXT("户头够了。你拖着行李箱站在樟宜，\n回头看了一眼这座亮着灯的岛，然后转身。"));
	case EEnding::Heartbreak:
		return FText::FromString(TEXT("续签没过，户头也空了。\n地铁载你最后一程——终点站，机场。"));
	case EEnding::Adrift:
		return FText::FromString(TEXT("你还在这儿。租约一年一签，谁也不真正属于谁。\n日子，照过。"));
	case EEnding::Breakdown:
		return FText::FromString(TEXT("那些深夜的声音，最后住进了你脑子里。\n你撑不住了——这座岛，把你压垮了。"));
	case EEnding::None:
	default:
		return FText::GetEmpty();
	}
}
