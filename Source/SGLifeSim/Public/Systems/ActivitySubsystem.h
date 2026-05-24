#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/ActivitySystem.h"
#include "Systems/ActivityTypes.h"
#include "ActivitySubsystem.generated.h"

/** 完成一次活动后广播反馈文本（供 UI 提示）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActivityPerformed, FText, Feedback);

/**
 * 活动子系统。spec §6.1 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 薄壳：把活动效果落到 PlayerState（属性）/ Economy（现金）/
 * Time（推进时间块）。能量是稀缺资源 —— 做不动的活动被挡。逻辑表在纯 C++ 的 FActivitySystem。
 */
UCLASS()
class SGLIFESIM_API UActivitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 执行一个活动：校验能量 → 改属性/现金 → 推进时间块。能量不足返回 false。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Activity")
	bool PerformActivity(EActivityType Activity);

	/** 当前是否做得动该活动（能量门槛）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Activity")
	bool CanPerform(EActivityType Activity) const;

	/** 当前关卡可做的活动列表（出租屋 vs 食阁）。供 UI 列按钮。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Activity")
	TArray<EActivityType> GetActivitiesForCurrentLevel() const;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Activity")
	FOnActivityPerformed OnActivityPerformed;

private:
	/** 读当前能量（PlayerState，缺省 0）。 */
	int32 GetCurrentEnergy() const;
};
