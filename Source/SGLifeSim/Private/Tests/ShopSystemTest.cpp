#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/ShopSystem.h"
#include "Systems/ShopTypes.h"
#include "Systems/PlayerStatsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 消费品商店纯核心（Plan 25）：每件商品有名/价 > 0；CanAfford 边界正确；
 * 护身符回理智（恐怖联动不回归）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopSystemTest,
	"SGLifeSim.Shop.System",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopSystemTest::RunTest(const FString& Parameters)
{
	// 每件商品定义完整。
	for (int32 i = 0; i < (int32)EShopItem::Count; ++i)
	{
		const FShopItemDef Def = FShopSystem::GetItemDef((EShopItem)i);
		TestFalse(FString::Printf(TEXT("item %d has a title"), i), Def.Title.IsEmpty());
		TestTrue(FString::Printf(TEXT("item %d price > 0"), i), Def.PriceCents > 0);
	}

	// CanAfford 边界：余额 = 价格 → 买得起；少一分 → 买不起。
	{
		const FShopItemDef Jacket = FShopSystem::GetItemDef(EShopItem::WarmJacket);
		TestTrue(TEXT("exact balance affords"), FShopSystem::CanAfford(Jacket, Jacket.PriceCents));
		TestFalse(TEXT("one cent short cannot afford"), FShopSystem::CanAfford(Jacket, Jacket.PriceCents - 1));
		TestTrue(TEXT("more than enough affords"), FShopSystem::CanAfford(Jacket, Jacket.PriceCents + 100000));
	}

	// 各商品效果走对路：咖啡 +精力、零食 +心情、外套 +健康、护身符 +理智。
	TestTrue(TEXT("kopi gives energy"),
		FShopSystem::GetItemDef(EShopItem::HotKopi).GetAttr(EPlayerAttribute::Energy) > 0);
	TestTrue(TEXT("snacks give mood"),
		FShopSystem::GetItemDef(EShopItem::Snacks).GetAttr(EPlayerAttribute::Mood) > 0);
	TestTrue(TEXT("jacket gives health"),
		FShopSystem::GetItemDef(EShopItem::WarmJacket).GetAttr(EPlayerAttribute::Health) > 0);
	TestTrue(TEXT("amulet restores sanity"),
		FShopSystem::GetItemDef(EShopItem::Amulet).SanityDelta > 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
