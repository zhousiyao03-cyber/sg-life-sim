#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"

#include "Systems/NightCommuteSystem.h"
#include "Systems/NightCommuteTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 鬼月夜归抉择纯核心（Plan 23）：守规矩选项确定且安全，「赶紧进去」是概率赌局，可复现。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNightCommuteSystemTest,
	"SGLifeSim.Horror.NightCommuteChoice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNightCommuteSystemTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(123);

	// 等下一趟：永远安全，理智略增、耗一点精力，不出事。
	{
		const FNightCommuteOutcome O = FNightCommuteSystem::Resolve(ENightCommuteChoice::WaitForNext, Stream);
		TestEqual(TEXT("wait gains sanity"), O.SanityDelta, FNightCommuteSystem::WaitSanityGain);
		TestEqual(TEXT("wait costs energy"), O.EnergyDelta, -FNightCommuteSystem::WaitEnergyCost);
		TestFalse(TEXT("wait nothing happens"), O.bSomethingHappened);
		TestFalse(TEXT("wait has message"), O.Message.IsEmpty());
	}

	// 走楼梯：永远安全，最耗精力，理智无损，不出事。
	{
		const FNightCommuteOutcome O = FNightCommuteSystem::Resolve(ENightCommuteChoice::TakeStairs, Stream);
		TestEqual(TEXT("stairs no sanity change"), O.SanityDelta, 0);
		TestEqual(TEXT("stairs costs most energy"), O.EnergyDelta, -FNightCommuteSystem::StairsEnergyCost);
		TestFalse(TEXT("stairs nothing happens"), O.bSomethingHappened);
	}

	// 赶紧进去：是赌局。多次抽样，赌赢/赌输两种结果都应出现，且后果与是否出事一致。
	{
		FRandomStream S(2024);
		bool bSawBad = false, bSawOk = false;
		for (int32 i = 0; i < 200; ++i)
		{
			const FNightCommuteOutcome O = FNightCommuteSystem::Resolve(ENightCommuteChoice::StepIn, S);
			TestEqual(TEXT("stepin always costs little energy"), O.EnergyDelta, -FNightCommuteSystem::StepInEnergyCost);
			if (O.bSomethingHappened)
			{
				bSawBad = true;
				TestEqual(TEXT("bad outcome heavy sanity hit"), O.SanityDelta, -FNightCommuteSystem::StepInBadSanityCost);
			}
			else
			{
				bSawOk = true;
				TestEqual(TEXT("ok outcome light sanity hit"), O.SanityDelta, -FNightCommuteSystem::StepInOkSanityCost);
			}
		}
		TestTrue(TEXT("stepin sometimes goes bad"), bSawBad);
		TestTrue(TEXT("stepin sometimes is fine"), bSawOk);
	}

	// 同种子可复现。
	{
		FRandomStream A(555), B(555);
		bool bSame = true;
		for (int32 i = 0; i < 100; ++i)
		{
			const FNightCommuteOutcome OA = FNightCommuteSystem::Resolve(ENightCommuteChoice::StepIn, A);
			const FNightCommuteOutcome OB = FNightCommuteSystem::Resolve(ENightCommuteChoice::StepIn, B);
			if (OA.bSomethingHappened != OB.bSomethingHappened) { bSame = false; break; }
		}
		TestTrue(TEXT("same seed -> same gamble sequence"), bSame);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
