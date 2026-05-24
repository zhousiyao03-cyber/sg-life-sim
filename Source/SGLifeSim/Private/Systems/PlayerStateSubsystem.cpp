#include "Systems/PlayerStateSubsystem.h"
#include "Systems/TimeSubsystem.h"

void UPlayerStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	if (UTimeSubsystem* TimeSys = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		TimeSys->OnTimeAdvanced.AddDynamic(this, &UPlayerStateSubsystem::HandleTimeAdvanced);
		LastDayNumber = TimeSys->GetDayNumber();
	}
}

void UPlayerStateSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UPlayerStateSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

void UPlayerStateSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	// 跨天 → 能量恢复。
	if (DayNumber > LastDayNumber)
	{
		LastDayNumber = DayNumber;
		Stats.RestoreEnergyDaily();
		NotifyAttribute(EPlayerAttribute::Energy);
	}
}

int32 UPlayerStateSubsystem::GetAttribute(EPlayerAttribute Attr) const
{
	return Stats.Get(Attr);
}

void UPlayerStateSubsystem::NotifyAttribute(EPlayerAttribute Attr)
{
	OnAttributeChanged.Broadcast(Attr, Stats.Get(Attr));
}

void UPlayerStateSubsystem::SetAttribute(EPlayerAttribute Attr, int32 Value)
{
	Stats.Set(Attr, Value);
	NotifyAttribute(Attr);
}

void UPlayerStateSubsystem::ModifyAttribute(EPlayerAttribute Attr, int32 Delta)
{
	Stats.Modify(Attr, Delta);
	NotifyAttribute(Attr);
}
