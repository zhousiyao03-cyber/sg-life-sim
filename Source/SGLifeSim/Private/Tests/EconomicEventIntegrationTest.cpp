#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/EconomicEventSubsystem.h"
#include "Systems/EconomicEventSystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace { constexpr int64 EVDollars(int64 D) { return D * 100; } }

/**
 * 经济事件端到端（Plan 9）：headless GameInstance 上验证事件效果落到现金/持仓，
 * 以及月度抽取接线（固定种子 → GetLastEvent 与离线抽取一致）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomicEventAffectsWalletTest,
	"SGLifeSim.Integration.EconomicEventAffectsWallet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomicEventAffectsWalletTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UEconomicEventSubsystem* Events = GI->GetSubsystem<UEconomicEventSubsystem>();
	UEconomySubsystem*       Eco    = GI->GetSubsystem<UEconomySubsystem>();
	UAssetsSubsystem*        Assets = GI->GetSubsystem<UAssetsSubsystem>();
	UTimeSubsystem*          Time   = GI->GetSubsystem<UTimeSubsystem>();
	if (!Events || !Eco || !Assets || !Time) { GI->Shutdown(); return false; }

	// 备点现金 + 投资 $10k。
	Eco->Deposit(ECurrencyAccount::Cash, EVDollars(100000), TEXT("Test"));
	TestTrue(TEXT("invest $10k"), Assets->Invest(EVDollars(10000)));
	TestEqual(TEXT("investment $10k"), Assets->GetInvestmentValue(), EVDollars(10000));

	// 币圈崩盘 -50% → 持仓 $5k。
	TestTrue(TEXT("crash applied"), Events->ApplyEvent(EEconomicEvent::CryptoCrash));
	TestEqual(TEXT("investment halved to $5k"), Assets->GetInvestmentValue(), EVDollars(5000));

	// 政府红包 +$600。
	const int64 CashBefore = Eco->GetBalance(ECurrencyAccount::Cash);
	TestTrue(TEXT("payout applied"), Events->ApplyEvent(EEconomicEvent::GovPayout));
	TestEqual(TEXT("cash +$600"),
		Eco->GetBalance(ECurrencyAccount::Cash) - CashBefore, EVDollars(600));

	// 年终奖 = 1.5 个月薪（默认 $5000 → +$7500）。
	const int64 CashBeforeBonus = Eco->GetBalance(ECurrencyAccount::Cash);
	TestTrue(TEXT("bonus applied"), Events->ApplyEvent(EEconomicEvent::YearEndBonus));
	TestEqual(TEXT("bonus = 1.5 months salary = $7500"),
		Eco->GetBalance(ECurrencyAccount::Cash) - CashBeforeBonus, EVDollars(7500));

	// 平静月不广播、返回 false。
	TestFalse(TEXT("None is a no-op"), Events->ApplyEvent(EEconomicEvent::None));
	TestEqual(TEXT("last event recorded None"), Events->GetLastEvent(), EEconomicEvent::None);

	// 月度抽取接线：固定种子推一个月，结果与离线同种子抽取一致。
	Events->SetSeed(424242);
	FRandomStream Mirror(424242);
	const EEconomicEvent Expected = FEconomicEventSystem::PickEvent(Mirror);
	for (int32 i = 0; i < 28 * 5; ++i) { Time->AdvanceBlock(); } // 跨一月
	TestEqual(TEXT("monthly roll matches deterministic pick"),
		Events->GetLastEvent(), Expected);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
