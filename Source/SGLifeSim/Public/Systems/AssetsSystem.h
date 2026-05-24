#pragma once

#include "CoreMinimal.h"
#include "Systems/AssetsTypes.h"

/**
 * 资产系统。spec §6.4 资产。
 *
 * 纯 C++：当前住房 / 车辆 tier + 投资本金（分），零 UE 依赖、可单测。
 * 「买东西要花钱」的扣款由 UAssetsSubsystem 经 Economy 处理；本核心只管资产状态 +
 * 投资增值 + 估值汇总（计入净资产）。房 / 车估值走静态 tier 表，后续可换 DataTable。
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

	/** 资产计入净资产的部分（分）：房估值 + 车估值 + 投资市值。 */
	int64 GetAssetNetWorthContribution() const;

	/** 某住房 tier 的估值（分）。租房 = 0（非自有资产）。 */
	static int64 HousingValuationCents(EHousingTier Tier);

	/** 某车辆 tier 的估值（分）。 */
	static int64 VehicleValuationCents(EVehicleTier Tier);

	/** 从存档恢复。 */
	void RestoreState(EHousingTier InHousing, EVehicleTier InVehicle, int64 InInvestmentCents);

private:
	EHousingTier Housing = EHousingTier::None;
	EVehicleTier Vehicle = EVehicleTier::None;
	int64 InvestmentCents = 0;
};
