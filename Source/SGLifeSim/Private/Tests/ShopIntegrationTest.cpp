#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/ShopSubsystem.h"
#include "Systems/ShopSystem.h"
#include "Systems/ShopTypes.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/SanitySubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 消费品商店端到端（Plan 25）：钱够买成功（扣钱 + 加属性 + 回理智），钱不够买失败（状态不变）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopIntegrationTest,
	"SGLifeSim.Integration.ShopPurchase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UShopSubsystem*        Shop = GI->GetSubsystem<UShopSubsystem>();
	UEconomySubsystem*     Eco  = GI->GetSubsystem<UEconomySubsystem>();
	UPlayerStateSubsystem* PS   = GI->GetSubsystem<UPlayerStateSubsystem>();
	USanitySubsystem*      San  = GI->GetSubsystem<USanitySubsystem>();
	if (!Shop || !Eco || !PS || !San) { GI->Shutdown(); return false; }

	// —— 钱够：买护身符成功 ——
	Eco->Deposit(ECurrencyAccount::Cash, 10000, TEXT("TestSeed")); // $100
	const FShopItemDef Amulet = FShopSystem::GetItemDef(EShopItem::Amulet);

	// 先扣点理智，留出回升空间（满理智时 Restore 会被 clamp）。
	San->Drain(30);
	const int32 SanityBefore = San->GetSanity();
	const int64 CashBefore   = Eco->GetBalance(ECurrencyAccount::Cash);

	TestTrue(TEXT("can afford amulet"), Shop->CanAfford(EShopItem::Amulet));
	const bool bBought = Shop->TryPurchase(EShopItem::Amulet);
	TestTrue(TEXT("purchase succeeds"), bBought);
	TestEqual(TEXT("cash reduced by price"),
		Eco->GetBalance(ECurrencyAccount::Cash), CashBefore - Amulet.PriceCents);
	TestTrue(TEXT("sanity restored"), San->GetSanity() > SanityBefore);

	// —— 咖啡加精力 ——
	const int32 EnergyBefore = PS->GetAttribute(EPlayerAttribute::Energy);
	Shop->TryPurchase(EShopItem::HotKopi);
	TestTrue(TEXT("kopi raised energy"), PS->GetAttribute(EPlayerAttribute::Energy) >= EnergyBefore);

	// —— 钱不够：花光后买不起外套，状态不变 ——
	// 先把现金取到一个买不起外套的水平。
	const int64 Remaining = Eco->GetBalance(ECurrencyAccount::Cash);
	const FShopItemDef Jacket = FShopSystem::GetItemDef(EShopItem::WarmJacket);
	if (Remaining >= Jacket.PriceCents)
	{
		Eco->TryWithdraw(ECurrencyAccount::Cash, Remaining - (Jacket.PriceCents - 1), TEXT("TestDrain"));
	}
	const int64 PoorCash    = Eco->GetBalance(ECurrencyAccount::Cash);
	const int32 PoorHealth  = PS->GetAttribute(EPlayerAttribute::Health);
	TestFalse(TEXT("cannot afford jacket now"), Shop->CanAfford(EShopItem::WarmJacket));
	const bool bFailed = Shop->TryPurchase(EShopItem::WarmJacket);
	TestFalse(TEXT("purchase fails when broke"), bFailed);
	TestEqual(TEXT("cash unchanged on failed purchase"), Eco->GetBalance(ECurrencyAccount::Cash), PoorCash);
	TestEqual(TEXT("health unchanged on failed purchase"), PS->GetAttribute(EPlayerAttribute::Health), PoorHealth);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
