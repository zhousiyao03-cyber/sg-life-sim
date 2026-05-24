#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/PlayerStats.h"
#include "Systems/PlayerStatsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerStatsDefaultsTest,
	"SGLifeSim.PlayerStats.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStatsDefaultsTest::RunTest(const FString& Parameters)
{
	FPlayerStats Stats;
	TestEqual(TEXT("energy starts full"), Stats.Get(EPlayerAttribute::Energy), 100);
	TestEqual(TEXT("health starts 80"), Stats.Get(EPlayerAttribute::Health), 80);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerStatsClampTest,
	"SGLifeSim.PlayerStats.ModifyClamps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStatsClampTest::RunTest(const FString& Parameters)
{
	FPlayerStats Stats;
	Stats.Set(EPlayerAttribute::Mood, 50);
	Stats.Modify(EPlayerAttribute::Mood, -100);
	TestEqual(TEXT("mood clamps at 0"), Stats.Get(EPlayerAttribute::Mood), 0);

	Stats.Modify(EPlayerAttribute::Mood, 1000);
	TestEqual(TEXT("mood clamps at 100"), Stats.Get(EPlayerAttribute::Mood), 100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerStatsDailyEnergyTest,
	"SGLifeSim.PlayerStats.DailyEnergyRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStatsDailyEnergyTest::RunTest(const FString& Parameters)
{
	FPlayerStats Stats;
	Stats.Modify(EPlayerAttribute::Energy, -70);  // 一天活动消耗
	TestEqual(TEXT("energy drained to 30"), Stats.Get(EPlayerAttribute::Energy), 30);

	Stats.RestoreEnergyDaily();
	TestEqual(TEXT("energy restored to 100"), Stats.Get(EPlayerAttribute::Energy), 100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerStatsSnapshotTest,
	"SGLifeSim.PlayerStats.SnapshotRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStatsSnapshotTest::RunTest(const FString& Parameters)
{
	FPlayerStats A;
	A.Set(EPlayerAttribute::Professional, 75);
	A.Set(EPlayerAttribute::Insight, 42);

	const TArray<int32> Snap = A.GetSnapshot();
	TestEqual(TEXT("snapshot length == attribute count"), Snap.Num(), (int32)EPlayerAttribute::Count);

	FPlayerStats B;
	B.RestoreSnapshot(Snap);
	TestEqual(TEXT("restored professional"), B.Get(EPlayerAttribute::Professional), 75);
	TestEqual(TEXT("restored insight"), B.Get(EPlayerAttribute::Insight), 42);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
