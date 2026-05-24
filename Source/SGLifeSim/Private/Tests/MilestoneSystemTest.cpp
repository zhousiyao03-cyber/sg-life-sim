#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/MilestoneSystem.h"
#include "Systems/MilestoneTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// 文件内唯一名，避开 unity build ODR。
namespace { constexpr int64 MSCents(int64 D) { return D * 100; } }

/**
 * 里程碑纯逻辑核心（Plan 13）：默认全未达成、各条件正确触达、当前目标按序推进、数值进度正确。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMilestoneSystemTest,
	"SGLifeSim.Milestone.EvaluatesAndProgresses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMilestoneSystemTest::RunTest(const FString& Parameters)
{
	// 开局快照：无薪水 / $0 / 待业 / 无房 / EP。
	FMilestoneContext Ctx;

	TestEqual(TEXT("seven milestones"), FMilestoneSystem::Num(), 7);
	TestEqual(TEXT("nothing completed at start"), FMilestoneSystem::CountCompleted(Ctx), 0);
	TestEqual(TEXT("active = FirstSalary at start"),
		FMilestoneSystem::GetActive(Ctx), EMilestone::FirstSalary);

	// 拿到第一份薪水 → FirstSalary 达成，目标推进到攒钱。
	Ctx.bHasFirstSalary = true;
	TestTrue(TEXT("FirstSalary complete"), FMilestoneSystem::IsComplete(EMilestone::FirstSalary, Ctx));
	TestEqual(TEXT("active = Save5k after salary"),
		FMilestoneSystem::GetActive(Ctx), EMilestone::Save5k);

	// 攒钱的数值进度：$3,200 时未达成、显示当前/目标。
	Ctx.CashCents = MSCents(3200);
	{
		const FMilestoneProgress P = FMilestoneSystem::Evaluate(EMilestone::Save5k, Ctx);
		TestTrue(TEXT("Save5k is numeric"), P.bIsNumeric);
		TestFalse(TEXT("Save5k not yet complete at $3200"), P.bComplete);
		TestEqual(TEXT("Save5k current = $3200"), P.CurrentCents, MSCents(3200));
		TestEqual(TEXT("Save5k target = $5000"), P.TargetCents, MSCents(5000));
	}

	// 攒够 $5,000 → 达成，推进到升职。
	Ctx.CashCents = MSCents(5000);
	TestTrue(TEXT("Save5k complete at $5000"), FMilestoneSystem::IsComplete(EMilestone::Save5k, Ctx));
	TestEqual(TEXT("active = PromoteToMid"),
		FMilestoneSystem::GetActive(Ctx), EMilestone::PromoteToMid);

	// 升到中级 → 达成；高级也算达成（>= Mid）。
	Ctx.Career = ECareerLevel::Mid;
	TestTrue(TEXT("PromoteToMid complete at Mid"), FMilestoneSystem::IsComplete(EMilestone::PromoteToMid, Ctx));
	Ctx.Career = ECareerLevel::Senior;
	TestTrue(TEXT("PromoteToMid still complete at Senior"), FMilestoneSystem::IsComplete(EMilestone::PromoteToMid, Ctx));

	// 买房 → 达成。
	Ctx.bOwnsHome = true;
	TestEqual(TEXT("active = BecomePR after home"),
		FMilestoneSystem::GetActive(Ctx), EMilestone::BecomePR);

	// PR → 达成；净资产门槛。
	Ctx.Residency = EResidencyStatus::PR;
	TestTrue(TEXT("BecomePR complete"), FMilestoneSystem::IsComplete(EMilestone::BecomePR, Ctx));
	TestEqual(TEXT("active = NetWorth100k"),
		FMilestoneSystem::GetActive(Ctx), EMilestone::NetWorth100k);

	Ctx.NetWorthCents = MSCents(100000);
	TestTrue(TEXT("NetWorth100k complete"), FMilestoneSystem::IsComplete(EMilestone::NetWorth100k, Ctx));

	// 成为公民 → 全部达成。
	Ctx.Residency = EResidencyStatus::Citizen;
	TestEqual(TEXT("all seven completed"), FMilestoneSystem::CountCompleted(Ctx), 7);
	TestEqual(TEXT("active = Count when all done"),
		FMilestoneSystem::GetActive(Ctx), EMilestone::Count);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
