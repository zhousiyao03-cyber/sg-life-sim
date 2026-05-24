#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/ActivitySystem.h"
#include "Systems/ActivityTypes.h"
#include "Systems/PlayerStatsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActivityDefsTest,
	"SGLifeSim.Activity.DefsCorrect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FActivityDefsTest::RunTest(const FString& Parameters)
{
	const FActivityDef Study = FActivitySystem::GetActivityDef(EActivityType::Study);
	TestEqual(TEXT("study +4 professional"), Study.GetAttr(EPlayerAttribute::Professional), 4);
	TestEqual(TEXT("study +2 insight"), Study.GetAttr(EPlayerAttribute::Insight), 2);
	TestEqual(TEXT("study -15 energy"), Study.GetAttr(EPlayerAttribute::Energy), -15);
	TestEqual(TEXT("study 1 block"), Study.TimeBlocks, 1);

	const FActivityDef Code = FActivitySystem::GetActivityDef(EActivityType::FreelanceCode);
	TestEqual(TEXT("freelance +$300"), Code.CashDeltaCents, (int64)30000);

	const FActivityDef Sleep = FActivitySystem::GetActivityDef(EActivityType::Sleep);
	TestEqual(TEXT("sleep +60 energy"), Sleep.GetAttr(EPlayerAttribute::Energy), 60);
	TestEqual(TEXT("sleep 2 blocks"), Sleep.TimeBlocks, 2);

	const FActivityDef Eat = FActivitySystem::GetActivityDef(EActivityType::EatHawker);
	TestEqual(TEXT("eat costs $5"), Eat.CashDeltaCents, (int64)-500);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActivityEnergyGateTest,
	"SGLifeSim.Activity.EnergyGatesActivities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FActivityEnergyGateTest::RunTest(const FString& Parameters)
{
	const FActivityDef Code = FActivitySystem::GetActivityDef(EActivityType::FreelanceCode); // -20
	TestTrue(TEXT("can code at energy 20"), FActivitySystem::CanPerform(Code, 20));
	TestFalse(TEXT("cannot code at energy 19"), FActivitySystem::CanPerform(Code, 19));
	TestFalse(TEXT("cannot code at energy 0"), FActivitySystem::CanPerform(Code, 0));

	// 恢复型（睡觉/吃饭）即使没能量也能做。
	const FActivityDef Sleep = FActivitySystem::GetActivityDef(EActivityType::Sleep);
	TestTrue(TEXT("can always sleep"), FActivitySystem::CanPerform(Sleep, 0));
	const FActivityDef Eat = FActivitySystem::GetActivityDef(EActivityType::EatHawker);
	TestTrue(TEXT("can always eat"), FActivitySystem::CanPerform(Eat, 0));
	return true;
}

// 理智恢复手段（Plan 17）：拜拜祈福强回理智、不耗能量随时可做；睡觉也回一点。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActivitySanityTest,
	"SGLifeSim.Activity.SanityRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FActivitySanityTest::RunTest(const FString& Parameters)
{
	const FActivityDef Pray = FActivitySystem::GetActivityDef(EActivityType::PrayPuja);
	TestTrue(TEXT("pray restores sanity"), Pray.SanityDelta > 0);
	TestEqual(TEXT("pray costs no energy"), Pray.GetAttr(EPlayerAttribute::Energy), 0);
	TestTrue(TEXT("can always pray (no energy cost)"), FActivitySystem::CanPerform(Pray, 0));
	TestEqual(TEXT("pray costs $2"), Pray.CashDeltaCents, (int64)-200);

	const FActivityDef Sleep = FActivitySystem::GetActivityDef(EActivityType::Sleep);
	TestTrue(TEXT("sleep restores some sanity"), Sleep.SanityDelta > 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
