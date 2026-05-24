#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/EconomySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

// 文件内唯一名，避开 unity build 合并 TU 时与其它测试的 Dollars 撞名（ODR）。
namespace { constexpr int64 MGDollars(int64 D) { return D * 100; } }

/**
 * 按揭购房端到端（Plan 7）：headless GameInstance 上「按揭买组屋→逐月还款→提前结清→存读档」。
 * 验证首付扣款、月供随时间自动扣且本金递减、未还本金计入净资产负债、存档复原房贷。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMortgageFinancedPurchaseTest,
	"SGLifeSim.Integration.MortgageFinancedPurchase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMortgageFinancedPurchaseTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UEconomySubsystem*  Eco  = GI->GetSubsystem<UEconomySubsystem>();
	UAssetsSubsystem*   Ast  = GI->GetSubsystem<UAssetsSubsystem>();
	UTimeSubsystem*     Time = GI->GetSubsystem<UTimeSubsystem>();
	USaveGameSubsystem* Save = GI->GetSubsystem<USaveGameSubsystem>();
	if (!Eco || !Ast || !Time || !Save) { GI->Shutdown(); return false; }

	// 攒够现金后按揭买组屋（$400k）：首付 25% = $100k，贷款 $300k。
	Eco->Deposit(ECurrencyAccount::Cash, MGDollars(500000), TEXT("Test")); // $500k
	TestTrue(TEXT("financed buy HDB"), Ast->BuyHousingFinanced(EHousingTier::OwnedHDB));
	TestEqual(TEXT("only down payment withdrawn → cash $400k"),
		Eco->GetBalance(ECurrencyAccount::Cash), MGDollars(400000));
	TestTrue(TEXT("has mortgage"), Ast->HasMortgage());
	TestEqual(TEXT("loan = $300k"), Ast->GetMortgageBalance(), MGDollars(300000));
	TestEqual(TEXT("owns home (financed)"), Ast->OwnsHome(), true);

	// 净资产里房产权益 = 估值 $400k − 欠款 $300k = $100k。
	TestEqual(TEXT("asset contribution = equity $100k"),
		Ast->GetAssetNetWorthContribution(), MGDollars(100000));

	// 月供本金 = 300000_00 / 300 = $1000；首月利息 = 30000000*26/12000 = $650。
	const int64 FirstMonthPayment = Ast->GetMortgageMonthlyPayment();
	TestEqual(TEXT("first month payment $1650"), FirstMonthPayment, MGDollars(1650));

	auto AdvanceOneMonth = [Time]() { for (int32 i = 0; i < 28 * 5; ++i) { Time->AdvanceBlock(); } };

	// 推进一个月 → 月供自动扣，本金降一档 $1000 → 余款 $299k。
	AdvanceOneMonth();
	TestEqual(TEXT("month 2"), Time->GetMonthNumber(), 2);
	TestEqual(TEXT("outstanding after 1 payment = $299k"),
		Ast->GetMortgageBalance(), MGDollars(299000));
	const int64 SecondMonthPayment = Ast->GetMortgageMonthlyPayment();
	TestTrue(TEXT("payment declines as balance shrinks"), SecondMonthPayment < FirstMonthPayment);

	// 再推一个月 → 余款 $298k。
	AdvanceOneMonth();
	TestEqual(TEXT("outstanding after 2 payments = $298k"),
		Ast->GetMortgageBalance(), MGDollars(298000));

	// 存档（此刻欠 $298k）。
	const FString Slot = TEXT("SGLifeSim_MortgageSlot");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	TestTrue(TEXT("save ok"), Save->SaveToSlot(Slot));

	// 提前结清：从现金一次性扣（余款 + 当月利息），之后无按揭。
	TestTrue(TEXT("prepay ok"), Ast->PrepayMortgage());
	TestFalse(TEXT("no mortgage after prepay"), Ast->HasMortgage());
	TestEqual(TEXT("balance zero"), Ast->GetMortgageBalance(), (int64)0);

	// 读档复原：房贷回到 $298k。
	TestTrue(TEXT("load ok"), Save->LoadFromSlot(Slot));
	TestTrue(TEXT("mortgage restored"), Ast->HasMortgage());
	TestEqual(TEXT("outstanding restored $298k"),
		Ast->GetMortgageBalance(), MGDollars(298000));

	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
