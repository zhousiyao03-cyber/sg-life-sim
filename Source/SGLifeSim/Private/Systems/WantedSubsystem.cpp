#include "Systems/WantedSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"

#include "Engine/GameInstance.h"

namespace
{
	constexpr int32 HeatPerStar = 100;
	constexpr int32 MaxHeat = 500;
}

void UWantedSubsystem::AddHeat(int32 Amount)
{
	if (Amount <= 0) { return; }
	const int32 Old = GetStars();
	Heat = FMath::Clamp(Heat + Amount, 0, MaxHeat);
	BroadcastIfChanged(Old);
}

bool UWantedSubsystem::ReportCrime(int32 Amount, float NowSeconds)
{
	if (Amount <= 0) { return false; }
	// 全局去重：同一桩事多名目击者只记一笔。
	if (NowSeconds - LastReportSeconds < ReportCooldownSeconds)
	{
		return false;
	}
	LastReportSeconds = NowSeconds;
	AddHeat(Amount);
	return true;
}

void UWantedSubsystem::Arrest()
{
	if (Heat <= 0) { return; }

	// 交保释金（现金→银行兜底，能扣多少扣多少，不阻断被捕）。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEconomySubsystem* Econ = GI->GetSubsystem<UEconomySubsystem>())
		{
			if (!Econ->TryWithdraw(ECurrencyAccount::Cash, ArrestBailCents, TEXT("ArrestBail")))
			{
				Econ->TryWithdraw(ECurrencyAccount::Bank, ArrestBailCents, TEXT("ArrestBail"));
			}
		}
	}
	ClearWanted();
}

int32 UWantedSubsystem::GetStars() const
{
	return FMath::Clamp(Heat / HeatPerStar, 0, 5);
}

void UWantedSubsystem::ClearWanted()
{
	const int32 Old = GetStars();
	Heat = 0;
	BroadcastIfChanged(Old);
}

void UWantedSubsystem::Decay(int32 Amount)
{
	if (Amount <= 0 || Heat <= 0) { return; }
	const int32 Old = GetStars();
	Heat = FMath::Max(0, Heat - Amount);
	BroadcastIfChanged(Old);
}

void UWantedSubsystem::RestoreFromSave(int32 InHeat)
{
	const int32 Old = GetStars();
	Heat = FMath::Clamp(InHeat, 0, MaxHeat);
	BroadcastIfChanged(Old);
}

void UWantedSubsystem::BroadcastIfChanged(int32 OldStars)
{
	const int32 NewStars = GetStars();
	if (NewStars != OldStars)
	{
		OnWantedChanged.Broadcast(NewStars);
	}
}
