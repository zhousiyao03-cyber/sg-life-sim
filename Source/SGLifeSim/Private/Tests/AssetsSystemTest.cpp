#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/AssetsSystem.h"
#include "Systems/AssetsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetsDefaultsTest,
	"SGLifeSim.Assets.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAssetsDefaultsTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	TestEqual(TEXT("no housing"), Sys.GetHousingTier(), EHousingTier::None);
	TestEqual(TEXT("no vehicle"), Sys.GetVehicleTier(), EVehicleTier::None);
	TestFalse(TEXT("does not own home"), Sys.OwnsHome());
	TestEqual(TEXT("no investment"), Sys.GetInvestmentValue(), (int64)0);
	TestEqual(TEXT("no asset net worth"), Sys.GetAssetNetWorthContribution(), (int64)0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetsOwnsHomeTest,
	"SGLifeSim.Assets.OwnsHomeOnlyWhenOwned",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAssetsOwnsHomeTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	Sys.SetHousingTier(EHousingTier::RentedFlat);
	TestFalse(TEXT("renting is not owning"), Sys.OwnsHome());
	TestEqual(TEXT("rented flat has no asset value"),
		Sys.GetAssetNetWorthContribution(), (int64)0);

	Sys.SetHousingTier(EHousingTier::OwnedHDB);
	TestTrue(TEXT("owned HDB counts as owning"), Sys.OwnsHome());
	TestEqual(TEXT("owned HDB valued $400k"),
		Sys.GetAssetNetWorthContribution(), (int64)40000000);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetsInvestmentReturnTest,
	"SGLifeSim.Assets.InvestmentAccruesReturn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAssetsInvestmentReturnTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	Sys.AddInvestment((int64)1000000);  // $10,000
	TestEqual(TEXT("invested $10k"), Sys.GetInvestmentValue(), (int64)1000000);

	// +5% (50 permille) -> $10,500
	Sys.AccrueInvestmentReturn(50);
	TestEqual(TEXT("after +5% return"), Sys.GetInvestmentValue(), (int64)1050000);

	// 赎回超额 -> 全部赎回
	const int64 Got = Sys.WithdrawInvestment((int64)9999999);
	TestEqual(TEXT("withdrew full balance"), Got, (int64)1050000);
	TestEqual(TEXT("investment now empty"), Sys.GetInvestmentValue(), (int64)0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetsNetWorthSumTest,
	"SGLifeSim.Assets.NetWorthSumsTiersAndInvestment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAssetsNetWorthSumTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	Sys.SetHousingTier(EHousingTier::OwnedCondo);  // $1.2M
	Sys.SetVehicleTier(EVehicleTier::NewCar);      // $100k
	Sys.AddInvestment((int64)5000000);             // $50k
	// 120000000 + 10000000 + 5000000 = 135000000
	TestEqual(TEXT("asset net worth = $1.35M"),
		Sys.GetAssetNetWorthContribution(), (int64)135000000);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
