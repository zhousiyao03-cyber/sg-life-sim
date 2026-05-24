#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"
#include "Systems/EconomicEventSystem.h"
#include "Systems/EconomicEventTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconEventDefsTest,
	"SGLifeSim.EconomicEvent.EffectDefsCorrect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconEventDefsTest::RunTest(const FString& Parameters)
{
	const FEconomicEventDef Crash = FEconomicEventSystem::GetEventDef(EEconomicEvent::CryptoCrash);
	TestEqual(TEXT("crypto crash is investment effect"), Crash.EffectType, EEventEffectType::InvestmentReturnPerMille);
	TestEqual(TEXT("crypto crash -50%"), Crash.Magnitude, -500);

	const FEconomicEventDef Bonus = FEconomicEventSystem::GetEventDef(EEconomicEvent::YearEndBonus);
	TestEqual(TEXT("bonus is salary-months effect"), Bonus.EffectType, EEventEffectType::CashBonusSalaryMonthsX10);
	TestEqual(TEXT("bonus 1.5 months"), Bonus.Magnitude, 15);

	const FEconomicEventDef Gov = FEconomicEventSystem::GetEventDef(EEconomicEvent::GovPayout);
	TestEqual(TEXT("gov payout +$600"), Gov.Magnitude, 60000);

	const FEconomicEventDef NoneDef = FEconomicEventSystem::GetEventDef(EEconomicEvent::None);
	TestEqual(TEXT("none has no effect"), NoneDef.EffectType, EEventEffectType::None);
	TestEqual(TEXT("total weight 100"), FEconomicEventSystem::TotalWeight(), 100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconEventDeterministicTest,
	"SGLifeSim.EconomicEvent.DeterministicWithSeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconEventDeterministicTest::RunTest(const FString& Parameters)
{
	FRandomStream A(1337);
	FRandomStream B(1337);
	for (int32 i = 0; i < 50; ++i)
	{
		const EEconomicEvent Ea = FEconomicEventSystem::PickEvent(A);
		const EEconomicEvent Eb = FEconomicEventSystem::PickEvent(B);
		TestEqual(TEXT("same seed → same event"), Ea, Eb);
		TestTrue(TEXT("event in valid range"), (int32)Ea >= 0 && (int32)Ea < (int32)EEconomicEvent::Count);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconEventNoneMostCommonTest,
	"SGLifeSim.EconomicEvent.NoneIsMostCommon",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconEventNoneMostCommonTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(98765);
	int32 NoneCount = 0;
	const int32 N = 10000;
	for (int32 i = 0; i < N; ++i)
	{
		if (FEconomicEventSystem::PickEvent(Stream) == EEconomicEvent::None)
		{
			++NoneCount;
		}
	}
	// None 权重 70% → 大样本下应明显过半。
	TestTrue(TEXT("None occurs >50% of months"), NoneCount > N / 2);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
