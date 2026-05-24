#include "Systems/ActivitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/SanitySubsystem.h"
#include "Kismet/GameplayStatics.h"

int32 UActivitySubsystem::GetCurrentEnergy() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
		{
			return PS->GetAttribute(EPlayerAttribute::Energy);
		}
	}
	return 0;
}

bool UActivitySubsystem::CanPerform(EActivityType Activity) const
{
	return FActivitySystem::CanPerform(FActivitySystem::GetActivityDef(Activity), GetCurrentEnergy());
}

bool UActivitySubsystem::PerformActivity(EActivityType Activity)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) { return false; }

	const FActivityDef Def = FActivitySystem::GetActivityDef(Activity);
	if (!FActivitySystem::CanPerform(Def, GetCurrentEnergy()))
	{
		return false; // 精力不够
	}

	// 改属性（含能量）。
	if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
	{
		for (int32 i = 0; i < (int32)EPlayerAttribute::Count; ++i)
		{
			const int32 Delta = Def.AttrDelta[i];
			if (Delta != 0)
			{
				PS->ModifyAttribute((EPlayerAttribute)i, Delta);
			}
		}
	}

	// 改理智（拜拜/睡觉回理智 —— 对抗恐惧螺旋）。
	if (Def.SanityDelta != 0)
	{
		if (USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>())
		{
			San->Restore(Def.SanityDelta);
		}
	}

	// 改现金。
	if (Def.CashDeltaCents != 0)
	{
		if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
		{
			if (Def.CashDeltaCents > 0)
			{
				Eco->Deposit(ECurrencyAccount::Cash, Def.CashDeltaCents, TEXT("Activity"));
			}
			else
			{
				Eco->GetEconomy().Charge(ECurrencyAccount::Cash, -Def.CashDeltaCents, TEXT("Activity"));
			}
		}
	}

	// 推进时间块（顺带触发月度发薪/账单/投资/事件）。
	if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
	{
		for (int32 b = 0; b < FMath::Max(1, Def.TimeBlocks); ++b)
		{
			TimeSys->AdvanceBlock();
		}
	}

	OnActivityPerformed.Broadcast(Def.Title);
	return true;
}

TArray<EActivityType> UActivitySubsystem::GetActivitiesForCurrentLevel() const
{
	const FString Level = UGameplayStatics::GetCurrentLevelName(GetGameInstance(), /*bRemovePrefix=*/true);

	// 食阁：吃饭 / 听八卦；其余（出租屋为主）：睡觉 / 学习 / 接私活 / 健身。
	if (Level.Contains(TEXT("Hawker")))
	{
		return { EActivityType::EatHawker, EActivityType::Gossip };
	}
	// 出租屋（含家里神台）：睡觉 / 学习 / 接私活 / 健身 / 拜拜祈福。
	return { EActivityType::Sleep, EActivityType::Study, EActivityType::FreelanceCode,
		EActivityType::Exercise, EActivityType::PrayPuja };
}
