#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "World/LocationManagerSubsystem.h"
#include "World/LocationTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 地点进出枢纽端到端（开放城市枢纽）：记城市坐标 / 回程消费一次性 / 当前地点状态。
 * OpenLevel 是运行时行为，自动化环境不切关卡，故验可独立测的状态机，切关卡留 PIE。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLocationManagerIntegrationTest,
	"SGLifeSim.Integration.LocationManager",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLocationManagerIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	USGLocationManagerSubsystem* Mgr = GI->GetSubsystem<USGLocationManagerSubsystem>();
	if (!Mgr) { GI->Shutdown(); return false; }

	// 开局在城市，无待处理回程。
	TestEqual(TEXT("starts at no location (city)"), Mgr->GetCurrentLocation(), ELocation::None);
	FVector OutLoc; FRotator OutRot;
	TestFalse(TEXT("no pending return at start"), Mgr->ConsumePendingReturn(OutLoc, OutRot));

	// 记下城市坐标（模拟离开城市那一刻的玩家位置）。
	const FVector DoorLoc(-1500.f, -1200.f, 90.f);
	const FRotator DoorRot(0.f, 45.f, 0.f);
	Mgr->RememberCityTransform(DoorLoc, DoorRot);

	// 回城市 → 安排一次性回程传送。
	Mgr->ReturnToCity();
	TestEqual(TEXT("back to city = no location"), Mgr->GetCurrentLocation(), ELocation::None);

	// 玩家消费回程：拿到门口坐标，且只生效一次。
	TestTrue(TEXT("has pending return after ReturnToCity"), Mgr->ConsumePendingReturn(OutLoc, OutRot));
	TestTrue(TEXT("return loc matches door"), OutLoc.Equals(DoorLoc, 1.f));
	TestTrue(TEXT("return rot matches door"), OutRot.Equals(DoorRot, 1.f));
	TestFalse(TEXT("pending return consumed (once only)"), Mgr->ConsumePendingReturn(OutLoc, OutRot));

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
