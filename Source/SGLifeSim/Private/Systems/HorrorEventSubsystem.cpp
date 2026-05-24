#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventSystem.h"

#include "Systems/TimeSubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"

void UHorrorEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	Collection.InitializeDependency(UPlayerStateSubsystem::StaticClass());

	// 默认随时间变化的种子，让每局不同；测试可 SetSeed 覆盖。
	Stream.Initialize((int32)(FDateTime::Now().GetTicks() & 0x7fffffff));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.AddDynamic(this, &UHorrorEventSubsystem::HandleTimeAdvanced);
			LastRolledDay = TimeSys->GetDayNumber();
		}
	}
}

void UHorrorEventSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UHorrorEventSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

bool UHorrorEventSubsystem::IsGhostMonth() const
{
	UGameInstance* GI = GetGameInstance();
	UTimeSubsystem* TimeSys = GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
	return TimeSys ? FHorrorEventSystem::IsGhostMonth(TimeSys->GetMonthNumber()) : false;
}

void UHorrorEventSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	if (!bEnabled || NewBlock != ETimeBlock::LateNight)
	{
		return;
	}
	// 每天入夜只掷一次。
	if (DayNumber == LastRolledDay)
	{
		return;
	}
	LastRolledDay = DayNumber;

	const bool bGhost = IsGhostMonth();
	ApplyEvent(FHorrorEventSystem::PickEvent(Stream, bGhost));
}

bool UHorrorEventSubsystem::ApplyEvent(EHorrorEvent Event)
{
	LastEvent = Event;
	if (Event == EHorrorEvent::None)
	{
		return false;
	}

	const FHorrorEventDef Def = FHorrorEventSystem::GetEventDef(Event);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
		{
			if (Def.MoodDelta != 0)
			{
				PS->ModifyAttribute(EPlayerAttribute::Mood, Def.MoodDelta);
			}
			if (Def.HealthDelta != 0)
			{
				PS->ModifyAttribute(EPlayerAttribute::Health, Def.HealthDelta);
			}
		}
	}

	OnHorrorEvent.Broadcast(Def.Title);
	return true;
}
