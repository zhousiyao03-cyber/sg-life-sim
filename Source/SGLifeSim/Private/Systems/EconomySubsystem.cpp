#include "Systems/EconomySubsystem.h"
#include "Systems/TimeSubsystem.h"

void UEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 保证 TimeSubsystem 先于本系统初始化，然后订阅其时间推进事件。
	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	if (UTimeSubsystem* TimeSys = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		TimeSys->OnTimeAdvanced.AddDynamic(this, &UEconomySubsystem::HandleTimeAdvanced);
		LastSettledMonth = TimeSys->GetMonthNumber();  // 游戏起始月不补发
	}
}

void UEconomySubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UEconomySubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

void UEconomySubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	UTimeSubsystem* TimeSys = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTimeSubsystem>() : nullptr;
	if (!TimeSys)
	{
		return;
	}

	// 跨入新月 → 月度结算。可能一次推进跨多月（理论上）：循环补齐。
	const int32 CurrentMonth = TimeSys->GetMonthNumber();
	while (CurrentMonth > LastSettledMonth)
	{
		++LastSettledMonth;
		RunMonthlySettlement();
	}
}

void UEconomySubsystem::RunMonthlySettlement()
{
	// 发薪（含 CPF 分账）
	ApplyMonthlySalary(MonthlyFinance.SalaryGrossCents);

	// 扣固定账单（现金，允许欠债）
	Economy.Charge(ECurrencyAccount::Cash, MonthlyFinance.RentCents, TEXT("Rent"));
	Economy.Charge(ECurrencyAccount::Cash, MonthlyFinance.UtilitiesCents, TEXT("Utilities"));
	Economy.Charge(ECurrencyAccount::Cash, MonthlyFinance.TransportCents, TEXT("Transport"));
	NotifyBalance(ECurrencyAccount::Cash);
}

int64 UEconomySubsystem::GetBalance(ECurrencyAccount Account) const
{
	return Economy.GetBalance(Account);
}

void UEconomySubsystem::NotifyBalance(ECurrencyAccount Account)
{
	OnBalanceChanged.Broadcast(Account, Economy.GetBalance(Account));
}

void UEconomySubsystem::Deposit(ECurrencyAccount Account, int64 Cents, FName Reason)
{
	Economy.Deposit(Account, Cents, Reason);
	NotifyBalance(Account);
}

bool UEconomySubsystem::TryWithdraw(ECurrencyAccount Account, int64 Cents, FName Reason)
{
	const bool bOk = Economy.TryWithdraw(Account, Cents, Reason);
	if (bOk)
	{
		NotifyBalance(Account);
	}
	return bOk;
}

void UEconomySubsystem::ApplyMonthlySalary(int64 GrossCents)
{
	Economy.ApplyMonthlySalary(GrossCents);
	// 发薪触及现金 + 三个 CPF 子账户，逐个广播最新余额。
	NotifyBalance(ECurrencyAccount::Cash);
	NotifyBalance(ECurrencyAccount::CPF_OA);
	NotifyBalance(ECurrencyAccount::CPF_SA);
	NotifyBalance(ECurrencyAccount::CPF_MA);
}

int64 UEconomySubsystem::GetNetWorth() const
{
	return Economy.GetNetWorth();
}
