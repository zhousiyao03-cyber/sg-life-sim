#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Math/RandomStream.h"

#include "Systems/NightCommuteSubsystem.h"
#include "Systems/NightCommuteSystem.h"
#include "Systems/NightCommuteTypes.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** 用纯核心找一个「第一次 StepIn 就赌输」的种子，避免硬编码脆裂。 */
	int32 FindSeedWhereStepInGoesBad()
	{
		for (int32 Seed = 1; Seed < 10000; ++Seed)
		{
			FRandomStream S(Seed);
			if (FNightCommuteSystem::Resolve(ENightCommuteChoice::StepIn, S).bSomethingHappened)
			{
				return Seed;
			}
		}
		return 1;
	}

	/** 找一个「第一次 StepIn 就赌赢（没事）」的种子。 */
	int32 FindSeedWhereStepInIsFine()
	{
		for (int32 Seed = 1; Seed < 10000; ++Seed)
		{
			FRandomStream S(Seed);
			if (!FNightCommuteSystem::Resolve(ENightCommuteChoice::StepIn, S).bSomethingHappened)
			{
				return Seed;
			}
		}
		return 1;
	}
}

/**
 * 鬼月夜归端到端（Plan 23）：MakeChoice 把结算落到理智/精力子系统；赌输重扣、守规矩不扣。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNightCommuteIntegrationTest,
	"SGLifeSim.Integration.NightCommuteResolves",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNightCommuteIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UNightCommuteSubsystem* Commute = GI->GetSubsystem<UNightCommuteSubsystem>();
	USanitySubsystem*       Sanity  = GI->GetSubsystem<USanitySubsystem>();
	UPlayerStateSubsystem*  PS      = GI->GetSubsystem<UPlayerStateSubsystem>();
	if (!Commute || !Sanity || !PS) { GI->Shutdown(); return false; }

	// 开局非鬼月深夜 → 不可触发。
	TestFalse(TEXT("not available at game start (month 1, not late night)"), Commute->IsAvailable());

	// 等下一趟：理智 +2、精力 -8。
	PS->SetAttribute(EPlayerAttribute::Energy, 80);
	const int32 SanityBeforeWait = Sanity->GetSanity();
	Commute->MakeChoice(ENightCommuteChoice::WaitForNext);
	TestEqual(TEXT("wait restores sanity +2"), Sanity->GetSanity(),
		FMath::Min(100, SanityBeforeWait + FNightCommuteSystem::WaitSanityGain));
	TestEqual(TEXT("wait costs energy 8"), PS->GetAttribute(EPlayerAttribute::Energy),
		80 - FNightCommuteSystem::WaitEnergyCost);

	// 赶紧进去且赌输：理智重扣 22。
	Sanity->RestoreFromSave(100);
	PS->SetAttribute(EPlayerAttribute::Energy, 80);
	Commute->SetSeed(FindSeedWhereStepInGoesBad());
	const FNightCommuteOutcome Bad = Commute->MakeChoice(ENightCommuteChoice::StepIn);
	TestTrue(TEXT("step-in went bad"), Bad.bSomethingHappened);
	TestEqual(TEXT("bad outcome drained sanity by 22"), Sanity->GetSanity(),
		100 - FNightCommuteSystem::StepInBadSanityCost);

	// 赶紧进去但赌赢：理智仅轻扣 4。
	Sanity->RestoreFromSave(100);
	Commute->SetSeed(FindSeedWhereStepInIsFine());
	const FNightCommuteOutcome Ok = Commute->MakeChoice(ENightCommuteChoice::StepIn);
	TestFalse(TEXT("step-in was fine"), Ok.bSomethingHappened);
	TestEqual(TEXT("ok outcome only lightly drained sanity"), Sanity->GetSanity(),
		100 - FNightCommuteSystem::StepInOkSanityCost);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
