#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventTypes.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"
#include "Systems/SaveGameSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 恐怖图鉴端到端（Plan 21）：施加恐怖事件 → 图鉴记录 + 首条/集齐成就 + 存档 round-trip。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCodexIntegrationTest,
	"SGLifeSim.Integration.HorrorCodexRecordsAndPersists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorCodexIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>();
	UHorrorCodexSubsystem* Codex  = GI->GetSubsystem<UHorrorCodexSubsystem>();
	UProgressSubsystem*    Prog   = GI->GetSubsystem<UProgressSubsystem>();
	USaveGameSubsystem*    Save   = GI->GetSubsystem<USaveGameSubsystem>();
	if (!Horror || !Codex || !Prog || !Save) { GI->Shutdown(); return false; }

	// 开局空，无首条成就。
	TestEqual(TEXT("codex starts empty"), Codex->GetDiscoveredCount(), 0);
	TestFalse(TEXT("no first-legend achievement yet"), Prog->HasAchieved(SGAchievementIds::FirstUrbanLegend()));

	// 施加一个事件 → 图鉴记录 + 解锁首条成就。
	Horror->ApplyEvent(EHorrorEvent::CorridorLights);
	TestEqual(TEXT("codex now has 1"), Codex->GetDiscoveredCount(), 1);
	TestTrue(TEXT("discovered the event"), Codex->HasDiscovered(EHorrorEvent::CorridorLights));
	TestTrue(TEXT("first-legend achievement unlocked"), Prog->HasAchieved(SGAchievementIds::FirstUrbanLegend()));

	// 重复同一事件不增加计数。
	Horror->ApplyEvent(EHorrorEvent::CorridorLights);
	TestEqual(TEXT("duplicate does not increment"), Codex->GetDiscoveredCount(), 1);

	// None 不计入图鉴。
	Horror->ApplyEvent(EHorrorEvent::None);
	TestEqual(TEXT("None not recorded"), Codex->GetDiscoveredCount(), 1);

	// GetEntries：总数对、已发现的有文案、未发现的无文案。
	const TArray<FHorrorCodexEntry> Entries = Codex->GetEntries();
	TestEqual(TEXT("entries cover all collectables"), Entries.Num(), Codex->GetTotalCount());
	for (const FHorrorCodexEntry& E : Entries)
	{
		if (E.Event == EHorrorEvent::CorridorLights)
		{
			TestTrue(TEXT("discovered entry marked"), E.bDiscovered);
			TestFalse(TEXT("discovered entry has title"), E.Title.IsEmpty());
		}
		else
		{
			TestFalse(TEXT("undiscovered entry not marked"), E.bDiscovered);
			TestTrue(TEXT("undiscovered entry hides title"), E.Title.IsEmpty());
		}
	}

	// 集齐 → CompleteHorrorCodex 成就。
	for (int32 i = 1; i < (int32)EHorrorEvent::Count; ++i)
	{
		Codex->RecordEncounter((EHorrorEvent)i);
	}
	TestEqual(TEXT("codex complete count"), Codex->GetDiscoveredCount(), Codex->GetTotalCount());
	TestTrue(TEXT("complete-codex achievement unlocked"), Prog->HasAchieved(SGAchievementIds::CompleteHorrorCodex()));

	// 存档 round-trip：存档 → 清空 → 读档 → 图鉴还原。
	Save->SaveToSlot(TEXT("CodexTestSlot"));
	const int32 FullCount = Codex->GetDiscoveredCount();
	Codex->RestoreFromSave(0);
	TestEqual(TEXT("codex cleared"), Codex->GetDiscoveredCount(), 0);
	const bool bLoaded = Save->LoadFromSlot(TEXT("CodexTestSlot"));
	TestTrue(TEXT("load succeeded"), bLoaded);
	TestEqual(TEXT("codex restored from save"), Codex->GetDiscoveredCount(), FullCount);
	TestTrue(TEXT("restored has CorridorLights"), Codex->HasDiscovered(EHorrorEvent::CorridorLights));

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
