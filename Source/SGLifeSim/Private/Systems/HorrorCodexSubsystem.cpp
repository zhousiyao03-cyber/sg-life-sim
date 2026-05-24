#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventSystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"

void UHorrorCodexSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UHorrorEventSubsystem::StaticClass());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>())
		{
			Horror->OnHorrorEventTyped.AddDynamic(this, &UHorrorCodexSubsystem::HandleHorrorEventTyped);
		}
	}
}

void UHorrorCodexSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>())
		{
			Horror->OnHorrorEventTyped.RemoveDynamic(this, &UHorrorCodexSubsystem::HandleHorrorEventTyped);
		}
	}
	Super::Deinitialize();
}

void UHorrorCodexSubsystem::HandleHorrorEventTyped(EHorrorEvent Event)
{
	RecordEncounter(Event);
}

bool UHorrorCodexSubsystem::RecordEncounter(EHorrorEvent Event)
{
	const bool bFirstEver = (Codex.CountDiscovered() == 0);
	const bool bNew = Codex.MarkEncountered(Event);
	if (!bNew)
	{
		return false;
	}

	// 解锁成就：第一条传说 / 集齐。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
		{
			if (bFirstEver)
			{
				Prog->MarkAchieved(SGAchievementIds::FirstUrbanLegend());
			}
			if (Codex.IsComplete())
			{
				Prog->MarkAchieved(SGAchievementIds::CompleteHorrorCodex());
			}
		}
	}

	OnEntryUnlocked.Broadcast(Event, FHorrorEventSystem::GetEventDef(Event).Title);
	return true;
}

FText UHorrorCodexSubsystem::GetProgressText() const
{
	return FText::FromString(FString::Printf(TEXT("都市传说 %d / %d"),
		Codex.CountDiscovered(), FHorrorCodexSystem::TotalCollectable()));
}

TArray<FHorrorCodexEntry> UHorrorCodexSubsystem::GetEntries() const
{
	TArray<FHorrorCodexEntry> Entries;
	for (int32 i = 1; i < (int32)EHorrorEvent::Count; ++i)
	{
		const EHorrorEvent E = (EHorrorEvent)i;
		FHorrorCodexEntry Entry;
		Entry.Event = E;
		Entry.bDiscovered = Codex.HasEncountered(E);
		if (Entry.bDiscovered)
		{
			Entry.Title = FHorrorEventSystem::GetEventDef(E).Title;
		}
		Entries.Add(Entry);
	}
	return Entries;
}
