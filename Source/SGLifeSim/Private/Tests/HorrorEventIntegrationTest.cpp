#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventTypes.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/TimeSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 恐怖事件端到端（Plan 15）：headless GameInstance 上施加恐怖事件 →
 * 扣心情/健康 + 广播 OnHorrorEvent；并验证开局不是鬼月。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventIntegrationTest,
	"SGLifeSim.Integration.HorrorEventAffectsMood",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorEventIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>();
	UPlayerStateSubsystem* PS     = GI->GetSubsystem<UPlayerStateSubsystem>();
	UTimeSubsystem*        Time   = GI->GetSubsystem<UTimeSubsystem>();
	if (!Horror || !PS || !Time) { GI->Shutdown(); return false; }

	// 开局第 1 月，不是鬼月。
	TestEqual(TEXT("starts month 1"), Time->GetMonthNumber(), 1);
	TestFalse(TEXT("month 1 is not ghost month"), Horror->IsGhostMonth());

	// 设一个已知心情/健康，施加 Pontianak（心情 -8 / 健康 -4）。
	PS->SetAttribute(EPlayerAttribute::Mood, 50);
	PS->SetAttribute(EPlayerAttribute::Health, 50);

	const bool bApplied = Horror->ApplyEvent(EHorrorEvent::Pontianak);
	TestTrue(TEXT("apply returns true for a real event"), bApplied);
	TestEqual(TEXT("mood dropped by 8"), PS->GetAttribute(EPlayerAttribute::Mood), 42);
	TestEqual(TEXT("health dropped by 4"), PS->GetAttribute(EPlayerAttribute::Health), 46);
	TestEqual(TEXT("last event recorded"), Horror->GetLastEvent(), EHorrorEvent::Pontianak);

	// None 不扣属性、返回 false。
	const bool bNone = Horror->ApplyEvent(EHorrorEvent::None);
	TestFalse(TEXT("None returns false"), bNone);
	TestEqual(TEXT("mood unchanged after None"), PS->GetAttribute(EPlayerAttribute::Mood), 42);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
