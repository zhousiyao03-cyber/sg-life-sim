#include "Systems/EconomySubsystem.h"

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
