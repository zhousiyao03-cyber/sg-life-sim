#include "Systems/SanitySubsystem.h"
#include "Systems/SanitySystem.h"
#include "Systems/HorrorEventSystem.h" // IsGhostMonth
#include "Systems/TimeSubsystem.h"

void USanitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.AddDynamic(this, &USanitySubsystem::HandleTimeAdvanced);
			LastRecoveredDay = TimeSys->GetDayNumber();
		}
	}
}

void USanitySubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &USanitySubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

ESanityState USanitySubsystem::GetState() const
{
	return FSanitySystem::GetState(Sanity);
}

int32 USanitySubsystem::GetExtraDreadWeight() const
{
	return FSanitySystem::ExtraDreadWeight(Sanity);
}

void USanitySubsystem::SetSanity(int32 NewValue)
{
	const int32 Clamped = FSanitySystem::Clamp(NewValue);
	const ESanityState OldState = FSanitySystem::GetState(Sanity);
	Sanity = Clamped;
	const ESanityState NewState = FSanitySystem::GetState(Sanity);
	if (NewState != OldState)
	{
		OnSanityChanged.Broadcast(Sanity, NewState);
	}
}

void USanitySubsystem::Drain(int32 Amount)
{
	SetSanity(Sanity - FMath::Abs(Amount));
}

void USanitySubsystem::Restore(int32 Amount)
{
	SetSanity(Sanity + FMath::Abs(Amount));
}

void USanitySubsystem::RestoreFromSave(int32 InSanity)
{
	Sanity = FSanitySystem::Clamp(InSanity);
}

void USanitySubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	// 每天恢复一次（跨天触发）。农历七月没有喘息，不恢复。
	if (DayNumber == LastRecoveredDay)
	{
		return;
	}
	LastRecoveredDay = DayNumber;

	UTimeSubsystem* TimeSys = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTimeSubsystem>() : nullptr;
	const bool bGhost = TimeSys && FHorrorEventSystem::IsGhostMonth(TimeSys->GetMonthNumber());
	if (!bGhost)
	{
		Restore(DailyRecovery);
	}
}
