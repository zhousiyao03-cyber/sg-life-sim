#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/EndingEvaluator.h"
#include "Systems/EndingTypes.h"
#include "Systems/ResidencyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr int64 EEDollars(int64 D) { return D * 100; }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEndingRootedTest,
	"SGLifeSim.Ending.Rooted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEndingRootedTest::RunTest(const FString& Parameters)
{
	// PR + 有房 + 朋友 → 扎根
	const EEnding E = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::PR, /*owns*/true, /*affinity*/60, EEDollars(50000), /*rej*/0);
	TestEqual(TEXT("PR + home + friend -> Rooted"), E, EEnding::Rooted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEndingHeartbreakTest,
	"SGLifeSim.Ending.Heartbreak",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEndingHeartbreakTest::RunTest(const FString& Parameters)
{
	// 破产 → 心碎离开（即便有房有 PR）
	TestEqual(TEXT("bankrupt -> Heartbreak"),
		FEndingEvaluator::EvaluateLeaning(EResidencyStatus::PR, true, 80, -EEDollars(1), 0),
		EEnding::Heartbreak);

	// PR 被拒过且仍是工作准证 → 心碎离开
	TestEqual(TEXT("PR rejected + still EP -> Heartbreak"),
		FEndingEvaluator::EvaluateLeaning(EResidencyStatus::WorkPermit_EP, false, 0, EEDollars(20000), 1),
		EEnding::Heartbreak);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEndingCashOutTest,
	"SGLifeSim.Ending.CashOut",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEndingCashOutTest::RunTest(const FString& Parameters)
{
	// 攒够 $300k 但没扎根（EP、没房） → 兑现离开
	const EEnding E = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::WorkPermit_EP, /*owns*/false, /*affinity*/10, EEDollars(300000), /*rej*/0);
	TestEqual(TEXT("rich but not rooted -> CashOut"), E, EEnding::CashOut);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEndingAdriftTest,
	"SGLifeSim.Ending.Adrift",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEndingAdriftTest::RunTest(const FString& Parameters)
{
	// 没 PR、租房、关系薄、钱不多、没被拒 → 留下漂着
	const EEnding E = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::WorkPermit_EP, /*owns*/false, /*affinity*/15, EEDollars(20000), /*rej*/0);
	TestEqual(TEXT("no PR, renting, thin ties -> Adrift"), E, EEnding::Adrift);

	// 边界：有 PR 有房但关系不够（<50）→ 还不算扎根，钱也不够兑现 → 漂着
	const EEnding E2 = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::PR, true, 49, EEDollars(20000), 0);
	TestEqual(TEXT("PR+home but lonely -> Adrift"), E2, EEnding::Adrift);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEndingBreakdownTest,
	"SGLifeSim.Ending.Breakdown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEndingBreakdownTest::RunTest(const FString& Parameters)
{
	// 理智 < 15 → 被压垮，盖过一切（即便 PR + 有房 + 朋友 + 有钱）。
	const EEnding E = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::Citizen, /*owns*/true, /*affinity*/90, EEDollars(500000), /*rej*/0, /*sanity*/5);
	TestEqual(TEXT("low sanity -> Breakdown overrides everything"), E, EEnding::Breakdown);

	// 理智 15（刚好不算崩溃）→ 走正常判定（这里 = 扎根）。
	const EEnding E2 = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::PR, true, 60, EEDollars(50000), 0, /*sanity*/15);
	TestEqual(TEXT("sanity 15 not breakdown -> Rooted"), E2, EEnding::Rooted);

	// 默认 sanity（不传）= 100，不触发崩溃（向后兼容）。
	const EEnding E3 = FEndingEvaluator::EvaluateLeaning(
		EResidencyStatus::WorkPermit_EP, false, 15, EEDollars(20000), 0);
	TestEqual(TEXT("default sanity -> not breakdown"), E3, EEnding::Adrift);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
