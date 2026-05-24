#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/TimeSystem.h"
#include "Systems/TimeBlock.h"
#include "TimeSubsystem.generated.h"

/** Time 推进时广播给 UI 等订阅者。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnTimeAdvanced, ETimeBlock, NewBlock, int32, DayNumber);

/**
 * 时间子系统。spec §6.1 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 的薄包装层 —— 内部委托给纯 C++ 的 FTimeSystem。
 * 职责：
 *   - 把 FTimeSystem 的 API 暴露给 Blueprint（UFUNCTION / UPROPERTY）
 *   - 广播 OnTimeAdvanced 事件给订阅者
 *   - 提供跨场景持久（GameInstance 生命周期）
 *
 * Blueprint 访问方式：Get Game Instance Subsystem → TimeSubsystem
 */
UCLASS()
class SGLIFESIM_API UTimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Blueprint 调用：推进一个时间块。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Time")
	void AdvanceBlock();

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
	ETimeBlock GetCurrentBlock() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
	int32 GetDayNumber() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
	EWeekday GetWeekday() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
	int32 GetMonthNumber() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
	int32 GetDayOfMonth() const;

	/** 订阅这个 delegate 接收时间推进通知。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Time")
	FOnTimeAdvanced OnTimeAdvanced;

private:
	FTimeSystem Time;
};
