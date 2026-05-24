#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/ActivitySubsystem.h"
#include "Systems/ActivityTypes.h"
#include "Systems/SanitySubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 理智恢复端到端（Plan 17）：拜拜祈福活动确实把理智补回来 —— 玩家对抗恐惧螺旋的手段。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActivityPrayRestoresSanityTest,
	"SGLifeSim.Integration.PrayRestoresSanity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FActivityPrayRestoresSanityTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UActivitySubsystem* Act = GI->GetSubsystem<UActivitySubsystem>();
	USanitySubsystem*   San = GI->GetSubsystem<USanitySubsystem>();
	if (!Act || !San) { GI->Shutdown(); return false; }

	// 理智跌到 40，拜一次（+20，1 个时间块，开局 Morning 不跨天）→ 60。
	San->RestoreFromSave(40);
	const bool bDone = Act->PerformActivity(EActivityType::PrayPuja);
	TestTrue(TEXT("pray performed"), bDone);
	TestEqual(TEXT("sanity restored to 60 after praying"), San->GetSanity(), 60);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
