#include "Systems/MilestoneSystem.h"

namespace
{
	// $5,000 与 $100,000（分）。
	constexpr int64 Save5kTargetCents = 500000;
	constexpr int64 NetWorth100kTargetCents = 10000000;
}

FText FMilestoneSystem::GetTitle(EMilestone Milestone)
{
	// 直接用枚举的 DisplayName（已含主题前缀，集中维护）。
	return UEnum::GetDisplayValueAsText(Milestone);
}

FMilestoneProgress FMilestoneSystem::Evaluate(EMilestone Milestone, const FMilestoneContext& Ctx)
{
	FMilestoneProgress P;
	P.Milestone = Milestone;

	switch (Milestone)
	{
	case EMilestone::FirstSalary:
		P.bComplete = Ctx.bHasFirstSalary;
		break;

	case EMilestone::Save5k:
		P.bIsNumeric = true;
		P.CurrentCents = Ctx.CashCents;
		P.TargetCents = Save5kTargetCents;
		P.bComplete = Ctx.CashCents >= Save5kTargetCents;
		break;

	case EMilestone::PromoteToMid:
		P.bComplete = (uint8)Ctx.Career >= (uint8)ECareerLevel::Mid;
		break;

	case EMilestone::BuyFirstHome:
		P.bComplete = Ctx.bOwnsHome;
		break;

	case EMilestone::BecomePR:
		P.bComplete = (uint8)Ctx.Residency >= (uint8)EResidencyStatus::PR;
		break;

	case EMilestone::NetWorth100k:
		P.bIsNumeric = true;
		P.CurrentCents = Ctx.NetWorthCents;
		P.TargetCents = NetWorth100kTargetCents;
		P.bComplete = Ctx.NetWorthCents >= NetWorth100kTargetCents;
		break;

	case EMilestone::BecomeCitizen:
		P.bComplete = Ctx.Residency == EResidencyStatus::Citizen;
		break;

	default:
		break;
	}

	return P;
}

EMilestone FMilestoneSystem::GetActive(const FMilestoneContext& Ctx)
{
	for (int32 i = 0; i < Num(); ++i)
	{
		const EMilestone M = (EMilestone)i;
		if (!IsComplete(M, Ctx))
		{
			return M;
		}
	}
	return EMilestone::Count; // 全部达成
}

int32 FMilestoneSystem::CountCompleted(const FMilestoneContext& Ctx)
{
	int32 N = 0;
	for (int32 i = 0; i < Num(); ++i)
	{
		if (IsComplete((EMilestone)i, Ctx))
		{
			++N;
		}
	}
	return N;
}
