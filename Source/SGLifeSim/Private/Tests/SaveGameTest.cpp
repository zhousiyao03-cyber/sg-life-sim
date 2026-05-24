#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/SGSaveGame.h"
#include "Systems/EconomyTypes.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSaveGameRoundTripTest,
	"SGLifeSim.Save.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSaveGameRoundTripTest::RunTest(const FString& Parameters)
{
	const FString Slot = TEXT("SGLifeSim_AutomationTestSlot");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);  // 干净起点

	// 1) 造一个填满五系统数据的存档对象
	USGSaveGame* Save = Cast<USGSaveGame>(
		UGameplayStatics::CreateSaveGameObject(USGSaveGame::StaticClass()));
	TestNotNull(TEXT("save object created"), Save);
	if (!Save) { return false; }

	Save->TimeTotalBlocks = 137;
	Save->EconomyBalances = { 400000, 0, 114700, 29600, 40700 };
	Save->EconomyTransactions.Add(FMoneyTransaction(ECurrencyAccount::Cash, 400000, TEXT("Salary")));
	Save->Achievements = { TEXT("First10k"), TEXT("FirstJob") };
	Save->Affinities.Add(TEXT("Auntie"), 55);
	Save->PlayerAttributes = { 80, 60, 100, 60, 40, 30 };

	// GTA 块状态。
	Save->WantedHeat = 230;       // 2 星
	Save->PlayerHealth = 47;
	Save->WeaponKind = 1;         // 手枪
	Save->WeaponAmmoInMag = 8;
	Save->Weather = 2;            // 雨

	// 2) 写盘
	TestTrue(TEXT("SaveGameToSlot succeeds"), UGameplayStatics::SaveGameToSlot(Save, Slot, 0));
	TestTrue(TEXT("slot now exists"), UGameplayStatics::DoesSaveGameExist(Slot, 0));

	// 3) 读回
	USGSaveGame* Loaded = Cast<USGSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	TestNotNull(TEXT("loaded object valid"), Loaded);
	if (!Loaded) { return false; }

	// 4) 逐字段比对
	TestEqual(TEXT("time blocks survive"), Loaded->TimeTotalBlocks, 137);
	TestEqual(TEXT("balances count"), Loaded->EconomyBalances.Num(), 5);
	TestEqual(TEXT("cash balance survives"), Loaded->EconomyBalances[0], (int64)400000);
	TestEqual(TEXT("transactions survive"), Loaded->EconomyTransactions.Num(), 1);
	TestEqual(TEXT("transaction reason survives"),
		Loaded->EconomyTransactions[0].Reason, FName(TEXT("Salary")));
	TestEqual(TEXT("achievements survive"), Loaded->Achievements.Num(), 2);
	TestTrue(TEXT("achievement First10k present"), Loaded->Achievements.Contains(TEXT("First10k")));
	TestEqual(TEXT("affinity survives"), Loaded->Affinities.FindRef(TEXT("Auntie")), 55);
	TestEqual(TEXT("player attrs count"), Loaded->PlayerAttributes.Num(), 6);
	TestEqual(TEXT("energy attr survives"), Loaded->PlayerAttributes[2], 100);

	// GTA 块状态存活。
	TestEqual(TEXT("wanted heat survives"), Loaded->WantedHeat, 230);
	TestEqual(TEXT("health survives"), Loaded->PlayerHealth, 47);
	TestEqual(TEXT("weapon kind survives"), (int32)Loaded->WeaponKind, 1);
	TestEqual(TEXT("ammo survives"), Loaded->WeaponAmmoInMag, 8);
	TestEqual(TEXT("weather survives"), (int32)Loaded->Weather, 2);

	// 5) 清理
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	TestFalse(TEXT("slot deleted"), UGameplayStatics::DoesSaveGameExist(Slot, 0));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
