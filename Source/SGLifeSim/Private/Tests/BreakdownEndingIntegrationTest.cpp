#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/SanitySubsystem.h"
#include "Systems/EndingSubsystem.h"
#include "Systems/EndingTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 精神崩溃端到端（Plan 18）：耗尽理智 → 终局倾向「被压垮」，且归零强制选定该结局。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBreakdownEndingIntegrationTest,
	"SGLifeSim.Integration.SanityBreakdownEnding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBreakdownEndingIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>();
	UEndingSubsystem* End = GI->GetSubsystem<UEndingSubsystem>();
	if (!San || !End) { GI->Shutdown(); return false; }

	// 开局：理智满，没崩溃，倾向不是被压垮。
	TestNotEqual(TEXT("not breakdown at start"), End->GetCurrentLeaning(), EEnding::Breakdown);
	TestEqual(TEXT("no chosen ending at start"), End->GetChosenEnding(), EEnding::None);

	// 理智掉到濒临崩溃档（<15）→ 倾向变「被压垮」（警示），但还没强制结束。
	San->RestoreFromSave(10);
	TestEqual(TEXT("leaning is Breakdown when sanity < 15"),
		End->GetCurrentLeaning(), EEnding::Breakdown);

	// 理智归零 → 强制选定「被压垮」结局。
	San->Drain(50); // 10 -> 0
	TestEqual(TEXT("sanity floored at 0"), San->GetSanity(), 0);
	TestEqual(TEXT("breakdown ending forced at sanity 0"),
		End->GetChosenEnding(), EEnding::Breakdown);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
