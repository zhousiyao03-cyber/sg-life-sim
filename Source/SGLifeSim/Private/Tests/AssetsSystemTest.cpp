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

// --- 房贷（Plan 7） ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMortgageOpensAndAmortizesTest,
	"SGLifeSim.Mortgage.OpensAndAmortizes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMortgageOpensAndAmortizesTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	// 贷款 $300k，年息 2.6%(26‰)，25 年(300 月)。月供本金 = 30000000/300 = 100000($1000)。
	Sys.OpenMortgage((int64)30000000, 26, 300);
	const FMortgage& M = Sys.GetMortgage();

	TestTrue(TEXT("mortgage active"), Sys.HasMortgage());
	TestEqual(TEXT("outstanding $300k"), M.OutstandingPrincipalCents, (int64)30000000);
	TestEqual(TEXT("monthly principal $1000"), M.MonthlyPrincipalCents, (int64)100000);

	// 当月利息 = 30000000 * 26 / 12000 = 65000($650)；月供 = 100000 + 65000 = 165000。
	TestEqual(TEXT("month1 interest $650"), M.InterestDueCents(), (int64)65000);
	TestEqual(TEXT("month1 payment $1650"), M.PaymentDueCents(), (int64)165000);

	const int64 Month1Interest = M.InterestDueCents();
	const int64 Paid = Sys.GetMortgage().PayScheduledMonth();
	TestEqual(TEXT("paid month1 = $1650"), Paid, (int64)165000);
	TestEqual(TEXT("outstanding after 1 month"), Sys.GetMortgage().OutstandingPrincipalCents, (int64)29900000);
	// 利息随余额递减。
	TestTrue(TEXT("interest declines"), Sys.GetMortgage().InterestDueCents() < Month1Interest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMortgageCountsAsLiabilityTest,
	"SGLifeSim.Mortgage.CountsAsLiability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMortgageCountsAsLiabilityTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	Sys.SetHousingTier(EHousingTier::OwnedHDB);   // 估值 $400k
	Sys.OpenMortgage((int64)30000000, 26, 300);   // 欠 $300k

	// 净资产贡献 = 40000000 - 30000000 = 10000000($100k 净房产权益)。
	TestEqual(TEXT("net worth = equity after debt"),
		Sys.GetAssetNetWorthContribution(), (int64)10000000);
	// 按揭中仍算自有房（扎根终局认这套房）。
	TestTrue(TEXT("financed home still owned"), Sys.OwnsHome());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMortgagePayoffAndClearTest,
	"SGLifeSim.Mortgage.PayoffAndFinalMonthClears",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMortgagePayoffAndClearTest::RunTest(const FString& Parameters)
{
	FAssetsSystem Sys;
	Sys.OpenMortgage((int64)30000000, 26, 300);
	// 提前结清额 = 未还本金 + 当月利息 = 30000000 + 65000。
	TestEqual(TEXT("payoff = principal + interest"),
		Sys.GetMortgage().PayoffAmountCents(), (int64)30065000);
	Sys.GetMortgage().Clear();
	TestFalse(TEXT("cleared mortgage inactive"), Sys.HasMortgage());

	// 直线本金：零息 $2000 / 2 月 → 两个月正好还清。
	Sys.OpenMortgage((int64)200000, 0, 2);
	TestEqual(TEXT("monthly principal $1000"), Sys.GetMortgage().MonthlyPrincipalCents, (int64)100000);
	Sys.GetMortgage().PayScheduledMonth();
	TestEqual(TEXT("half left"), Sys.GetMortgage().OutstandingPrincipalCents, (int64)100000);
	TestTrue(TEXT("still active"), Sys.HasMortgage());
	Sys.GetMortgage().PayScheduledMonth();
	TestFalse(TEXT("fully repaid"), Sys.HasMortgage());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
