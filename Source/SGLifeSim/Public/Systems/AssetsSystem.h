#pragma once

#include "CoreMinimal.h"
#include "Systems/AssetsTypes.h"

/**
 * 房贷账本（纯 C++，等额本金 / 直线本金）。Plan 7。
 *
 * 每月还固定本金 `MonthlyPrincipalCents` + 当月利息（利息随未还余额递减）。
 * 利息按整数运算：当月利息 = 未还本金 × 年利率千分比 / 12000（即 /1000 年息再 /12 月）。
 * 不持有现金——扣款由 UAssetsSubsystem 经 Economy 处理；本结构只算「该还多少、还完没」。
 */
struct SGLIFESIM_API FMortgage
{
	/** 未还本金（分）。> 0 即按揭未结清。 */
	int64 OutstandingPrincipalCents = 0;

	/** 年利率（千分比），26 = 2.6%/年。 */
	int32 AnnualRatePerMille = 0;

	/** 每月固定偿还的本金（分）= 原始贷款 / 年限。 */
	int64 MonthlyPrincipalCents = 0;

	bool IsActive() const { return OutstandingPrincipalCents > 0; }

	/** 当月应付利息（分）= 未还本金 × 年息千分比 / 12000。 */
	int64 InterestDueCents() const { return OutstandingPrincipalCents * (int64)AnnualRatePerMille / 12000; }

	/** 当月应还本金（分）：正常一档月供本金，余额不足一档时还清剩余。 */
	int64 PrincipalDueCents() const { return FMath::Min(OutstandingPrincipalCents, MonthlyPrincipalCents); }

	/** 当月应付现金（分）= 本金 + 利息。 */
	int64 PaymentDueCents() const { return PrincipalDueCents() + InterestDueCents(); }

	/** 按计划还一个月：扣减本金，返回本月应付现金（分，供调用方扣款）。 */
	int64 PayScheduledMonth()
	{
		const int64 Payment = PaymentDueCents();
		OutstandingPrincipalCents -= PrincipalDueCents();
		return Payment;
	}

	/** 提前一次性结清需付现金（分）= 未还本金 + 当月利息。 */
	int64 PayoffAmountCents() const { return OutstandingPrincipalCents + InterestDueCents(); }

	/** 结清（本金清零）。 */
	void Clear() { OutstandingPrincipalCents = 0; }
};

/**
 * 资产系统。spec §6.4 资产。
 *
 * 纯 C++：当前住房 / 车辆 tier + 投资本金（分）+ 房贷，零 UE 依赖、可单测。
 * 「买东西要花钱」的扣款由 UAssetsSubsystem 经 Economy 处理；本核心只管资产状态 +
 * 投资增值 + 房贷账本 + 估值汇总（计入净资产）。房 / 车估值走静态 tier 表，后续可换 DataTable。
 */
class SGLIFESIM_API FAssetsSystem
{
public:
	EHousingTier GetHousingTier() const { return Housing; }
	EVehicleTier GetVehicleTier() const { return Vehicle; }

	void SetHousingTier(EHousingTier Tier) { Housing = Tier; }
	void SetVehicleTier(EVehicleTier Tier) { Vehicle = Tier; }

	/** 当前是否拥有房产（自购及以上，租房不算）。终局判定用。 */
	bool OwnsHome() const;

	/** 投资本金 / 当前市值（分）。本金随回报复利增长。 */
	int64 GetInvestmentValue() const { return InvestmentCents; }

	/** 追加投资（分）。 */
	void AddInvestment(int64 Cents);

	/** 赎回投资（分）。超过市值则全部赎回，返回实际赎回额。 */
	int64 WithdrawInvestment(int64 Cents);

	/** 月度投资回报：本金 *= (1 + permille/1000)。permille 可正可负（亏损）。 */
	void AccrueInvestmentReturn(int32 ReturnPerMille);

	/** 资产计入净资产的部分（分）：房估值 + 车估值 + 投资市值 − 未还房贷（负债）。 */
	int64 GetAssetNetWorthContribution() const;

	/** 某住房 tier 的估值（分）。租房 = 0（非自有资产）。 */
	static int64 HousingValuationCents(EHousingTier Tier);

	/** 某车辆 tier 的估值（分）。 */
	static int64 VehicleValuationCents(EVehicleTier Tier);

	// --- 房贷（Plan 7） ---

	/** 开一笔按揭：贷款本金（分）、年利率千分比、年限（月）。覆盖既有按揭。 */
	void OpenMortgage(int64 PrincipalCents, int32 AnnualRatePerMille, int32 TenureMonths);

	FMortgage& GetMortgage() { return Mortgage; }
	const FMortgage& GetMortgage() const { return Mortgage; }

	/** 是否有未结清房贷。 */
	bool HasMortgage() const { return Mortgage.IsActive(); }

	/** 从存档恢复（含房贷）。 */
	void RestoreState(EHousingTier InHousing, EVehicleTier InVehicle, int64 InInvestmentCents,
		int64 InMortgageOutstandingCents = 0, int32 InMortgageRatePerMille = 0, int64 InMortgageMonthlyPrincipalCents = 0);

private:
	EHousingTier Housing = EHousingTier::None;
	EVehicleTier Vehicle = EVehicleTier::None;
	int64 InvestmentCents = 0;
	FMortgage Mortgage;
};
