#include "Systems/CareerSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"

void UCareerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	Collection.InitializeDependency(UEconomySubsystem::StaticClass());
	Collection.InitializeDependency(UPlayerStateSubsystem::StaticClass());

	if (UTimeSubsystem* TimeSys = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		TimeSys->OnTimeAdvanced.AddDynamic(this, &UCareerSubsystem::HandleTimeAdvanced);
		LastTickedMonth = TimeSys->GetMonthNumber();
	}

	// 把起始月薪推给 Economy，覆盖其默认常量 → 月度发薪用职业薪资。
	SyncSalaryToEconomy();
}

void UCareerSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UCareerSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

int32 UCareerSubsystem::GetProfessional() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
		{
			return PS->GetAttribute(EPlayerAttribute::Professional);
		}
	}
	return 0;
}

void UCareerSubsystem::SyncSalaryToEconomy()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
		{
			Eco->SetMonthlyGrossSalary(Career.GetGrossSalaryCents());
		}
	}
}

void UCareerSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	UTimeSubsystem* TimeSys = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTimeSubsystem>() : nullptr;
	if (!TimeSys) { return; }

	const int32 CurrentMonth = TimeSys->GetMonthNumber();
	bool bAdvanced = false;
	while (CurrentMonth > LastTickedMonth)
	{
		++LastTickedMonth;
		Career.AdvanceMonth();
		bAdvanced = true;
	}
	if (bAdvanced)
	{
		OnCareerChanged.Broadcast(); // 在职月数变了，HUD/升职资格可能变
	}
}

bool UCareerSubsystem::CanPromote() const
{
	return Career.CanPromote(GetProfessional());
}

bool UCareerSubsystem::TryPromote()
{
	if (!Career.Promote(GetProfessional()))
	{
		return false;
	}
	SyncSalaryToEconomy();

	// 第一次升职解锁成就。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
		{
			Prog->MarkAchieved(SGAchievementIds::FirstPromotion());
		}
	}
	OnCareerChanged.Broadcast();
	return true;
}

bool UCareerSubsystem::JobHop(int32 RaisePercent)
{
	if (!Career.JobHop(RaisePercent))
	{
		return false;
	}
	SyncSalaryToEconomy();
	OnCareerChanged.Broadcast();
	return true;
}
