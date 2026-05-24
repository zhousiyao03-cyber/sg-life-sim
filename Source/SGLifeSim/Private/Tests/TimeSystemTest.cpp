#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/TimeSystem.h"
#include "Systems/TimeBlock.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTimeSystemAdvancesBlockTest,
	"SGLifeSim.TimeSystem.AdvancesOneBlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemAdvancesBlockTest::RunTest(const FString& Parameters)
{
	FTimeSystem Sys;
	TestEqual(TEXT("initial block is Morning"), Sys.GetCurrentBlock(), ETimeBlock::Morning);

	Sys.AdvanceBlock();
	TestEqual(TEXT("after one advance: Forenoon"), Sys.GetCurrentBlock(), ETimeBlock::Forenoon);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTimeSystemWrapsToNextDayTest,
	"SGLifeSim.TimeSystem.WrapsToNextDay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemWrapsToNextDayTest::RunTest(const FString& Parameters)
{
	FTimeSystem Sys;
	const int32 InitialDay = Sys.GetDayNumber();

	// 推进 5 次（5 个时间块 = 一整天）
	for (int32 i = 0; i < 5; ++i)
	{
		Sys.AdvanceBlock();
	}

	TestEqual(TEXT("after 5 advances: back to Morning"),
		Sys.GetCurrentBlock(), ETimeBlock::Morning);
	TestEqual(TEXT("day number incremented by 1"),
		Sys.GetDayNumber(), InitialDay + 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTimeSystemWeekdayRotatesTest,
	"SGLifeSim.TimeSystem.WeekdayRotates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemWeekdayRotatesTest::RunTest(const FString& Parameters)
{
	FTimeSystem Sys;  // 默认从 Monday 早晨开始
	TestEqual(TEXT("initial weekday: Monday"), Sys.GetWeekday(), EWeekday::Monday);

	// 推进 7 天 = 35 个 block
	for (int32 i = 0; i < 35; ++i)
	{
		Sys.AdvanceBlock();
	}

	TestEqual(TEXT("after 7 days: back to Monday"), Sys.GetWeekday(), EWeekday::Monday);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTimeSystemTotalBlocksTest,
	"SGLifeSim.TimeSystem.TotalBlocksMatchesAdvances",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemTotalBlocksTest::RunTest(const FString& Parameters)
{
	FTimeSystem Sys;
	TestEqual(TEXT("initial total blocks is 0"), Sys.GetTotalBlocks(), 0);

	for (int32 i = 0; i < 12; ++i)
	{
		Sys.AdvanceBlock();
	}

	TestEqual(TEXT("after 12 advances: total blocks is 12"), Sys.GetTotalBlocks(), 12);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
