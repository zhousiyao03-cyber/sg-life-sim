#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/EndingTypes.h"
#include "EndingSubsystem.generated.h"

/** 玩家主动选定结局时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingChosen, EEnding, Ending);

/**
 * 终局子系统。spec §6.5。
 *
 * 读 Residency / Assets / Relationship / Economy 四子系统的当前状态，经
 * FEndingEvaluator 算出「当前倾向」；也支持玩家任何时候主动选定结局。
 */
UCLASS()
class SGLIFESIM_API UEndingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 当前最可能的软终局倾向（不代表已结束）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Ending")
	EEnding GetCurrentLeaning() const;

	/** 玩家主动选定结局（菜单「我想结束这段人生」）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Ending")
	void ChooseEnding(EEnding Ending);

	/** 已选定的结局（None = 尚未结束）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Ending")
	EEnding GetChosenEnding() const { return ChosenEnding; }

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Ending")
	FOnEndingChosen OnEndingChosen;

	/** 从存档恢复已选结局。 */
	void RestoreChosenEnding(EEnding Ending) { ChosenEnding = Ending; }

private:
	EEnding ChosenEnding = EEnding::None;

	/** 跨子系统读取当前状态算总净资产（经济 + 资产估值）。 */
	int64 ComputeTotalNetWorth() const;

	/** 当前所有 NPC 里的最高好感。 */
	int32 ComputeMaxAffinity() const;
};
