#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/CareerSystem.h"
#include "Systems/CareerTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCareerStartsAsJuniorTest,
	"SGLifeSim.Career.StartsAsJunior",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCareerStartsAsJuniorTest::RunTest(const FString& Parameters)
{
	FCareerSystem Sys;
	TestEqual(TEXT("starts Junior"), Sys.GetLevel(), ECareerLevel::Junior);
	TestEqual(TEXT("starts $5000"), Sys.GetGrossSalaryCents(), (int64)500000);
	TestEqual(TEXT("tenure 0"), Sys.GetMonthsInRole(), 0);
	TestTrue(TEXT("employed"), Sys.IsEmployed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCareerPromotionGatedByTenureAndSkillTest,
	"SGLifeSim.Career.PromotionGatedByTenureAndSkill",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCareerPromotionGatedByTenureAndSkillTest::RunTest(const FString& Parameters)
{
	FCareerSystem Sys;

	// 专业技能足够(60≥50) 但在职不足 3 月 → 不能升。
	TestFalse(TEXT("no promote before tenure"), Sys.CanPromote(60));
	Sys.AdvanceMonth();
	Sys.AdvanceMonth();
	TestFalse(TEXT("still not enough at 2 months"), Sys.CanPromote(60));
	Sys.AdvanceMonth(); // 3 个月

	// 在职够了，但技能不足(40<50) → 仍不能升。
	TestFalse(TEXT("no promote with low skill"), Sys.CanPromote(40));

	// 技能够 + 在职够 → 升 Mid，涨薪到 $7500，在职清零。
	TestTrue(TEXT("can promote now"), Sys.CanPromote(60));
	TestTrue(TEXT("promote ok"), Sys.Promote(60));
	TestEqual(TEXT("now Mid"), Sys.GetLevel(), ECareerLevel::Mid);
	TestEqual(TEXT("salary $7500"), Sys.GetGrossSalaryCents(), (int64)750000);
	TestEqual(TEXT("tenure reset"), Sys.GetMonthsInRole(), 0);

	// 升 Senior 需技能 65：当前 60 不够。
	for (int32 i = 0; i < 3; ++i) { Sys.AdvanceMonth(); }
	TestFalse(TEXT("Senior needs skill 65"), Sys.CanPromote(60));
	TestTrue(TEXT("Senior ok at 65"), Sys.Promote(65));
	TestEqual(TEXT("now Senior"), Sys.GetLevel(), ECareerLevel::Senior);
	TestEqual(TEXT("salary $11000"), Sys.GetGrossSalaryCents(), (int64)1100000);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCareerJobHopRaisesSalaryTest,
	"SGLifeSim.Career.JobHopRaisesSalary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCareerJobHopRaisesSalaryTest::RunTest(const FString& Parameters)
{
	FCareerSystem Sys; // Junior $5000
	Sys.AdvanceMonth();
	Sys.AdvanceMonth();

	// 跳槽 +35% → $6750，在职清零，等级不变。
	TestTrue(TEXT("jobhop ok"), Sys.JobHop(35));
	TestEqual(TEXT("salary +35% = $6750"), Sys.GetGrossSalaryCents(), (int64)675000);
	TestEqual(TEXT("still Junior"), Sys.GetLevel(), ECareerLevel::Junior);
	TestEqual(TEXT("tenure reset by jobhop"), Sys.GetMonthsInRole(), 0);

	// 升 Mid 时不砍掉跳槽涨上去的薪资（$6750 > $7500? 否 → 取 $7500）。
	for (int32 i = 0; i < 3; ++i) { Sys.AdvanceMonth(); }
	Sys.Promote(50);
	TestEqual(TEXT("promote keeps higher of jobhop/base"),
		Sys.GetGrossSalaryCents(), (int64)750000);

	// 反过来：薪资已高于目标级基薪时，升职不降薪。
	FCareerSystem Sys2;          // Junior $5000
	Sys2.JobHop(60);             // → $8000，高于 Mid 基薪 $7500
	for (int32 i = 0; i < 3; ++i) { Sys2.AdvanceMonth(); }
	Sys2.Promote(50);            // 升 Mid
	TestEqual(TEXT("keeps $8000, not cut to $7500"),
		Sys2.GetGrossSalaryCents(), (int64)800000);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCareerCannotPromotePastTopTest,
	"SGLifeSim.Career.CannotPromotePastTop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCareerCannotPromotePastTopTest::RunTest(const FString& Parameters)
{
	FCareerSystem Sys;
	Sys.RestoreState(ECareerLevel::Principal, (int64)2400000, 12);
	TestFalse(TEXT("cannot promote past Principal"), Sys.CanPromote(100));
	TestFalse(TEXT("promote returns false at top"), Sys.Promote(100));
	TestEqual(TEXT("still Principal"), Sys.GetLevel(), ECareerLevel::Principal);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
