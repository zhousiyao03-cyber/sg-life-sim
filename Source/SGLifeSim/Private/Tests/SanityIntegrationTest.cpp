#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "Systems/SanitySubsystem.h"
#include "Systems/SanityTypes.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventTypes.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/SaveGameSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 理智端到端（Plan 16）：恐怖事件扣理智、低理智升恐惧、每日恢复（鬼月除外）、存档 round-trip。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSanityIntegrationTest,
	"SGLifeSim.Integration.SanitySpiralAndSave",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSanityIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	USanitySubsystem*      San    = GI->GetSubsystem<USanitySubsystem>();
	UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>();
	UTimeSubsystem*        Time   = GI->GetSubsystem<UTimeSubsystem>();
	USaveGameSubsystem*    Save   = GI->GetSubsystem<USaveGameSubsystem>();
	if (!San || !Horror || !Time || !Save) { GI->Shutdown(); return false; }

	// 关随机夜间恐怖，让理智只受我显式操作影响（确定性）。
	Horror->SetHorrorEnabled(false);

	auto AdvanceOneDay = [Time]() { for (int32 i = 0; i < 5; ++i) { Time->AdvanceBlock(); } };

	// 开局：理智满、平静、无恐惧加注。
	TestEqual(TEXT("starts sane"), San->GetSanity(), 100);
	TestEqual(TEXT("starts Calm"), San->GetState(), ESanityState::Calm);
	TestEqual(TEXT("calm dread = 0"), San->GetExtraDreadWeight(), 0);

	// 恐怖事件扣理智（电梯空楼层 SanityCost=6）。
	Horror->ApplyEvent(EHorrorEvent::ElevatorGhostFloor);
	TestEqual(TEXT("sanity drained by horror"), San->GetSanity(), 94);

	// 跌到失常档 → 恐惧加注 > 0（恐惧螺旋）。
	San->Drain(60); // 94 -> 34
	TestEqual(TEXT("now Disturbed"), San->GetState(), ESanityState::Disturbed);
	TestTrue(TEXT("low sanity raises dread weight"), San->GetExtraDreadWeight() > 0);

	// 每日恢复（第 1 月，非鬼月）：跨一天 +8。
	const int32 Before = San->GetSanity();
	AdvanceOneDay();
	TestEqual(TEXT("daily recovery +8 outside ghost month"), San->GetSanity(), Before + 8);

	// 推进到农历七月（第 7 月）。
	while (Time->GetMonthNumber() < 7) { Time->AdvanceBlock(); }
	TestTrue(TEXT("reached ghost month"), Horror->IsGhostMonth());

	// 鬼月：设已知理智，跨一天 → 不恢复（没有喘息）。
	San->RestoreFromSave(50);
	const int32 GhostBefore = San->GetSanity();
	AdvanceOneDay();
	TestEqual(TEXT("no recovery during ghost month"), San->GetSanity(), GhostBefore);

	// 存档 round-trip。
	San->RestoreFromSave(33);
	const FString Slot = TEXT("SGLifeSim_SanitySlot");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	TestTrue(TEXT("save ok"), Save->SaveToSlot(Slot));
	San->Drain(20); // 改动
	TestNotEqual(TEXT("sanity changed after save"), San->GetSanity(), 33);
	TestTrue(TEXT("load ok"), Save->LoadFromSlot(Slot));
	TestEqual(TEXT("sanity restored to 33"), San->GetSanity(), 33);

	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
