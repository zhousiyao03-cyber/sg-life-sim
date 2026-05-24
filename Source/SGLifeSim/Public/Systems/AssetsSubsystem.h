#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/AssetsSystem.h"
#include "Systems/AssetsTypes.h"
#include "Systems/TimeBlock.h"
#include "AssetsSubsystem.generated.h"

/** 资产变化时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAssetsChanged);

/**
 * 资产子系统。spec §6.4 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 薄壳，内部委托给纯 C++ 的 FAssetsSystem。
 * 与 Economy 接线：买房/车、投资都从现金扣款；订阅 TimeSubsystem 月初给投资计回报。
 */
UCLASS()
class SGLIFESIM_API UAssetsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	EHousingTier GetHousingTier() const { return Assets.GetHousingTier(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	EVehicleTier GetVehicleTier() const { return Assets.GetVehicleTier(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	int64 GetInvestmentValue() const { return Assets.GetInvestmentValue(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	bool OwnsHome() const { return Assets.OwnsHome(); }

	/** 资产计入净资产的部分（房+车+投资估值，分）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	int64 GetAssetNetWorthContribution() const { return Assets.GetAssetNetWorthContribution(); }

	/** 买房：从现金扣该 tier 估值，成功才升级住房 tier。现金不足返回 false。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	bool BuyHousing(EHousingTier Tier);

	/** 买车：从现金扣该 tier 估值，成功才升级车辆 tier。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	bool BuyVehicle(EVehicleTier Tier);

	/** 投资：从现金转入投资本金。现金不足返回 false。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	bool Invest(int64 Cents);

	/** 赎回投资：从投资市值转回现金，返回实际转回额。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	int64 Divest(int64 Cents);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Assets")
	FOnAssetsChanged OnAssetsChanged;

	/** 月度投资回报率（千分比）。默认 +0.5%/月（约 6%/年）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SGLifeSim|Assets")
	int32 MonthlyReturnPerMille = 5;

	FAssetsSystem& GetAssets() { return Assets; }
	const FAssetsSystem& GetAssets() const { return Assets; }

private:
	FAssetsSystem Assets;

	/** 上次结算投资回报的月号，检测跨月。 */
	int32 LastReturnMonth = 1;

	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);
};
