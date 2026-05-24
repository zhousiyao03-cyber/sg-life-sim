#pragma once

#include "CoreMinimal.h"
#include "Systems/EconomyTypes.h"

/**
 * 经济系统。spec §6.2。
 *
 * 纯 C++ 钱包 + 收支记账 + 月度结算，零 UE GameplayFramework 依赖，方便单元测试。
 * 后续被 Blueprint 包装的 UEconomySubsystem 持有。
 *
 * 所有金额以「分」(cents, int64) 存——避免浮点累积误差，显示时除以 100。
 * 各账户余额是唯一状态，净资产/流水都从存取操作派生。
 */
class SGLIFESIM_API FEconomySystem
{
public:
	FEconomySystem();

	/** 查询某账户余额（分）。 */
	int64 GetBalance(ECurrencyAccount Account) const;

	/** 向账户存入（分）。记一笔入账流水。 */
	void Deposit(ECurrencyAccount Account, int64 Cents, FName Reason);

	/** 从账户取出（分）。余额不足返回 false 且不改动任何状态。 */
	bool TryWithdraw(ECurrencyAccount Account, int64 Cents, FName Reason);

	/**
	 * 按月发薪。spec §6.2 CPF 规则（55 岁以下默认费率）：
	 * - 雇员自付 20% → 进 CPF（OA/SA/MA 按 0.62 / 0.16 / 0.22 分配）
	 * - 雇主额外 17% → 也进 CPF（同比例分配），不从工资里扣
	 * - 到手现金 = gross − 自付 20%
	 * @param GrossCents 税前月薪（分）
	 */
	void ApplyMonthlySalary(int64 GrossCents);

	/** 净资产 = 所有账户余额之和（分）。 */
	int64 GetNetWorth() const;

	/** 交易流水（最新在末尾），存档 / UI 用。 */
	const TArray<FMoneyTransaction>& GetTransactions() const { return Transactions; }

	// CPF 费率常量（spec §6.2，公开供测试断言用）。
	static constexpr int32 CpfEmployeePercent = 20;  // 雇员自付
	static constexpr int32 CpfEmployerPercent = 17;  // 雇主额外
	// CPF 进三个子账户的分配比例（千分比，和 = 1000）。
	static constexpr int32 CpfAllocOaPerMille = 620;
	static constexpr int32 CpfAllocSaPerMille = 160;
	static constexpr int32 CpfAllocMaPerMille = 220;

private:
	/** 各账户余额（分）。索引 = (int32)ECurrencyAccount。单一状态来源。 */
	int64 Balances[(int32)ECurrencyAccount::Count];

	TArray<FMoneyTransaction> Transactions;

	/** 内部入账：改余额 + 记流水。负数即出账。 */
	void RecordChange(ECurrencyAccount Account, int64 DeltaCents, FName Reason);

	/** 把一笔 CPF 总额按 OA/SA/MA 千分比分账，余数补进 OA（保证不丢分）。 */
	void DistributeCpf(int64 TotalCpfCents, FName Reason);
};
