#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Systems/EconomyTypes.h"
#include "Systems/ResidencyTypes.h"
#include "Systems/AssetsTypes.h"
#include "Systems/CareerTypes.h"
#include "Systems/EndingTypes.h"
#include "SGSaveGame.generated.h"

/**
 * 存档数据。spec §6.4 / Plan 2 Task 7。
 *
 * 聚合 Time / Economy / Progress / Relationship / PlayerState 五大系统的可序列化状态。
 * 只放「数据」，不放逻辑——由 USaveGameSubsystem 负责从各系统采集 / 回灌。
 * 字段都是 UPROPERTY，UE 的 SaveGame 序列化器（SaveGameToSlot）会自动存读。
 */
UCLASS()
class SGLIFESIM_API USGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 存档格式版本，便于将来迁移旧档。 */
	UPROPERTY(VisibleAnywhere, Category = "Save|Meta")
	int32 SaveVersion = 1;

	/** 存档写入时的真实世界时间戳（UI 展示用）。 */
	UPROPERTY(VisibleAnywhere, Category = "Save|Meta")
	FDateTime SavedAtUtc;

	// --- Time ---
	UPROPERTY()
	int32 TimeTotalBlocks = 0;

	// --- Economy ---
	UPROPERTY()
	TArray<int64> EconomyBalances;

	UPROPERTY()
	TArray<FMoneyTransaction> EconomyTransactions;

	// --- Progress ---
	UPROPERTY()
	TArray<FName> Achievements;

	// --- Relationship ---
	UPROPERTY()
	TMap<FName, int32> Affinities;

	// --- PlayerState ---
	UPROPERTY()
	TArray<int32> PlayerAttributes;

	// --- Residency ---
	UPROPERTY()
	EResidencyStatus ResidencyStatus = EResidencyStatus::WorkPermit_EP;

	UPROPERTY()
	int32 PRRejectionCount = 0;

	// --- Assets ---
	UPROPERTY()
	EHousingTier HousingTier = EHousingTier::None;

	UPROPERTY()
	EVehicleTier VehicleTier = EVehicleTier::None;

	UPROPERTY()
	int64 InvestmentCents = 0;

	// 房贷（Plan 7）
	UPROPERTY()
	int64 MortgageOutstandingCents = 0;

	UPROPERTY()
	int32 MortgageAnnualRatePerMille = 0;

	UPROPERTY()
	int64 MortgageMonthlyPrincipalCents = 0;

	// --- Career（Plan 8） ---
	UPROPERTY()
	ECareerLevel CareerLevel = ECareerLevel::Junior;

	UPROPERTY()
	int64 CareerGrossSalaryCents = 500000;

	UPROPERTY()
	int32 CareerMonthsInRole = 0;

	// --- Ending ---
	UPROPERTY()
	EEnding ChosenEnding = EEnding::None;
};
