#include "Systems/MilestoneSubsystem.h"
#include "Systems/MilestoneSystem.h"

#include "Systems/TimeSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/CareerSubsystem.h"
#include "Systems/ResidencySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"

namespace
{
	FString MSFormatMoney(int64 Cents)
	{
		const TCHAR* Sign = (Cents < 0) ? TEXT("-") : TEXT("");
		const int64 Abs = FMath::Abs(Cents);
		return FString::Printf(TEXT("%s$%lld"), Sign, Abs / 100);
	}
}

void UMilestoneSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	Collection.InitializeDependency(UEconomySubsystem::StaticClass());
	Collection.InitializeDependency(UCareerSubsystem::StaticClass());
	Collection.InitializeDependency(UResidencySubsystem::StaticClass());
	Collection.InitializeDependency(UAssetsSubsystem::StaticClass());
	Collection.InitializeDependency(UProgressSubsystem::StaticClass());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.AddDynamic(this, &UMilestoneSubsystem::HandleTimeAdvanced);
		}
	}

	// 建立基线：开局已满足的里程碑不弹 toast（正常开局全未满足）。
	Refresh();
}

void UMilestoneSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UMilestoneSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

void UMilestoneSubsystem::HandleTimeAdvanced(ETimeBlock /*NewBlock*/, int32 /*DayNumber*/)
{
	Refresh();
}

FMilestoneContext UMilestoneSubsystem::BuildContext() const
{
	FMilestoneContext Ctx;
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return Ctx;
	}

	if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
	{
		Ctx.bHasFirstSalary = Prog->HasAchieved(SGAchievementIds::FirstSalary());
	}
	if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
	{
		Ctx.CashCents = Eco->GetBalance(ECurrencyAccount::Cash);
		Ctx.NetWorthCents = Eco->GetNetWorth();
	}
	if (UCareerSubsystem* Career = GI->GetSubsystem<UCareerSubsystem>())
	{
		Ctx.Career = Career->GetLevel();
	}
	if (UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
	{
		Ctx.bOwnsHome = Assets->OwnsHome();
	}
	if (UResidencySubsystem* Res = GI->GetSubsystem<UResidencySubsystem>())
	{
		Ctx.Residency = Res->GetStatus();
	}
	return Ctx;
}

void UMilestoneSubsystem::Refresh()
{
	const FMilestoneContext Ctx = BuildContext();

	for (int32 i = 0; i < FMilestoneSystem::Num(); ++i)
	{
		const EMilestone M = (EMilestone)i;
		if (FMilestoneSystem::IsComplete(M, Ctx) && !bToasted[i])
		{
			bToasted[i] = true;
			if (bPrimed)
			{
				OnMilestoneCompleted.Broadcast(M);
			}
		}
	}

	bPrimed = true;
}

EMilestone UMilestoneSubsystem::GetActiveMilestone()
{
	return FMilestoneSystem::GetActive(BuildContext());
}

int32 UMilestoneSubsystem::GetCompletedCount()
{
	return FMilestoneSystem::CountCompleted(BuildContext());
}

FText UMilestoneSubsystem::GetActiveObjectiveText()
{
	Refresh(); // 顺带检测达成 → 及时弹 toast

	const FMilestoneContext Ctx = BuildContext();
	const EMilestone Active = FMilestoneSystem::GetActive(Ctx);
	const int32 Done = FMilestoneSystem::CountCompleted(Ctx);
	const int32 Total = FMilestoneSystem::Num();

	if (Active == EMilestone::Count)
	{
		return FText::FromString(TEXT("🎯 主线全部达成！这座岛，已经是你的家。"));
	}

	const FMilestoneProgress P = FMilestoneSystem::Evaluate(Active, Ctx);
	FString Line = FString::Printf(TEXT("🎯 当前目标：%s"),
		*FMilestoneSystem::GetTitle(Active).ToString());

	if (P.bIsNumeric)
	{
		Line += FString::Printf(TEXT("（%s / %s）"),
			*MSFormatMoney(P.CurrentCents), *MSFormatMoney(P.TargetCents));
	}

	Line += FString::Printf(TEXT("   ·   已完成 %d/%d"), Done, Total);
	return FText::FromString(Line);
}
