#include "Systems/AssetsSystem.h"

bool FAssetsSystem::OwnsHome() const
{
	return Housing == EHousingTier::OwnedHDB
		|| Housing == EHousingTier::OwnedCondo
		|| Housing == EHousingTier::Multiple;
}

void FAssetsSystem::AddInvestment(int64 Cents)
{
	if (Cents > 0)
	{
		InvestmentCents += Cents;
	}
}

int64 FAssetsSystem::WithdrawInvestment(int64 Cents)
{
	if (Cents <= 0)
	{
		return 0;
	}
	const int64 Actual = FMath::Min(Cents, InvestmentCents);
	InvestmentCents -= Actual;
	return Actual;
}

void FAssetsSystem::AccrueInvestmentReturn(int32 ReturnPerMille)
{
	if (InvestmentCents <= 0)
	{
		return;
	}
	// 用 int64 中间量避免溢出；本金 += 本金 * permille / 1000。
	const int64 Delta = InvestmentCents * (int64)ReturnPerMille / 1000;
	InvestmentCents = FMath::Max((int64)0, InvestmentCents + Delta);
}

int64 FAssetsSystem::HousingValuationCents(EHousingTier Tier)
{
	switch (Tier)
	{
	case EHousingTier::OwnedHDB:   return 40000000;   // $400k
	case EHousingTier::OwnedCondo: return 120000000;  // $1.2M
	case EHousingTier::Multiple:   return 250000000;  // $2.5M
	default:                       return 0;          // 无 / 租房不计自有资产
	}
}

int64 FAssetsSystem::VehicleValuationCents(EVehicleTier Tier)
{
	switch (Tier)
	{
	case EVehicleTier::UsedCar:   return 3000000;    // $30k
	case EVehicleTier::NewCar:    return 10000000;   // $100k（含 SG COE）
	case EVehicleTier::LuxuryCar: return 40000000;   // $400k
	default:                      return 0;          // 无 / Grab 月卡不计资产
	}
}

int64 FAssetsSystem::GetAssetNetWorthContribution() const
{
	return HousingValuationCents(Housing)
		+ VehicleValuationCents(Vehicle)
		+ InvestmentCents;
}

void FAssetsSystem::RestoreState(EHousingTier InHousing, EVehicleTier InVehicle, int64 InInvestmentCents)
{
	Housing = InHousing;
	Vehicle = InVehicle;
	InvestmentCents = FMath::Max((int64)0, InInvestmentCents);
}
