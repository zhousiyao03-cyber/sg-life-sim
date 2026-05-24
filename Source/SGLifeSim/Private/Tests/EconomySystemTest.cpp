#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/EconomySystem.h"
#include "Systems/EconomyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// 测试用：$1 = 100 cents。
	constexpr int64 Dollars(int64 D) { return D * 100; }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomyStartsEmptyTest,
	"SGLifeSim.Economy.StartsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomyStartsEmptyTest::RunTest(const FString& Parameters)
{
	FEconomySystem Sys;
	TestEqual(TEXT("cash starts 0"), Sys.GetBalance(ECurrencyAccount::Cash), (int64)0);
	TestEqual(TEXT("bank starts 0"), Sys.GetBalance(ECurrencyAccount::Bank), (int64)0);
	TestEqual(TEXT("CPF_OA starts 0"), Sys.GetBalance(ECurrencyAccount::CPF_OA), (int64)0);
	TestEqual(TEXT("net worth starts 0"), Sys.GetNetWorth(), (int64)0);
	TestEqual(TEXT("no transactions"), Sys.GetTransactions().Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomyDepositWithdrawTest,
	"SGLifeSim.Economy.DepositWithdraw",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomyDepositWithdrawTest::RunTest(const FString& Parameters)
{
	FEconomySystem Sys;
	Sys.Deposit(ECurrencyAccount::Cash, Dollars(100), TEXT("Test"));
	TestEqual(TEXT("after deposit $100"), Sys.GetBalance(ECurrencyAccount::Cash), Dollars(100));

	const bool bOk = Sys.TryWithdraw(ECurrencyAccount::Cash, Dollars(30), TEXT("Test"));
	TestTrue(TEXT("withdraw $30 succeeds"), bOk);
	TestEqual(TEXT("balance now $70"), Sys.GetBalance(ECurrencyAccount::Cash), Dollars(70));

	// 入账 + 出账 = 2 笔流水
	TestEqual(TEXT("two transactions logged"), Sys.GetTransactions().Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomyOverdraftFailsTest,
	"SGLifeSim.Economy.OverdraftFailsAndKeepsBalance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomyOverdraftFailsTest::RunTest(const FString& Parameters)
{
	FEconomySystem Sys;
	Sys.Deposit(ECurrencyAccount::Cash, Dollars(50), TEXT("Test"));

	const bool bOk = Sys.TryWithdraw(ECurrencyAccount::Cash, Dollars(80), TEXT("Test"));
	TestFalse(TEXT("over-withdraw fails"), bOk);
	TestEqual(TEXT("balance unchanged at $50"), Sys.GetBalance(ECurrencyAccount::Cash), Dollars(50));
	// 失败的取款不应记流水（只有那 1 笔存款）
	TestEqual(TEXT("failed withdraw logs nothing"), Sys.GetTransactions().Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomySalaryCpfSplitTest,
	"SGLifeSim.Economy.SalaryCpfSplit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomySalaryCpfSplitTest::RunTest(const FString& Parameters)
{
	FEconomySystem Sys;
	Sys.ApplyMonthlySalary(Dollars(5000));  // 税前 $5000

	// 自付 20% = $1000 进 CPF；到手现金 = $4000
	TestEqual(TEXT("take-home cash = $4000"), Sys.GetBalance(ECurrencyAccount::Cash), Dollars(4000));

	// 雇主 17% = $850；CPF 总额 = $1000 + $850 = $1850 = 185000 分
	const int64 TotalCpf =
		  Sys.GetBalance(ECurrencyAccount::CPF_OA)
		+ Sys.GetBalance(ECurrencyAccount::CPF_SA)
		+ Sys.GetBalance(ECurrencyAccount::CPF_MA);
	TestEqual(TEXT("total CPF = $1850"), TotalCpf, Dollars(1850));

	// 千分比分账：SA = 185000*160/1000 = 29600；MA = 185000*220/1000 = 40700；OA = 余 114700
	TestEqual(TEXT("CPF_SA = 29600 cents"), Sys.GetBalance(ECurrencyAccount::CPF_SA), (int64)29600);
	TestEqual(TEXT("CPF_MA = 40700 cents"), Sys.GetBalance(ECurrencyAccount::CPF_MA), (int64)40700);
	TestEqual(TEXT("CPF_OA = 114700 cents (remainder)"), Sys.GetBalance(ECurrencyAccount::CPF_OA), (int64)114700);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomyNetWorthTest,
	"SGLifeSim.Economy.NetWorthSumsAllAccounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomyNetWorthTest::RunTest(const FString& Parameters)
{
	FEconomySystem Sys;
	Sys.ApplyMonthlySalary(Dollars(5000));

	// 净资产 = 到手现金 + 全部 CPF = gross + 雇主缴纳 = $5000 + $850 = $5850
	TestEqual(TEXT("net worth = gross + employer CPF = $5850"),
		Sys.GetNetWorth(), Dollars(5850));

	// 再手动存点银行，净资产应增加对应额度
	Sys.Deposit(ECurrencyAccount::Bank, Dollars(150), TEXT("Test"));
	TestEqual(TEXT("net worth grows with bank deposit = $6000"),
		Sys.GetNetWorth(), Dollars(6000));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomyChargeAllowsDebtTest,
	"SGLifeSim.Economy.ChargeAllowsDebt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomyChargeAllowsDebtTest::RunTest(const FString& Parameters)
{
	FEconomySystem Sys;
	// 现金为 0，账单照扣 → 变负（欠债），区别于 TryWithdraw
	Sys.Charge(ECurrencyAccount::Cash, Dollars(100), TEXT("Rent"));
	TestEqual(TEXT("charge into debt: cash = -$100"), Sys.GetBalance(ECurrencyAccount::Cash), Dollars(-100));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEconomyMonthlySettlementTest,
	"SGLifeSim.Economy.MonthlySettlement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEconomyMonthlySettlementTest::RunTest(const FString& Parameters)
{
	// 复现 UEconomySubsystem::RunMonthlySettlement 的纯 C++ 部分（默认配置）
	FEconomySystem Sys;
	Sys.ApplyMonthlySalary(Dollars(5000));                 // 到手现金 $4000
	Sys.Charge(ECurrencyAccount::Cash, Dollars(800), TEXT("Rent"));
	Sys.Charge(ECurrencyAccount::Cash, Dollars(150), TEXT("Utilities"));
	Sys.Charge(ECurrencyAccount::Cash, Dollars(120), TEXT("Transport"));

	// $4000 − ($800+$150+$120) = $2930
	TestEqual(TEXT("cash after month settlement = $2930"),
		Sys.GetBalance(ECurrencyAccount::Cash), Dollars(2930));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
