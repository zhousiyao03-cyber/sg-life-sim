#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/CareerSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"
#include "Systems/SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

// 文件内唯一名，避开 unity build ODR。
namespace { constexpr int64 CRDollars(int64 D) { return D * 100; } }

/**
 * 职业收入端到端（Plan 8）：headless GameInstance 上「升职涨薪→月结到手变多→跳槽→存读档」。
 * 验证薪资推给 Economy、月度发薪用职业薪资、首次升职成就、薪资进存档。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCareerPromotionRaisesIncomeTest,
	"SGLifeSim.Integration.CareerPromotionRaisesIncome",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCareerPromotionRaisesIncomeTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UCareerSubsystem*      Career = GI->GetSubsystem<UCareerSubsystem>();
	UEconomySubsystem*     Eco    = GI->GetSubsystem<UEconomySubsystem>();
	UTimeSubsystem*        Time   = GI->GetSubsystem<UTimeSubsystem>();
	UPlayerStateSubsystem* PS     = GI->GetSubsystem<UPlayerStateSubsystem>();
	UProgressSubsystem*    Prog   = GI->GetSubsystem<UProgressSubsystem>();
	USaveGameSubsystem*    Save   = GI->GetSubsystem<USaveGameSubsystem>();
	if (!Career || !Eco || !Time || !PS || !Prog || !Save) { GI->Shutdown(); return false; }

	// 起步：初级工程师，薪资已推给 Economy（$5000）。
	TestEqual(TEXT("starts Junior"), Career->GetLevel(), ECareerLevel::Junior);
	TestEqual(TEXT("economy salary synced to $5000"),
		Eco->GetMonthlyGrossSalary(), CRDollars(5000));

	auto AdvanceOneMonth = [Time]() { for (int32 i = 0; i < 28 * 5; ++i) { Time->AdvanceBlock(); } };

	// 专业技能拉到 70（够升 Mid 的 50 门槛）。
	PS->SetAttribute(EPlayerAttribute::Professional, 70);

	// 在职不足 3 月不能升。
	TestFalse(TEXT("cannot promote at tenure 0"), Career->CanPromote());
	AdvanceOneMonth(); AdvanceOneMonth(); AdvanceOneMonth(); // 在职 3 月（→ month 4）

	// 升职 → 中级，薪资 $7500，推给 Economy，首次升职解锁成就。
	TestTrue(TEXT("can promote now"), Career->CanPromote());
	TestTrue(TEXT("promote ok"), Career->TryPromote());
	TestEqual(TEXT("now Mid"), Career->GetLevel(), ECareerLevel::Mid);
	TestEqual(TEXT("economy salary now $7500"), Eco->GetMonthlyGrossSalary(), CRDollars(7500));
	TestTrue(TEXT("FirstPromotion unlocked"), Prog->HasAchieved(SGAchievementIds::FirstPromotion()));

	// 推一个月 → 用中级薪资发薪。到手现金增量 = 净薪($7500-20%CPF=$6000) - 固定账单($1070) = $4930。
	const int64 CashBefore = Eco->GetBalance(ECurrencyAccount::Cash);
	AdvanceOneMonth();
	const int64 CashDelta = Eco->GetBalance(ECurrencyAccount::Cash) - CashBefore;
	TestEqual(TEXT("monthly take-home at Mid = $4930"), CashDelta, CRDollars(4930));

	// 跳槽 +35%：$7500 → $10125，推给 Economy。
	TestTrue(TEXT("jobhop ok"), Career->JobHop(35));
	TestEqual(TEXT("salary after jobhop $10125"), Career->GetGrossSalaryCents(), CRDollars(10125));
	TestEqual(TEXT("economy salary after jobhop"), Eco->GetMonthlyGrossSalary(), CRDollars(10125));

	// 存档（Mid，$10125）。
	const FString Slot = TEXT("SGLifeSim_CareerSlot");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	TestTrue(TEXT("save ok"), Save->SaveToSlot(Slot));

	// 改动：再跳一次槽。
	Career->JobHop(50);
	TestNotEqual(TEXT("salary changed after second jobhop"),
		Career->GetGrossSalaryCents(), CRDollars(10125));

	// 读档复原：薪资回 $10125，并重新同步给 Economy。
	TestTrue(TEXT("load ok"), Save->LoadFromSlot(Slot));
	TestEqual(TEXT("level restored Mid"), Career->GetLevel(), ECareerLevel::Mid);
	TestEqual(TEXT("salary restored $10125"), Career->GetGrossSalaryCents(), CRDollars(10125));
	TestEqual(TEXT("economy salary re-synced"), Eco->GetMonthlyGrossSalary(), CRDollars(10125));

	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
