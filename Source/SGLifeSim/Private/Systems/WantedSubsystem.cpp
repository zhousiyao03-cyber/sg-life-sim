#include "Systems/WantedSubsystem.h"

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

void UWantedSubsystem::BroadcastIfChanged(int32 OldStars)
{
	const int32 NewStars = GetStars();
	if (NewStars != OldStars)
	{
		OnWantedChanged.Broadcast(NewStars);
	}
}
