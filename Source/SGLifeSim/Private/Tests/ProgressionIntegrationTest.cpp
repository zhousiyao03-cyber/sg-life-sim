#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/EconomySubsystem.h"
#include "Systems/ResidencySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/EndingSubsystem.h"
#include "Systems/SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace { constexpr int64 PIDollars(int64 D) { return D * 100; } }

/**
 * 进阶+终局端到端：headless GameInstance 上跑「攒钱→投资→买房→拿 PR→拉好感→扎根」，
 * 再存档→改动→读档复原。验证 Residency/Assets/Ending 子系统连线 + economy 扣款 + 存档。
 * Plan 4 Task 5。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionRootedAndSaveTest,
	"SGLifeSim.Integration.ProgressionRootedAndSave",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProgressionRootedAndSaveTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UEconomySubsystem*      Eco  = GI->GetSubsystem<UEconomySubsystem>();
	UResidencySubsystem*    Res  = GI->GetSubsystem<UResidencySubsystem>();
	UAssetsSubsystem*       Ast  = GI->GetSubsystem<UAssetsSubsystem>();
	URelationshipSubsystem* Rel  = GI->GetSubsystem<URelationshipSubsystem>();
	UEndingSubsystem*       End  = GI->GetSubsystem<UEndingSubsystem>();
	USaveGameSubsystem*     Save = GI->GetSubsystem<USaveGameSubsystem>();
	if (!Eco || !Res || !Ast || !Rel || !End || !Save) { GI->Shutdown(); return false; }

	// 起点：EP、无房、无关系 → 漂着
	TestEqual(TEXT("start leaning Adrift"), End->GetCurrentLeaning(), EEnding::Adrift);

	// 攒钱 → 投资 → 买房（都从现金扣）
	Eco->Deposit(ECurrencyAccount::Cash, PIDollars(500000), TEXT("Test"));  // $500k
	TestTrue(TEXT("invest $50k"), Ast->Invest(PIDollars(50000)));
	TestTrue(TEXT("buy HDB $400k"), Ast->BuyHousing(EHousingTier::OwnedHDB));
	TestTrue(TEXT("owns home now"), Ast->OwnsHome());
	// 现金 = 500k - 50k(投资) - 400k(房) = 50k
	TestEqual(TEXT("cash now $50k"), Eco->GetBalance(ECurrencyAccount::Cash), PIDollars(50000));

	// 买房现金不够时应失败（再买一套公寓 $1.2M）
	TestFalse(TEXT("cannot afford condo"), Ast->BuyHousing(EHousingTier::OwnedCondo));
	TestEqual(TEXT("still HDB after failed buy"), Ast->GetHousingTier(), EHousingTier::OwnedHDB);

	// 拿 PR
	TestTrue(TEXT("apply PR"), Res->ApplyForPR());
	TestTrue(TEXT("resolve approved"), Res->ResolvePRApplication(true));
	TestEqual(TEXT("now PR"), Res->GetStatus(), EResidencyStatus::PR);

	// 拉好感到朋友
	Rel->AddAffinity(TEXT("Partner"), 60);

	// PR + 有房 + 朋友 → 扎根
	TestEqual(TEXT("leaning Rooted"), End->GetCurrentLeaning(), EEnding::Rooted);

	// 存档
	const FString Slot = TEXT("SGLifeSim_ProgressionSlot");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	TestTrue(TEXT("save ok"), Save->SaveToSlot(Slot));

	// 改动：卖掉房（直接改核心）+ 主动选一个结局
	Ast->GetAssets().SetHousingTier(EHousingTier::None);
	End->ChooseEnding(EEnding::CashOut);
	TestNotEqual(TEXT("after losing home no longer Rooted"),
		End->GetCurrentLeaning(), EEnding::Rooted);
	TestEqual(TEXT("chosen ending recorded"), End->GetChosenEnding(), EEnding::CashOut);

	// 读档复原
	TestTrue(TEXT("load ok"), Save->LoadFromSlot(Slot));
	TestEqual(TEXT("housing restored to HDB"), Ast->GetHousingTier(), EHousingTier::OwnedHDB);
	TestEqual(TEXT("investment restored $50k"), Ast->GetInvestmentValue(), PIDollars(50000));
	TestEqual(TEXT("residency restored PR"), Res->GetStatus(), EResidencyStatus::PR);
	TestEqual(TEXT("leaning Rooted again after load"), End->GetCurrentLeaning(), EEnding::Rooted);
	TestEqual(TEXT("chosen ending restored to None"), End->GetChosenEnding(), EEnding::None);

	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
