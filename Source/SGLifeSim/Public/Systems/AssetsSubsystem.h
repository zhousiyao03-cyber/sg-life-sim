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

	/** 买房（全款）：从现金扣该 tier 估值，成功才升级住房 tier。现金不足返回 false。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	bool BuyHousing(EHousingTier Tier);

	/**
	 * 按揭买房（Plan 7）：首付 DownPaymentPercent% 从现金扣，余额开按揭。
	 * 现金不够首付返回 false。成功则升 tier + 开贷，之后月初自动扣月供。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	bool BuyHousingFinanced(EHousingTier Tier);

	/** 提前一次性结清房贷：从现金扣（未还本金+当月利息），现金不足返回 false。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Assets")
	bool PrepayMortgage();

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	bool HasMortgage() const { return Assets.HasMortgage(); }

	/** 未还房贷本金（分）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	int64 GetMortgageBalance() const { return Assets.GetMortgage().OutstandingPrincipalCents; }

	/** 当月房贷应付现金（分，本金+利息）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Assets")
	int64 GetMortgageMonthlyPayment() const { return Assets.GetMortgage().PaymentDueCents(); }

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

	/** 按揭首付比例（%）。LTV 75% → 首付 25%，贴近 SG HDB/银行房贷。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SGLifeSim|Assets")
	int32 DownPaymentPercent = 25;

	/** 按揭年利率（千分比）。26 = 2.6%/年（HDB 优惠贷款档）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SGLifeSim|Assets")
	int32 MortgageAnnualRatePerMille = 26;

	/** 按揭年限（月）。300 = 25 年。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SGLifeSim|Assets")
	int32 MortgageTenureMonths = 300;

	FAssetsSystem& GetAssets() { return Assets; }
	const FAssetsSystem& GetAssets() const { return Assets; }

private:
	FAssetsSystem Assets;

	/** 上次结算投资回报的月号，检测跨月。 */
	int32 LastReturnMonth = 1;

	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);
};
