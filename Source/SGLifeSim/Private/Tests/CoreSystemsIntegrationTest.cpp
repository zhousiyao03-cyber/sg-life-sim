#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "Systems/TimeSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/SaveGameSubsystem.h"
#include "Systems/PlayerStatsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 端到端集成：在 headless GameInstance 上跑真正的子系统（含 OnTimeAdvanced 绑定）。
 * 验证链路：推时间 → 月度自动结算（发薪+账单）→ 每日能量恢复 → 成就 → 存档 → 读档回灌。
 * 这测的是子系统之间的真实连线，不是纯核心。Plan 2 Task 8。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCoreSystemsIntegrationTest,
	"SGLifeSim.Integration.MonthlyLoopAndSave",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoreSystemsIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("game instance created"), GI);
	if (!GI) { return false; }

	// 初始化一个独立 GameInstance —— 这会创建并 Initialize 所有子系统（触发跨系统绑定）。
	GI->InitializeStandalone();

	UTimeSubsystem*        Time = GI->GetSubsystem<UTimeSubsystem>();
	UEconomySubsystem*     Eco  = GI->GetSubsystem<UEconomySubsystem>();
	UProgressSubsystem*    Prog = GI->GetSubsystem<UProgressSubsystem>();
	UPlayerStateSubsystem* PS   = GI->GetSubsystem<UPlayerStateSubsystem>();
	USaveGameSubsystem*    Save = GI->GetSubsystem<USaveGameSubsystem>();

	TestNotNull(TEXT("TimeSubsystem"), Time);
	TestNotNull(TEXT("EconomySubsystem"), Eco);
	TestNotNull(TEXT("ProgressSubsystem"), Prog);
	TestNotNull(TEXT("PlayerStateSubsystem"), PS);
	TestNotNull(TEXT("SaveGameSubsystem"), Save);
	if (!Time || !Eco || !Prog || !PS || !Save) { GI->Shutdown(); return false; }

	// 起点：现金为 0，第 1 月。
	TestEqual(TEXT("cash starts 0"), Eco->GetBalance(ECurrencyAccount::Cash), (int64)0);
	TestEqual(TEXT("start month 1"), Time->GetMonthNumber(), 1);

	// 推进 28 天（140 个 block）→ 跨入第 2 月 → 经济子系统经 OnTimeAdvanced 自动结算一次。
	for (int32 i = 0; i < 28 * 5; ++i)
	{
		Time->AdvanceBlock();
	}
	TestEqual(TEXT("now month 2"), Time->GetMonthNumber(), 2);
	// 一次结算：到手 $4000 − 账单($800+$150+$120=$1070) = $2930 = 293000 分。
	TestEqual(TEXT("cash after auto monthly settlement = $2930"),
		Eco->GetBalance(ECurrencyAccount::Cash), (int64)293000);
	TestTrue(TEXT("CPF accrued after settlement"),
		Eco->GetBalance(ECurrencyAccount::CPF_OA) > 0);

	// 成就：发过薪 → 标记「第一次发薪」。
	const bool bNewAch = Prog->MarkAchieved(TEXT("FirstSalary"));
	TestTrue(TEXT("FirstSalary newly unlocked"), bNewAch);

	// 每日能量恢复：耗一半，跨一天后回满。
	PS->ModifyAttribute(EPlayerAttribute::Energy, -50);
	TestEqual(TEXT("energy drained to 50"), PS->GetAttribute(EPlayerAttribute::Energy), 50);
	for (int32 i = 0; i < 5; ++i) { Time->AdvanceBlock(); }  // 跨一天
	TestEqual(TEXT("energy restored to 100 next day"),
		PS->GetAttribute(EPlayerAttribute::Energy), 100);

	// 存档（经子系统聚合 → 磁盘）。
	const FString Slot = TEXT("SGLifeSim_IntegrationSlot");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	TestTrue(TEXT("SaveToSlot succeeds"), Save->SaveToSlot(Slot));

	// 存档后改动状态。
	Eco->Deposit(ECurrencyAccount::Cash, (int64)999900, TEXT("Test"));
	Prog->MarkAchieved(TEXT("PostSaveAchievement"));
	TestNotEqual(TEXT("cash mutated after save"),
		Eco->GetBalance(ECurrencyAccount::Cash), (int64)293000);

	// 读档回灌 → 状态回到存档时刻。
	TestTrue(TEXT("LoadFromSlot succeeds"), Save->LoadFromSlot(Slot));
	TestEqual(TEXT("cash restored to $2930"),
		Eco->GetBalance(ECurrencyAccount::Cash), (int64)293000);
	TestTrue(TEXT("pre-save achievement survives load"), Prog->HasAchieved(TEXT("FirstSalary")));
	TestFalse(TEXT("post-save achievement gone after load"),
		Prog->HasAchieved(TEXT("PostSaveAchievement")));

	// 清理。
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
