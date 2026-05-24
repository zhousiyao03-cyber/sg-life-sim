#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/HorrorSequenceSubsystem.h"
#include "Systems/HorrorSceneTypes.h"
#include "Systems/HorrorSceneRegistry.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/HorrorEventTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 恐怖场景演出端到端（Plan 24）：ResolveOutcome 把结算落到理智 / 图鉴并广播事后文案。
 *
 * 注：EnterScene / ExitScene 的 OpenLevel 是运行时行为，自动化环境无法切关卡，
 * 故这里直接验证可独立测的结算逻辑 ResolveOutcome，OpenLevel 流程留 PIE 验证。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSequenceIntegrationTest,
	"SGLifeSim.Integration.HorrorSceneResolves",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorSequenceIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UHorrorSequenceSubsystem* Seq   = GI->GetSubsystem<UHorrorSequenceSubsystem>();
	USanitySubsystem*         San   = GI->GetSubsystem<USanitySubsystem>();
	UHorrorCodexSubsystem*    Codex = GI->GetSubsystem<UHorrorCodexSubsystem>();
	if (!Seq || !San || !Codex) { GI->Shutdown(); return false; }

	// 开局未在演出中。
	TestFalse(TEXT("not in scene at start"), Seq->IsInScene());
	TestEqual(TEXT("no active scene at start"), Seq->GetActiveScene(), EHorrorScene::None);

	// 结算电梯场景：扣 20 理智、图鉴记下 ElevatorGhostFloor、返回带文案的 def。
	// （OnAftermath 是动态多播委托，UI/BP 订阅；其广播留 PIE 验证。）
	San->RestoreFromSave(100);
	TestFalse(TEXT("elevator not in codex yet"), Codex->HasDiscovered(EHorrorEvent::ElevatorGhostFloor));

	const FHorrorSceneDef Def = Seq->ResolveOutcome(EHorrorScene::Elevator);

	TestEqual(TEXT("sanity drained by 20"), San->GetSanity(), 100 - 20);
	TestTrue(TEXT("elevator now in codex"), Codex->HasDiscovered(EHorrorEvent::ElevatorGhostFloor));
	TestFalse(TEXT("resolve returns aftermath text"), Def.AftermathText.IsEmpty());
	TestEqual(TEXT("resolve returns elevator def"), Def.SanityCost, 20);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
