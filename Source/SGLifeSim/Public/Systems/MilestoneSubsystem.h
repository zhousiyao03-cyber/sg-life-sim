#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/MilestoneTypes.h"
#include "Systems/TimeBlock.h"
#include "MilestoneSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneCompleted, EMilestone, Milestone);

/**
 * 人生里程碑子系统。Plan 13。FMilestoneSystem 的 UE 薄壳：
 * 从经济/职业/身份/资产/进度子系统聚合状态快照，评估主线进度，
 * 在里程碑达成时广播（玩家 HUD 弹庆祝 toast），并提供「当前目标」文本给 HUD。
 */
UCLASS()
class SGLIFESIM_API UMilestoneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 里程碑达成时广播（玩家订阅 → 庆祝 toast）。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Milestone")
	FOnMilestoneCompleted OnMilestoneCompleted;

	/** 重新评估，检测新达成的里程碑并广播。HUD 每帧调用即可保持及时。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Milestone")
	void Refresh();

	/** 当前应奔的目标（第一个未完成）；全部完成返回 EMilestone::Count。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Milestone")
	EMilestone GetActiveMilestone();

	/** 已完成的里程碑数量。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Milestone")
	int32 GetCompletedCount();

	/** 给 HUD 的一行目标文本（含进度，如「🎯 当前目标：攒下 $5,000（$3,200 / $5,000）· 已完成 1/7」）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Milestone")
	FText GetActiveObjectiveText();

private:
	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	/** 从各子系统读出当前状态快照。 */
	FMilestoneContext BuildContext() const;

	/** 每个里程碑是否已弹过 toast（避免重复 + 首次 Prime 静默）。 */
	bool bToasted[(int32)EMilestone::Count] = { false };

	/** 首次 Refresh 静默建立基线（不为开局已满足项弹 toast）。 */
	bool bPrimed = false;
};
