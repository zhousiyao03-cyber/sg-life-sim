#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/ProgressSystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressMarkAndQueryTest,
	"SGLifeSim.Progress.MarkAndQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProgressMarkAndQueryTest::RunTest(const FString& Parameters)
{
	FProgressSystem Sys;
	TestFalse(TEXT("unknown achievement not achieved"), Sys.HasAchieved(TEXT("First10k")));

	const bool bNew = Sys.MarkAchieved(TEXT("First10k"));
	TestTrue(TEXT("first mark returns true (newly unlocked)"), bNew);
	TestTrue(TEXT("now achieved"), Sys.HasAchieved(TEXT("First10k")));
	TestEqual(TEXT("count is 1"), Sys.GetAchievedCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressDedupTest,
	"SGLifeSim.Progress.DedupesRepeatMark",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProgressDedupTest::RunTest(const FString& Parameters)
{
	FProgressSystem Sys;
	Sys.MarkAchieved(TEXT("FirstJob"));
	const bool bSecond = Sys.MarkAchieved(TEXT("FirstJob"));
	TestFalse(TEXT("re-mark returns false"), bSecond);
	TestEqual(TEXT("count stays 1 after dup"), Sys.GetAchievedCount(), 1);

	// 委托只应在首次触发一次
	int32 FireCount = 0;
	Sys.OnAchievementUnlocked.AddLambda([&FireCount](FName){ ++FireCount; });
	Sys.MarkAchieved(TEXT("FirstJob"));        // 已有 → 不触发
	Sys.MarkAchieved(TEXT("FirstPromotion"));  // 新 → 触发
	TestEqual(TEXT("delegate fires only for new unlock"), FireCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressRestoreTest,
	"SGLifeSim.Progress.RestoreFromSave",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProgressRestoreTest::RunTest(const FString& Parameters)
{
	FProgressSystem Sys;
	Sys.MarkAchieved(TEXT("Stale"));
	Sys.RestoreAchieved({ TEXT("First10k"), TEXT("FirstCar") });

	TestEqual(TEXT("count after restore is 2"), Sys.GetAchievedCount(), 2);
	TestTrue(TEXT("restored First10k"), Sys.HasAchieved(TEXT("First10k")));
	TestFalse(TEXT("pre-restore entry cleared"), Sys.HasAchieved(TEXT("Stale")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
