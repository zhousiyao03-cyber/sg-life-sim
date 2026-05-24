#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"

#include "Systems/RoadsideOfferingSystem.h"
#include "Systems/RoadsideOfferingTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 鬼月路边祭品抉择纯核心（Plan 26）：绕开/拜一拜确定且安全，「跨过去」是概率赌局，可复现。
 * 「拜一拜」回理智应多于「绕开」（敬畏化解恐惧 vs 单纯避让）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRoadsideOfferingSystemTest,
	"SGLifeSim.Horror.RoadsideOfferingChoice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRoadsideOfferingSystemTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(123);

	// 绕开：永远安全，理智略增、费精力，不出事。
	{
		const FRoadsideOfferingOutcome O = FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice::DetourAround, Stream);
		TestEqual(TEXT("detour gains sanity"), O.SanityDelta, FRoadsideOfferingSystem::DetourSanityGain);
		TestEqual(TEXT("detour costs energy"), O.EnergyDelta, -FRoadsideOfferingSystem::DetourEnergyCost);
		TestFalse(TEXT("detour nothing happens"), O.bSomethingHappened);
		TestFalse(TEXT("detour has message"), O.Message.IsEmpty());
	}

	// 拜一拜：永远安全，回理智最多，不出事。
	{
		const FRoadsideOfferingOutcome O = FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice::PayRespects, Stream);
		TestEqual(TEXT("pay respects gains most sanity"), O.SanityDelta, FRoadsideOfferingSystem::PayRespectsSanityGain);
		TestFalse(TEXT("pay respects nothing happens"), O.bSomethingHappened);
		TestTrue(TEXT("pay respects beats detour for sanity"),
			FRoadsideOfferingSystem::PayRespectsSanityGain > FRoadsideOfferingSystem::DetourSanityGain);
	}

	// 跨过去：是赌局。多次抽样，赌赢/赌输都应出现，后果与是否出事一致。
	{
		FRandomStream S(2024);
		bool bSawBad = false, bSawOk = false;
		for (int32 i = 0; i < 200; ++i)
		{
			const FRoadsideOfferingOutcome O = FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice::StepOver, S);
			TestEqual(TEXT("stepover always costs little energy"), O.EnergyDelta, -FRoadsideOfferingSystem::StepOverEnergyCost);
			if (O.bSomethingHappened)
			{
				bSawBad = true;
				TestEqual(TEXT("bad outcome heavy sanity hit"), O.SanityDelta, -FRoadsideOfferingSystem::StepOverBadSanityCost);
			}
			else
			{
				bSawOk = true;
				TestEqual(TEXT("ok outcome light sanity hit"), O.SanityDelta, -FRoadsideOfferingSystem::StepOverOkSanityCost);
			}
		}
		TestTrue(TEXT("stepover sometimes goes bad"), bSawBad);
		TestTrue(TEXT("stepover sometimes is fine"), bSawOk);
	}

	// 同种子可复现。
	{
		FRandomStream A(555), B(555);
		bool bSame = true;
		for (int32 i = 0; i < 100; ++i)
		{
			const FRoadsideOfferingOutcome OA = FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice::StepOver, A);
			const FRoadsideOfferingOutcome OB = FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice::StepOver, B);
			if (OA.bSomethingHappened != OB.bSomethingHappened) { bSame = false; break; }
		}
		TestTrue(TEXT("same seed -> same gamble sequence"), bSame);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
