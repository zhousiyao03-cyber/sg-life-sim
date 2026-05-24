#include "Systems/EconomySystem.h"

FEconomySystem::FEconomySystem()
{
	for (int32 i = 0; i < (int32)ECurrencyAccount::Count; ++i)
	{
		Balances[i] = 0;
	}
}

int64 FEconomySystem::GetBalance(ECurrencyAccount Account) const
{
	const int32 Index = (int32)Account;
	if (Index < 0 || Index >= (int32)ECurrencyAccount::Count)
	{
		return 0;
	}
	return Balances[Index];
}

void FEconomySystem::RecordChange(ECurrencyAccount Account, int64 DeltaCents, FName Reason)
{
	const int32 Index = (int32)Account;
	if (Index < 0 || Index >= (int32)ECurrencyAccount::Count)
	{
		return;
	}
	Balances[Index] += DeltaCents;
	Transactions.Emplace(Account, DeltaCents, Reason);
}

void FEconomySystem::Deposit(ECurrencyAccount Account, int64 Cents, FName Reason)
{
	if (Cents <= 0)
	{
		return;
	}
	RecordChange(Account, Cents, Reason);
}

bool FEconomySystem::TryWithdraw(ECurrencyAccount Account, int64 Cents, FName Reason)
{
	if (Cents <= 0)
	{
		return false;
	}
	if (GetBalance(Account) < Cents)
	{
		return false;  // 余额不足，不改动任何状态
	}
	RecordChange(Account, -Cents, Reason);
	return true;
}

void FEconomySystem::Charge(ECurrencyAccount Account, int64 Cents, FName Reason)
{
	if (Cents <= 0)
	{
		return;
	}
	RecordChange(Account, -Cents, Reason);  // 允许变负（欠债）
}

void FEconomySystem::DistributeCpf(int64 TotalCpfCents, FName Reason)
{
	if (TotalCpfCents <= 0)
	{
		return;
	}
	// 千分比分账；OA 兜底余数，保证三账户之和 == TotalCpfCents（不丢分）。
	const int64 SaCents = TotalCpfCents * CpfAllocSaPerMille / 1000;
	const int64 MaCents = TotalCpfCents * CpfAllocMaPerMille / 1000;
	const int64 OaCents = TotalCpfCents - SaCents - MaCents;

	RecordChange(ECurrencyAccount::CPF_OA, OaCents, Reason);
	RecordChange(ECurrencyAccount::CPF_SA, SaCents, Reason);
	RecordChange(ECurrencyAccount::CPF_MA, MaCents, Reason);
}

void FEconomySystem::ApplyMonthlySalary(int64 GrossCents)
{
	if (GrossCents <= 0)
	{
		return;
	}

	static const FName SalaryReason(TEXT("Salary"));
	static const FName CpfReason(TEXT("CPF"));

	// 雇员自付 20% 进 CPF，到手现金 = gross − 自付。
	const int64 EmployeeCpf = GrossCents * CpfEmployeePercent / 100;
	const int64 TakeHomeCash = GrossCents - EmployeeCpf;

	// 雇主额外 17% 也进 CPF（不从工资扣）。
	const int64 EmployerCpf = GrossCents * CpfEmployerPercent / 100;

	RecordChange(ECurrencyAccount::Cash, TakeHomeCash, SalaryReason);
	DistributeCpf(EmployeeCpf + EmployerCpf, CpfReason);
}

int64 FEconomySystem::GetNetWorth() const
{
	int64 Sum = 0;
	for (int32 i = 0; i < (int32)ECurrencyAccount::Count; ++i)
	{
		Sum += Balances[i];
	}
	return Sum;
}

TArray<int64> FEconomySystem::GetBalancesSnapshot() const
{
	TArray<int64> Out;
	Out.Reserve((int32)ECurrencyAccount::Count);
	for (int32 i = 0; i < (int32)ECurrencyAccount::Count; ++i)
	{
		Out.Add(Balances[i]);
	}
	return Out;
}

void FEconomySystem::RestoreBalances(const TArray<int64>& Snapshot)
{
	if (Snapshot.Num() != (int32)ECurrencyAccount::Count)
	{
		return;  // 长度不符，忽略（防脏存档）
	}
	for (int32 i = 0; i < (int32)ECurrencyAccount::Count; ++i)
	{
		Balances[i] = Snapshot[i];
	}
}
