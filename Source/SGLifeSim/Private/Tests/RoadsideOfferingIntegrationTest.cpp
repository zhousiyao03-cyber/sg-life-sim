#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Math/RandomStream.h"

#include "Systems/RoadsideOfferingSubsystem.h"
#include "Systems/RoadsideOfferingSystem.h"
#include "Systems/RoadsideOfferingTypes.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/HorrorEventTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** 用纯核心找一个「第一次 StepOver 就赌输」的种子，避免硬编码脆裂。 */
	int32 FindSeedWhereStepOverGoesBad()
	{
		for (int32 Seed = 1; Seed < 10000; ++Seed)
		{
			FRandomStream S(Seed);
			if (FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice::StepOver, S).bSomethingHappened)
			{
				return Seed;
			}
		}
		return 1;
	}
}

/**
 * 鬼月路边祭品端到端（Plan 26）：MakeChoice 把结算落到理智/精力；拜一拜回理智；
 * 跨过去赌输重扣理智 + 记进图鉴（ZhiQianTaboo），与 AhMei 冥纸共鸣对话闭环。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRoadsideOfferingIntegrationTest,
	"SGLifeSim.Integration.RoadsideOfferingResolves",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRoadsideOfferingIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	URoadsideOfferingSubsystem* Road = GI->GetSubsystem<URoadsideOfferingSubsystem>();
	USanitySubsystem*           San  = GI->GetSubsystem<USanitySubsystem>();
	UPlayerStateSubsystem*      PS   = GI->GetSubsystem<UPlayerStateSubsystem>();
	UHorrorCodexSubsystem*      Codex= GI->GetSubsystem<UHorrorCodexSubsystem>();
	if (!Road || !San || !PS || !Codex) { GI->Shutdown(); return false; }

	// 开局非鬼月深夜 → 不可触发。
	TestFalse(TEXT("not available at game start"), Road->IsAvailable());

	// 拜一拜：理智 +8、精力 -5。
	San->Drain(40); // 留出回升空间
	PS->SetAttribute(EPlayerAttribute::Energy, 80);
	const int32 SanityBeforePray = San->GetSanity();
	Road->MakeChoice(ERoadsideOfferingChoice::PayRespects);
	TestEqual(TEXT("pay respects restores sanity +8"), San->GetSanity(),
		SanityBeforePray + FRoadsideOfferingSystem::PayRespectsSanityGain);
	TestEqual(TEXT("pay respects costs energy 5"), PS->GetAttribute(EPlayerAttribute::Energy),
		80 - FRoadsideOfferingSystem::PayRespectsEnergyCost);

	// 跨过去且赌输：理智重扣 20，图鉴记上冥纸禁忌。
	San->RestoreFromSave(100);
	PS->SetAttribute(EPlayerAttribute::Energy, 80);
	TestFalse(TEXT("codex has no zhiqian yet"), Codex->HasDiscovered(EHorrorEvent::ZhiQianTaboo));
	Road->SetSeed(FindSeedWhereStepOverGoesBad());
	const FRoadsideOfferingOutcome Bad = Road->MakeChoice(ERoadsideOfferingChoice::StepOver);
	TestTrue(TEXT("step-over went bad"), Bad.bSomethingHappened);
	TestEqual(TEXT("bad step-over drains sanity 20"), San->GetSanity(),
		100 - FRoadsideOfferingSystem::StepOverBadSanityCost);
	TestEqual(TEXT("step-over costs energy 2"), PS->GetAttribute(EPlayerAttribute::Energy),
		80 - FRoadsideOfferingSystem::StepOverEnergyCost);
	TestTrue(TEXT("codex now records zhiqian taboo"), Codex->HasDiscovered(EHorrorEvent::ZhiQianTaboo));

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
