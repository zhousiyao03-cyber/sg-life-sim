#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/SanitySystem.h"
#include "Systems/SanityTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 理智纯核心（Plan 16）：状态阈值、clamp、低理智 dread 权重单调递增。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSanitySystemTest,
	"SGLifeSim.Sanity.StatesAndDread",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSanitySystemTest::RunTest(const FString& Parameters)
{
	// 状态阈值。
	TestEqual(TEXT("100 = Calm"), FSanitySystem::GetState(100), ESanityState::Calm);
	TestEqual(TEXT("70 = Calm"), FSanitySystem::GetState(70), ESanityState::Calm);
	TestEqual(TEXT("69 = Uneasy"), FSanitySystem::GetState(69), ESanityState::Uneasy);
	TestEqual(TEXT("40 = Uneasy"), FSanitySystem::GetState(40), ESanityState::Uneasy);
	TestEqual(TEXT("39 = Disturbed"), FSanitySystem::GetState(39), ESanityState::Disturbed);
	TestEqual(TEXT("15 = Disturbed"), FSanitySystem::GetState(15), ESanityState::Disturbed);
	TestEqual(TEXT("14 = Breaking"), FSanitySystem::GetState(14), ESanityState::Breaking);
	TestEqual(TEXT("0 = Breaking"), FSanitySystem::GetState(0), ESanityState::Breaking);

	// clamp。
	TestEqual(TEXT("clamp high"), FSanitySystem::Clamp(130), 100);
	TestEqual(TEXT("clamp low"), FSanitySystem::Clamp(-20), 0);

	// dread 权重随理智下降单调不减，且崩溃 > 平静。
	const int32 Calm = FSanitySystem::ExtraDreadWeight(90);
	const int32 Uneasy = FSanitySystem::ExtraDreadWeight(55);
	const int32 Disturbed = FSanitySystem::ExtraDreadWeight(25);
	const int32 Breaking = FSanitySystem::ExtraDreadWeight(5);
	TestEqual(TEXT("calm dread = 0"), Calm, 0);
	TestTrue(TEXT("uneasy > calm"), Uneasy > Calm);
	TestTrue(TEXT("disturbed > uneasy"), Disturbed > Uneasy);
	TestTrue(TEXT("breaking > disturbed"), Breaking > Disturbed);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
